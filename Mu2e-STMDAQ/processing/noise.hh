#ifndef NOISE_hh
#define NOISE_hh

#include <cmath>
#include <iomanip>
#include <algorithm> 

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Ring buffer code
#include "Mu2e-STMDAQ/buffers/buffer_queue.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

class Noise : public OperationMap {

private:

  // Store reference to the Config instance
  Config& cfg;

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Number of ADC values to ignore before peak
  size_t before_peak = 0;
  // Total number of ADC values ignored per peak
  size_t total_peak = 0;
  
  // Pulse decay time constant
  size_t decay_len = 0;

  // Ensure data length is zero
  uint64_t noise_len = 0;  

  // Average baseline statistics
  double avg_baseline = 0;
  double baseline_rms = 0;

  // Max noise length to store
  size_t max_noise_len = 0;
  
  // Noise data
  std::vector<int16_t> noise_data;
  
  // The peak end for baseline calculations
  int peak_end = -1;
  
  // The first found peak
  bool first_peak = true;
  
  // The runtime peak location
  uint64_t runtime_peak_loc = 0;
  
  // Previous peak location
  int64_t prev_peak_start = 0;
  
  // Max allowed ADC mean baseline
  int64_t max_allowed_baseline = 0;
  
  // Max allowed ADC baseline rms
  int64_t max_allowed_noise_rms = 0;
  
  // A function map for the process manager
  std::unordered_map<std::string, std::function<void(std::shared_ptr<DataStruct>&)>> functionMap;

public:

  // Constructor
  Noise(Config& cfg_,
       const std::shared_ptr<AsyncLogger>& logger_,
       const std::shared_ptr<STMdata>& stm_);
  
  // Destructor                                                          
  ~Noise() {
    // Log to user
    if (noise_len > 0){
      logger->log("Noise: ADC baseline = " + std::to_string(avg_baseline) +
		" ± " + std::to_string(baseline_rms) + ".",1);
      // Update baseline values in config file
      std::ostringstream mean_ss;
      mean_ss << std::fixed << std::setprecision(2) << avg_baseline;
      cfg.setValue("stm.zs.baseline.last_value.mean", mean_ss.str().c_str());
      std::ostringstream rms_ss;
      rms_ss << std::fixed << std::setprecision(2) << baseline_rms;
      cfg.setValue("stm.zs.baseline.last_value.rms", rms_ss.str().c_str());
      // Log to user
      logger->log("Noise: ADC baseline written to " +
		  cfg.getXMLpath() + ",.",1);
    }

    std::cout << "Noise destructor called.\n";
  }

  // Cut around ZS peaks to get noise data
  void get_noise_data(std::shared_ptr<DataStruct>& buffer);

  // Check noise mean and rms
  void check_noise(std::shared_ptr<DataStruct>& buffer);

  // Do an FFT noise
  void noise_fft(std::shared_ptr<DataStruct>& buffer);

  // Return max noise data length
  size_t get_max_noise_len() const {
    // Check variables have been set
    if (max_noise_len == 0) logger->log("ERROR in Noise: max_noise_len not set (= 0)!",0);
    return max_noise_len; 
  }

  
  // Execute function for OperationManager
  //void execute(const std::string& methodName, std::shared_ptr<DataStruct>& buffer) override {
    //if (functionMap.find(methodName) != functionMap.end()) {
    //  functionMap[methodName](buffer);
   // } else {
     // std::cerr << "Error: Invalid method name '" << methodName << "' in Noise\n";
    //}
  //}  

};

#endif
