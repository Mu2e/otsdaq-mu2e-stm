#ifndef ZERO_SUPPRESS_hh_
#define ZERO_SUPPRESS_hh_ 

// Zero suppression class
class ZeroSuppress {

private:

public:

  // Constructor
  ZeroSuppress();

  // Destructor for logging (not actually needed with shared_ptr)
  ~ZeroSuppress() { 
    std::cout << "ZeroSuppress destructor called.\n";
  }

  // Example data operation
  void operation(const std::shared_ptr<DataStruct>& buffer);
  
};

#endif
