#ifndef MASTER_CONFIG_HH_
#define MASTER_CONFIG_HH_

// Include config file
#include "Mu2e-STMDAQ/config/config.hh"

// Master configurable variables
struct master_info{

  // The channel number
  const int ch_num;
  // The channel name
  const std::string ch_name;

  // Use software simulation as data source
  const bool use_sw_sim;
  
  // Constructor
  master_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger) :
    ch_num(cfg.getValue<int>("stm.channel")), // Channel number
    ch_name((ch_num) ? "Labr" : "HPGe"), // Channel name
    use_sw_sim(cfg.getValue<int>("stm.use_sw_sim")) // Use software simulation
  {
    
    // Notify user
    if (logger){
      logger->log("Config:master_info: Channel = " +
                  std::to_string(ch_num) +
                  " (" + ch_name + ")",1);
      if (ch_num != 0 && ch_num != 1){
        logger->log("Config:master_info: Error in config file --> stm.channel must be equal to 0 (HPGe) or 1 (LaBr)",0);
      }
      std::string option;
      if (use_sw_sim){
        option = "SOFTWARE SIMULATION";
      }
      else{
        option = "HARDWARE/FIRMWARE";
      }
      logger->log("Config:master_info: Using " +
                  option + " as data input.",1);      
    }
    
  }
  
};

#endif
