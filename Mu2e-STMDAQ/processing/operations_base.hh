#ifndef OPERATIONS_BASE_HH
#define OPERATIONS_BASE_HH

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

// Adjust these includes to match your project layout
#include "Mu2e-STMDAQ/buffers/data_struct.hh"

// -----------------------------------------------------------------------------
// Abstract base for all operations
// -----------------------------------------------------------------------------
class OperationBase {
public:
  virtual ~OperationBase() = default;

  // OperationManager calls this with an operation name and buffer
  virtual void execute(const std::string& operation_name,
                       std::shared_ptr<DataStruct>& buffer) = 0;

  // Optional: operation can be executed with a second buffer as well
  virtual void execute(const std::string& operation_name,
                       std::shared_ptr<DataStruct>& buffer,
                       std::shared_ptr<DataStruct>& prev_buffer) = 0;

  
  virtual bool requires_two_buffers(const std::string& methodName) const = 0;
  
};

using op1 = std::function<void(std::shared_ptr<DataStruct>&)>;
using op2 = std::function<void(std::shared_ptr<DataStruct>&,
                               std::shared_ptr<DataStruct>&)>;
using op_any = std::variant<op1, op2>;

// -----------------------------------------------------------------------------
// Operation map + a shared execute() implementation
// -----------------------------------------------------------------------------
class OperationMap : public OperationBase {

protected:


  std::unordered_map<std::string, op_any> operation_map;

  // Derived classes register their operations in the constructor
  void register_operation(const std::string& name, op1 fn) {
    operation_map.emplace(name, op_any{std::move(fn)});
  }

  // Derived classes register their operations in the constructor
  void register_operation(const std::string& name, op2 fn) {
    operation_map.emplace(name, op_any{std::move(fn)});
  }

  // Optional: derived can override for better error messages
  virtual const char* op_name() const { return "Operation"; }

public:

  // Execute operation (1-buffer)
  void execute(const std::string& operation_name,
               std::shared_ptr<DataStruct>& buffer) override {
    auto it = operation_map.find(operation_name);
    if (it == operation_map.end()) {
      std::cerr << "Error: Invalid operation name '" << operation_name
                << "' in " << op_name() << "\n";
      return;
    }

    if (auto* f1 = std::get_if<op1>(&it->second)) {
      (*f1)(buffer);
      return;
    }

    // A 2-buffer op was registered, but only 1 buffer was provided
    std::cerr << "Error: Operation '" << operation_name << "' in " << op_name()
              << " requires 2 buffers.\n";
  }

  // Execute operation (2-buffer)
  void execute(const std::string& operation_name,
               std::shared_ptr<DataStruct>& buffer,
               std::shared_ptr<DataStruct>& prev_buffer) override {
    auto it = operation_map.find(operation_name);
    if (it == operation_map.end()) {
      std::cerr << "Error: Invalid operation name '" << operation_name
                << "' in " << op_name() << "\n";
      return;
    }

    if (auto* f2 = std::get_if<op2>(&it->second)) {
      (*f2)(buffer, prev_buffer);
      return;
    }

    // A 1-buffer op was registered; ignore prev_buffer safely
    auto* f1 = std::get_if<op1>(&it->second);
    (*f1)(buffer);
  }

  bool requires_two_buffers(const std::string& operation_name) const override {
    auto it = operation_map.find(operation_name);
    if (it == operation_map.end()) {
      return false;
    }
    return std::holds_alternative<op2>(it->second);
  }
  
  
};

#endif
