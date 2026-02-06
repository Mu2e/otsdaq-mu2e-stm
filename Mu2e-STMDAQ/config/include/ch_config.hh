#ifndef CH_CONFIG_HH_
#define CH_CONFIG_HH_

// Include config file
#include "Mu2e-STMDAQ/config/config.hh"

// Channel configurable variables
struct ch_info{

  // The channel number
  const int num;
  // The channel name
  const std::string name;

  // Constructor
  ch_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger) :
    num(cfg.getValue<int>("stm.channel")), // channel number
    name((num) ? "Labr" : "HPGe") // Channel name
  {
    
    // Notify user
    if (logger){
      logger->log("Config:ch_info: Channel = " +
                  std::to_string(num) +
                  " (" + name + ")",1);
      if (num != 0 && num != 1){
        logger->log("Config:ch_info: Error in config file --> stm.channel must be equal to 0 (HPGe) or 1 (LaBr)",0);
      }
    }
    
  }
  
};

#endif
