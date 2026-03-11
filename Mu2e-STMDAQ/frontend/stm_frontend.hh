#ifndef STM_FRONTEND_hh_
#define STM_FRONTEND_hh_

// Thread Manager header                                                 
#include "Mu2e-STMDAQ/processing/thread_manager.hh"

// Frontend class
class STMfrontend {

private:

  // DAQ timer
  Timer t;

  // Load the config xml
  const std::string xml_config_path;

  // Config class
  Config& cfg;
  
  // Store reference to CPU utils instance
  const std::shared_ptr<cpu_utils> cpu;
  
  // Async Logger
  const std::shared_ptr<AsyncLogger> logger;

  // Signal Handler
  const std::shared_ptr<SignalHandler> signal;
  
  // STM data info
  const std::shared_ptr<STMdata> stm;  
  
  // Hardware manager
  std::shared_ptr<HardwareManager> hw;
  
  // Operation Manager
  std::shared_ptr<OperationManager> om;

  // Buffer pool
  std::shared_ptr<BufferPool> pool;
  
  // Thread Manager
  std::shared_ptr<ThreadManager> tm;
    
public:
  
  // STM frontend conststuro
  STMfrontend();

  // Destructor
  ~STMfrontend() {
    std::cout << "STMfrontend destructor called.\n";    
  }
  
  // Start STM DAQ
  void start_stmdaq();
  
  // Wait until stop signal is called
  void wait();

};

#endif 



