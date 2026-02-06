#ifndef FW_CONFIG_HH_
#define FW_CONFIG_HH_

// Include config file
#include "Mu2e-STMDAQ/config/config.hh"

// Firmware configurable variables
struct fw_info{

  // The ADC sampling frequency
  const double fADC;
  // The ADC sampling period
  const double tADC;

  // Constructor
  fw_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger) :
    fADC(cfg.getValue<double>("stm.fw.fADC")), // fADC in MHz
    tADC(1.0/fADC) // tADC in µs per value
  {
    
    // Notify user
    if(logger){
      logger->log("Config:fw_info: ADC sampling frequency = " +
                  std::to_string(fADC) +
                  " MHz (" + std::to_string(tADC*1e3) +
                  " ns per ADC value).",1);
    }
  }
  
};

#endif
