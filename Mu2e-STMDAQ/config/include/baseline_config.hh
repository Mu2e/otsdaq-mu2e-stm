#ifndef BASELINE_CONFIG_HH_
#define BASELINE_CONFIG_HH_

#include <algorithm>
#include <cstdint>

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/fw_config.hh"
#include "Mu2e-STMDAQ/config/include/buffer_config.hh"

// Baseline configurable variables
struct baseline_info{

  // Histogram
  const size_t hist_bin_num; // Number of histogram bins
  const int16_t hist_min_adc; // Minimum adc value
  const int16_t hist_max_adc; // Maximum adc value
  const double hist_bin_width; // Histogram bin width
  const double hist_inv_bin_width; // Histogram inverse bin width
  const std::vector<double> hist_bin_centres; // Histogram bin centres

  // Sliding window histogram
  const double hist_window_period; // The period in time for a hist window
  const size_t hist_window_buffers; // Num of hist window buffers
  const bool window_delay; // Delay DAQ to calculate baseline?
  
  // Expectation-Maximization variables
  const size_t EM_max_iters; // Maximum EM iterations   
  const double EM_LL_tol; // Convergence tolerance for log-likelihood change
  const double EM_var_floor; // Minimum allowed variance (prevents numerical collapse)
    
  // Baseline mean and sigma from previous run
  const double prev_mean, prev_sigma;

  // Constructor
  baseline_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger,
                fw_info fw_config, buffer_info buffer_config, 
                double max_adc_in_buffer) :
    hist_bin_num(cfg.getValue<int>("stm.baseline.hist.bin_num")), // Number of bins
    hist_min_adc(std::clamp(cfg.getValue<int>("stm.baseline.hist.min_adc"), // Minimum adc value
                            static_cast<int>(std::numeric_limits<int16_t>::min()),
                            static_cast<int>(std::numeric_limits<int16_t>::max()))),
    hist_max_adc(std::clamp(cfg.getValue<int>("stm.baseline.hist.max_adc"), // Maximum adc value
                            static_cast<int>(std::numeric_limits<int16_t>::min()),
                            static_cast<int>(std::numeric_limits<int16_t>::max()))),
    hist_bin_width(((double)hist_max_adc - (double)hist_min_adc)/(double)hist_bin_num), // Bin width
    hist_inv_bin_width(1/hist_bin_width), // Inverse bin width
    hist_bin_centres([&] { // Calculate histogram bin centres
      std::vector<double> centres(hist_bin_num);
      for (std::size_t i = 0; i < hist_bin_num; ++i) {
        centres[i] =
          static_cast<double>(hist_min_adc) +
          (static_cast<double>(i) + 0.5) * hist_bin_width;
      }
      return centres;
    }()), 
    hist_window_period(cfg.getValue<double>("stm.baseline.hist.window_period")), // The period in time for a hist window
    hist_window_buffers(std::ceil(hist_window_period*1e6/(double)(max_adc_in_buffer*buffer_config.max_packet_num*fw_config.tADC))),
    window_delay(cfg.getValue<int>("stm.baseline.window_delay")), // Delay DAQ for baseline calc
    EM_max_iters(cfg.getValue<int>("stm.baseline.EM.max_iters")), // Maximum iterations   
    EM_LL_tol(cfg.getValue<double>("stm.baseline.EM.LL_tol")), // Convergence tolerance 
    EM_var_floor(cfg.getValue<double>("stm.baseline.EM.var_floor")), // Minimum  variance
    prev_mean(cfg.getValue<double>("stm.baseline.last_value.mean")),
    prev_sigma(cfg.getValue<double>("stm.baseline.last_value.sigma"))
  {

    // Notify user
    if (logger){
      logger->log("Config:baseline_info: Number of histogram bins = " +
                  std::to_string(hist_bin_num) +
                  ".",1);
      logger->log("Config:baseline_info: Histogram ADC range set between " +
                  std::to_string(hist_min_adc) +
                  " and " +
                  std::to_string(hist_max_adc) +
                  ".",1);
      logger->log("Config:baseline_info: Histogram bin width =  " +
                  std::to_string(hist_bin_width) +
                  " ADCs (inverse =  " +
                  std::to_string(hist_inv_bin_width) +
                  ").",1);
      logger->log("Config:baseline_info: Histogram window period =  " +
                  std::to_string(hist_window_period) +
                  " s ~ " +
                  std::to_string(hist_window_buffers) +
                  " * " +
                  std::to_string(buffer_config.raw_size*1e-3) +
                  " kB raw data buffers.",1);

      // Check whether baseline operations are on
      const bool ops_baseline_class =
        cfg.getValue<int>("stm.operations.Baseline");
      const bool ops_baseline_func =
        cfg.getValue<int>("stm.operations.Baseline.calc_baseline");

      // Check any of the conditions requiring fallback to no window delay
      if (!ops_baseline_class || !ops_baseline_func){        
        std::string msg =
          "Config:baseline_info: Baseline operations disabled in config. Forcing baseline calculation window delay to be OFF.";
        logger->log(msg, 2);        
        // Force manual baseline
        const_cast<bool&>(window_delay) = false;
      }
      
      logger->log(std::string("Config:baseline_info: Delay DAQ by 1 window period to calculate baseline = ") +
                  (window_delay ? "TRUE." : "FALSE."),1);
      logger->log("Config:baseline_info: Maximum EM iterations = " +
                  std::to_string(EM_max_iters) +
                  ".",1);
      logger->log("Config:baseline_info: EM Convergence tolerance = " +
                  std::to_string(EM_LL_tol) +
                  ".",1);
      logger->log("Config:baseline_info: EM Minimum variance = " +
                  std::to_string(EM_var_floor) +
                  ".",1);
      logger->log("Config:baseline_info: Baseline value from previous run = " +
                  std::to_string(prev_mean) + " ± " + std::to_string(prev_sigma) +
                  " ADCs.",1);
    }

  }

 
};

#endif
