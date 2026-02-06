#ifndef RING_BUFFER_HH_
#define RING_BUFFER_HH_

#include <boost/lockfree/spsc_queue.hpp>
#include <memory>
#include <atomic>
#include <iostream>

// Data Struct
#include "Mu2e-STMDAQ/utils/data_struct.hh"


template<typename T>
class RingBuffer {

private:

  // The lock-free queue
  boost::lockfree::spsc_queue<std::shared_ptr<T>> queue;

  // Last popped item (for DQM)
  mutable std::shared_ptr<T> last_popped_item;

  // Prevent multiple threads from clearing simultaneously
  std::atomic<bool> clearing {false};

public:

  // Constructor to set queue capacity
  RingBuffer(size_t queue_capacity_);

  // Destructor to ensure cleanup
  ~RingBuffer() {
    clearQueue();
    std::cout << "RingBuffer destructor called." << std::endl;
  }

  // Push item to buffer
  void push(const std::shared_ptr<T>& item);

  // Pop item from buffer
  std::shared_ptr<T> try_pop();

  // Copy item from buffer without removing it
  std::shared_ptr<T> try_pop_copy();

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

#endif // RING_BUFFER_HH_


// // Lock-free ring buffer supporting multiple producers & consumers
// class RingBuffer {
  
// private:

//   // The lock-free queue
//   boost::lockfree::spsc_queue<std::shared_ptr<DataStruct>> queue;
//   // Last popped iterm (for DQM)
//   mutable std::shared_ptr<DataStruct> last_popped_item;

//   // Prevent multiple threads from clearing simultaneously
//   std::atomic<bool> clearing {false};  
  
// public:
  
//   // Constructor to set queue capacity
//   RingBuffer(size_t queue_capacity_);

//   // Destructor to ensure cleanup
//   ~RingBuffer() {
//     clearQueue();
//     std::cout << "RingBuffer destructor called." << std::endl;
//   }
  
//   // Push item to buffer 
//   void push(const std::shared_ptr<DataStruct>& item);
  
//   // Pop item from buffer
//   std::shared_ptr<DataStruct> try_pop();
  
//   // Copy item from buffer without removing it
//   std::shared_ptr<DataStruct> try_pop_copy();

//   // Clear buffer queue to free memory
//   void clearQueue();
  
//   // Check if buffer is empty
//   bool is_empty(){
//     return queue.empty();
//   }
  
//   // Get current buffer size
//   size_t size(){
//     return queue.read_available();
//   }
  
// };

// #endif // RING_BUFFER_HH_
