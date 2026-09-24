#ifndef OPERATION_MANAGER_hh_
#define OPERATION_MANAGER_hh_

#include <unordered_map>
#include <map>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// UDP header
#include "Mu2e-STMDAQ/processing/udp.hh" 
// Check data header
#include "Mu2e-STMDAQ/processing/check_data.hh" 
// Form events header
#include "Mu2e-STMDAQ/processing/form_events.hh" 
// Baseline header
#include "Mu2e-STMDAQ/processing/baseline.hh" 
// Zero suppression header
#include "Mu2e-STMDAQ/processing/zero_suppress.hh" 
// Noise header
#include "Mu2e-STMDAQ/processing/noise.hh" 
// MWD header
#include "Mu2e-STMDAQ/processing/mwd.hh"
// Pulse height header
#include "Mu2e-STMDAQ/processing/pulse_height.hh"
// Prescale header
#include "Mu2e-STMDAQ/processing/prescale.hh"
// Template module header
#include "Mu2e-STMDAQ/processing/template_module.hh"
// Write manager header
#include "Mu2e-STMDAQ/processing/write_manager.hh"
// UDP header
#include "Mu2e-STMDAQ/processing/tcp.hh" 
// Write manager header
#include "Mu2e-STMDAQ/processing/dqm_manager.hh"
// UDP header
#include "Mu2e-STMDAQ/debug/test_funcs.hh" 
// SHM header
#include "Mu2e-STMDAQ/utils/shm_manager.hh" 

// Standard thread functions
class OperationManager {

private:
  
  // Store reference to the Config instance
  Config& cfg;

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;
  
  // Stores instances of operation classes
  std::unordered_map<std::string, std::shared_ptr<OperationBase>> classes;
  // Seleceted operations to use
  std::vector<std::pair<std::string, op_any>> useOps;
  // Selected operations flag
  std::vector<std::pair<std::string,std::string>> useOpsFlag;

  // Define a struct to hold both the class constructor and operation names
  struct ClassEntry {
    std::function<std::shared_ptr<OperationBase>()> constructor;
    std::vector<std::string> operations;
  };
  
  // Define class map for creating class instances dynamically 
  const std::unordered_map<std::string, ClassEntry> class_map = {
    {"UDP",
     { [this]() { return std::make_shared<UDP>(logger, stm, signal, false); },
       {"receive_data"} }},
    {"CheckData",
     { [this]() { return std::make_shared<CheckData>(logger,stm); },
       {"check_packets"} }},
    {"FormEvents",
     { [this]() { return std::make_shared<FormEvents>(logger,stm); },
       {"get_events"} }},
    {"Baseline",
     { [this]() { return std::make_shared<Baseline>(cfg,logger,stm); },
       {"calc_baseline"} }},
    {"MWD",
     { [this]() { return std::make_shared<MWD>(logger, stm);},
       {"subtract_baseline", "deconv", "diff", "averaging", "find_peaks"} }},    
    {"ZeroSuppress",
     { [this]() { return std::make_shared<ZeroSuppress>(cfg,logger,stm, signal);},
       {"prep_data", "find_peaks", "suppress_data"} }},
    {"PulseHeight",
     { [this]() { return std::make_shared<PulseHeight>(cfg, logger, stm);},
       {"detectPulseCandidates", "processPulseCandidates"} }},
    {"Noise",
     { [this]() { return std::make_shared<Noise>(cfg, logger, stm);},
       {"get_noise_data", "check_noise", "noise_fft"} }},
    {"Prescale",
     { [this]() { return std::make_shared<Prescale>(logger,stm);},
       {"prescale_data"} }},    
    {"TemplateClass",
     { [this]() { return std::make_shared<TemplateClass>(logger, stm);},
       {"temp_func_1", "temp_func_2"} }},    
    {"WriteManager",
     { [this]() { return std::make_shared<WriteManager>(logger, stm, signal);},
       {"write_events", "write_raw", "write_zs", "write_ph"} }},
    {"TCP",
     { [this]() { return std::make_shared<TCP>(logger, stm, signal); },
       {"send_data"} }},
    {"DQM",
     { [this]() { return std::make_shared<DQM>(cfg,logger, stm, signal, this); },
       {"update_dqm"} }},
    {"TestFuncs",
     { [this]() { return std::make_shared<TestFuncs>(signal); },
       {"mod_prev_buffer","read_mod_buffer","check_form_events", "do_nothing"} }}
  };
  
public:
  
  // Constructor to initialize threads
  explicit OperationManager(Config& cfg_,
                            const std::shared_ptr<AsyncLogger>& logger,
                            const std::shared_ptr<STMdata>& stm_,
                            const std::shared_ptr<SignalHandler>& signal_);
  
  // Return the selected subset of operations
  std::vector<std::pair<std::string, op_any>> getUseOps() const;

  // Return how many classes have been selected
  std::size_t class_num() const { return classes.size(); }
  
  // Function to check if a class name exists
  bool isValidClass(std::string_view class_) {
    for (const auto& [className, map] : class_map) {
      if (className == class_) return true;
    }
    return false;
  }  

  // Function to check if a operation name exists within class
  bool isValidOp(std::string_view class_, std::string_view op_) {
    for (const auto& [className, map] : class_map) {
      if (className == class_) {
        for (const auto& op : map.operations) {
          if (op == op_) {
            return true;
          }
        }
      }
    }
    return false;
  }  

  // Get the pointer to a class in the operation base
  std::shared_ptr<OperationBase>& get_class(std::string name){    
    if (classes.find(name) == classes.end()){
      if (name == "Baseline"){
        static std::shared_ptr<OperationBase> null_ptr = nullptr;
        return null_ptr;
      }
      else{
        logger->log("ERROR in operation_manager.hh. Cannot find class named: " + name + ". Exiting...",0);
      }
    }
    return classes[name];
  }

  // Check whether a class is in use
  bool check_class(std::string name){
    // Find the class name in the name
    auto it = classes.find(name);
    // If class is within the map
    if (it != classes.end()) {
      // Return true
      return true;
    }
    // Return false
    return false;    
  }
  
  // Destructor for logging
  ~OperationManager() {
    //std::cout << "Operation Manager destructor called.\n";
    if (logger) logger->log("Operation Manager destructor called.",1);
  }

};

#endif 
