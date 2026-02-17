#ifndef _MWD_hh_
#define _MWD_hh_ 

// MWD class
class MWD {

private:

public:

  // Constructor
  MWD();

  // Destructor for logging (not actually needed with shared_ptr)
  ~MWD() { 
    std::cout << "MWD destructor called.\n";
  }

  // Example data operation
  void operation(const std::shared_ptr<DataStruct>& buffer);
  
};

#endif
