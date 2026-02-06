#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cstdio>

// Include write manager header
#include "Mu2e-STMDAQ/processing/write_manager.hh"

// Constructor: Load config, initialize state, and start threads
WriteManager::WriteManager(const Config& cfg_,
			   const std::shared_ptr<AsyncLogger>& logger_,
			   const std::shared_ptr<STMdata>& stm_,
			   const std::shared_ptr<SignalHandler>& signal_,
			   const int CHAN_) :
  cfg(cfg_), logger(logger_), stm(stm_), signal(signal_), CHAN(CHAN_), shutdown_requested(false) {
  
  // Read config values
  output_dir = cfg.getValue<std::string>("stm.write_data.write_dir"); // The write directory
  file_buffer_depth = cfg.getValue<int>("stm.write_data.file_buffer_depth"); // Number of buffered files
  log_interval_sec = cfg.getValue<int>("stm.write_data.log_interval_sec"); // The write logging frequency

  // Create the output directory if it doesn't already exist
  std::filesystem::create_directories(output_dir);
  
  // Read enable flags for each stream
  raw_ctx.enabled = cfg.getValue<bool>("stm.write_data.write_raw");
  zs_ctx.enabled  = cfg.getValue<bool>("stm.write_data.write_zs");
  mwd_ctx.enabled = cfg.getValue<bool>("stm.write_data.write_mwd");
  
  // Get and store the filename prefix
  raw_ctx.prefix = cfg.getValue<std::string>("stm.write_data.filename_prefix");
  zs_ctx.prefix  = cfg.getValue<std::string>("stm.write_data.filename_prefix");
  mwd_ctx.prefix = cfg.getValue<std::string>("stm.write_data.filename_prefix");
  
  // Get and store the maximum file size for each stream
  raw_ctx.max_file_size = cfg.getValue<int>("stm.write_data.max_file_size");
  zs_ctx.max_file_size  = cfg.getValue<int>("stm.write_data.max_file_size");
  mwd_ctx.max_file_size = cfg.getValue<int>("stm.write_data.max_file_size");
  
  // Initialize lock-free file queues with configured buffer depth
  // Files to write to
  raw_ctx.file_queue = std::make_unique<FileQueue>(file_buffer_depth);
  zs_ctx.file_queue  = std::make_unique<FileQueue>(file_buffer_depth);
  mwd_ctx.file_queue = std::make_unique<FileQueue>(file_buffer_depth);
  // Files to close
  raw_ctx.retired_files = std::make_unique<FileQueue>(file_buffer_depth);
  zs_ctx.retired_files  = std::make_unique<FileQueue>(file_buffer_depth);
  mwd_ctx.retired_files = std::make_unique<FileQueue>(file_buffer_depth);
  
  // Launch file manager threads for each enabled stream
  if (raw_ctx.enabled)
    raw_file_opener = std::thread(&WriteManager::file_manager_thread, this, "raw");
  if (zs_ctx.enabled)
    zs_file_opener = std::thread(&WriteManager::file_manager_thread, this, "zs");
  if (mwd_ctx.enabled)
    mwd_file_opener = std::thread(&WriteManager::file_manager_thread, this, "mwd");
  
  // Start statistics logger thread
  stats_logger = std::thread(&WriteManager::logger_thread, this);
  
}

// Accept buffer and write the requested stream to disk
void WriteManager::push(std::shared_ptr<DataStruct> buffer, const std::string& type) {
  if (type == "raw")
    total_written_raw += buffer->raw.size() * sizeof(int16_t);
  else if (type == "zs")
    total_written_zs += buffer->zs.size() * sizeof(int16_t);
  else if (type == "mwd")
    total_written_mwd += buffer->mwd.size() * sizeof(int16_t);
  
  // Forward to internal writer
  write_vector(type, buffer);
  
}

// Core logic to write data and manage rotation
void WriteManager::write_vector(const std::string& type, std::shared_ptr<DataStruct> buffer) {

  // Get the data stream context
  auto& ctx = select_context(type);
  // Get the data stream vector
  const auto& vec = select_vector(type, buffer);
  // Get the number of bytes to write
  size_t data_bytes = vec.size() * sizeof(int16_t);
  // Load the current file
  auto file = std::atomic_load(&ctx.current_file); 
  
  // Check if we need to switch to a new file (if no file or reached max size)
  if (!file || ctx.current_file_size + data_bytes > ctx.max_file_size) {
    
    // Get the new file and ready next_file for the next in the queue
    auto new_file = std::atomic_exchange(&ctx.next_file, std::shared_ptr<std::ofstream>{});
    if (new_file) {
      // Swap in new file, save the old one to retire
      auto old_file = std::atomic_exchange(&ctx.current_file, new_file); 
      // Set the current size to zero
      ctx.current_file_size = 0;
      // If the old file exists ans is open
      if (old_file && old_file->is_open()) {
	// Send to file_manager for async close
        ctx.retired_files->push(old_file);  
      }
    }
    // Else if no new file exists
    else {
      // Wait and retry soon
      std::this_thread::sleep_for(std::chrono::microseconds(10));
      return;
    }
    
    // Reload the current file (now the new/next file)
    file = std::atomic_load(&ctx.current_file);
    
  }

  // Perform the actual write
  if (file) { // If file exists
    // Write data
    file->write(reinterpret_cast<const char*>(vec.data()), data_bytes);
    // Keep track of the current file size
    ctx.current_file_size += data_bytes;
  }
  else{
    logger->log("WriteManager: ERROR! Trying to write to a null file!",2);
  }
}

// Selects the correct context by stream name
WriteManager::Context& WriteManager::select_context(const std::string& type) {
  if (type == "raw") return raw_ctx;
  if (type == "zs") return zs_ctx;
  return mwd_ctx;
}

// Extract the correct vector from the buffer
const std::vector<int16_t>& WriteManager::select_vector(const std::string& type,
							std::shared_ptr<DataStruct> buffer) {
  if (type == "raw") return buffer->raw;
  if (type == "zs") return buffer->zs;
  return buffer->mwd;
}

// Asynchronous thread that manages file opening and closing
void WriteManager::file_manager_thread(const std::string& type) {

  // Get the data stream context
  auto& ctx = select_context(type);

  // While shutdown hasn't been requested
  while (!shutdown_requested.load()) {
    
    // If no next file is loaded and one is needed
    if (!std::atomic_load(&ctx.next_file)) {
      // Pop a file from the pre-opened file queue
      std::shared_ptr<std::ofstream> next;
      if (ctx.file_queue->pop(next)) {
        // Atomically store the next file into next_file
        std::atomic_store(&ctx.next_file, next);  // Install as next file
      }
    }
    
    // Refill file_queue to maintain buffer depth    
    while (true) {
      std::string timestamp = current_timestamp();
      std::string filename = generate_filename(ctx.prefix, type, timestamp, ctx.subrun++);
      auto filepath = output_dir / filename;
      ctx.last_opened_path = filepath.string();      
      auto f = std::make_shared<std::ofstream>(filepath, std::ios::binary);
      if (f->is_open()) {
        if (!ctx.file_queue->push(f)) {
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
    std::shared_ptr<std::ofstream> closing;
    while (ctx.retired_files->pop(closing)) {
      if (closing && closing->is_open()) {
        closing->close();
        std::cout << "[write_manager] Closed file: " << ctx.last_opened_path << std::endl;
      }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Throttle loop
  }
}



// Constructs a unique output filename
std::string WriteManager::generate_filename(const std::string& prefix,
					    const std::string& type,
					    const std::string& timestamp,
					    int subrun) {
  std::ostringstream oss;
  oss << prefix << "_" << type << "_" << timestamp << "_subrun" << subrun << ".bin";
  return oss.str();
}

// Generates current timestamp in human-readable format
std::string WriteManager::current_timestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
  return oss.str();
}

// Converts byte size to GB string
std::string WriteManager::format_gb(uint64_t bytes) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1) << (bytes / 1e9) << " GB";
  return oss.str();
}

// Logs throughput and disk space every N seconds
void WriteManager::logger_thread() {

  // Varables to track last data stream size total
  size_t last_raw = 0, last_zs = 0, last_mwd = 0;

  // While shutdown not requested
  while (!shutdown_requested.load()) {

    // Sleep for the user confirurable amount
    std::this_thread::sleep_for(std::chrono::seconds(log_interval_sec));

    // Get the total written data size for each stream
    size_t cur_raw = total_written_raw.load();
    size_t cur_zs  = total_written_zs.load();
    size_t cur_mwd = total_written_mwd.load();

    // The time interval since the last check
    double dt = log_interval_sec;
    // Calculate total write speed in Gbit/s
    double raw_gbps = 8.0 * (cur_raw - last_raw) / (1e9 * dt);
    double zs_gbps  = 8.0 * (cur_zs  - last_zs ) / (1e9 * dt);
    double mwd_gbps = 8.0 * (cur_mwd - last_mwd) / (1e9 * dt);
    // Store last size for next round
    last_raw = cur_raw;
    last_zs  = cur_zs;
    last_mwd = cur_mwd;

    logger->log("WriteManager: Speed: raw=" + std::to_string(raw_gbps)+
		" Gbit/s | zs=" + std::to_string(zs_gbps) +
		" Gbit/s | mwd="+ std::to_string(mwd_gbps) + " Gbit/s.",1);

    // Get disk space
    auto space = std::filesystem::space(output_dir);
    // Calculate remaining space
    auto used = space.capacity - space.available;
    // Calculate total write speed in bytes per second
    double total_bps = (cur_raw + cur_zs + cur_mwd - last_raw - last_zs - last_mwd) / dt;
    // If writing data
    if (total_bps > 0) {
      // Calculate the time remaining until the disk is full
      double seconds_remaining = space.available / total_bps;
      int minutes = static_cast<int>(seconds_remaining / 60);
      // Log to user
      logger->log("WriteManager:  Disk: used=" +
		  format_gb(used) +
		  " | available=" +
		  format_gb(space.available) +
		  " | est. time until full: " +
		  std::to_string(minutes) + " min.",1);
    }

    // If the remaining disk space is less than 10%
    double remaining = (double)space.available / space.capacity;
    if (remaining < 0.1) {
      logger->log("WriteManager: WARNING: Remaining disk space below " +
		  std::to_string(remaining*100) + " %.",2);
    }
  }
}

// Cleanly stop threads and close any unused resources
void WriteManager::shutdown() {

  // Signal shutdown has been requested
  shutdown_requested.store(true);

  // Join all threads
  for (auto& t : {&raw_file_opener, &zs_file_opener, &mwd_file_opener, &stats_logger}) {
    if (t->joinable()) t->join();
  }

  // Cleanup any unused files
  cleanup_unused_files(raw_ctx);
  cleanup_unused_files(zs_ctx);
  cleanup_unused_files(mwd_ctx);
  
}

// Delete any pre-opened files that were never used
void WriteManager::cleanup_unused_files(Context& ctx) {

  // If stream not enabled, no need
  if (!ctx.enabled) return;

  // Create pointer to file to close
  std::shared_ptr<std::ofstream> f;
  // While there are files left in the queue
  while (ctx.file_queue->pop(f)) {
    // If the file exists and is open
    if (f && f->is_open()) {
      // Close the file
      f->close();
      // Remove the file
      std::remove(ctx.last_opened_path.c_str());
      std::cout << "WriteManager: Deleted unused file: " << ctx.last_opened_path << std::endl;
    }
  }
  
}
