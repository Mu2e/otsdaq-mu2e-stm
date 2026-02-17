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

// MWD peak struct
struct MWD_peaks {
  int npeaks = 0;
  std::vector<double> peak_heights;
  std::vector<double> peak_times;
};

// MWD class
class MWD {

 public:

  // Constructors
  MWD();    
  MWD(double _M, 
      double _L, 
      double _tau, 
      double _nsigma_cut, 
      double _thresholdgrad, 
      double _fADC, 
      int _cut_mode, 
      double _fixed_cut);
  
  // MWD algorithm
  double* mwd_algorithm_original(uint64_t n, int16_t* data);
  void mwd_algorithm(uint64_t n, int16_t* data, double* l);

  // Find peaks in data
  MWD_peaks find_peaks(uint64_t n, double* l,
		       double baseline_mean, 
		       double baseline_rms, 
		       double time_offset); // time offset is in us.
  
  // Calcuate adc baseline
  std::vector<double> calculate_baseline(uint64_t n, double* l);
    
 private:

  int M, L, cut_mode;
  double tau, nsigma_cut, thresholdgrad, fADC, fixed_cut_parameter;
  double threshold_cut;
  double T0;

};

#endif
