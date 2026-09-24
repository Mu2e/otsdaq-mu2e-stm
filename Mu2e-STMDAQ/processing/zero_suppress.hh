#ifndef ZERO_SUPPRESS_hh
#define ZERO_SUPPRESS_hh

#include <cmath>
#include <iomanip>
#include <algorithm> 

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

class ZeroSuppress : public OperationMap {

private:

  // Store reference to the Config instance
  Config& cfg;
  
  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;

  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;
  
  // Number of ADC values to store before peak
  const size_t before_peak;
  // Number of ADC values to store after peak
  const size_t after_peak;
  // Total number of ADC values stored per peak
  const size_t total_peak;

  // Gradient threshold
  const int threshold;
  // Gradient window
  const size_t window;
  // Gradient average
  const size_t n_average;
  
  // The first found peak
  bool first_peak = true;
    
  // Actual amount of overflow data to store from previous buffer
  size_t prev_num = 0;

  // The overflow data from the previous buffer
  std::vector<int16_t> prev_data;

  // The next buffers peak data 
  std::vector<zs_region> next_peak_data;
  
  // The first buffer
  bool first_buffer = true;

  // Boolean to signal a peak has been found
  bool peak_found = false;

  // Incrementing peak counter
  size_t peak_count = 0;
  
  // The total suppressed data length
  uint64_t tot_sup_len = 0;

public:

  // Constructor
  ZeroSuppress(Config& cfg_,
               const std::shared_ptr<AsyncLogger>& logger_,
               const std::shared_ptr<STMdata>& stm_,
               const std::shared_ptr<SignalHandler>& signal_);
  
  // Destructor                                                          
  ~ZeroSuppress() {
    // Log to user
    logger->log("ZeroSuppress: Found " + std::to_string(peak_count) +
     		" pulses.",1);
    // Log to user
    logger->log("ZeroSuppress: Total suppressed data size = " +
		std::to_string(tot_sup_len*sizeof(int16_t)*1e-9) + " GB ("+
		std::to_string(tot_sup_len) + " ADCs).",1);
    if (!prev_data.empty()) {
      prev_data.clear();
    }
    std::cout << "ZeroSuppress destructor called.\n";
  }

  // Find peaks in data
  void find_peaks(std::shared_ptr<DataStruct>& buffer,
                  std::shared_ptr<DataStruct>& prev);

  // Suppress data
  void suppress_data(std::shared_ptr<DataStruct>& buffer);
  
};

#endif
