// Buffer pool header
#include "Mu2e-STMDAQ/processing/buffer_pool.hh"

// Constructor: Pre-allocate buffers and push them into the lock-free stack
BufferPool::BufferPool(const std::shared_ptr<cpu_utils>& cpu_,
                       const std::shared_ptr<AsyncLogger>& logger_,
                       const std::shared_ptr<STMdata>& stm_,
                       const std::shared_ptr<OperationManager>& om) :
  cpu(cpu_), logger(logger_), stm(stm_),
  raw_size(stm->buffer_config.raw_size),
  raw_len(stm->buffer_config.raw_len),
  zs_size(stm->buffer_config.zs_size),
  zs_len(stm->buffer_config.zs_len),
  ph_size(stm->buffer_config.ph_size),
  ph_len(stm->buffer_config.ph_len),
  baseline_size(stm->buffer_config.baseline_size),
  baseline_len(stm->buffer_config.baseline_len),
  max_event_num(stm->buffer_config.max_event_num),
  extra(stm->buffer_config.extra),
  queue_capacity(stm->buffer_config.queue_capacity),
  buffer_pool_size(stm->buffer_config.pool_size),
  thread_num(om->getUseOps().size()),
  buffer_pool_total(buffer_pool_size * thread_num)
{

  // Calculate raw header size
  double header_size_tot = max_event_num*sizeof(sw_event_header); // bytes
  logger->log("BufferPool: Estimating total header creation size of " +
	      std::to_string(max_event_num) +
	      " events per struct * " +
	      std::to_string(sizeof(sw_event_header)) +
	      " byte raw event headers = " +
	      std::to_string(header_size_tot) +
	      " bytes.",1);

  // Calculate total struct size
  double struct_size = (raw_size + zs_size + ph_size + baseline_size
			+ header_size_tot)*(double)(1+extra/100);
  logger->log("BufferPool: Estimating total struct size ~ " +
              std::to_string(struct_size) + " bytes = " +
              std::to_string(raw_size) + " bytes (raw) + " +
              std::to_string(zs_size) + " bytes (zs) + " +
              std::to_string(ph_size) + " bytes (ph) + " +
              std::to_string(baseline_size) + " bytes (baseline) + " +
              std::to_string(header_size_tot) + " bytes (headers) + " +
              std::to_string(extra) + " % (metadata).",1);

  // Calculate the total intended buffer memory
  size_t buffer_mem = struct_size*buffer_pool_total; // bytes
  
  // Log to user
  logger->log("BufferPool: Starting STM DAQ with " + std::to_string(thread_num) + " threads...",1);
  logger->log("BufferPool: "+std::to_string(thread_num) +
	      " queues (capacity = " + std::to_string(queue_capacity) +
	      " * " + std::to_string(struct_size) + " byte data structs).",1);
  logger->log("BufferPool: Buffer pool of " + std::to_string(thread_num) +
	      " * " + std::to_string(buffer_pool_size) +
	      " = " + std::to_string(buffer_pool_total) + " data structs.",1);
  logger->log("BufferPool: Intended total buffer memory = " +
	      std::to_string(buffer_mem*1e-9) + " GB.",1);

  // Get the available memeory of the system
  //  const size_t FREE_MEM = sysconf(_SC_AVPHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
  size_t FREE_MEM = 0;
  std::ifstream meminfo("/proc/meminfo");
  std::string key;
  size_t value;
  std::string unit;  
  while (meminfo >> key >> value >> unit) {
    if (key == "MemAvailable:") {
      FREE_MEM = value * 1024; // kB → bytes
      break;
    }
  }
  logger->log("BufferPool: Available system memory = " + std::to_string(FREE_MEM*1e-9) + " Gbytes.",1);

  // If the total intended buffer memory is more than the available memory
  const double SAFETY = 0.8; // 80%
  if(buffer_mem > FREE_MEM*SAFETY){
    // Throw error
    std::string warning = "ERROR. Intended total buffer memory = " +
      std::to_string(buffer_mem*1e-9) + 
      " GB > 80% of available memory = " +
      std::to_string(FREE_MEM*1e-9) +
      " GB";
    logger->log(warning,0);
    return;
  }
  
  // Set the buffer pool size
  pool = std::make_unique<boost::lockfree::stack<std::shared_ptr<DataStruct>>>(buffer_pool_total);

  // Set the reset queue size
  resetQueue = std::make_unique<boost::lockfree::spsc_queue<std::shared_ptr<DataStruct>>>(buffer_pool_total);

  // Get the names of all the selected operations
  const std::vector<std::pair<std::string, op_any>>& selectedOperations = om->getUseOps();
  std::vector<std::string> op_names(thread_num);
  for (int i = 0; i < thread_num; i++) op_names[i] = selectedOperations[i].first;
 
  // Set numa preference 
  int target_sock = stm->master_config.numa_sock;
  if (numa_available() != -1) {
    numa_set_preferred(target_sock);
    logger->log("BufferPool: Binding buffer allocations to NUMA node " 
                 + std::to_string(target_sock), 1);
  }

  // Create pool of data struct buffers
  for (size_t i = 0; i < buffer_pool_total; ++i){
    pool->push(std::make_shared<DataStruct>(stm, // STM data class
                                            buffer_pool_total-i-1, // Buffer ID Num
                                            op_names)); // Number of operations (for performance metrics)
  }

  // Start the reset buffer thread
  resetThread = std::thread(&BufferPool::resetWorker, this);
  
  // Restore default policy
  if (numa_available() != -1) numa_set_preferred(-1);

}



// Acquire a buffer from the pool (Non-blocking)
std::shared_ptr<DataStruct> BufferPool::acquire() {
  std::shared_ptr<DataStruct> buffer;
  if (pool->pop(buffer)) {
    return buffer;
  }
  return nullptr; // No available buffer
}

// Return a buffer to the pool (Non-blocking)
void BufferPool::release(std::shared_ptr<DataStruct>& buffer) {
  while (!resetQueue->push(buffer));  // Non-blocking push
}


// Worker thread to reset the buffers
void BufferPool::resetWorker() {

  // Pin thread to core                                                                               
  size_t core = cpu->get_next_core("BufferPool");
  
  while (running) {
    std::shared_ptr<DataStruct> buffer;
    // Single-threaded safe pop
    if (resetQueue->pop(buffer)) {
      if (!buffer) {
	std::cerr << "[ERROR] Skipping nullptr in resetQueue. Possible corruption.\n";
	continue;
      }
      
      // Reset buffer
      buffer->reset();  
      
      // Push back into the pool
      while (!pool->push(buffer));
    }
  }
  
}  


// Clear buffer pool to ensure safe memory release
void BufferPool::clearPool() {

   std::cout << "Clearing buffer pool.\n";

  static std::mutex pool_mutex;
  std::lock_guard<std::mutex> lock(pool_mutex);  // Prevent concurrent access

  // Check if the pool exists before attempting to clear it
  if (pool) {
    bool expected = false;

    // Atomically check and set the `clearing` flag to prevent concurrent access
    if (!clearing.compare_exchange_strong(expected, true)) {
      // If another thread is already clearing, exit immediately
      return;  
    }

    // Temporary buffer for popping elements
    std::shared_ptr<DataStruct> buffer = nullptr;

    // Clear the reset queue
    std::cout << "Clearing resetQueue.\n";
    while (resetQueue->pop(buffer)) {}

    // Manual counter for the number of elements
    int count = 0;  

    // If no pool
    if (!pool) {
      std::cerr << "BufferPool is already null, skipping clearPool().\n";
      return;
    }

    // If buffer pool is already empty
    if (pool->empty()) {
      std::cerr << "BufferPool is empty, no buffers to clear.\n";
    }
    else{
      // Loop to pop all elements from the pool
      while (pool->pop(buffer)) {
	// Check if the popped shared_ptr is null
	if (!buffer) {  
	  std::cerr << "Warning: Popped a null buffer!" << std::endl;
	}
	// Explicitly reset shared_ptr
	buffer.reset();
	// Increment the counter for each popped element
	count++;  
      }
    }
    
    // Reset the unique_ptr, deallocating the stack
    pool.reset();

    // Set pool to nullptr to prevent accidental use
    pool = nullptr;

    // Print the number of cleared elements 
    std::cout << "Cleared BufferPool of " << count << " elements (expected " << buffer_pool_total << ")." << std::endl;

  }
    
}
