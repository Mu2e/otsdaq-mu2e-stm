// Include DQM code
#include "Mu2e-STMDAQ/dqm/dqm.hh"

// Minimum required memory for struct
const size_t MIN_SHM_SIZE = 4096;  

// Constructor: Initializes shared memory for storing DQM data
DQM::DQM(const Config& cfg_,
	 const std::shared_ptr<AsyncLogger>& logger_,
	 const std::shared_ptr<SignalHandler>& signal_,
	 const int CHAN_) :
  cfg(cfg_), logger(logger_), signal(signal_),  CHAN(CHAN_)  {
  
  // try {

  //   // Read shared memory name and size from the Config class
  //   shm_name = cfg.getValue<std::string>("stm.DQM.name");
  //   shm_size = cfg.getValue<int>("stm.DQM.size");
    
  //   // Check if the size is too small
  //   if (shm_size < MIN_SHM_SIZE) {
  //     logger->log("Error: DQM dhared memory size too small! Increase size in config.xml",0);
  //   }
    
  //   logger->log("Loaded Config: Shared Memory Name = " + shm_name + ", Size = " + std::to_string(shm_size) + " bytes",1);

  //   // Remove shared memory if it exists
  //   try {
  //       if (shared_memory_object::remove(shm_name.c_str())) {
  //           std::cout << "Removed existing shared memory segment: " << shm_name << std::endl;
  //       }
  //   } catch (const std::exception& e) {
  //       std::cerr << "Error removing old shared memory: " << e.what() << std::endl;
  //   }
    
  //   // Open or create shared memory
  //   shm = shared_memory_object(open_or_create, shm_name.c_str(), read_write);
    
  //   // Resize it to the correct size
  //   shm.truncate(shm_size);
    
  //   // Map the shared memory region
  //   region = mapped_region(shm, read_write);
    
  //   // Zero initialize the shared memory
  //   std::memset(region.get_address(), 0, shm_size);
    
  //   // Initialize the shared memory data structure
  //   void* addr = region.get_address();
  //   DQMData* data = static_cast<DQMData*>(addr);
  //   std::memset(data, 0, sizeof(shm_size));
  //   data->threshold = 1000.0f;  // Default threshold for alerts
  //   data->paused = 0;  // Initialize pause flag (0 = running, 1 = paused)
    
  //   logger->log("Shared memory initialized successfully.",1);
    
  // }
  // catch (const std::exception& e) {
  //   logger->log("Error initializing shared memory.",0);
  //   throw;
  // }

  // Initialise function map for Operation Manager
  functionMap["dqm_thread"] = [this](std::shared_ptr<DataStruct>& buffer) {
    dqm_thread(buffer);
  };
  
}

// Thread to get DQM information from data buffer
void DQM::dqm_thread(std::shared_ptr<DataStruct>& buffer){

}


// // Function to update shared memory with latest buffer and processing statistics
// void DQM::updateBufferData(uint64_t bytesProcessed, 
//                                        const std::vector<uint64_t>& threadBytes, 
//                                        const std::vector<std::vector<int16_t>>& bufferData) {

//   try {

//     // Get pointer to shared memory data structure
//     void* addr = region.get_address();
//     DQMData* data = static_cast<DQMData*>(addr);
    
//     // Update total bytes processed across all worker threads
//     data->totalBytesProcessed = bytesProcessed;

//     // Update number of active worker threads
//     data->activeThreads = threadBytes.size();

//     // Store per-thread byte processing data
//     for (size_t i = 0; i < threadBytes.size(); ++i) {
//       data->threadBytesProcessed[i] = threadBytes[i];
//     }

//     // Store buffer data and calculate averages
//     data->bufferCount = bufferData.size();

//     // Loop over buffer data
//     for (size_t i = 0; i < bufferData.size(); ++i) {
//       // Determine how many data points to store (up to BUFFER_MAX_ENTRIES)
//       size_t sampleCount = std::min(bufferData[i].size(), (size_t)BUFFER_MAX_ENTRIES);
//       data->bufferSizes[i] = sampleCount;
      
//       // Copy buffer data into shared memory
//       std::memcpy(data->bufferData[i], bufferData[i].data(), sampleCount * sizeof(int16_t));

//       // Compute the average value for the buffer
//       int64_t sum = 0.0f;
//       for (size_t j = 0; j < sampleCount; ++j) {
// 	sum += bufferData[i][j];
//       }      
//       data->bufferAverages[i] = (sampleCount > 0) ? static_cast<float>(sum) / sampleCount : 0.0f;
//     }

//     data->baseline_mean_prev = prev_mean;
//     data->baseline_rms_prev = prev_rms;
//     data->baseline_mean_current = curr_mean;
//     data->baseline_rms_current = curr_rms;

//     data->baseline_inst_count = times_ns.size();
//     for (int i = 0; i < times_ns.size(); ++i) {
//         data->baseline_inst_time[i] = times_ns[i];
//         data->baseline_inst[i] = inst_means[i];
//         data->baseline_inst_rms[i] = inst_rms[i];
//     }
    
// }

    
//   }
//   catch (const std::exception& e) {
//     std::cerr << "Error updating shared memory: " << e.what() << std::endl;
//   }
//}

