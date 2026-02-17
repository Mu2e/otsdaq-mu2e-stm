#ifndef DQM_MANAGER_HH
#define DQM_MANAGER_HH

#include <string>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// Data structs header                                                                             
#include "Mu2e-STMDAQ/buffers/data_struct.hh"
// Shared memory helpers
#include "Mu2e-STMDAQ/dqm/dqm_shm_block.hh"
#include "Mu2e-STMDAQ/dqm/shm_manager.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

// Forward declare Boost shared memory classes to reduce header dependencies
namespace boost {
  namespace interprocess {
    class shared_memory_object;
    class mapped_region;
  }
}

// Forward declare Op manager so can access number of operations
class OperationManager;


// Manager class that controls all shared memory blocks and updates them periodically
class DQM : public OperationMap {
  
public:

  // Constructor registers shared memory
  DQM(const Config& cfg_,
      const std::shared_ptr<AsyncLogger>& logger,
      const std::shared_ptr<STMdata>& stm_,
      const std::shared_ptr<SignalHandler>& signal_,
      OperationManager* op_man_);
 
  // Destructor stops and joins background thread
  ~DQM(){
    std::cout << "DQM destructor called." << std::endl;

  };     

  // Need operations number after init 
  void init_shm(); 

  // Registers a new SHM block
  template <typename T>
  void registerBlock(DQMPageType type, const std::string& shm_name, size_t size);  

  // Gets typed pointer to SHM block
  template <typename T>
  T* get(DQMPageType type);  
    
private:

  // Store reference to the Config instnace
  const Config& cfg;
  
  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Shared SignalHandler
  const std::shared_ptr<SignalHandler>& signal;
  
  // Operation manager pointer
  OperationManager* op_man;

  // The reference baseline mean and sigma to be displayed
  const double baseline_mean_prev;
  const double baseline_sigma_prev;

  // Histogram bins in baseline
  size_t baseline_nbins;

  // Max noise length
  size_t noise_len;

  // Raw data length
  size_t raw_len;
  
  // Peak hist param
  size_t peak_nbins;
  size_t min_ph;
  float inv_wid;

  // Sliding window histogram
  std::vector<uint64_t> peak_hist_window;
  // All data histogram
  std::vector<uint64_t> peak_hist_all;

  // The number of buffers in the sliding window histogram
  const size_t window_size_buffers;
  // Total histogram counts
  uint64_t total_window = 0; // Window histogram
  uint64_t total_all = 0; // All

  // Buffer histogram variables
  std::vector<std::vector<uint64_t>> per_buffer_hists;
  std::size_t window_index = 0;
  
  // Running counter of dropped packets
  size_t num_dropped_packets;
  
  // Number of operations
  size_t op_num;

  // Write alarms to shared memory
  void alarm_info(std::shared_ptr<DataStruct>& buffer);   

  // Histogram peak data for DQM
  void histogram_peaks(std::shared_ptr<DataStruct>& buffer);   

  // Periodic writer loop for shared memory updates
  void update_dqm(std::shared_ptr<DataStruct>& buffer);   

  // Periodic writer loop for baseline updates
  void update_dqm_baseline(std::shared_ptr<DataStruct>& buffer);   

  // Periodic writer loop for raw adc data
  void update_dqm_raw(std::shared_ptr<DataStruct>& buffer);   

  // Periodic writer loop for peak data
  void update_dqm_peak(std::shared_ptr<DataStruct>& buffer);   

  // Update CPU performace DQM
  void update_cpu_performance(std::shared_ptr<DataStruct>& buffer);
  
  // Get the current time in ns
  uint64_t get_current_time_ns();
  
  // SHM block map
  std::unordered_map<DQMPageType, std::unique_ptr<DQMBlockBase>> blocks_;
  // Background thread handle
  std::thread updater_;
  // Flag for thread control
  std::atomic<bool> running_;

};


// Include template implementation
#include "Mu2e-STMDAQ/dqm/dqm_manager.tpp"  

#endif  // DQM_MANAGER_HH
