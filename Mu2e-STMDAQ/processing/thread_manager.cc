// Thread Manager code                                                       
#include "Mu2e-STMDAQ/processing/thread_manager.hh"

// Constructor
ThreadManager::ThreadManager(const std::shared_ptr<cpu_utils>& cpu_,
                             const std::shared_ptr<AsyncLogger>& logger_,
                             const std::shared_ptr<STMdata>& stm_,
                             const std::shared_ptr<SignalHandler>& signal_,
                             const std::shared_ptr<BufferPool>& pool_,
                             const std::shared_ptr<OperationManager>& om,
                             const std::shared_ptr<HardwareManager> hw_) :
  cpu(cpu_), logger(logger_), stm(stm_), signal(signal_), pool(pool_), hw(hw_),
  udp(std::dynamic_pointer_cast<UDP>(om->get_class("UDP"))),
  thread_num(om->getUseOps().size()),
  thread_name(thread_num+1),
  finished(thread_num),
  buffer_count(thread_num,0),
  tot_bytes_processed(thread_num,0),
  avg_speed_num(thread_num,0), avg_speed_den(thread_num,0),
  baseline_window_buffers(stm->baseline_config.hist_window_buffers) {

  // Check for udp initliasation
  if (!udp) {
    logger->log("ThreadManager: Instance of UDP class not found",0);
    return;
  }
      
  // Get selected operations from operation manager
  const std::vector<std::pair<std::string, op_any>>& ops = om->getUseOps();

  // For all selected threads
  for (size_t i = 0; i < thread_num; ++i) {
    // Store finished flag as false
    finished[i].store(false);
    // Get the thread name
    thread_name[i] = ops[i].first;
    // Initialise queues
    if (i != thread_num-1) queues.emplace_back(std::make_shared<BufferQueue<DataStruct>>(stm->buffer_config.queue_capacity));
    // Get input queue
    auto inq = (i == 0) ? nullptr : queues[i - 1];
    // Get output queue
    auto outq = (i == thread_num-1) ? nullptr : queues[i];

    logger->log("ThreadManager: Creating " + thread_name[i] + " thread with index " + std::to_string(i) + "...",1);
     
    // Get the data operation
    const op_any& operation = ops[i].second;
    // Start thread
    threads.emplace_back(&ThreadManager::worker_thread, this,
                         i, inq, outq, operation);
  }

  // Run data source thread in this thread
  data_source_thread(thread_num);

}

// General worker thread function
void ThreadManager::worker_thread(const size_t thrd_idx,
                                  const std::shared_ptr<BufferQueue<DataStruct>>& inq,
                                  const std::shared_ptr<BufferQueue<DataStruct>>& outq,
                                  const op_any operation){

  // Get thread name
  const std::string thread = thread_name[thrd_idx];

  // Check data operation is valid
  std::visit([&](auto const& fn){
  if (!fn) {
    logger->log("ThreadManager::workerThread received an empty operation function.",0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return;  
  }
  }, operation);

  // Pin thread to core
  size_t core = cpu->get_next_core(thread);
  
  // First buffer
  bool first_buffer = true;

  // Current buffer
  std::shared_ptr<DataStruct> buffer = nullptr;
  // Prev buffer
  std::shared_ptr<DataStruct> prev_buffer = nullptr;
  
  // Start timing
  auto thread_start_time = std::chrono::high_resolution_clock::now();

  // Continue until first thread is done, input queue is empty
  // and previous thread is done
  while (!finished[0].load() ||
	 (thrd_idx > 0 && !finished[thrd_idx - 1].load()) ||
	 (inq && !inq->is_empty())) {

    // Dummy buffer
    //    std::shared_ptr<DataStruct> buffer = nullptr;    

    // First thread case
    if (thrd_idx == 0) {
      // Acquire new data struct from buffer pool
      buffer = pool->acquire();
    }       
    // Else, fetch data from queue
    else{
      // Retrieve a buffer from the input queue
      buffer = inq->try_pop();
    }

    // If no buffer available...
    if (!buffer){
      std::this_thread::yield();
      continue;
    }

    // If this buffer has NOT been set to do not use
    if (!buffer->do_not_use){      
      // Perform operation on buffer
      op_on_buffer(thrd_idx,operation,buffer,prev_buffer);
      // If first successful op, restart timer
      if (first_buffer) thread_start_time = std::chrono::high_resolution_clock::now();
    }

    // Push/release outgoing buffer
    if(!first_buffer) push_buffer(thrd_idx,outq,prev_buffer);

    // Shift buffers
    prev_buffer = std::move(buffer);
    
    // Set first buffer to false;
    first_buffer = false;
    
    // If in receive data thread and the user has requested a stop
    if (thrd_idx == 0 && stop::should_stop()){
      // Notify other threads to finish
      finished[0].store(true);
    }    
    
  }

  // Ensure last buffer is released
  if (prev_buffer) push_buffer(thrd_idx,outq,prev_buffer);
  
  // Log thread performance
  log_performance(thrd_idx,thread_start_time);
  
  // Signal completion to the next thread
  finished[thrd_idx].store(true);

  // If the last thread
  if (thread_num != 1 && thrd_idx == thread_num - 1) {
    if (inq->is_empty()) logger->log("Final queue emptied.", 1);
  }

  
}

// Perform operation on buffer
void ThreadManager::op_on_buffer(const size_t thrd_idx,
                                 const op_any operation,
                                 std::shared_ptr<DataStruct>& buffer,
                                 std::shared_ptr<DataStruct>& prev_buffer){


  // Get the wait time before the call
  std::chrono::duration<double> total_wait_before = wait();
      
  // Operation start time
  auto op_start_time = std::chrono::high_resolution_clock::now();
      
  // Process the data
  if (auto* f2 = std::get_if<op2>(&operation)) {
    (*f2)(buffer, prev_buffer);
  }
  else {
    auto* f1 = std::get_if<op1>(&operation);
    (*f1)(buffer);
  }      
  
      
  // Operation start time
  auto op_end_time = std::chrono::high_resolution_clock::now();
            
  // Calculate bytes processed
  uint64_t bytesProcessed = buffer->orig_len * sizeof(int16_t);
  // Update total bytes processed
  tot_bytes_processed[thrd_idx] += bytesProcessed;
      
  // Calculate elapsed operatoin time
  std::chrono::duration<double> op_time;
  // If UDP receiving...
  if (thrd_idx == 0){
    // Operation processing time minus any wait time
    op_time = op_end_time - op_start_time - (wait() - total_wait_before);
  }
  // Else all other operations
  else{
    op_time = op_end_time - op_start_time;
  }    
  // Calculate processing speed in Gbits
  double speed = (bytesProcessed * 8 / 1e9) / op_time.count() ;
      
  // Add performace to buffer
  buffer->cpu_performance[thrd_idx].second = speed;
  // Add to average speed
  avg_speed_num[thrd_idx] += speed;
  ++avg_speed_den[thrd_idx];
      
  // Increment the buffer count
  if (buffer->orig_len > 0){
    ++buffer_count[thrd_idx];
  }
  else{
    // Set buffer to do not use
    buffer->do_not_use = true;
  }

}

// Push/release outgoing buffer 
void ThreadManager::push_buffer(const size_t thrd_idx,
                                const std::shared_ptr<BufferQueue<DataStruct>>& outq,
                                std::shared_ptr<DataStruct>& buffer){

  // If outq exists, push processed data
  if (outq) {
    // If in last baseline thread
    if (thread_name[thrd_idx] == "Baseline::calc_baseline" &&
        // ... and we want to delay the DAQ for a baseline calculation
        stm->baseline_config.window_delay
        // .. and we haven't yet calculated the entire baseline window
        && buffer_count[thrd_idx] < baseline_window_buffers){
      // Set buffer to do not use
      buffer->do_not_use = true;
    }
    outq->push(buffer);      
  }
  // Last thread case
  else {
    // Return data struct to buffer pool
    pool->release(buffer);
  }

}

// Log thread performance 
void ThreadManager::log_performance(const size_t thrd_idx,
                                    const std::chrono::time_point<
                                    std::chrono::high_resolution_clock> thread_start_time){
  
  // End timing
  auto thread_end_time = std::chrono::high_resolution_clock::now();
  
  // Calculate elapsed time
  std::chrono::duration<double> elapsed = thread_end_time - thread_start_time - wait();
  // Convert bytes to Gbits
  double gbytes = tot_bytes_processed[thrd_idx] / 1e9;
  double gbits = gbytes * 8.0;
  
  // Print processing speed
  if (thrd_idx != 0) while (!finished[thrd_idx - 1].load()) continue;
  std::ostringstream performance;
  performance << "ThreadManager: " << thread_name[thrd_idx] << " processed "
              << std::fixed << std::setprecision(2)
              << gbytes << " Gbytes ("
              << buffer_count[thrd_idx] << " buffers), average speed [Gbit/s] = "
              << std::fixed << std::setprecision(2)
              << (avg_speed_num[thrd_idx] / avg_speed_den[thrd_idx]) << " (operation) / "
              << (gbits / elapsed.count()) << " (worker thread).";
  logger->log(performance.str(), 1);

}

// Data source thread function
void ThreadManager::data_source_thread(const size_t thrd_idx){

  // Get thread name
  const std::string thread = thread_name[thrd_idx];

  // Pin thread to core
  size_t core = cpu->get_next_core(thread);

  // Log to user
  if (stm->master_config.use_sw_sim){
    logger->log("ThreadManager::data_source_thread: Initialising software data sending...",1);
  }
  else{
    logger->log("ThreadManager::data_source_thread: Initialising firmware data sending...",1);
    if (stm->fw_config.use_dtc_sim){
      logger->log("ThreadManager::data_source_thread: Initialising DTC simulation...",1);
    }
  }
    
  // If using software simulation for data source
  if (stm->master_config.use_sw_sim){
    // Sleep for 3 seconds
    for (int i = 0; i < 3; i++){
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "Starting softwate data sending in " << 3-i << std::endl;
    }
    // Log to user
    logger->log("ThreadManager::data_source_thread: Starting data sending...",1);
  }
  // If you dtc_sim_test_new.py
  else if (stm->fw_config.use_dtc_sim){
    // If we are using version of the hardware manager z
    if (hw){
      // Sleep for 3 seconds
      for (int i = 0; i < 3; i++){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Starting firmware data sending in " << 3-i << std::endl;
      }
      // Log to user
      logger->log("ThreadManager::data_source_thread: Starting data sending...",1);
      // Run dtc simulation
      hw->run_dtc_sim();
    }
  }
  // If using firmware for data source
  else{
    // Log to user
    logger->log("Expecting signal from DTC...",1);
  }  
  
}

                                  
