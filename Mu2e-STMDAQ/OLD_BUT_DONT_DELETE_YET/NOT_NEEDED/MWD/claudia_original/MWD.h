#ifndef MWD_H
#define MWD_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair                                                                           
#include <stdexcept> // std::runtime_error                                                                
#include <sstream> // std::stringstream                                                                   
#include <fstream>
#include <boost/chrono.hpp>
#include <cmath>
#include <sys/stat.h>
#include <cstring>

#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

//#include "TFile.h"
//#include "TH1F.h"

#include "STMDAQ-TestBeam/MWD/claudia_original/data.hh"
#include "STMDAQ-TestBeam/MWD/claudia_original/peaks.hh"

class MWD {

 public:

  //Constructors
  MWD();    
  MWD(double _M, double _L, double _tau, double _nsigma_cut, double _thresholdgrad, double _fADC, int _cut_mode, double _fixed_cut);
  MWD(Xml* xml);

  //MWD methods
  void mwd_algorithm(data* adc_values);
  std::string print();
  std::vector<double> calculate_baseline();
  void write_l_to_binary_file(std::string filename);
  void write_adc_to_binary_file(std::string filename, data* adc_values);
  peaks* find_peaks(double baseline_mean, double baseline_rms, double time_offset); // time offset is in us.

  double* return_lvalue() { return l; }

 private:
  int M, L, cut_mode;
  double tau, nsigma_cut, thresholdgrad, fADC, fixed_cut_parameter;
  double threshold_cut;

  double* l; // output of MWD algorithm
  int nadc;
};

#endif
