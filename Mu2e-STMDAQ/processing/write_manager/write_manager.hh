#ifndef WRITE_MANAGER_hh_
#define WRITE_MANAGER_hh_

#include <vector>                   
#include <memory>                   
#include <filesystem>               
#include <atomic>                   
#include <thread>                   
#include <fstream>                  
#include <string>                   

// Config code
#include "Mu2e-STMDAQ/config/config_new.hh"
// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// Ring Buffer code
#include "Mu2e-STMDAQ/utils/ring_buffer.hh" 

class WriteManager {

private:

  // Store reference to the Config instance
  const Config& cfg;   
  
  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;
  
  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;
  
  // The channel number
  const int CHAN;

  // Lock-free queue for pre-opened file pointers
  using FileQueue = boost::lockfree::spsc_queue<std::shared_ptr<std::ofstream>>;
  
  // Output directory for file writing
  std::filesystem::path output_dir;
  
  // Flag to request thread shutdown
  std::atomic<bool> shutdown_requested{false};
  
  // File pre-opening depth (user-configurable)
  int file_buffer_depth;
  
  // Interval in seconds for stats logging
  int log_interval_sec;
  
  // Stats counters per stream (total bytes written)
  std::atomic<size_t> total_written_raw{0};
  std::atomic<size_t> total_written_zs{0};
  std::atomic<size_t> total_written_mwd{0};
  
  // Threads: file managers for raw, zs, mwd and a logger
  std::thread raw_file_opener;
  std::thread zs_file_opener;
  std::thread mwd_file_opener;
  std::thread stats_logger;
  
  // Per-stream context containing rotation and queue state
  struct Context {
    // File being written to
    std::shared_ptr<std::ofstream> current_file;
    // File prepared for next switch
    std::shared_ptr<std::ofstream> next_file;
    // Queue of pre-opened files
    std::unique_ptr<FileQueue> file_queue;
    // Lock-free queue for files to close
    std::unique_ptr<FileQueue> retired_files;
    // Bytes written so far
    std::atomic<size_t> current_file_size = 0;
    // File prefix (from config)
    std::string prefix;
    // Last full file path used
    std::string last_opened_path;
    // Subrun number counter
    int subrun = 0;
    // Max bytes before file switch
    size_t max_file_size = 0;
    // Whether this stream is enabled
    bool enabled = false;                                      
  };

  // Stream-specific contexts
  Context raw_ctx, zs_ctx, mwd_ctx;
        
public:
  
  // Constructor
  WriteManager(const Config& cfg_,
	       const std::shared_ptr<AsyncLogger>& logger,
	       const std::shared_ptr<STMdata>& stm_,
	       const std::shared_ptr<SignalHandler>& signal_,
	       const int CHAN_);
  
  // Destructor: 
  ~WriteManager() {
    // Safely shuts down all threads and close unused files
    shutdown();
    std::cout << "Write Manager destructor called.\n";
  }
  
  // External entry point to push one buffer for a specific stream
  void push(std::shared_ptr<DataStruct> buffer, const std::string& stream_type);

  // Core writing logic for one stream from a buffer
  void write_vector(const std::string& type, std::shared_ptr<DataStruct> buffer);
  
  // Utility: get the context for a specific stream type
  Context& select_context(const std::string& type);

  // Utility: extract the correct data vector from buffer
  const std::vector<int16_t>& select_vector(const std::string& type,
					     std::shared_ptr<DataStruct> buffer);
  
  // File manager thread — opens, closes, and queues files
  void file_manager_thread(const std::string& type);
    
  // Utility: create a file name using prefix, type, timestamp, and subrun
  std::string generate_filename(const std::string& prefix,
				const std::string& type,
				const std::string& timestamp,
				int subrun);
  
  // Get current local timestamp formatted for filenames
  std::string current_timestamp();
  
  // Format byte count as human-readable GB string
  std::string format_gb(uint64_t bytes);
  
  // Periodically logs disk space and write throughput
  void logger_thread();
    
  // Graceful shutdown
  void shutdown();

  // Cleanup routine for any unused files
  void cleanup_unused_files(Context& ctx);
  
};

#endif 
