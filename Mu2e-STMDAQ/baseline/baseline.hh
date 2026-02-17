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

// Baseline class
class Baseline {

private:

  // Histogram variables
  int nbins; // Number of histogram bins
  double min_adc; // Minimum adc value
  double max_adc; // Maximum adc value
  double width; // Bin width
  double invw; // Inverse bin width
  std::vector<uint64_t> hist_counts; // Histogram bin counts
  std::vector<double> hist_bin_centres; // Histogram bin centres
  double total_hist_counts = 0; // Total histogram counts
  
  // Expectation-Maximisation Variables
  int max_iters; // Maximum EM iterations
  double tol; // Convergence tolerance for log-likelihood change
  double var_floor; // Minimum allowed variance 

  // // Get the index of the histogram mode.
  // int get_hist_mode_idx(const std::vector<double>& hist) {
  //   return int(std::max_element(hist.begin(), hist.end()) - hist.begin());
  // }

  // Get the index of the histogram mode.
  int get_hist_mode_idx(const std::vector<uint64_t>& hist) {
    return int(std::distance(hist.begin(),std::max_element(hist.begin(), hist.end())));
  }

  
  // Gaussian probability density function (normalised)
  double gaussian_pdf(double x, double mu, double var) {
    const double inv = 1.0 / std::sqrt(2.0 * M_PI * var);
    const double z2  = (x - mu) * (x - mu) / (2.0 * var);
    return inv * std::exp(-z2);
  }
  
public:

  // Constructor
  Baseline(const std::string& config_filename);
  
  // Destructor                                                          
  ~Baseline() {
    //    for (int i = 0; i < nbins; i++) std::cout << i << " " << hist_bin_centres[i] << " " << hist_counts[i] << std::endl;
    std::cout << "Baseline destructor called.\n";
  }

  // Configure baseline
  void configure_baseline(const std::string& filename);
  
  // Fill histogram of ADC data 
  std::pair<const std::vector<uint64_t>&, const std::vector<double>&>
  fill_hist(const std::vector<int16_t>& ADC);

  // Expectation–Maximization (EM) 
  std::pair<std::tuple<double,double,double>,std::tuple<double,double,double>> EM_algorithm();
  
  // Estimate percentile position from histogram 
  double hist_percentile(double p01);
  
};

#endif


