// Process Manager code                                                       
#include "Mu2e-STMDAQ/processing/operation_manager.hh"

// Constructor
OperationManager::OperationManager(Config& cfg_,
                                   const std::shared_ptr<AsyncLogger>& logger_,
                                   const std::shared_ptr<STMdata>& stm_,
                                   const std::shared_ptr<SignalHandler>& signal_) :
  cfg(cfg_), logger(logger_), stm(stm_), signal(signal_) {

  // Get class names from the config file
  auto class_names = cfg.extractKeysAndValues("stm.operations");

  // Boolean to signal form events is off
  bool form_events = false;

  // Loop over all class names in config file
  for (const auto& class_ : class_names) {
    // Check if class name is valid
    if (!isValidClass(class_.first)){
      // Log critical error to user
      std::stringstream output;
      output << "Operation class key: \"" << class_.first
             << "\" in " << cfg.getXMLpath()
             << " is NOT a valid class name.";
      logger->log(output.str(),0);
      return;
    }
    
    // Get the xml config file node
    std::string class_node = "stm.operations." + class_.first;
    // Get the config file boolean to use or not 
    bool use_class = cfg.getValue<int>(class_node);
    
    // If class is switched off
    if (!use_class){
      // Log class is off and continue
      logger->log(class_.first + " class is OFF.", 1);
      continue;
    }

    // Signal if FormEvents is on
    if (class_.first == "FormEvents") form_events = true;
    // If ZeroSuppress or MWD are on and FormEvents is off, switch them off
    if ((class_.first == "ZeroSuppress" || class_.first == "MWD") && !form_events) {
      // Log class is off and continue
      logger->log("WARNING: " + class_.first +
                  " ON when FormEvents OFF. Switching off " + class_.first + ".", 2);
      logger->log(class_.first + " is OFF.", 1);
      continue;
    }

    // Check if ZS find peaks is on for Noise module
    // If it isn't on, turn Noise off
    std::pair<std::string,std::string> find_peaks_op = {"ZeroSuppress","find_peaks"};
    if (class_.first == "Noise" && std::find(useOpsFlag.begin(), useOpsFlag.end(), find_peaks_op) == useOpsFlag.end()){
      logger->log("WARNING: " + class_.first +
		  " ON when " + find_peaks_op.first + "::" +
		  find_peaks_op.second + 
		  " operation is OFF. Switching off " + class_.first
		  + ".",2);
      logger->log(class_.first + " is OFF.", 1);
      continue;
    }

    // Log class is on
    logger->log(class_.first + " class is ON.", 1);

    // Find the class name in the name
    auto it = class_map.find(class_.first);
    // If class is within the map
    if (it != class_map.end()) {
      // Instantiate and store the class
      classes[class_.first] = it->second.constructor();
    }
    // Else, class not found in map
    else {
      // Log critical error
      logger->log("ERROR: No class_map entry for class: " + class_.first, 0);
    }

    // Get the class operation names
    auto op_names = cfg.extractKeysAndValues(class_node);
    
    // Boolean to turn later class operation off if earlier one is off 
    bool rest_off = false;

    // Loop over all operation names
    for (const auto& op_ : op_names) {

      // Check if operation name is valid
      if (!isValidOp(class_.first, op_.first)){
        // Log critical error to user
        std::stringstream output;
        output << "Operation key: \"" << class_.first
               << "::" << op_.first
               << "\" in " << cfg.getXMLpath()
               << " is NOT a valid class.operation name.";
        logger->log(output.str(),0);
        return;
      }

      // Get the xml config file node
      std::string op_node = class_node + "." + op_.first;
      // Get the operation name to print
      std::string op_name = class_.first + "::" + op_.first;
      // Get the config file boolean to use or not 
      bool use_op = cfg.getValue<int>(op_node);
      // If it has been signalled that we should turn the rest off
      // (UNLESS WRITE STREAM!)
      if (use_op && rest_off && class_.first != "WriteManager"){
        // Warn user
        logger->log("WARNING: " + class_.first + "::" + op_.first +
                    " ON when earlier " + class_.first +
                    " operations OFF. Switching off " + class_.first
                    + "::" + op_.first + ".",2);
        use_op = false;
      }
      // If operation is switched off
      if (!use_op) {
        // Log operation is off
        logger->log("- " + op_name + " is OFF.", 1);
        // Ensure rest of class is switched off and continue
        rest_off = true;
        continue;
      }

      // Log class is on
      logger->log("- " + op_name + " is ON.", 1);
      // Push back operation to use list
      useOpsFlag.push_back({class_.first, op_.first});
    }
  }  
  
  // // FOR DEVELOPMENT ONLY: Add last function (defined above) to the use list
  // if (useOpsFlag.size() > 0 && useOpsFlag[0].first == "UDP"){ // If receiving data...
  //   std::string last_class_name = "TestFuncs";
  //   std::string last_op_name = "doNothing";
  //   // std::string last_op_name = "print";
  //   // std::string last_op_name = "printPackets";
  //   // std::string last_op_name = "check_form_events";
  //   // std::string last_op_name = "check_prep_zs";
  //   //  std::string last_op_name = "check_zs";
  //   auto last_class = class_map.find(last_class_name);
  //   classes[last_class_name] = last_class->second.constructor();
  //   logger->log(last_class_name + " is ON.",1);
  //   logger->log("- " + last_class_name + "::" + last_op_name + " is ON.",1);
  //   useOpsFlag.push_back({last_class_name,last_op_name});
  // }
  
  // Loop over all selected operations
  // for (const auto& [className, opName] : useOpsFlag) {
  //   useOps.emplace_back(className + "::" + opName,
  //                       [this, className, opName](std::shared_ptr<DataStruct>& buffer) {
  //                         classes[className]->execute(opName, buffer);});
  // }

  // Loop over all selected operations
  for (const auto& [className, opName] : useOpsFlag) {
    
    // Defensive: class must exist
  if (classes.find(className) == classes.end() || !classes[className]) {
    logger->log("ERROR: OperationManager: class '" + className + "' is null.", 0);
    continue;
  }
  
  // Decide whether op needs 2 buffers
  const bool needs2 = classes[className]->requires_two_buffers(opName);
  
  if (needs2) {
    // Store a 2-buffer operation
    useOps.emplace_back(className + "::" + opName,
                        op2{[this, className, opName](std::shared_ptr<DataStruct>& buffer,
                                                      std::shared_ptr<DataStruct>& prev_buffer) {
                          classes[className]->execute(opName, buffer, prev_buffer);
                        }}
                        );
  }
  else {
    // Store a 1-buffer operation
    useOps.emplace_back(className + "::" + opName,
                        op1{[this, className, opName](std::shared_ptr<DataStruct>& buffer) {
                          classes[className]->execute(opName, buffer);
                        }}
                        );
  }
  }
  
  
  // Notify user
  logger->log("OperationManager initialised a total of " + std::to_string(useOps.size()) + " operations",1);

  // Now number of ops known DQM can allocate shm
  if (classes.count("DQM")) {
    auto dqm_ptr = std::dynamic_pointer_cast<DQM>(classes["DQM"]);
    if (dqm_ptr) dqm_ptr->init_shm(); 
  }
  
}

// Get the list of selected operations
std::vector<std::pair<std::string, op_any>>
OperationManager::getUseOps() const {
  return useOps;
}
