#ifndef MWD_HH
#define MWD_HH

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <cmath>
#include <sys/stat.h>
#include <cstring>

// Include multi file handler code
#include "multi_file_handler.hh"

// MWD peak struct
struct mwd_peaks {
  int npeaks = 0;
  std::vector<double> peak_heights;
  std::vector<double> peak_times;
};

// MWD class
class MWD {

 public:

  // Constructors
  MWD(const std::string& config_filename);

  //  Load config file and configure MWD algorithm  
  void configure_mwd(const std::string& filename);
  
  // MWD algorithm
  mwd_peaks calc_mwd(MultiFileHandler& provider);

  // Deconvolution
  template <typename T>
  void deconv_and_diff(const std::vector<int16_t>& samples,
		       std::vector<T>& D);

  // Averaging
  template <typename T_D, typename T_avg>
  void averaging(const std::vector<T_D>& D,
		 std::vector<T_avg>& avgs);
  
  // Find peaks
  template <typename T>
  void find_peaks(const std::vector<T>& avgs, mwd_peaks& peak_data);

  // The MWD algorithm
  template <typename T>
  void mwd_algorithm(const std::vector<int16_t>& samples,
		     std::vector<T>& avgs);

  // Do everything
  void do_all(const std::vector<int16_t>& samples,
	      mwd_peaks& peak_data);
  
  // Calcuate adc baseline
  std::vector<double> calculate_baseline(uint64_t n, double* l);
    
 private:

  /// Main MWD parameters
  int M, L;
  double inv_L;
  size_t m_mask, l_mask;
  
  // Ring buffer for a[i] (deconvolution) values
  std::vector<double> a_ring;
  // Ring buffer for D[i] (differentiation) values
  std::vector<double> D_ring;
  
  // Peak parameters
  double tau, tau_norm;

  // ADC parameters
  double fADC, tADC;
  double inv_fADC;
  
  // Threshold cut mode: 0 = dynamic, 1 = fixed
  bool use_fixed_cut;
  double threshold_cut; // The threshold cut value

  //  Parameters for dynamic cut mode
  double baseline_mean, baseline_rms, nsigma_cut, thresholdgrad;

  // Parameters for fixed cut mode
  double fixed_cut_value;

  // auxlow, minimum value of the peak
  double auxlow_init;
  
  // Function to round a value if of integer type
  template<typename T>
  inline T round_if_integral(double val) {
    if constexpr (std::is_integral<T>::value) {
      return static_cast<T>(val + (val >= 0 ? 0.5 : -0.5));
    } else {
      return val;
    }
  }
  
};

#endif
