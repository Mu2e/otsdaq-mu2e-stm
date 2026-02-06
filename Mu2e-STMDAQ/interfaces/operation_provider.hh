#ifndef OPERATION_PROVIDER_HH
#define OPERATION_PROVIDER_HH

#include <vector>
#include <string>
#include <utility>

#include "Mu2e-STMDAQ/processing/operations_base.hh"

class IOperationProvider {
public:
  virtual ~IOperationProvider() = default;

  // Pure virtual function
  virtual const std::vector<std::pair<std::string, op_any>>& getUseOps() const = 0;

  virtual std::shared_ptr<OperationBase>& get_class(std::string name) = 0;

  virtual size_t class_num() const = 0;
  
};

#endif
