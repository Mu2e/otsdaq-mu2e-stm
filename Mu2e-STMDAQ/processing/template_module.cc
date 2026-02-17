// Template module code
#include "Mu2e-STMDAQ/processing/template_module.hh"

// Constructor
TemplateClass::TemplateClass(const std::shared_ptr<AsyncLogger>& logger_,
                             const std::shared_ptr<STMdata>& stm_) :
  logger(logger_), stm(stm_) {

  // Instantiate any class variables in the constructor  
  
  // Register operations for OperationManager
  register_operation("temp_func_1", [this](auto& b){ temp_func_1(b); });
  register_operation("temp_func_2", [this](auto& b){ temp_func_2(b); });

}

// First template function activated by operation manager
// and implemented by thread manager
void TemplateClass::temp_func_1(std::shared_ptr<DataStruct>& buffer){

  // Do something to buffer

  // Call inner function to do something else to buffer
  temp_func_1_inner(buffer);
}

// Inner function of temp_func_1 - called by temp_func_1
// NOT a seperate threaded process. Is executed in the same
// thread as temp_func_1
void TemplateClass::temp_func_1_inner(std::shared_ptr<DataStruct>& buffer){

  // Do something to buffer

  // The adc length
  uint64_t n = buffer->raw_len;
  
  // Define a pointer to the adc data
  std::vector<int16_t>* data = &buffer->raw;

  // Loop over all data
  for (size_t i = 0; i < n; ++i) {

    // Cast the value as a double
    const double data_i = (*data)[i];
    
  }
  
}

// Second template function activated by operation manager
// and implemented by thread manager
void TemplateClass::temp_func_2(std::shared_ptr<DataStruct>& buffer){
  
  // Do something to buffer
  
}

