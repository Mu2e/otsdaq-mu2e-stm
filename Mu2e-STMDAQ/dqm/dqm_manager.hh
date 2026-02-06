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
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

// Forward declare Boost shared memory classes to reduce header dependencies
namespace boost {
  namespace interprocess {
    class shared_memory_object;
    class mapped_region;
  }
}

// Enum representing unique pages of the DQM
enum class DQMPageType {
  BASELINE,
  RAW,
  PHYSICS
};

// Base class for all shared memory blocks
class DQMBlockBase {  
public:
  // Get raw void* pointer to shared memory
  virtual void* get() = 0;
  // Virtual destructor
  virtual ~DQMBlockBase() = default;  
};

// Template class that manages shared memory for a data structure of type T
template <typename T>
class DQMBlock : public DQMBlockBase {
  
public:

  // Constructor that creates SHM block
  DQMBlock(const std::string& shm_name);
  // Return typed pointer to shared memory
  T* getTyped();
  // Raw pointer for base access
  void* get() override;
  // Destructor cleans up SHM
  ~DQMBlock();                            
  
private:

  // Shared memory name
  std::string shm_name_;
  // Size of the shared memory region
  size_t shm_size_;
  // Boost shm object
  std::unique_ptr<boost::interprocess::shared_memory_object> shm_;
  // Boost shm region
  std::unique_ptr<boost::interprocess::mapped_region> region_;
  
};

// Manager class that controls all shared memory blocks and updates them periodically
class DQM : public OperationMap {
  
public:

  // Constructor registers shared memory
  DQM(const Config& cfg_,
      const std::shared_ptr<AsyncLogger>& logger,
      const std::shared_ptr<STMdata>& stm_,
      const std::shared_ptr<SignalHandler>& signal_);
  
  // Destructor stops and joins background thread
  ~DQM(){
    std::cout << "DQM destructor called." << std::endl;

  };     

  // Registers a new SHM block
  template <typename T>
  void registerBlock(DQMPageType type, const std::string& shm_name);  

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

  // The reference baseline mean and sigma to be displayed
  const double baseline_mean_prev;
  const double baseline_sigma_prev;

  // Max noise length
  static constexpr size_t noise_len = 3000;

  // Raw data length
  static constexpr size_t raw_len = 300000;
  
  // Periodic writer loop for shared memory updates
  void update_dqm(std::shared_ptr<DataStruct>& buffer);   

  // Periodic writer loop for baseline updates
  void update_dqm_baseline(std::shared_ptr<DataStruct>& buffer);   

  // Periodic writer loop for raw adc data
  void update_dqm_raw(std::shared_ptr<DataStruct>& buffer);   

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
#include "dqm_manager.tpp"  

#endif  // DQM_MANAGER_HH
