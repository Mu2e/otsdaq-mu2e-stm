#ifndef TEMPLATE_MODULE_hh
#define TEMPLATE_MODULE_hh

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

class TemplateClass : public OperationMap {

private:

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
public:
  
  // Constructor
  TemplateClass(const std::shared_ptr<AsyncLogger>& logger_,
                const std::shared_ptr<STMdata>& stm_);
  
  // Destructor                                                          
  ~TemplateClass() {
    std::cout << "TemplateClass destructor called.\n";
  }

  // First template function activated by operation manager
  // and implemented by thread manager
  void temp_func_1(std::shared_ptr<DataStruct>& buffer);

  // Inner function of temp_func_1 - called by temp_func_1
  // NOT a seperate threaded process. Is executed in the same
  // thread as temp_func_1
  void temp_func_1_inner(std::shared_ptr<DataStruct>& buffer);

  // Second template function activated by operation manager
  // and implemented by thread manager
  void temp_func_2(std::shared_ptr<DataStruct>& buffer);

};

#endif
