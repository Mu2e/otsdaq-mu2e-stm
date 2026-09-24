#ifndef ZERO_SUPPRESS_hh
#define ZERO_SUPPRESS_hh

#include <cmath>
#include <iomanip>
#include <algorithm> 

class ZeroSuppress {

private:

  // ADC sampling frequency (MHz)
  double fADC = 0;  
  // ADC sampling time
  double tADC = 0;

  // Time to store before peak (us)
  double before_peak_time = 0;
  // Number of ADC values to store before peak
  size_t before_peak = 0;
  // Time to store after peak (us)
  double after_peak_time = 0;
  // Number of ADC values to store after peak
  size_t after_peak = 0;
  // Total number of ADC values stored per peak
  size_t total_peak = 0;
  
  // HPGe decay time constant (us)
  double hpge_decay_time = 0;
  size_t hpge_decay = 0;
  
  // Average baseline statistics
  double avg_baseline_sum = 0;
  double avg_baseline_sum2 = 0;
  uint64_t avg_baseline_count = 0;
  double avg_baseline_mean = 0;
  double avg_baseline_rms = 0;

  // Max noise length to store
  size_t max_noise_len = 0;
  
  // The peak end for baseline calculations
  int peak_end = -1;
  
  // Gradient threshold
  int threshold = 0;
  // Gradient window
  size_t window = 0;
  // Gradient average
  size_t n_average = 0;

  // Maximum amount of overfloww data to store from previous buffer
  size_t prev_data_max = 0;

  // Actual amount of overflow data to store from previous buffer
  size_t prev_num = 0;

  // The maximum number of events in the overflow data
  size_t overflow_event_max = 0;

  // The first found peak
  bool first_peak = true;
  
  // The overflow data from the previous buffer
  std::vector<int16_t> prev_data;

  // The data left to copy from the previous call
  uint64_t left_to_copy = 0;
  
  // The potential previous peak data from the previous buffer
  std::vector<int16_t> prev_peak_data;
  
  // The first buffer
  bool first_buffer = true;

  // Boolean to signal a peak has been found
  bool peak_found = false;

  // The runtime peak location
  uint64_t runtime_peak_loc = 0;
  
  // Previous peak location
  int64_t prev_peak_loc = 0;
  
  // Incrementing peak counter
  size_t peak_count = 0;
  
  // The total suppressed data length
  uint64_t tot_sup_len = 0;
  
public:

  // Constructor
  ZeroSuppress();
  
  // Destructor                                                          
  ~ZeroSuppress() {
    // Log to user
    logger->log("ZeroSuppress: Found " + std::to_string(peak_count) + " pulsess.",1);
    // Log to user
    logger->log("ZeroSuppress: Total suppressed data size = " +
		std::to_string(tot_sup_len*sizeof(int16_t)*1e-9) + " GB ("+
		std::to_string(tot_sup_len) + " ADCs).",1);
    // Log to user
    if (avg_baseline_count > 0){
      logger->log("ZeroSuppress: ADC baseline = " + std::to_string(avg_baseline_mean) +
		" ± " + std::to_string(avg_baseline_rms) + ".",1);
    }
    // Clear data
    if (!prev_data.empty()) {
      prev_data.clear();
    }
    std::cout << "ZeroSuppress destructor called.\n";
  }

  // Find peaks in data
  void find_peaks(std::shared_ptr<DataStruct>& buffer);

  // Suppress data
  void suppress_data(std::shared_ptr<DataStruct>& buffer);
 

};

#endif
