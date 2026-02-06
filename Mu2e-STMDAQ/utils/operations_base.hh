#ifndef OPERATION_BASE_HH
#define OPERATION_BASE_HH

#include <memory>
#include <iostream>
#include <unordered_map>
#include <functional>

// Ring Buffer code
#include "Mu2e-STMDAQ/utils/ring_buffer.hh" 

// Base class for operations
class OperationBase {
public:
  //  virtual void execute(std::shared_ptr<DataStruct>& buffer) = 0; // Pure virtual function
  virtual void execute(const std::string& methodName, std::shared_ptr<DataStruct>& buffer) = 0;
  virtual ~OperationBase() = default; // Virtual destructor for polymorphism
};

#endif
