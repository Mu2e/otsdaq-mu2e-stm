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
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <sys/stat.h>
#include <cstring>

#include "queue_zs.hh"
#include "dataVars.hh"

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

  // First deconvolution call
  bool first_deconv[CHNUM] = {true,true};

  // The last deconvolution data point from the previous call
  int16_t last_deconv_data[CHNUM];

  // The last deconvolution value from the previous call
  double last_a[CHNUM];

  // First differentition call
  bool first_diff[CHNUM] = {true,true};

  // The last M values from the previous call
  double* prev_M[CHNUM];

  // First averaging call
  bool first_avg[CHNUM] = {true,true};

  // The average sum
  double avg_sum[CHNUM] = {};

  // The last L values from the previous call
  double* prev_L[CHNUM];

  // MWD deconvolution thread
  void deconv_thread(int chan, queue_zs<int16_t> &pullq, queue_zs<double> &pushq, bool *timeout);

  // MWD differentiation thread
  void diff_thread(int chan, queue_zs<double> &pullq, queue_zs<double> &pushq, bool *timeout);

  // MWD averaging thread
  void avg_thread(int chan, queue_zs<double> &pullq, queue_zs<double> &pushq, bool *timeout);
  

  // Deconvlution algorithm
  data_struct<double> deconvolute(int chan, data_struct<int16_t> &data);

  // Differentiation algorithm
  void differentiate(int chan, data_struct<double> &data);

  // Averaging algorithm
  void average(int chan, data_struct<double> &data);

  // Calcuate adc baseline
  std::pair<double,double> calculate_baseline(uint64_t n, double* l);

  // Find peaks in data
  MWD_peaks find_peaks(uint64_t n, double* l,
		       double baseline_mean, 
		       double baseline_rms,
		       double time_offset); // time offset is in us.
  

  // Original MWD algorithm
  double* mwd_algorithm_original(uint64_t n, int16_t* data);
    

 private:

  int M, L, cut_mode;
  double tau, nsigma_cut, thresholdgrad, fADC, fixed_cut_parameter;
  double threshold_cut;
  double T0;

};

#endif
