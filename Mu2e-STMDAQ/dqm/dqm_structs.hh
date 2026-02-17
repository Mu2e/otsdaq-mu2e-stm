#ifndef DQM_STRUCTS_HH
#define DQM_STRUCTS_HH

#include <cstdint>     // For fixed-width integer types like uint64_t
#include <atomic>      // For std::atomic used in generation counters

#pragma pack(push, 1)

// Struct for ADC Baseline monitoring with generation counters for lock-free reading
struct dqm_data_baseline {  
  std::atomic<uint64_t> gen_start;       // Start generation counter
  uint64_t timestamp_ns;             // Time the struct was written  
  double baseline_mean_prev;             // Mean baseline from previous run
  double baseline_sigma_prev;              // SIGMA from previous run
  double baseline_mean_avg;              // Average across runs or channels
  double baseline_sigma_avg;               // SIGMA of average baseline
  double baseline_mean_current;          // Current run mean
  double baseline_sigma_current;           // Current run SIGMA
  double baseline_fit_mean_all;           // Fit mean all time
  double baseline_fit_sigma_all;           // Fit width all time
  double baseline_fit_mean_wind;           // Fit mean all time
  double baseline_fit_sigma_wind;           // Fit width all time
  size_t baseline_bins;
  size_t noise_len;
  uint8_t baseline_data[1]; // Baseline hist, noise data and end generation counter
};

// Struct for Physics monitoring (no generation counter used here for simplicity)
struct dqm_data_physics {
    uint64_t event_count;   // Total number of events processed
    double average_energy;  // Average energy of physics events
    double sigma_energy;      // sigma of energy distribution
};

// Struct for raw data monitoring with generation counters for lock-free reading
struct dqm_data_raw {  
  std::atomic<uint64_t> gen_start;       // Start generation counter
  uint64_t timestamp_ns;                 // Time the struct was written    
  size_t raw_len;
  uint8_t raw_data[1];   // Raw ADC samples and end generation counter
};

// Struct for peak histogramming with generation counters for lock-free reading
struct dqm_data_peak {  
  std::atomic<uint64_t> gen_start;       // Start generation counter
  uint64_t timestamp_ns;                 // Time the struct was written    
  size_t peak_nbins;
  uint64_t peak_data[1];   		 // Peak samples and generation counter
};


// Struct for operations name and CPU performance
struct op_entry {
  char name[32];    // Fixed 32-byte buffer for the name
  double speed;     // 8-byte double
};

struct dqm_data_daq {  
  std::atomic<uint64_t> gen_start;       // Start generation counter
  uint64_t timestamp_ns;                 // Time the struct was written    
  size_t num_dropped_packets;
  double adc_temp;
  size_t num_ops;
  op_entry ops_data[1];   		 // CPU speed and generation counter
};

struct alarm_entry {
  char text[128];
  size_t level;
  uint64_t time_ns;
};

struct dqm_data_alarm {
  std::atomic<uint64_t> gen_start;       // Start generation counter
  uint64_t timestamp_ns;                 // Time the struct was written    
  size_t max_alarms;
  size_t num_alarms;
  size_t write_idx;
  alarm_entry alarms[1];
};

#pragma pack(pop)

#endif // DQM_STRUCTS_HH
