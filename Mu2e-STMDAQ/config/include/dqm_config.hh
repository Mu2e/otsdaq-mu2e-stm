#ifndef DQM_CONFIG_HH_
#define DQM_CONFIG_HH_

#include <limits>

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/fw_config.hh"

// DQM configurable variables
struct dqm_info{

  // Get the right channel sub-string
  const std::string channel;
  
  // Number of raw ADC values to store
  const size_t raw_period;
  const size_t raw_len;

  // Number of noise ADC values to store
  const size_t noise_period;
  const size_t noise_len;

  // Peak hist info
  const size_t peak_nbins;
  const int16_t min_ph;
  const double inv_bin_wid;

  // Constructor
  dqm_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger,
	  fw_info fw_config) :
    raw_period(cfg.getValue<int>("stm.dqm.raw_data.raw_length")), // Raw time to store
    raw_len(raw_period*fw_config.fADC), // Number of raw ADC values to store
    noise_period(cfg.getValue<int>("stm.dqm.baseline.noise_length")), // Noise time to store
    noise_len(noise_period*fw_config.fADC), // Number of noise ADC values to store
    peak_nbins(cfg.getValue<int>("stm.dqm.pulses.peak_nbins")), // Number of bins in peak histogram
    min_ph(std::clamp(cfg.getValue<int>("stm.dqm.pulses.min_height"), // Min adc of peak histogram
                      static_cast<int>(std::numeric_limits<int16_t>::min()),
                      static_cast<int>(std::numeric_limits<int16_t>::max()))),
    inv_bin_wid(-1.0 * (double)peak_nbins/(double)min_ph) // inverse bin width
  {
    // Notify user
    if (logger){
      logger->log("Config:dqm_info: Storing " +
                  std::to_string(raw_len) +
                  " raw ADC values for DQM.",1);       
      logger->log("Config:dqm_info: Storing " +
                  std::to_string(noise_len) +
                  " noise ADC values for DQM.",1);       
      logger->log("Config:dqm_info: Peak histogram bins = " +
		   std::to_string(peak_nbins) + ", min height = " +
		   std::to_string(min_ph) + ", inverse bin width = " +
                  std::to_string(inv_bin_wid) ,1);       
    }
  }


};

#endif
