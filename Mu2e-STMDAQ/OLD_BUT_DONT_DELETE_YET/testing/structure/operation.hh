#ifndef DUMMY_OPERATION_hh_
#define DUMMY_OPERATION_hh_ 

// Dummy data operation class
class DummyOperation {

private:

public:

  // Constructor
  DummyOperation();

  // Destructor for logging (not actually needed with shared_ptr)
  ~DummyOperation() { 
    std::cout << "DummyOperation destructor called.\n";
  }

  // Example data operation
  void operation(const std::shared_ptr<DataStruct>& buffer);
  //  void operation();
  
};

#endif
