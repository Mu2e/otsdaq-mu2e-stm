#ifndef TEST_FUNCS_HH_
#define TEST_FUNCS_HH_

#include <stdio.h>
#include <string.h>

// Operations base header
#include "Mu2e-STMDAQ/utils/operations_base.hh" 

// TestFuncs Class for socket setup and data reception
class TestFuncs : public OperationBase {
  
private:

  // FOR TESTING ONLY
  std::vector<int16_t> dummy_data;
  const int len = (60*1024*1024)/2;   
  
  // A function map for the process manager
  std::unordered_map<std::string, std::function<void(std::shared_ptr<DataStruct>&)>> functionMap;

  // Incrementing counter check
  bool first_counter = true;
  int16_t counter_check = 0;

  // ZS incrementing counter checks
  int16_t low_check = 0;
  int16_t high_check = 0;
  bool low_check_failed = false;
  bool high_check_failed = false;
  
  // Buffer counter
  int buffer_num = 0;
  
public:

  // Constructor
  TestFuncs();

  // Destructor
  ~TestFuncs() {
    std::cout << "TestFuncs destructor called." << std::endl;
  }

  // FOR TESTING ONLY
  void genData(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void addOne(std::shared_ptr<DataStruct>& buffer);
  
  // FOR TESTING ONLY
  void confirmData(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void doNothing(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void print_packets(std::shared_ptr<DataStruct>& buffer);
  
  // FOR TESTING ONLY
  void print(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void check_form_events(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void check_prep_zs(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void check_zs(std::shared_ptr<DataStruct>& buffer);

  // Execute function for ProcessManager
  void execute(const std::string& methodName, std::shared_ptr<DataStruct>& buffer) override {
    if (functionMap.find(methodName) != functionMap.end()) {
      functionMap[methodName](buffer);
    } else {
      std::cerr << "Error: Invalid method name '" << methodName << "' in TestFuncs\n";
    }
  }
  
};


#endif 
