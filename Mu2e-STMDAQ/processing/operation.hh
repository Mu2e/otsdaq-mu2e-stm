#ifndef DUMMY_OPERATION_HH_
#define DUMMY_OPERATION_HH_

#include <stdio.h>
#include <string.h>

// Operations base header
#include "Mu2e-STMDAQ/utils/operations_base.hh" 

// DummyOperation Class 
class DummyOperation : public OperationBase {
  
private:

  // A function map for the process manager
  std::unordered_map<std::string, std::function<void(std::shared_ptr<DataStruct>&)>> functionMap;
  
public:

  // Constructor
  DummyOperation();

  // Destructor
  ~DummyOperation() {
    std::cout << "DummyOperation destructor called." << std::endl;
  }

  // FOR TESTING ONLY
  void operation1(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void operation2(std::shared_ptr<DataStruct>& buffer);
  
  // Execute function for ProcessManager
  void execute(const std::string& methodName, std::shared_ptr<DataStruct>& buffer) override {
    if (functionMap.find(methodName) != functionMap.end()) {
      functionMap[methodName](buffer);
    } else {
      std::cerr << "Error: Invalid method name '" << methodName << "' in DummyOperation\n";
    }
  }
  
};


#endif 
