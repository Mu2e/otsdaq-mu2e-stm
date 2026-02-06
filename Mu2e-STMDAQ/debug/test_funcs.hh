#ifndef TEST_FUNCS_HH_
#define TEST_FUNCS_HH_

#include <stdio.h>
#include <string.h>

// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh" 
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"

// TestFuncs Class for socket setup and data reception
class TestFuncs : public OperationMap {
  
private:

  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;
  
  // FOR TESTING ONLY
  std::vector<int16_t> dummy_data;
  const int len = (60*1024*1024)/2;   

  // Last buffer seen
  std::shared_ptr<DataStruct> prev_buffer = nullptr;  
  size_t prev_buffer_raw_len = 0;
  size_t read_buffer_count = 0;
  size_t mod_buffer_count = 0;
  size_t prev_buffer_tot = 0;
  size_t this_buffer_tot = 0;
  bool first_buffer_mod = true;
  bool first_buffer_read = true;
  
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
  TestFuncs(const std::shared_ptr<SignalHandler>& signal_);

  // Destructor
  ~TestFuncs() {
    std::cout << "TestFuncs destructor called." << std::endl;
  }

  // FOR TESTING ONLY
  void mod_prev_buffer(std::shared_ptr<DataStruct>& buffer);

  void read_mod_buffer(std::shared_ptr<DataStruct>& buffer);
  
  // FOR TESTING ONLY
  void genData(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void addOne(std::shared_ptr<DataStruct>& buffer);
  
  // FOR TESTING ONLY
  void confirmData(std::shared_ptr<DataStruct>& buffer);

  // FOR TESTING ONLY
  void do_nothing(std::shared_ptr<DataStruct>& buffer);

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
  
};


#endif 
