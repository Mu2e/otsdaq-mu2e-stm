#ifndef ZS_CONFIG_HH_
#define ZS_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/fw_config.hh"
#include "Mu2e-STMDAQ/config/include/mu2e_config.hh"

// Zero suppression configurable variables
struct zs_info{

  // Get the right channel sub-string
  const std::string channel;
  
  // Time to store before peak (us)
  const double before_peak_time;
  // Number of ADC values to store before peak
  const size_t before_peak;
  // Time to store after peak (us)
  const double after_peak_time;
  // Number of ADC values to store after peak
  const size_t after_peak;
  // Total number of ADC values stored per peak
  const size_t total_peak;
  
  // Gradient threshold
  const int grad_threshold;
  // Gradient window
  const size_t grad_window;
  // Gradient average number
  const size_t grad_n_avg;

  // Maximum amount of overflow data to store from previous buffer
  const size_t prev_data_max;

  // Constructor
  zs_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger,
          ch_info ch_config,
          fw_info fw_config,
          mu2e_info mu2e_config) :
    channel((ch_config.num) ? "stm.zs.labr." : "stm.zs.hpge."), // Channel descriptor
    before_peak_time(cfg.getValue<double>(channel+"store_before_peak")), // Time to store before peak (us)
    before_peak(before_peak_time*fw_config.fADC), // Number of ADC values to store before peak
    after_peak_time(cfg.getValue<double>(channel+"store_after_peak")), // Time to store after peak (us)
    after_peak(after_peak_time*fw_config.fADC), // Number of ADC values to store after peak
    total_peak(before_peak+after_peak), // Total number of ADC values stored per peak
    grad_threshold(cfg.getValue<int>(channel+"grad_threshold")), // Gradient threshold
    grad_window(cfg.getValue<int>(channel+"grad_window")), // Gradient window
    grad_n_avg(cfg.getValue<int>(channel+"n_avg")), // Gradient average number
    prev_data_max(before_peak+grad_window+grad_n_avg) // Maximum overflow data to store from previous buffer
  {
    // Notify user
    if (logger){
      logger->log("Config:zs_info: Storing " +
                  std::to_string(before_peak_time) +
                  " us / " + std::to_string(before_peak) +
                  " ADC values / " +
                  std::to_string(before_peak*sizeof(int16_t)) +
                  " bytes before peak.",1);       
      logger->log("Config:zs_info: Storing " +
                  std::to_string(after_peak_time) +
                  " us / " + std::to_string(after_peak) +
                  " ADC values / " +
                  std::to_string(after_peak*sizeof(int16_t)) +
                  " bytes after peak.",1);
      logger->log("Config:zs_info: Storing a total of " +
                  std::to_string(total_peak/fw_config.fADC) +
                  " us / " +
                  std::to_string(total_peak) +
                  " ADC values / " +
                  std::to_string(total_peak*sizeof(int16_t)) +
                  " bytes per peak.",1);   
      logger->log("Config:zs_info: Gradient threshold = " +
                  std::to_string(grad_threshold) +
                  ".",1);
      logger->log("Config:zs_info: Gradient window = " +
                  std::to_string(grad_window) +
                  " ADC values.",1);   
      logger->log("Config:zs_info: Gradient average number = " +
                  std::to_string(grad_n_avg) +
                  " ADC values.",1);     
      logger->log("Config:zs_info: Maximum overflow data = before_peak + window + n_average = " +
                  std::to_string(prev_data_max) +
                  " ADC values.",1);
    }
  }


};

#endif
