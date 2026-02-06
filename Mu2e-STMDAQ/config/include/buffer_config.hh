#ifndef BUFFER_CONFIG_HH_ 
#define BUFFER_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/zs_config.hh"

// Buffer configurable variables
struct buffer_info{

  // Raw data buffer size (bytes, int16_t data type)
  const size_t raw_size;
  // Raw data buffer len (ADC values)
  const size_t raw_len;

  // Maximum number of packets in buffer
  const size_t max_packet_num;
  // Maximum size of packet data in buffer (bytes)
  const size_t max_packet_size;
  // Maximum length of packetm data in buffer
  const size_t max_packet_len;

  // Maximum number of mu2e events in buffer
  const size_t max_event_num;
  
  // ZS data buffer size (bytes, int16_t data type)
  const size_t zs_size;
  // ZS data buffer len (ADC values)
  const size_t zs_len;
  
  // Pulse height buffer (bytes, double data type)
  const size_t ph_size;
  // PH data buffer len (ADC values)
  const size_t ph_len;
  
  // ADC baseline buffer (bytes, double data type)
  const size_t baseline_size;
  // PH data buffer len (ADC values)
  const size_t baseline_len;

  // Additional of the total allocated buffer size for extras
  const double extra;

  // Queue capacity per operation
  const size_t queue_capacity;

  // Buffer pool size per operation
  const size_t pool_size;
  
  // Constructor
  buffer_info(Config& cfg,
              const std::shared_ptr<AsyncLogger> logger,
              fw_info fw_config,
              mu2e_info mu2e_config,
              zs_info zs_config,
              const uint16_t MAX_PACKET_SIZE) :
    raw_size(cfg.getValue<int>("stm.buffers.raw_size")), // bytes, int16_t type
    raw_len(raw_size/sizeof(int16_t)),
    max_packet_num(std::floor(( // Max number of packets = round down (
                               (double)raw_size // Raw data size
                               - (double)mu2e_config.max_event_size) // - space for 1 max size mu2e event (for FormEvents)
                              /(double)MAX_PACKET_SIZE // Divided by max packet size
                              )),
    max_packet_size(max_packet_num*MAX_PACKET_SIZE),
    max_packet_len(max_packet_size/sizeof(int16_t)),
    max_event_num(std::ceil((double)max_packet_size //  Max packet size (ignore headers = extra space)
                            /(double)mu2e_config.min_event_size) // Divded by min event size
                             + 1), // Plus 1 extra mu2e event (for FormEvents)
    zs_size(raw_size), // bytes, int16_t type
    zs_len(zs_size/sizeof(int16_t)), 
    ph_size(raw_size/sizeof(int16_t)*sizeof(double)), // bytes, double type
    ph_len(ph_size/sizeof(double)),
    baseline_size(cfg.getValue<int>("stm.buffers.baseline_size")), // bytes, int16_t type
    baseline_len(baseline_size/sizeof(int16_t)),
    extra(cfg.getValue<double>("stm.buffers.extra")),
    queue_capacity(cfg.getValue<int>("stm.buffers.queue_capacity")),
    pool_size(cfg.getValue<int>("stm.buffers.buffer_pool_size"))
  {
    
    // Notify user
    if (logger){
      logger->log("Config:buffer_info: Raw data (int16_t) buffer size = " +
                  std::to_string(raw_size) + " bytes = " +
                  std::to_string(raw_len) + " ADC values.",1);
      logger->log("Config:buffer_info: Maximum 8198 byte packets per raw buffer = " +
                  std::to_string(max_packet_num) + " packets = " +
                  std::to_string(max_packet_size) + " bytes.",1);
      logger->log("Config:buffer_info: Maximum number of " +
                  std::to_string(mu2e_config.min_event_period) + 
                  " us (minimum) Mu2e events per raw buffer = " +
                  std::to_string(max_event_num) + " events.",1);
      logger->log("Config:buffer_info: Zero suppressed (int16_t) data buffer size = " +
                  std::to_string(zs_size) + " bytes = " +
                  std::to_string(zs_len) + " ADC values.",1);
      logger->log("Config:buffer_info: Pulse height (double) data buffer size = " +
                  std::to_string(ph_size) + " bytes = " +
                  std::to_string(ph_len) + " ADC values.",1);
      logger->log("Config:buffer_info: ADC baseline (int16_t) data buffer size = " +
                  std::to_string(baseline_size) + " bytes = " +
                  std::to_string(baseline_len) + " ADC values = " +
                  std::to_string(baseline_len*fw_config.tADC) + " us .",1);
      logger->log("Config:buffer_info: % extra of total allocated buffer size (metadata) = " +
                  std::to_string(extra) + " %.",1);
      logger->log("Config:buffer_info: Queue capacity for each operation = " +
                  std::to_string(queue_capacity) + " .",1);
      logger->log("Config:buffer_info: Buffer pool for each operation = " +
                  std::to_string(pool_size) + " .",1);
    
    }
  }
    
};

#endif
