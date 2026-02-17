#ifndef RING_BUFFER_hh_
#define RING_BUFFER_hh_ 

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <atomic>

// Global total data tracker
static std::atomic<uint64_t> globalTotalBytes{0};
//static const uint64_t targetTotalBytes = 10L * 1024 * 1024 * 1024; // 10 GB
static const uint64_t targetTotalBytes = 28800L * 1024 * 1024 * 1024; // 10 GB

// Data structure representing a buffer with metadata
struct DataStruct {
  std::vector<int16_t> data; // Dynamically sized data buffer
  std::vector<int16_t> data2; // Dynamically sized data buffer
  std::vector<int16_t> data3; // Dynamically sized data buffer
  size_t size;              // Actual size of the data used
  size_t size2;              // Actual size of the data used
  size_t size3;              // Actual size of the data used
  char metadata[128];       // Fixed-size metadata array

  DataStruct(size_t buffer_size)
    //    : data(buffer_size), size(0) {
    : data(buffer_size), data2(), data3(), size(0), size2(0), size3(0) {
    std::fill(std::begin(metadata), std::end(metadata), '\0');
  }
};

// Ring buffer for thread-safe buffer management
template <typename T> class RingBuffer {

private:

  // Circular buffer storage
  std::vector<std::shared_ptr<T>> buffer;
  // Index of next insertion point
  size_t head = 0;
  // Index of next removal point
  size_t tail = 0;
  // Maxaimum capacity of buffer
  size_t capacity;
  // Current size of buffer
  std::atomic<size_t> size = 0;
  // Mutex for synchronisation
  std::mutex mutex;
  // Conditional variable for producers
  std::condition_variable cv_push;
  // Conditiona variable for consumers
  std::condition_variable cv_pop;
  // Flag to stop buffer operations
  bool stop = false;

public:

  // Constructor to initialise buffer
  explicit RingBuffer(size_t max_size) : capacity(max_size), buffer(max_size) {}

  // Destructor for logging (not actually needed with shared_ptr)
  ~RingBuffer() { 
    std::cout << "RingBuffer destructor called.\n";
  }

  // Add an item to the buffer
  void push(const std::shared_ptr<T>& item);

  // Remove and return an item from the buffer
  std::shared_ptr<T> pop();

  // Stop buffer operations
  void shutdown();

  // Check if buffer is empty
  bool is_empty();
  
};

#endif
