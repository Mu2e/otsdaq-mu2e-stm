#ifndef PRESCALE_MODULE_hh
#define PRESCALE_MODULE_hh

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/utils/stm_data.hh"
// Ring buffer code
#include "Mu2e-STMDAQ/utils/ring_buffer.hh"
// Operations base header
#include "Mu2e-STMDAQ/utils/operations_base.hh"

class PrescaleRaw : public OperationBase {

private:

  // Store reference to the Config instance
  Config& cfg;

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // The channel number
  const int CHAN;
  
  // A function map for the process manager
  std::unordered_map<std::string, std::function<void(std::shared_ptr<DataStruct>&)>> functionMap;

  // What number to prescale by
  int16_t raw_ps_size = 0;

public:
  
  // Constructor
  PrescaleRaw(Config& cfg_,
		const std::shared_ptr<AsyncLogger>& logger_,
		const std::shared_ptr<STMdata>& stm_,
		const int CHAN_);
  
  // Destructor                                                          
  ~PrescaleRaw() {
    std::cout << "PrescaleRaw destructor called.\n";
  }

  // Function activated by operation manager
  void random_prescale(std::shared_ptr<DataStruct>& buffer);
  void peak_based_prescale(std::shared_ptr<DataStruct>& buffer);

  // Execute function for OperationManager
  void execute(const std::string& methodName, std::shared_ptr<DataStruct>& buffer) override {
    if (functionMap.find(methodName) != functionMap.end()) {
      functionMap[methodName](buffer);
    } else {
      std::cerr << "Error: Invalid method name '" << methodName << "' in PrescaleRaw\n";
    }
  }  

};

#endif
