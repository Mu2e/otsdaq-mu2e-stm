// Buffer pool header
#include "Mu2e-STMDAQ/utils/buffer_pool.hh"

// Constructor: Pre-allocate buffers and push them into the lock-free stack
BufferPool::BufferPool(const Config& cfg,
		       const std::shared_ptr<cpu_utils>& cpu_,
		       const std::shared_ptr<AsyncLogger>& logger,
		       const std::shared_ptr<OperationManager>& om) : cpu(cpu_) {

  // Get number of threads to execute
  const size_t numThreads = om->getUseOps().size();

  // Get the decided data struct size
  double raw_buffer_size = cfg.getValue<int>("stm.raw_buffer_size");

  // MWD buffer size should be same as raw buffer size
  double mwd_buffer_size = raw_buffer_size;
  
  // Get the maximum number of packets per call
  size_t max_packets_per_call = 0;
  if (om->check_class("UDP")){
    max_packets_per_call = std::dynamic_pointer_cast<UDP>(om->get_class("UDP"))->get_max_packets_per_call();
  }
  
  // Calculate unused buffer size
  int buffer_remainder = int(raw_buffer_size*1e6) - int(max_packets_per_call*MAX_PACKET_SIZE);

  // Check buffer allocation is correct
  if (buffer_remainder < 0){
    logger->log("BufferPool: UDP initialisation code error: more packets than allocated buffer size.",0);
  }
  
  // Get the zero suppression overflow size
  double zs_overflow = 0;
  if (om->check_class("ZeroSuppress")){
    zs_overflow = (double)(std::dynamic_pointer_cast<ZeroSuppress>(om->get_class("ZeroSuppress"))->get_prev_data_max()*sizeof(int16_t)*1e-6);
  }
  
  // Check if there is already unused buffer space for the zs_overflow
  bool need_overflow = false;
  double overflow = 0;
  if(zs_overflow > buffer_remainder){
    overflow = zs_overflow;
    need_overflow = true;
  }
  
  // Log buffer sizes
  double zs_buffer_size = (double)raw_buffer_size+overflow;
  raw_buffer_size = zs_buffer_size;
  std::string print_opt = (need_overflow) ? "accounting for "  : "including ";
  std::string buffer_size_log = "BufferPool: Raw/ZS buffer sizes = " + std::to_string(raw_buffer_size) + " MB (" + print_opt + std::to_string(zs_overflow) + " MB ZS overflow).";
  logger->log(buffer_size_log,1);
  
  // Calculate maximum number of events per data struct
  double min_event_period = cfg.getValue<double>("stm.mu2e.min_event_period");
  double fADC = cfg.getValue<double>("stm.fw.fADC");
  size_t min_event_len = std::ceil(min_event_period*fADC);
  size_t max_events_per_struct =
    std::ceil(((double)raw_buffer_size*1e6)/((double)min_event_len*sizeof(int16_t)));
  logger->log("BufferPool: Initialising data structs to hold a maximum of " +
	      std::to_string(max_events_per_struct) + " mu2e events.",1);

  // Increase max_events_per_struct by ZS overflow event number
  size_t zs_overflow_event_max = 0;
  if (om->check_class("ZeroSuppress")){
    zs_overflow_event_max = (double)(std::dynamic_pointer_cast<ZeroSuppress>(om->get_class("ZeroSuppress"))->get_overflow_event_max());
  }
  max_events_per_struct += zs_overflow_event_max;
  
  // Calculate header map size
  double hdrMapSize = max_events_per_struct*sizeof(hdrMapTuple)*1e-6; // MB 
  logger->log("BufferPool: Initialising header map arrays of " +
	      std::to_string(max_events_per_struct) +
	      " (" + std::to_string(max_events_per_struct-zs_overflow_event_max) +
	      " + " + std::to_string(zs_overflow_event_max) +
	      " ZS overflow events) events per struct * " +
	      std::to_string(sizeof(hdrMapTuple)) +
	      " byte header maps = " +
	      std::to_string(hdrMapSize) +
	      " MB.",1);

  // Determine noise data vector size
  size_t zs_max_noise_len = 0;
  if (om->check_class("ZeroSuppress")){
    zs_max_noise_len = (std::dynamic_pointer_cast<ZeroSuppress>(om->get_class("ZeroSuppress"))->get_max_noise_len());
  }
  
  // Get the extra size (%) of the total struct size to be allocated for headers, noise, MWD, etc
  const int struct_extra = cfg.getValue<int>("stm.struct_extra");
  
  // Calculate total struct size
  double struct_size = (raw_buffer_size + zs_buffer_size + mwd_buffer_size
			+ hdrMapSize)*(double)(1+(double)struct_extra/100);
  logger->log("BufferPool: Estimating total struct size ~ " + std::to_string(struct_size)
	      + " MB = " + std::to_string(raw_buffer_size) 
	      + " MB (raw) + " + std::to_string(zs_buffer_size)
	      + " MB (ZS) + " + std::to_string(hdrMapSize)
	      + " MB (raw/zs header maps) + " + std::to_string(struct_extra)
	      + " MB (MWD) + " + std::to_string(mwd_buffer_size)
	      + " % (metadata).",1);

  // Get the user defined queue capacity
  size_t queue_capacity = cfg.getValue<int>("stm.queue_capacity");
  // The buffer pool size (number of pre-allocated data structs)
  size_t buffer_pool_size = cfg.getValue<int>("stm.buffer_pool_size");
  // Calculate the total size of the buffer pool as function of the number of queues
  buffer_pool_total = buffer_pool_size * numThreads;

  // Calculate the total intended buffer memory
  size_t buffer_mem = struct_size*buffer_pool_total*1e6; // bytes
  
  // Log to user
  logger->log("BufferPool: Starting STM DAQ with " + std::to_string(numThreads) + " threads...",1);
  logger->log("BufferPool: "+std::to_string(numThreads) +
	      " queues (capacity = " + std::to_string(queue_capacity) +
	      " * " + std::to_string(struct_size) + " MB data structs).",1);
  logger->log("BufferPool: Buffer pool of " + std::to_string(numThreads) +
	      " * " + std::to_string(buffer_pool_size) +
	      " = " + std::to_string(buffer_pool_total) + " data structs.",1);
  logger->log("BufferPool: Intended total buffer memory = " +
	      std::to_string(buffer_mem*1e-9) + " GB.",1);

  // Get the available memeory of the system
  const size_t FREE_MEM = sysconf(_SC_AVPHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
  logger->log("BufferPool: Available system memory = " + std::to_string(FREE_MEM*1e-9) + " Gbytes.",1);

  // If the total intended buffer memory is more than the available memory
  if(buffer_mem > FREE_MEM){
    // Throw error
    std::string warning = "ERROR. Intended total buffer memory = " +
      std::to_string(buffer_mem*1e-9) + 
      " GB > available memory = " +
      std::to_string(FREE_MEM*1e-9) +
      " GB";
    logger->log(warning,0);
  }
  
  // Set the buffer pool size
  pool = std::make_unique<boost::lockfree::stack<std::shared_ptr<DataStruct>>>(buffer_pool_total);

  // Set the reset queue size
  resetQueue = std::make_unique<boost::lockfree::spsc_queue<std::shared_ptr<DataStruct>>>(buffer_pool_total);

  // Create pool of data struct buffers
  for (size_t i = 0; i < buffer_pool_total; ++i){
    pool->push(std::make_shared<DataStruct>(buffer_pool_total-i-1, // Buffer ID Num
					    raw_buffer_size*1e6, // Raw buffer size (bytes)
					    zs_buffer_size*1e6, // ZS buffer size (bytes)
					    zs_max_noise_len, // The length of noise data stored for the DQM (ADCs)
					    mwd_buffer_size*1e6, // ZS buffer size (bytes)
					    max_events_per_struct)); 
  }

  // Start the reset buffer thread
  resetThread = std::thread(&BufferPool::resetWorker, this);
  
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
