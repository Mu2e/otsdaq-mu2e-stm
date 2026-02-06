#ifndef WRITE_CONFIG_HH_
#define WRITE_CONFIG_HH_

#include <numeric> // std::accumulate

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/fw_config.hh"

// Zero suppression configurable variables
struct write_info{

  // Output directory for file writing
  const std::string output_dir;

  // Filename prefixes
  const std::string file_prefix;

  // Compile time write stream names
  const std::vector<std::string> streamID = {"events","raw","zs","ph"};
  
  // Write stream enabled?
  const std::vector<bool> stream_enabled;

  // Total number of stream enabled
  const int tot_streams_enabled;
  
  // Maximum file size
  const size_t max_file_size;

  // File pre-opening depth
  const int file_buffer_depth;

  // Interval in seconds for stats logging
  const int log_interval_sec;

  // Constructor
  write_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger) :
    output_dir(cfg.getValue<std::string>("stm.write_data.write_dir")), // Directory
    file_prefix(cfg.getValue<std::string>("stm.write_data.filename_prefix")), // Raw filename prefix
    stream_enabled({cfg.getValue<bool>("stm.operations.WriteManager.write_events"), // Event write enabled
                   cfg.getValue<bool>("stm.operations.WriteManager.write_raw"), // Raw write enabled
                   cfg.getValue<bool>("stm.operations.WriteManager.write_zs"), // ZS write enbaled
                   cfg.getValue<bool>("stm.operations.WriteManager.write_ph")}), // PH write enabled
    tot_streams_enabled(static_cast<int>(std::accumulate(
                                                         stream_enabled.begin(), stream_enabled.end(), 0,
                                                         [](int sum, bool b){ return sum + (b ? 1 : 0); }
                                                         ))),
    max_file_size(cfg.getValue<double>("stm.write_data.max_file_size") * 1e9), // Maximum file size
    file_buffer_depth(cfg.getValue<int>("stm.write_data.file_buffer")), // File pre-opening depth
    log_interval_sec(cfg.getValue<int>("stm.write_data.log_interval")) // Stats logging interval
  {    
    // Notify user
    if (logger){
      logger->log("Config:write_info: Output_dir = " + output_dir, 1);    
      logger->log("Config:write_info:  Filename prefix = " + file_prefix, 1);
      logger->log("Config:write_info: Maximum file size set to  " +
                  std::to_string(max_file_size*1e-9) + " GB.", 1);
      logger->log("Config:write_info: Set file buffer depth to " +
                  std::to_string(file_buffer_depth) + " files.", 1);
      logger->log("Config:write_info: Set log interval = " +
                  std::to_string(log_interval_sec) + " s.", 1);
      for (size_t i = 0; i < streamID.size(); i++){
        logger->log("Config:write_info: " + streamID[i] + " stream " +
                    std::string(stream_enabled[i] ? "ENABLED" : "DISABLED"),1);
      }
      logger->log("Config:write_info: total number of enabled write streams =  " +
                  std::to_string(tot_streams_enabled),1);     
              
    }
    
  }
  
};

#endif
