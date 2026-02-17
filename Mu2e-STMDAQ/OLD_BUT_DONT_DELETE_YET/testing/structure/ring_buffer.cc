// Include header
#include "ring_buffer.hh"

// Add an item to the buffer
template <typename T> void RingBuffer<T>::push(const std::shared_ptr<T>& item) {
  std::unique_lock<std::mutex> lock(mutex); // Lock for thread safety
  cv_push.wait(lock, [this]() { return stop || size < capacity; }); // Wait if buffer is full
  if (stop) return; // Exit if stop flag is set
  buffer[head] = item; // Add item to buffer
  head = (head + 1) % capacity; // Update head index
  ++size; // Increment buffer size
  cv_pop.notify_one(); // Notify consumers
}

// Remove and return an item from the buffer
template <typename T> std::shared_ptr<T> RingBuffer<T>::pop() {
  std::unique_lock<std::mutex> lock(mutex); // Lock for thread safety
  cv_pop.wait(lock, [this]() { return stop || size > 0; }); // Wait if buffer is empty
  if (size == 0) return nullptr; // Return null if buffer is empty
  auto item = buffer[tail]; // Retrieve item from buffer
  tail = (tail + 1) % capacity; // Update tail index
  --size; // Decrement buffer size
  cv_push.notify_one(); // Notify producers
  return item; // Return the item
}

// Stop buffer operations
template <typename T> void RingBuffer<T>::shutdown() {
  {
    std::lock_guard<std::mutex> lock(mutex); // Lock for thread safety
    stop = true; // Set stop flag
  }
  cv_push.notify_all(); // Notify all waiting producers
  cv_pop.notify_all(); // Notify all waiting consumers
}

// Check if buffer is empty
template <typename T> bool RingBuffer<T>::is_empty() {
  std::lock_guard<std::mutex> lock(mutex); // Lock for thread safety
  return size == 0; // Return true if buffer size is zero
}

template class RingBuffer<DataStruct>; 
