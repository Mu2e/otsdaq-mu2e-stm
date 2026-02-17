// Thread Manager header
#include "Mu2e-STMDAQ/processing/thread_manager.hh"
#include "Mu2e-STMDAQ/hardware/hw_manager.hh" 

// Main
int main() {  

  // DAQ timer
  Timer t("DAQ");
  
  // Load the configuration
  std::string xml_path = EnvVars::expand("${STM_XML}");
  //static const Config& cfg = Config::getInstance(xml_path);
  Config& cfg = Config::getInstance(xml_path);

  // Initialise CPU utils
  std::shared_ptr<cpu_utils> cpu = cpu_utils::getInstance(cfg);
  
  // Initalise the logger  
  std::shared_ptr<AsyncLogger> logger = std::make_shared<AsyncLogger>(cfg,cpu);

  // Initialise the signal handler
  std::shared_ptr<SignalHandler> signal = std::make_shared<SignalHandler>(logger,cpu);

  // Initialise the STM data information
  std::shared_ptr<STMdata> stm = std::make_shared<STMdata>(cfg,logger);

  // Initialise hardward manager
  std::shared_ptr<HardwareManager> hw = std::make_shared<HardwareManager>(logger,stm);
  
  // Instance of operation manager
  std::shared_ptr<OperationManager> om;
  if (!stop::should_stop()) om = std::make_shared<OperationManager>(cfg,logger,stm,signal);
  
  // If any operations have been selected...
  if (om->class_num() > 0){ 

    // // Instantiate selected operations
    // om->select_ops(); 
    
    // Create buffer pool
    std::shared_ptr<BufferPool> pool;
    if (!stop::should_stop()) pool = std::make_shared<BufferPool>(cpu,logger,stm,om);
    
    // Instance of thread manager
    if (!stop::should_stop()) std::shared_ptr<ThreadManager> tm = std::make_shared<ThreadManager>(cpu,logger,stm,signal,pool,om,hw);
    
  }

  return 0;
  
}
