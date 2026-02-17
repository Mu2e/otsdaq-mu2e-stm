#ifndef DQM_BLOCK_WRITER_HH
#define DQM_BLOCK_WRITER_HH

// Include Boost libraries for shared memory
#include <boost/interprocess/shared_memory_object.hpp>  // for shared_memory_object
#include <boost/interprocess/mapped_region.hpp>         // for mapped_region
#include <cstring>   // for memset
#include <iostream>  // for std::cerr (optional logging)

namespace bip = boost::interprocess;  // alias for convenience

// Template class to manage a block of shared memory that stores a structure of type T
template <typename T>
class DQMBlockWriter {
public:
  
  // Constructor: takes the shared memory name and its size in bytes
  DQMBlockWriter(const std::string& shm_name, size_t shm_size)
    : shm_name(shm_name), shm_size(shm_size) {
    
    // First, remove any existing shared memory segment with the same name
    bip::shared_memory_object::remove(shm_name.c_str());

    // Create a new shared memory object with read/write permissions
    shm = bip::shared_memory_object(bip::open_or_create, shm_name.c_str(), bip::read_write);

    // Set the size of the shared memory segment
    shm.truncate(shm_size);

    // Map the shared memory into this process's address space
    region = bip::mapped_region(shm, bip::read_write);

    // Zero-initialize the entire shared memory region
    std::memset(region.get_address(), 0, shm_size);
  }

  // Destructor: remove the shared memory segment when this object goes out of scope
  ~DQMBlockWriter() {
    bip::shared_memory_object::remove(shm_name.c_str());
  }

  // Accessor: returns a typed pointer to the shared memory region
  T* get() {
    return static_cast<T*>(region.get_address());
  }

private:
  std::string shm_name;                   // Name of the shared memory block
  size_t shm_size;                        // Size in bytes
  bip::shared_memory_object shm;          // Shared memory object handle
  bip::mapped_region region;              // Memory-mapped view of the shared memory
};

#endif // DQM_BLOCK_WRITER_HH
