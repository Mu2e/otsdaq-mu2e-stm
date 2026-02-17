// Include header
#include "Mu2e-STMDAQ/utils/ring_buffer.hh"

// Constructor to initialize buffer size
template<typename T>
RingBuffer<T>::RingBuffer(size_t queue_capacity_) : queue(queue_capacity_)  {}

// Try to push an item into the buffer
template<typename T>
void RingBuffer<T>::push(const std::shared_ptr<T>& item) {
  while (!queue.push(item)) {
    std::this_thread::sleep_for(std::chrono::microseconds(10)); // Wait and retry
  }
}

// Try to remove an item from the buffer
template<typename T>
std::shared_ptr<T> RingBuffer<T>::try_pop() {
    std::shared_ptr<T> item;
    if (queue.pop(item)){
      if (!item) {  // Prevent using invalid memory
        std::cerr << "Error: Popped a null buffer!" << std::endl;
        return nullptr;
      }
      last_popped_item = item;
      return item;
    }
    return nullptr;
}

// Try to copy an item from the buffer without removing it
template<typename T>
std::shared_ptr<T> RingBuffer<T>::try_pop_copy(){
  return last_popped_item;
}

// Clear buffer queue to free memory
template<typename T>
void RingBuffer<T>::clearQueue() {

  // Check if the pool exists before attempting to clear it
  bool expected = false;
  // Atomically check and set the `clearing` flag to prevent concurrent access
  if (!clearing.compare_exchange_strong(expected, true)) {
    return;  // Another thread is already clearing, exit
  }

  // Temporary buffer for popping elements
  std::shared_ptr<T> item = nullptr;

  // Manual counter for the number of elements
  int count = 0;

  // Pop all remaining elements
  while (queue.pop(item)) {
    if(item){
      item.reset();  // Explicitly reset shared_ptr
      count++;
    }
  }

  // Print the number of cleared elements
  std::cout << "Cleared " << count << " elements from RingBuffer queue." << std::endl;

  // Reset the unique_ptr, deallocating the stack
  queue.reset();

  // Set pool to nullptr to prevent accidental use
  last_popped_item.reset();
}

// Explicit template instantiation for DataStruct type
template class RingBuffer<DataStruct>;

// Explicit template instantiation for int16_t vector type
template class RingBuffer<std::vector<int16_t>>;

