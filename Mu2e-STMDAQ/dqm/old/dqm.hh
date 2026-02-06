#ifndef DQM_HH
#define DQM_HH

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <vector>
#include <cstring>
#include <iostream>
#include <memory>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Signal handler header                                                     
#include "Mu2e-STMDAQ/utils/signal_handler.hh"  
// Ring buffer code
#include "Mu2e-STMDAQ/utils/ring_buffer.hh"
// Operations base header
#include "Mu2e-STMDAQ/utils/operations_base.hh"

using namespace boost::interprocess;

// const int MAX_BUFFERS = 10;  // Maximum buffers tracked
// const int BUFFER_MAX_ENTRIES = 100;  // Max entries stored per buffer
// const int MAX_THREADS = 16;  // Maximum number of worker threads

// Structure to hold DQM data in shared memory
// struct DQMData {
//   uint64_t totalBytesProcessed;  // Total bytes processed across all threads
//   size_t activeThreads;  // Number of active worker threads
//   uint64_t threadBytesProcessed[MAX_THREADS];  // Bytes processed per thread
//   int16_t bufferData[MAX_BUFFERS][BUFFER_MAX_ENTRIES];  // Raw buffer data
//   float bufferAverages[MAX_BUFFERS];  // Average of buffer values
//   int bufferSizes[MAX_BUFFERS];  // Number of entries per buffer
//   int bufferCount;  // Total number of buffers
//   float threshold;  // Threshold for alerts
//   int paused;  // Pause flag for stopping monitoring


//   double baseline_mean_prev;     // previous run mean
//   double baseline_rms_prev;      // previous run rms
//   double baseline_mean_current;  // current run mean
//   double baseline_rms_current;   // current run rms
//   double baseline_inst[MAX_BUFFERS];   // instantaneous values per buffer
//   double baseline_inst_rms[MAX_BUFFERS]; // instantaneous rms
//   uint64_t baseline_inst_time[MAX_BUFFERS]; // timestamps for instantaneous values
//   int baseline_inst_count;       // number of instantaneous points stored
  
// };

// Class for managing shared memory communication
class DQM : public OperationBase {
  
private:
  
  // Store reference to the Config instance
  const Config& cfg;     
  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;
  // Shared SignalHandler
  const std::shared_ptr<SignalHandler>& signal;  

  // The channel number  
  const int CHAN;
  
  // Shared memory object
  boost::interprocess::shared_memory_object shm;
  // Maps shared memory to process address space
  boost::interprocess::mapped_region region;
  // Shared memory name
  std::string shm_name;
  // Shared memory size
  size_t shm_size;
  // Track whether this instance created the shared memory  
  bool isOwner;  

  // A function map for the operation manager
  std::unordered_map<std::string, std::function<void(std::shared_ptr<DataStruct>&)>> functionMap;
  
public:

  // Constructor
  DQM(const Config& cfg_,
      const std::shared_ptr<AsyncLogger>& logger_,
      const std::shared_ptr<SignalHandler>& signal_,
      const int CHAN_);  

  // Destructor: Cleans up shared memory when the program exits
  ~DQM() {
    // try {
    //   // Loger to user
    //   std::cout << "Cleaning up shared memory: " << shm_name << std::endl;      
    //   // Remove shared memory if it exists
    //   shared_memory_object::remove(shm_name.c_str());      
    //   std::cout << "Shared memory removed successfully.\n";
    // } catch (const std::exception& e) {
    //   std::cerr << "Error removing shared memory: " << e.what() << std::endl;
    // }
    std::cout << "DQM destructor called.\n"; 
  }  

  // Thread to get DQM data from buffer
  void dqm_thread(std::shared_ptr<DataStruct>& buffer);
  
  // Function to update shared memory with buffer and processing statistics
  // void updateBufferData(uint64_t bytesProcessed,
  // 			const std::vector<uint64_t>& threadBytes,
  // 			const std::vector<std::vector<int16_t>>& bufferData);
  
  // Execute function for the operation manager
  void execute(const std::string& methodName, std::shared_ptr<DataStruct>& buffer) override {
    if (functionMap.find(methodName) != functionMap.end()) {
      functionMap[methodName](buffer);
    } else {
      std::cerr << "Error: Invalid method name '" << methodName << "' in DQM\n";
    }
  }
    
};

#endif // DQM_SHM_HH
