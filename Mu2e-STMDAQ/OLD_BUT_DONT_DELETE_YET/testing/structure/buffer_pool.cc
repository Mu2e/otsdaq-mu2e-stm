// Buffer pool header
#include "buffer_pool.hh" 

// Constructor
BufferPool::BufferPool(size_t pool_size, size_t buffer_size) {
  
  // Create specified number of buffers
  for (size_t i = 0; i < pool_size; ++i) {
    // Add buffers to pool
    pool.emplace_back(std::make_shared<DataStruct>(buffer_size));
  }
  
}

// Acquire a buffer from the pool
std::shared_ptr<DataStruct> BufferPool::acquire() {

  // Lock for thread safety
  std::lock_guard<std::mutex> lock(mutex);

  // If pool is empty
  if (pool.empty()) {
    // Return nullptr to signal no available buffer
    return nullptr; 
  }
  else {
    // Retrieve buffer from the pool
    auto buffer = pool.back();
    // Remove buffer from pool
    pool.pop_back();
    // Return the buffer
    return buffer;
  }
  
}

// Return a buffer to the pool
void BufferPool::release(std::shared_ptr<DataStruct> buffer) {

  // Lock for thread safety 
  std::lock_guard<std::mutex> lock(mutex);
  // Add buffer back to pool
  pool.push_back(buffer);
  
}
