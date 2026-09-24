#ifndef BUFFER_POOL_hh_
#define BUFFER_POOL_hh_

// Ring buffer code
#include "ring_buffer.hh" 

// Pre-allocated buffer pool for memory reuse
class BufferPool {

private:

  // Pool of pre-allocated buffers
  std::vector<std::shared_ptr<DataStruct>> pool;
  // Mutex for thread safety
  std::mutex mutex;

public:

  // Constructor to initialize buffer pool
  BufferPool(size_t pool_size, size_t buffer_size);

  // Destructor for logging
  ~BufferPool() { 
    std::cout << "BufferPool destructor called.\n";
  }

  // Acquire a buffer from the pool
  std::shared_ptr<DataStruct> acquire();

  // Return a buffer to the pool
  void release(std::shared_ptr<DataStruct> buffer);
  
};

#endif 
