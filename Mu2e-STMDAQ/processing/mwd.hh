#ifndef MWD_hh
#define MWD_hh

#include <cmath>
#include <iomanip>
#include <algorithm> 

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

class MWD : public OperationMap {

private:

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;

  // Baseline subtraction
  const bool use_manual_baseline; // Use manual value?
  const double manual_baseline_mean, manual_baseline_sigma; // Manual (user config) mean and sigma 
  const bool use_window_baseline; // If not manual, use dynamic sliding window baseline?
  double baseline = 0.0; // The baseline value to be used
  
  // Deconvolution  
  const double tADC; // The ADC sampling period
  const double tau, tau_norm; // HPGe decay time constant (us)
  double prev_sample = 0.0; // Prev ADC sample
  double prev_a = 0.0; // Prev deconv value
    
  //  Differentiation
  const int M; // M value (must be power of 2)
  const size_t m_mask; // M-1    
  std::vector<double> a_ring; // Ring buffer for a[i] (deconvolution) values
  uint64_t diff_index = 0; // Deconvolution index
  
  // Averaging
  const int L; // M value (must be power of 2)
  const double inv_L; // 1/L
  const size_t l_mask; // L-1
  std::vector<double> D_ring; // Ring buffer for D[i] (differentiation) values
  double sum = 0.0; // Averaging sum
  uint64_t avg_index = 0; // Averaging index
  
  // Peak finding
  const double peak_init; // The initial minimum value for the peak
  EWT_info* peak_EWT = nullptr; // EWT of peak candidate minimum
  double peak_height; // The current minimum value for the peak
  double peak_time; // Time of peak candidate minimum
  int peak_zs_start; // The start of the peak zs window
  size_t peak_zs_len; // The start of the peak zs window
  const bool use_fixed_cut; // Use fixed peak finding threshold cut?                              
  const double fixed_cut_value; // Fixed cut value
  const double nsigma_cut; // Dynamic cut: number of sigma below baseline     
  double prev_avg = 0.0; // Prev averaged value
  uint64_t pf_index = 0; // Peak finding index   
  
  // Incrementing peak counter (all run)
  size_t peak_count_all = 0;
  
  // The total mwd data length                                 
  uint64_t tot_mwd_data_len = 0;
  
public:

  // Constructor
  MWD(const std::shared_ptr<AsyncLogger>& logger_,
      const std::shared_ptr<STMdata>& stm_);
  
  // Destructor                                                          
  ~MWD() {
    // Log to user
    logger->log("MWD: Found " + std::to_string(peak_count_all) + " peaks.  Total pulse height data size = " +
                std::to_string(tot_mwd_data_len*sizeof(int16_t)*1e-9) + " GB ("+
                std::to_string(tot_mwd_data_len) + " ADCs).",1);
    std::cout << "MWD destructor called.\n";
  }

  // Subtract baseline
  void subtract_baseline(std::shared_ptr<DataStruct>& buffer);
  
  // Deconvolution
  void deconvolution(std::shared_ptr<DataStruct>& buffer);

  // Differentiation
  void differentiation(std::shared_ptr<DataStruct>& buffer);

  // Averaging                                            
  void averaging(std::shared_ptr<DataStruct>& buffer);

  // Find peaks in data
  void find_peaks(std::shared_ptr<DataStruct>& buffer);
  
};

#endif
