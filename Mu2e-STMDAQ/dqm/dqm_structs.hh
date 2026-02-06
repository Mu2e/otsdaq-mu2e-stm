#ifndef DQM_STRUCTS_HH
#define DQM_STRUCTS_HH

#include <cstdint>     // For fixed-width integer types like uint64_t
#include <atomic>      // For std::atomic used in generation counters

#pragma pack(push, 1)

// Struct for ADC Baseline monitoring with generation counters for lock-free reading
template <size_t noise_len>
struct dqm_data_baseline {  
  std::atomic<uint64_t> gen_start;       // Start generation counter
  uint64_t timestamp_ns = 0;             // Time the struct was written  
  double baseline_mean_prev;             // Mean baseline from previous run
  double baseline_sigma_prev;              // SIGMA from previous run
  double baseline_mean_avg;              // Average across runs or channels
  double baseline_sigma_avg;               // SIGMA of average baseline
  double baseline_mean_current;          // Current run mean
  double baseline_sigma_current;           // Current run SIGMA
  int16_t noise_data[noise_len];      // Noise data
  std::atomic<uint64_t> gen_end;         // End generation counter
};

// Struct for Physics monitoring (no generation counter used here for simplicity)
struct dqm_data_physics {
    uint64_t event_count;   // Total number of events processed
    double average_energy;  // Average energy of physics events
    double sigma_energy;      // sigma of energy distribution
};

// Struct for raw data monitoring with generation counters for lock-free reading
template <size_t raw_len>
struct dqm_data_raw {  
  std::atomic<uint64_t> gen_start;       // Start generation counter
  uint64_t timestamp_ns;                 // Time the struct was written    
  int16_t samples[raw_len];   // Raw ADC samples
  std::atomic<uint64_t> gen_end;         // End generation counter

};

#pragma pack(pop)

#endif // DQM_STRUCTS_HH
