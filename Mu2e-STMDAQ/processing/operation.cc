// DummyOperation header
#include "Mu2e-STMDAQ/processing/operation.hh" 

// Constructor
DummyOperation::DummyOperation() {

  // Initialise function map for ProcessManager
  functionMap["operation1"] = [this](std::shared_ptr<DataStruct>& buffer) {
    operation1(buffer);
  };
  functionMap["operation2"] = [this](std::shared_ptr<DataStruct>& buffer) {
    operation2(buffer);
  };

}

// Dummy Operation
void DummyOperation::operation1(std::shared_ptr<DataStruct>& buffer){
  // Loop over all data in buffer
  for (size_t i = 0; i < buffer->raw_len; ++i) {
    // Increment each data element by 1
    buffer->raw[i] += 1; 
  }  
}

// Dummy Operation
void DummyOperation::operation2(std::shared_ptr<DataStruct>& buffer){
  // Loop over all data in buffer
  for (size_t i = 0; i < buffer->raw_len; ++i) {
    // Increment each data element by 1
    buffer->raw[i] += 2; 
  }  
}

