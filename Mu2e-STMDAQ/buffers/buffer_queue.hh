#ifndef BUFFER_HH_
#define BUFFER_HH_

#include <boost/lockfree/spsc_queue.hpp>
#include <memory>
#include <atomic>
#include <iostream>

// STM data header
#include "Mu2e-STMDAQ/buffers/data_struct.hh"

template<typename T>
class BufferQueue {

private:

  // The lock-free queue
  boost::lockfree::spsc_queue<std::shared_ptr<T>> queue;

  // Last popped item (for DQM)
  mutable std::shared_ptr<T> last_popped_item;

  // Prevent multiple threads from clearing simultaneously
  std::atomic<bool> clearing {false};

public:

  // Constructor to set queue capacity
  BufferQueue(size_t queue_capacity_);

  // Destructor to ensure cleanup
  ~BufferQueue() {
    clearQueue();
    //std::cout << "BufferQueue destructor called." << std::endl;
  }

  // Push item to buffer
  void push(const std::shared_ptr<T>& item);

  // Pop item from buffer
  std::shared_ptr<T> try_pop();

  // Clear buffer queue to free memory
  void clearQueue();

  // Check if buffer is empty
  bool is_empty(){
    return queue.empty();
  }
  
  // Get current buffer size
  size_t size(){
    return queue.read_available();
  }

  
};

#endif // BUFFER_HH_
