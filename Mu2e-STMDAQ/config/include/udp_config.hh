#ifndef UDP_CONFIG_HH_
#define UDP_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/master_config.hh"

// UDP configurable variables
struct udp_info{

  const std::string channel;
  
  // Receiving UDP socket IP address
  const std::string rcv_ip;
  // Receiving UDP socket port
  const uint16_t rcv_port;
  // Sending UDP socket IP address
  const std::string snd_ip;
  // Sending UDP socket port
  const uint16_t snd_port;

  // The recvmmsg timout
  const int recv_timeout_s; // seconds
  const int recv_timeout_us; // microseconds

  // The poll timout
  const int poll_timeout_ms; // milliseconds
  
  // Constructor
  udp_info(Config& cfg,
           const std::shared_ptr<AsyncLogger> logger,
           master_info master_config) :
    channel((master_config.ch_num) ? "stm.udp.labr." : "stm.udp.hpge."), // Channel descriptor 
    rcv_ip(cfg.getValue<std::string>(channel+"fw.ip")), // Receiving IP address
    rcv_port(cfg.getValue<int>(channel+"fw.port")), // Receiving port
    snd_ip(cfg.getValue<std::string>(channel+"sw.ip")), // Receiving IP address
    snd_port(cfg.getValue<int>(channel+"sw.port")), // Receiving port
    recv_timeout_s(cfg.getValue<int>("stm.udp.recv_timeout_secs")), // recvmmsg timeout (s)
    recv_timeout_us(cfg.getValue<int>("stm.udp.recv_timeout_usecs")), // recvmmsg timeout (us)
    poll_timeout_ms(cfg.getValue<int>("stm.udp.poll_timeout_ms")) // poll timeout (ms)
  {
    
    // Notify user
    if (logger){
      logger->log("Config:udp_info: UDP receive socket IP address: = " +
                  rcv_ip + ",  port = " + std::to_string(rcv_port) + ".",1);
      logger->log("Config:udp_info: UDP send socket IP address: = " +
                  snd_ip + ",  port = " + std::to_string(snd_port) + ".",1);
      logger->log("Config:udp_info: RECVMMSG timeout: = " +
                  std::to_string(recv_timeout_s) + " s + " +
                  std::to_string(recv_timeout_us) + " us.",1);
      logger->log("Config:udp_info: Poll timeout: = " +
                  std::to_string(poll_timeout_ms) + " ms.",1);
    }

  }
    
};

#endif
