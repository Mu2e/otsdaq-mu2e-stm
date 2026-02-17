#ifndef TCP_CONFIG_HH_
#define TCP_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/master_config.hh"

// TCP configurable variables
struct tcp_cfg_info{

  const std::string channel;
  
  // Sending TCP socket IP address
  const std::string snd_ip;
  // Sending TCP socket port
  const uint16_t snd_port;

  // Server connection attempt timers
  const int reconnect_period_ms; // milliseconds
  const int disconnected_log_period; // seconds

  // The poll timout
  const int poll_timeout_ms; // milliseconds
  
  // Constructor
  tcp_cfg_info(Config& cfg,
           const std::shared_ptr<AsyncLogger> logger,
           master_info master_config) :
    channel((master_config.ch_num) ? "stm.tcp.labr." : "stm.tcp.hpge."), // Channel descriptor 
    snd_ip(cfg.getValue<std::string>(channel+"ip")), // Receiving IP address
    snd_port(cfg.getValue<int>(channel+"port")), // Receiving port
    reconnect_period_ms(cfg.getValue<int>("stm.tcp.reconnect_period_ms")), 
    disconnected_log_period(cfg.getValue<int>("stm.tcp.disconnected_log_period")), 
    poll_timeout_ms(cfg.getValue<int>("stm.tcp.poll_timeout_ms")) // poll timeout (ms)
  {
    
    // Notify user
    if (logger){
      logger->log("Config:tcp_info: TCP send socket IP address: = " +
                  snd_ip + ",  port = " + std::to_string(snd_port) + ".",1);
      logger->log("Config:tcp_info: Server reconnection attempt period = " +
                  std::to_string(reconnect_period_ms) + " ms (no connection logged every " +
                  std::to_string(disconnected_log_period) + " s).",1);
      logger->log("Config:tcp_info: Poll timeout: = " +
                  std::to_string(poll_timeout_ms) + " ms.",1);
    }

  }
    
};

#endif
