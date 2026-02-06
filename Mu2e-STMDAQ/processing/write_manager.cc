#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cstdio>

// Include write manager header
#include "Mu2e-STMDAQ/processing/write_manager.hh"

// Constructor: Load config, initialize state, and start threads
WriteManager::WriteManager(const std::shared_ptr<AsyncLogger>& logger_,
                           const std::shared_ptr<STMdata>& stm_,
                           const std::shared_ptr<SignalHandler>& signal_) :
  logger(logger_), stm(stm_), signal(signal_),
  eb(std::make_shared<EventBuilder>(logger,stm)),
  output_dir(stm->write_config.output_dir),
  timestamp(Timer::run_start_timestamp()),
  stream_num(stm->write_config.tot_streams_enabled),
  streamID(stream_num),
  total_bytes(stream_num),
  file_opener(stream_num),
  ctx(stream_num),
  event(3*stm->buffer_config.raw_len,0) {

  // Get the stream IDs
  int num = 0;
  for (size_t i = 0; i < stm->write_config.streamID.size(); i++){
    if (stm->write_config.stream_enabled[i]){
      streamID[num] = stm->write_config.streamID[i];
      ++num;
    }
  }
  // Check the number of streams is right
  if (num != stream_num){
    logger->log("WriteManager:: Error! Mismatch with number of write streams from config! num = " + std::to_string(num) + ", stream_num = " + std::to_string(stream_num),0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return;
  }
    
  // Initialise total bytes written
  for (auto& tb : total_bytes) tb.store(0, std::memory_order_relaxed);

  // Create the output directory if it doesn't already exist
  std::filesystem::create_directories(output_dir);

  // Loop over all streams
  for (int i = 0; i < stream_num; i++){
    // Store streamID in context
    ctx[i].streamID = streamID[i];
    // Store the maximum file size for each stream 
    // AND Initialize lock-free file queues with configured buffer depth
    // AND Launch file manager threads for each enabled stream
    // Set file prefix
    ctx[i].prefix = stm->write_config.file_prefix;
    // Max file size
    ctx[i].max_file_size = stm->write_config.max_file_size;
    // Queues
    ctx[i].file_queue = std::make_unique<FileQueue>(stm->write_config.file_buffer_depth);
    ctx[i].retired_files = std::make_unique<FileQueue>(stm->write_config.file_buffer_depth);
    logger->log("WriteManager: Initialized file_queue and retired_files for " + streamID[i] + " ctx.", 1);
    // Thread
    file_opener[i] = std::thread(&WriteManager::file_manager_thread, this, i, std::ref(ctx[i]));
    logger->log("WriteManager: Launched file_manager_thread for " + streamID[i] + ".", 1);
  }

  // If any writing, start statistics logger thread
  if (stream_num > 0){
    stats_logger = std::thread(&WriteManager::logger_thread, this);
    logger->log("WriteManager: Launched loggerthread.", 1);
  }

  // Register operations for OperationManager
  register_operation("write_events", [this](auto& b){ push(b,"events"); });
  register_operation("write_raw", [this](auto& b){ push(b,"raw"); });
  register_operation("write_zs", [this](auto& b){ push(b,"zs"); });
  register_operation("write_ph", [this](auto& b){ push(b,"ph"); });
  
}

// Accept buffer and write the requested stream to disk
void WriteManager::push(std::shared_ptr<DataStruct> buffer, const std::string& type) {

  // Get context
  auto [idx, this_ctx] = select_context(type);
  if (!this_ctx) return;

  // Bytes written this call
  size_t bytes = 0;

  // If writing event data
  if (type == "events"){
    // Call write event loop function
    bytes = write_events(*this_ctx, buffer);
  }
  // Else if raw, zs, ph
  else{
    // Write data type
    bytes = write_vector(type, *this_ctx, buffer);
  }
    
  // Increase total bytes written
  total_bytes[idx] = bytes;
  
}

// Write event data (all types) event by event 
size_t WriteManager::write_events(Context& this_ctx, std::shared_ptr<DataStruct> buffer){

  // Bytes written this call
  size_t tot_bytes = 0;
  
  // Loop through all EWTs/events in this buffer
  const size_t n = buffer->EWT_count;

  // Loop over EWTs
  for (size_t i = 0; i < n; ++i) {

    // Build one packed event into scratch
    const size_t len = eb->build_event(buffer, i, event);
    if (len == 0){
      logger->log("WriteManager::write_events: Event builder returned event data length of zero size...", 2);
      continue;
    }

    // Get data and byte payload
    const void* data_ptr = static_cast<const void*>(event.data());
    const size_t data_bytes = len * sizeof(int16_t);

    // Write data to file
    write_bytes(this_ctx, data_ptr, data_bytes);

    // Increase bytes written
    tot_bytes += data_bytes;
    
  }

  // Return bytes written this call
  return tot_bytes;
  
}


// Write full buffer of single data type
size_t WriteManager::write_vector(const std::string& type, Context& this_ctx, std::shared_ptr<DataStruct> buffer) {

  // Get the data stream data (variant)
  const auto selected = select_data(type, buffer);

  // Lift info out to outer scope
  const void* data_ptr = nullptr;   // raw pointer to contiguous bytes
  size_t data_bytes    = 0;         // total bytes to write

  // Extract pointer and compute byte size in a type-safe way
  std::visit([&](const auto& data) {
    using Vec = std::decay_t<decltype(data.vec)>;
    using T   = typename Vec::value_type;

    data_ptr   = static_cast<const void*>(data.vec.data());
    data_bytes = data.length * sizeof(T);
  }, selected);

  // Write bytes to file
  write_bytes(this_ctx, data_ptr, data_bytes);

  // Return bytes written this call
  return data_bytes;
  

}

// Core binary file writing logic
void WriteManager::write_bytes(Context& this_ctx, const void* data_ptr, size_t data_bytes){

  // Return if no bytes to write
  if (data_bytes == 0) return;
  
  // Load the current file
  auto file = std::atomic_load(&this_ctx.current_file);

  // Check if we need to switch to a new file 
  check_file(this_ctx, file, data_bytes);

  // Check file is valid
  if (!file || !file->stream || !file->stream->good()) {
    logger->log("WriteManager: ERROR! Trying to write to a null or bad file!", 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  // Perform the actual write
  file->stream->write(reinterpret_cast<const char*>(data_ptr), static_cast<std::streamsize>(data_bytes));
  this_ctx.current_file_size += data_bytes;

}

// Check if we need to switch to a new file 
void WriteManager::check_file(Context& this_ctx, std::shared_ptr<FileHandle>& file, size_t data_bytes){

  // Check if we need to have enough space in this file
  if (file && this_ctx.current_file_size + data_bytes <= this_ctx.max_file_size) return;

  // Infinite loop - for when not enough space in file
  while (true) {

    // Get the new file and reset next_file
    auto new_file = std::atomic_exchange(&this_ctx.next_file, std::shared_ptr<FileHandle>{});

    // If new file is valid
    if (new_file) {
      // Retire old file
      auto old_file = std::atomic_exchange(&this_ctx.current_file, new_file);
      // Reset the size counter
      this_ctx.current_file_size = 0;
      // Push old file to retired queue if it's valid
      if (old_file && old_file->stream && old_file->stream->is_open()) {
        this_ctx.retired_files->push(old_file);
      }
      // Swap in the new file, retire the old one
      file = new_file;
      // Return with new file
      return;
    }

    // No new file available: wait a bit and retry soon 
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    
  }
  
}



// Asynchronous thread that manages file opening and closing
void WriteManager::file_manager_thread(int idx, Context& this_ctx) {
  
  // While no stop signal
  while (!stop::should_stop()) {
    
    // If no next file is loaded and one is needed
    if (!std::atomic_load(&this_ctx.next_file)) {
      // Pop a file from the pre-opened file queue
      std::shared_ptr<FileHandle> next;
      if (this_ctx.file_queue->pop(next)) {
        // Decrement the counter
        this_ctx.file_queue_size--;  
        // Atomically store the next file into next_file
        std::atomic_store(&this_ctx.next_file, next);  // Install as next file
      }
    }
    
    // Refill file_queue to maintain buffer depth
    // Only refill if queue size is below buffer depth    
    while (this_ctx.file_queue_size.load() < stm->write_config.file_buffer_depth) {
      std::string filename = generate_filename(this_ctx.prefix, streamID[idx], timestamp, this_ctx.subrun++);
      auto filepath = output_dir / filename;
      
      // Create a new FileHandle
      auto f = std::make_shared<FileHandle>();
      f->stream = std::make_shared<std::ofstream>(filepath, std::ios::binary);
      f->filepath = filepath.string();
      if (f->stream->is_open()) {
        if (this_ctx.file_queue->push(f)) {
	  // Increment the counter to track queue size
	  this_ctx.file_queue_size++;  
        }
	else {
	  // Queue full: stop trying to fill
	  break;
        }
      } else {
        // File open failed
	logger->log("WriteManager: ERROR! Failed to open file: " + filepath.string() +
		    ". Check disk space, directory existence, and file permissions.",0);
        break;
      }
    }

    // Close any retired files passed by writer
    std::shared_ptr<FileHandle> closing;
    while (this_ctx.retired_files->pop(closing)) {
      if (closing && closing->stream && closing->stream->is_open()) {
        closing->stream->close();
        std::cout << "WriteManager: Closed completed file " << closing->filepath << std::endl;
      }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Throttle loop
  }
}

// Logs throughput and disk space every N seconds
void WriteManager::logger_thread() {

  // Varables to track last data stream size total
  //  size_t last_raw = 0, last_zs = 0, last_ph = 0;
  std::vector<size_t> cur(stream_num,0);
  std::vector<size_t> last(stream_num,0);
  std::vector<double> gbps(stream_num,0);
  
  // Track last log time
  auto last_log_time = std::chrono::steady_clock::now();

  // Track last sample time (for dt)
  auto last_sample_time = std::chrono::steady_clock::now();
  
  // While no stop signal
  while (!stop::should_stop()) {

    // Check/update every 0.1s
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Get the total written data size for each stream    
    for (int i = 0; i < stream_num; i++) cur[i] = total_bytes[i].load();

    // Measure actual elapsed time since last sample
    auto now_sample = std::chrono::steady_clock::now();
    double dt = std::chrono::duration<double>(now_sample - last_sample_time).count();
    last_sample_time = now_sample;
    
    // Guard against pathological cases
    if (dt <= 0.0) continue;    
    
    // Calculate total write speed in Gbit/s
    for (int i = 0; i < stream_num; i++) gbps[i] = 8.0 * (cur[i] - last[i]) / (1e9 * dt); 

    // Calculate total write speed in bytes per second
    double total_bps = 0;
    for (int i = 0; i < stream_num; i++) total_bps += (cur[i] - last[i]);
    total_bps /= dt;    
    // and in Gbit/s
    double total_gbps = 8.0 * total_bps / 1e9;

    // Store last size for next round
    for (int i = 0; i < stream_num; i++) last[i] = cur[i];
    
    // Check if it's time to log
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_log_time).count();

    // If its time to log...
    if (elapsed >= stm->write_config.log_interval_sec) {
      
      // Log to user
      if (total_gbps > 0){
        std::string output = "WriteManager: Speed (Gbit/s): ";
        for (int i = 0; i < stream_num; i++) output += streamID[i] + " = " + format_gbps(gbps[i]) + " | ";
        output += "total = "+ format_gbps(total_gbps) + ".";
        logger->log(output,1);
      }
      
      // Get disk space
      auto space = std::filesystem::space(output_dir);
      // Calculate remaining space
      auto used = space.capacity - space.available;
      
      // If writing data
      if (total_bps > 0) {
        // Calculate the time remaining until the disk is full
        double seconds_remaining = space.available / total_bps; // in bytes
        // Break down into hours, minutes, seconds
        int total_seconds = static_cast<int>(seconds_remaining);
        int hours = total_seconds / 3600;
        int minutes = (total_seconds % 3600) / 60;
        int seconds = total_seconds % 60;
        std::ostringstream time_ss;
        if (hours > 0) {
          // e.g., "2 hr 15 min"
          time_ss << hours << " hr " << minutes << " min";
        }
        else if (minutes > 0) {
          // e.g., "45 min 30 sec"
          time_ss << minutes << " min " << seconds << " sec";
        }
        else {
          // Less than a minute
          time_ss << seconds << " sec";
        }
        // Calculate percetanges
        double percent_used = 100.0 * static_cast<double>(used) / space.capacity;
        double percent_available = 100.0 - percent_used;	
        // Log to user
        logger->log("WriteManager: Disk: free = " +
                    format_gb(space.available) +
                    " (" + format_percent(percent_available) + "%) | time until full: " +
                    time_ss.str(), 1);
      }
            
      // If the remaining disk space is less than 10%
      double remaining = (double)space.available / space.capacity;
      if (remaining < 0.1) {
        logger->log("WriteManager: WARNING: Remaining disk space below " +
                    std::to_string(remaining*100) + " %.",2);
      }
      
      // reset timer
      last_log_time = now;  
      
    }
    
  }

}

// Cleanly stop threads and close any unused resources
void WriteManager::shutdown() {
  
  // Join file openers
  for (auto& t : file_opener) {
    if (t.joinable()) t.join();
  }
  
  // Join stats logger
  if (stats_logger.joinable()) stats_logger.join();
  
  // Cleanup any unused files
  for (int i = 0; i < stream_num; i++) cleanup_unused_files(ctx[i]);
  
}

// Delete any pre-opened files that were never used
void WriteManager::cleanup_unused_files(Context& this_ctx) {

  // First: clean up the current file if it was never written to
  auto current = std::atomic_load(&this_ctx.current_file);
  if (current && current->stream && current->stream->is_open()) {
    if (this_ctx.current_file_size == 0) {
      current->stream->close();
      std::remove(current->filepath.c_str());
      std::cout << "WriteManager: Deleted unused file: " << current->filepath << std::endl;
    }
    else {
      current->stream->close();  // close but keep the data file
      std::cout << "WriteManager: Closed file: " << current->filepath << std::endl;
    }
  }
  
  // Second: clean up the next_file if it was never used  
  auto next = std::atomic_load(&this_ctx.next_file);
  if (next && next->stream && next->stream->is_open()) {
    next->stream->close();
    std::remove(next->filepath.c_str());
    std::cout << "WriteManager: Deleted unused file: " << next->filepath << std::endl;
  }
  
  // Finally: clean up remaining files left in the queue
  std::shared_ptr<FileHandle> f;
  while (this_ctx.file_queue->pop(f)) {
    if (f && f->stream && f->stream->is_open()) {
      f->stream->close();
      std::remove(f->filepath.c_str());
      std::cout << "WriteManager: Deleted unused file: " << f->filepath << std::endl;
    }
  }
    
}
