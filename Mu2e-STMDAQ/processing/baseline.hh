#ifndef BASELINE_hh
#define BASELINE_hh

#include <algorithm>     // std::max_element
#include <cmath>         // std::sqrt
#include <cstddef>       // size_t
#include <functional>    // std::function
#include <iostream>      // std::cout, std::cerr
#include <memory>        // std::shared_ptr
#include <string>        // std::string
#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector
#include <fstream>       // for std::ifstream                                                     
#include <sstream>       // for std::istringstream                                                
#include <iomanip>
#include <cstdint>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

// Baseline class
class Baseline : public OperationMap {

private:

  // Store reference to the Config instance
  Config& cfg;
  
  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Histogram variables 
  const int nbins; // Number of histogram bins (affects speed/resolution)
  const double min_adc; // Minimum adc value
  const double max_adc; // Maximum adc value
  const double width; // Bin width
  const double invw; // Inverse bin width

  // The number of buffers in the sliding window histogram
  const size_t window_size_buffers;
  // Sliding window histogram
  std::vector<uint64_t> hist_counts_window;
  // All data histogram
  std::vector<uint64_t> hist_counts_all;
  // Histogram bin centres
  const std::vector<double> hist_bin_centres; 
  // Total histogram counts  
  uint64_t total_window = 0; // Window histogram
  uint64_t total_all = 0; // All

  // Buffer histrogram variables
  std::vector<std::vector<uint64_t>> per_buffer_hists;
  std::size_t window_index = 0;

  // Buffer counter
  uint64_t buffer_count = 0;
  
  // Expectation-Maximisation Variables
  const int max_iters; // Maximum EM iterations
  const double tol; // Convergence tolerance for log-likelihood change
  const double var_floor; // Minimum allowed variance (prevents numerical collapse)

  // Baseline values for run
  double mu0_all = -9999, sigma0_all = -9999;
   
  // Get the index of the histogram mode.
  int get_hist_mode_idx(const std::vector<uint64_t>& hist) {
    return int(std::distance(hist.begin(),std::max_element(hist.begin(), hist.end())));
  }
  
  // Gaussian probability density function (normalised
  double gaussian_pdf(double x, double mu, double var) {
    const double inv = 1.0 / std::sqrt(2.0 * M_PI * var);
    const double z2  = (x - mu) * (x - mu) / (2.0 * var);
    return inv * std::exp(-z2);
  }
    
public:
  
  // Constructor
  Baseline(Config& cfg_,
           const std::shared_ptr<AsyncLogger>& logger_,
           const std::shared_ptr<STMdata>& stm_);
  
  // Destructor
  ~Baseline() {
    if ((mu0_all != -9999) or (sigma0_all != -9999)){
      // Update baseline values in config file
      std::ostringstream mean_ss;
      mean_ss << std::fixed << std::setprecision(2) << mu0_all;
      cfg.setValue("stm.baseline.last_value.mean", mean_ss.str().c_str());
      std::ostringstream sigma_ss;
      sigma_ss << std::fixed << std::setprecision(2) << sigma0_all;
      cfg.setValue("stm.baseline.last_value.sigma", sigma_ss.str().c_str());
      // Log to user
      logger->log("Baseline: ADC baseline for run determined to be " + std::to_string(mu0_all) +
                  " ± " + std::to_string(sigma0_all) + ". Value written to" +
                  cfg.getXMLpath() + ",.",1);
    }
    std::cout << "Baseline destructor called.\n";
  }
  

  // Calculate baselne by fitting histogrammed ADC data
  void calc_baseline(std::shared_ptr<DataStruct>& buffer);

  // Expectation–Maximization (EM) Algorithm
  // Gaussian (baseline) + one-sided exponential tail (pulses)
  baseline_fit EM_algorithm(const std::vector<uint64_t>& hist_counts,
                            const uint64_t total_hist_counts);

  // Estimate percentile position from histogram 
  double hist_percentile(const std::vector<uint64_t>& hist_counts,
                         const uint64_t total_hist_counts,
                         const double p01);

};

#endif
