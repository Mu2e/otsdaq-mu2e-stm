#ifndef SIM_FW_hh_
#define SIM_FW_hh_

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <limits>
#include <iterator>
#include <cmath>
#include <atomic>

// STM DATA code
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Include buffer queue code
#include "Mu2e-STMDAQ/buffers/buffer_queue.hh"
// Signal handler code                                                       
#include "Mu2e-STMDAQ/utils/signal_handler.hh"

// Simulate Firmware Class
class SimFW {

private:

  // Config
  const Config& cfg;

  // STM data
  const std::shared_ptr<STMdata> stm;

  // Signal handler                                        
  const std::shared_ptr<SignalHandler> signal;
  
public:

  // Constructor 
  SimFW();

};

#endif 
