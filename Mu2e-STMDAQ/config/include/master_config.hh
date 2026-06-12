#ifndef MASTER_CONFIG_HH_
#define MASTER_CONFIG_HH_

// Environment variable code
#include "Mu2e-STMDAQ/utils/EnvVars.hh"
// Include config file
#include "Mu2e-STMDAQ/config/config.hh"

// Master configurable variables
struct master_info{

  // Server host
  const std::string host;
  // Ch0 (HPGe) server host name
  const std::string ch0_host;
  // Ch1 (LaBr) server host name
  const std::string ch1_host;
  // The channel number
  const int ch_num;
  // The channel name
  const std::string ch_name;
  // Numa node
  const int numa_sock;

  // Use software simulation as data source
  const bool use_sw_sim;

  static int compute_ch_num(const std::string& host,
                            const std::string& ch0_host,
                            const std::string& ch1_host)
  {
    if (host == ch0_host) return 0;
    if (host == ch1_host) return 1;
    throw std::runtime_error(
      "master_info: HOSTNAME '" + host +
      "' does not match stm.ch0_host ('" + ch0_host +
      "') or stm.ch1_host ('" + ch1_host + "')"
    );
  }

  static std::string compute_ch_name(int ch_num)
  {
    return (ch_num == 0) ? "HPGe" : "Labr";
  }

  static int get_daq_socket(int ch_num) {
    return (ch_num == 0) ? 1 : 0;
  }
  
  // Constructor
  master_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger) :
    host(EnvVars::expand("${HOSTNAME}")), // Server host name
    ch0_host(cfg.getValue<std::string>("stm.ch0_host")), // Channel 0 host name
    ch1_host(cfg.getValue<std::string>("stm.ch1_host")), // Channel 1 host name 
    ch_num(compute_ch_num(host, ch0_host, ch1_host)), // Channel number
    ch_name(compute_ch_name(ch_num)), // Channel name
    numa_sock(get_daq_socket(ch_num)),
    use_sw_sim(cfg.getValue<int>("stm.use_sw_sim")) // Use software simulation
  {
    
    // Notify user
    if (logger){
      logger->log("Config:master_info: HOST = " +
                  host + ", NUMA socket = " + 
		  std::to_string(numa_sock),1);      
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
