#ifndef WRITE_MANAGER_hh_
#define WRITE_MANAGER_hh_

#include <boost/lockfree/spsc_queue.hpp>
#include <vector>                   
#include <memory>                   
#include <filesystem>               
#include <atomic>                   
#include <thread>                   
#include <fstream>                  
#include <string>
#include <variant>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// Operations base header                                                                     
#include "Mu2e-STMDAQ/processing/operations_base.hh"
// Event builder code
#include "Mu2e-STMDAQ/processing/event_builder.hh"
// Timer code
#include "Mu2e-STMDAQ/utils/timer.hh"


class WriteManager : public OperationMap {

private:

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;
  
  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;

  // Event Builder
  const std::shared_ptr<EventBuilder> eb;
  
  // Output directory for file writing
  const std::filesystem::path output_dir;

  // Filename timestamp for start of run
  const std::string timestamp;
  
  // Wrapped file ofsteam and filepath
  struct FileHandle {
    std::shared_ptr<std::ofstream> stream;
    std::string filepath;
  };
  
  // Lock-free queue for pre-opened file pointers
  using FileQueue = boost::lockfree::spsc_queue<std::shared_ptr<FileHandle>>;

  // Total number of write stream enabled
  const int stream_num;
  
  // String IDs for the enabled write streams only
  std::vector<std::string> streamID;
  
  // Stats counters per stream (total bytes written)
  std::vector<std::atomic<size_t>> total_bytes;

  // Per stream data to write
  template <typename T>
  struct data_to_write {
    const std::vector<T>& vec;
    const size_t length;
    
    explicit data_to_write(const std::vector<T>& v)
      : vec(v), length(v.size()) {}
    data_to_write(const std::vector<T>& v, size_t len)
      : vec(v), length(len) {}
  };

  // A variant that can hold either type
  using data_variant = std::variant<data_to_write<int16_t>, data_to_write<double>>;
  
  // Threads: file managers for raw, zs, ph and a logger
  std::vector<std::thread> file_opener;
  std::thread stats_logger;
  
  // Per-stream context containing rotation and queue state
  struct Context {
    // The context stream ID
    std::string streamID;
    // File being written to
    std::shared_ptr<FileHandle> current_file;
    // File prepared for next switch
    std::shared_ptr<FileHandle> next_file;    
    // Queue of pre-opened files
    std::unique_ptr<FileQueue> file_queue;
    // Tracks how many files are currently in the queue.
    std::atomic<int> file_queue_size = 0;
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
  };

  // Stream-specific contexts
  std::vector<Context> ctx;
  //  Context events_ctx, raw_ctx, zs_ctx, ph_ctx;

  // Event to write for events data case
  std::vector<int16_t> event;
  
public:
  
  // Constructor
  WriteManager(const std::shared_ptr<AsyncLogger>& logger,
               const std::shared_ptr<STMdata>& stm_,
               const std::shared_ptr<SignalHandler>& signal_);
  
  // Destructor: 
  ~WriteManager() {
    // Safely shuts down all threads and close unused files
    shutdown();

    // Loop over write streams
    for (int i = 0; i < stream_num; i++){
      // Get total byte written
       uint64_t bytes = total_bytes[i].load();
       // Format total with units
       std::string size = format_size(bytes);
       logger->log("WriteManager total " + streamID[i] + " data written = " + size,1);
    }
    std::cout << "Write Manager destructor called.\n";
  }
  
  // External entry point to push one buffer for a specific stream
  void push(std::shared_ptr<DataStruct> buffer, const std::string& stream_type);

  // Write event data (all types) event by event
  size_t write_events(Context& ctx, std::shared_ptr<DataStruct> buffer);
  
  // Write full buffer of single data type
  size_t write_vector(const std::string& type, Context& ctx, std::shared_ptr<DataStruct> buffer);

  // Core binary file writing logic
  void write_bytes(Context& ctx, const void* data_ptr, size_t data_bytes);

  // Check if we need to switch to a new file
  void check_file(Context& ctx, std::shared_ptr<FileHandle>& file, size_t data_bytes);
  
  // File manager thread — opens, closes, and queues files
  void file_manager_thread(const int idx, Context& ctx);

  // Periodically logs disk space and write throughput
  void logger_thread();
    
  // Graceful shutdown
  void shutdown();

  // Cleanup routine for any unused files
  void cleanup_unused_files(Context& ctx);

  // // Get the context for a specific stream type
  std::pair<int,Context*> select_context(const std::string& type) {
    // Loop over streams
    for (int i = 0; i < stream_num; i++){
      if (type == streamID[i]) return {i,&ctx[i]};
    }
    logger->log("WriteManager::select_context: Error! Trying to select context of unknown type: " +
                type + ".",0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return {-1,nullptr};
  }
 
  // Extract the correct data vector from buffer
  const data_variant select_data(const std::string& type, const std::shared_ptr<DataStruct>& buffer) {
    if (type == "raw") return data_to_write<int16_t>(buffer->raw, buffer->raw_len);
    if (type == "zs")  return data_to_write<int16_t>(buffer->zs,  buffer->zs_len);
    return data_to_write<double>(buffer->ph, buffer->ph_len);
  } 
  
  // Create a file name using prefix, channel, type, timestamp, and subrun
  std::string generate_filename(const std::string& prefix,
				const std::string& type,
				const std::string& timestamp,
				int subrun) {
    std::ostringstream oss;
    oss << prefix << "_" << stm->ch_config.name << "_" << type << "_" << timestamp << "_subrun" << subrun << \
      ".bin";
    return oss.str();
  }
  
  // Format byte count as human-readable GB string
  std::string format_gb(uint64_t bytes){
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (bytes / 1e9) << " GB";
    return oss.str();
  }
  
  // Format byte count as human-readable order-based string
  std::string format_size(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = static_cast<double>(bytes);
    int unit_index = 0;
    while (size >= 1024.0 && unit_index < 4) {
      size /= 1024.0;
      unit_index++;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
    return oss.str();
  }

  // Format byte count as human-readable precision-set string
  std::string format_gbps(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
  }

  // Format percent to 1 decimal place
  std::string format_percent(double val) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << val;
    return ss.str();
  }
    
};

#endif 
