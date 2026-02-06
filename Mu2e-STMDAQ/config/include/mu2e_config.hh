#ifndef MU2E_CONFIG_HH_
#define MU2E_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/fw_config.hh"

// Mu2e configurable variables
struct mu2e_info{

  // The minumum mu2e event period (us)
  const double min_event_period;
  // The minumum mu2e event length (ADC  values)
  const size_t min_event_len;  
  // The minumum mu2e event size (bytes)
  const size_t min_event_size;  
  // The maximum mu2e event period (us)
  const double max_event_period;
  // The maximum mu2e event length (ADC  values)
  const size_t max_event_len;  
  // The maximum mu2e event size (bytes)
  const size_t max_event_size;  
  // The on-spill mu2e event period (us)
  const double on_spill_period;
  // The mu2e on-spill event length (ADC  values)
  const size_t on_spill_len;    
  // The mu2e on-spill event size (bytes)
  const size_t on_spill_size;    
  // The off-spill mu2e event period (us)
  const double off_spill_period;
  // The mu2e off-spill event length (ADC  values)
  const size_t off_spill_len;    
  // The mu2e off-spill event size (bytes)
  const size_t off_spill_size;    
  
  // Constructor
  mu2e_info(Config& cfg,
            const std::shared_ptr<AsyncLogger> logger,
            fw_info fw_config) :
    min_event_period(cfg.getValue<double>("stm.mu2e.min_event_period")),
    min_event_len(std::ceil(min_event_period*fw_config.fADC)),
    min_event_size(min_event_len*sizeof(int16_t)),
    max_event_period(cfg.getValue<double>("stm.mu2e.max_event_period")),
    max_event_len(std::ceil(max_event_period*fw_config.fADC)),
    max_event_size(max_event_len*sizeof(int16_t)),
    on_spill_period(cfg.getValue<double>("stm.mu2e.on_spill_period")),
    on_spill_len(std::ceil(on_spill_period*fw_config.fADC)),
    on_spill_size(on_spill_len*sizeof(int16_t)),
    off_spill_period(cfg.getValue<double>("stm.mu2e.off_spill_period")),
    off_spill_len(std::ceil(off_spill_period*fw_config.fADC)),
    off_spill_size(off_spill_len*sizeof(int16_t))
  {
    
    // Notify user
    if (logger){
      logger->log("Config:mu2e_info: Minimum Mu2e event period set to " +
                  std::to_string(min_event_period) +
                  " us = " +
                  std::to_string(min_event_len) +
                  " ADC values = " +
                  std::to_string(min_event_size) +                
                  " bytes.",1);    
      logger->log("Config:mu2e_info: Maximum Mu2e event period set to " +
                  std::to_string(max_event_period) +
                  " us = " +
                  std::to_string(max_event_len) +
                  " ADC values = " +
                  std::to_string(max_event_size) +                
                  " bytes.",1);
      logger->log("Config:mu2e_info: Mu2e on-spill event period set to " +
                  std::to_string(on_spill_period) +
                  " us = " +
                  std::to_string(on_spill_len) +
                  " ADC values = " +
                  std::to_string(on_spill_size) +                
                  " bytes.",1);
      logger->log("Config:mu2e_info: Mu2e off-spill event period set to " +
                  std::to_string(off_spill_period) +
                  " us = " +
                  std::to_string(off_spill_len) +
                  " ADC values = " +
                  std::to_string(off_spill_size) +                
                  " bytes.",1);
    }
  }
  
};

#endif
