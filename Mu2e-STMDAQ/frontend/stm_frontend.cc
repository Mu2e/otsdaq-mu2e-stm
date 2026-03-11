// STM Frontend Header
#include "Mu2e-STMDAQ/frontend/stm_frontend.hh"

// STM frontend constructor
STMfrontend::STMfrontend() :
  xml_config_path(EnvVars::expand("${STM_XML}")),
  cfg(Config::getInstance(xml_config_path)),
  cpu(cpu_utils::getInstance(cfg)),
  logger(std::make_shared<AsyncLogger>(cfg,cpu)),
  signal(std::make_shared<SignalHandler>(logger,cpu)),
  stm(std::make_shared<STMdata>(cfg,logger)){
    
  // If we are on the hardware control server
  if (stm->master_config.host == stm->fw_config.ctrl_srvr){
    // Initialise hardware manager
    hw = std::make_shared<HardwareManager>(logger,stm);
  }
  
  // Instance of operation manager
  if (!stop::should_stop()) om = std::make_shared<OperationManager>(cfg,logger,stm,signal);
  
  // If any operations have been selected...
  if (om->class_num() > 0){ 
    
    // Create buffer pool
    if (!stop::should_stop()) pool = std::make_shared<BufferPool>(cpu,logger,stm,om);
    
  }
}

// Start STMDAQ
void STMfrontend::start_stmdaq(){

  // Instance of thread manager
  if (!stop::should_stop()) tm = std::make_shared<ThreadManager>(cpu,logger,stm,signal,pool,om,hw);

  return;

}


// Wait until stop signal is called
void STMfrontend::wait(){
  
  while (!stop::should_stop()) {}

  return;

}

