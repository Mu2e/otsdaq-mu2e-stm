#define _GNU_SOURCE // Enable GNU-specific features
#include <iostream> // Include for standard input/output operations
#include <thread> // Include for thread support
#include <mutex> // Include for mutex synchronization
#include <condition_variable> // Include for condition variables
#include <vector> // Include for dynamic arrays
#include <queue> // Include for queue data structure
#include <atomic> // Include for atomic variables
#include <memory> // Include for smart pointers
#include <cstring> // Include for memory operations
#include <array> // Include for fixed-size arrays
#include <functional> // Include for function objects
#include <chrono> // Include for timing utilities
#include <iomanip> // Include for formatting output
#include <sched.h> // Include for CPU affinity management
#include <unistd.h> // Include for POSIX system calls

// Data structure representing a buffer with metadata
struct DataStruct {
  
  std::vector<int16_t> data; // Dynamically sized data buffer
  size_t size;              // Actual size of the data used
  char metadata[128];       // Fixed-size metadata array

  DataStruct(size_t buffer_size) : data(buffer_size), size(0) { // Constructor to initialize data buffer
    std::fill(std::begin(metadata), std::end(metadata), '\0'); // Initialize metadata to null characters
  }

};

// Ring buffer for thread-safe buffer management
template <typename T> class RingBuffer {

private:

  std::vector<std::shared_ptr<T>> buffer; // Circular buffer storage
  size_t head = 0; // Index of the next insertion point
  size_t tail = 0; // Index of the next removal point
  size_t capacity; // Maximum capacity of the buffer
  std::atomic<size_t> size = 0; // Current size of the buffer
  std::mutex mutex; // Mutex for synchronization
  std::condition_variable cv_push; // Condition variable for producers
  std::condition_variable cv_pop; // Condition variable for consumers
  bool stop = false; // Flag to stop buffer operations

public:

  // Constructor to initialize buffer
  explicit RingBuffer(size_t max_size) : capacity(max_size), buffer(max_size) {} 

  // Destructor for logging
  ~RingBuffer() { 
    std::cout << "RingBuffer destructor called.\n";
  }
  
  // Add an item to the buffer
  void push(const std::shared_ptr<T>& item) { 
    std::unique_lock<std::mutex> lock(mutex); // Lock for thread safety
    cv_push.wait(lock, [this]() { return stop || size < capacity; }); // Wait if buffer is full
    if (stop) return; // Exit if stop flag is set
    buffer[head] = item; // Add item to buffer
    head = (head + 1) % capacity; // Update head index
    ++size; // Increment buffer size
    cv_pop.notify_one(); // Notify consumers
  }

  // Remove and return an item from the buffer
  std::shared_ptr<T> pop() { 
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
  void shutdown() { 
    {
      std::lock_guard<std::mutex> lock(mutex); // Lock for thread safety
      stop = true; // Set stop flag
    }
    cv_push.notify_all(); // Notify all waiting producers
    cv_pop.notify_all(); // Notify all waiting consumers
  }

  bool is_empty() { // Check if buffer is empty
    std::lock_guard<std::mutex> lock(mutex); // Lock for thread safety
    return size == 0; // Return true if buffer size is zero
  }
};

// Pre-allocated buffer pool for memory reuse
class BufferPool {
  
private:
  std::vector<std::shared_ptr<DataStruct>> pool; // Pool of pre-allocated buffers
  std::mutex mutex; // Mutex for thread safety

public:

  // Constructor to initialize buffer pool
  BufferPool(size_t pool_size, size_t buffer_size) { 
    for (size_t i = 0; i < pool_size; ++i) { // Create specified number of buffers
      pool.emplace_back(std::make_shared<DataStruct>(buffer_size)); // Add buffers to pool
    }
  }

  // Destructor for logging
  ~BufferPool() { 
    std::cout << "BufferPool destructor called.\n";
  }

  // Acquire a buffer from the pool
  std::shared_ptr<DataStruct> acquire() { 
    std::lock_guard<std::mutex> lock(mutex); // Lock for thread safety
    if (pool.empty()) { // If pool is empty
      return std::make_shared<DataStruct>(60 * 1024 * 1024 / sizeof(int16_t)); // Create a new buffer
    }
    else {
      auto buffer = pool.back(); // Retrieve buffer from the pool
      pool.pop_back(); // Remove buffer from pool
      return buffer; // Return the buffer
    }
  }

  // Return a buffer to the pool
  void release(std::shared_ptr<DataStruct> buffer) { 
    std::lock_guard<std::mutex> lock(mutex); // Lock for thread safety
    pool.push_back(buffer); // Add buffer back to pool
  }
  
};

// Global total data tracker
std::atomic<uint64_t> globalTotalBytes{0}; // Total bytes processed
const uint64_t targetTotalBytes = 1000L * 1024 * 1024 * 1024; // 10 GB target
std::atomic<bool> kernelDone{false}; // Signal when kernelBufferWorker is finished

// Pin thread to specific CPU core
void pin_thread_to_core(size_t core_id) {
  cpu_set_t cpuset; // Define CPU set
  CPU_ZERO(&cpuset); // Clear CPU set
  CPU_SET(core_id, &cpuset); // Add specified core to set
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset); // Set thread affinity
}

// General worker function for processing stages
void workerThread(RingBuffer<DataStruct>& inputQueue, RingBuffer<DataStruct>& outputQueue, BufferPool& pool, const char* stage, std::atomic<uint64_t>& totalBytesProcessed, size_t core_id) {
  pin_thread_to_core(core_id); // Bind thread to specific core
  auto start_time = std::chrono::high_resolution_clock::now(); // Start timing
  while (!kernelDone || !inputQueue.is_empty()) { // Continue until kernel is done and input queue is empty
    auto buffer = inputQueue.pop(); // Retrieve a buffer from the input queue
    if (buffer) { // If buffer is valid
      strncat(buffer->metadata, stage, sizeof(buffer->metadata) - strlen(buffer->metadata) - 1); // Append stage info to metadata
      uint64_t bytesProcessed = buffer->size * sizeof(int16_t); // Calculate bytes processed
      totalBytesProcessed += bytesProcessed; // Update total bytes processed
      outputQueue.push(buffer); // Push buffer to output queue
    }
  }
  auto end_time = std::chrono::high_resolution_clock::now(); // End timing
  std::chrono::duration<double> elapsed = end_time - start_time; // Calculate elapsed time
  double gbits = totalBytesProcessed.load() * 8.0 / 1e9; // Convert bytes to Gbits
  std::cout << "[" << stage << "] Processing speed: " << std::fixed << std::setprecision(2) << (gbits / elapsed.count()) << " Gbit/s\n"; // Print processing speed
}

// Kernel buffer simulation function
void kernelBufferWorker(RingBuffer<DataStruct>& outputQueue, BufferPool& pool, size_t core_id) {

  pin_thread_to_core(core_id); // Bind thread to specific core
  while (globalTotalBytes < targetTotalBytes) { // Continue until total bytes processed reaches target
    auto buffer = pool.acquire(); // Acquire a buffer from the pool
    buffer->size = (rand() % (60 * 1024 * 1024 / sizeof(int16_t))) + 1000; // Simulate variable buffer size
    memset(buffer->data.data(), rand() % 100, buffer->size * sizeof(int16_t)); // Fill buffer with simulated data
    strncpy(buffer->metadata, "Generated by kernelBufferWorker", sizeof(buffer->metadata)); // Add metadata
    uint64_t bytesGenerated = buffer->size * sizeof(int16_t); // Calculate bytes generated
    globalTotalBytes += bytesGenerated; // Update global total bytes processed
    outputQueue.push(buffer); // Push buffer to output queue
  }
  kernelDone = true; // Signal that kernel is done producing data
  
}

// Consumer thread to ensure the final buffer is emptied
void consumerThread(RingBuffer<DataStruct>& inputQueue, BufferPool& pool, size_t core_id) {
  pin_thread_to_core(core_id); // Bind thread to specific core
  while (!kernelDone || !inputQueue.is_empty()) { // Continue until kernel is done and input queue is empty
    auto buffer = inputQueue.pop(); // Retrieve a buffer from the input queue
    if (buffer) { // If buffer is valid
      pool.release(buffer); // Return buffer to pool
    }
  }
  std::cout << "[Consumer] Final buffer emptied.\n"; // Print completion message
}

int main() {

  const size_t max_queue_size = 20; // Set maximum size for each ring buffer
  const size_t buffer_pool_size = 30; // Set number of pre-allocated buffers
  const size_t buffer_size = 60 * 1024 * 1024 / sizeof(int16_t); // Set size of each buffer in elements

  BufferPool pool(buffer_pool_size, buffer_size); // Create buffer pool
  RingBuffer<DataStruct> kernelBufferQueue(max_queue_size); // Create kernel buffer queue
  RingBuffer<DataStruct> checkDataQueue(max_queue_size); // Create check data queue
  RingBuffer<DataStruct> formEventsQueue(max_queue_size); // Create form events queue
  RingBuffer<DataStruct> rawBufferQueue(max_queue_size); // Create raw buffer queue
  RingBuffer<DataStruct> zsBufferQueue(max_queue_size); // Create ZS buffer queue
  RingBuffer<DataStruct> mwdBufferQueue(max_queue_size); // Create MWD buffer queue

  std::atomic<uint64_t> totalBytesKernel{0}; // Initialize total bytes processed for kernel
  std::atomic<uint64_t> totalBytesCheck{0}; // Initialize total bytes processed for check data
  std::atomic<uint64_t> totalBytesForm{0}; // Initialize total bytes processed for form events
  std::atomic<uint64_t> totalBytesRaw{0}; // Initialize total bytes processed for raw buffer
  std::atomic<uint64_t> totalBytesZS{0}; // Initialize total bytes processed for ZS buffer
  std::atomic<uint64_t> totalBytesMWD{0}; // Initialize total bytes processed for MWD buffer

  std::thread kernelThread(kernelBufferWorker, std::ref(kernelBufferQueue), std::ref(pool), 0); // Launch kernel buffer worker thread
  std::thread checkDataThread(workerThread, std::ref(kernelBufferQueue), std::ref(checkDataQueue), std::ref(pool), " | Checked", std::ref(totalBytesCheck), 1); // Launch check data worker thread
  std::thread formEventsThread(workerThread, std::ref(checkDataQueue), std::ref(formEventsQueue), std::ref(pool), " | Events Formed", std::ref(totalBytesForm), 2); // Launch form events worker thread
  std::thread rawProcessThread(workerThread, std::ref(formEventsQueue), std::ref(rawBufferQueue), std::ref(pool), " | Algorithm Processed", std::ref(totalBytesRaw), 3); // Launch raw processing worker thread
  std::thread zsProcessThread(workerThread, std::ref(rawBufferQueue), std::ref(zsBufferQueue), std::ref(pool), " | ZS Processed", std::ref(totalBytesZS), 4); // Launch ZS processing worker thread
  std::thread mwdProcessThread(workerThread, std::ref(zsBufferQueue), std::ref(mwdBufferQueue), std::ref(pool), " | MWD Processed", std::ref(totalBytesMWD), 5); // Launch MWD processing worker thread
  std::thread finalConsumerThread(consumerThread, std::ref(mwdBufferQueue), std::ref(pool), 6); // Launch consumer thread

  // Wait for all threads to complete
  kernelThread.join(); // Wait for kernel thread to finish
  checkDataThread.join(); // Wait for check data thread to finish
  formEventsThread.join(); // Wait for form events thread to finish
  rawProcessThread.join(); // Wait for raw processing thread to finish
  zsProcessThread.join(); // Wait for ZS processing thread to finish
  mwdProcessThread.join(); // Wait for MWD processing thread to finish
  finalConsumerThread.join(); // Wait for consumer thread to finish

  // Shutdown all queues
  kernelBufferQueue.shutdown(); // Shut down kernel buffer queue
  checkDataQueue.shutdown(); // Shut down check data queue
  formEventsQueue.shutdown(); // Shut down form events queue
  rawBufferQueue.shutdown(); // Shut down raw buffer queue
  zsBufferQueue.shutdown(); // Shut down ZS buffer queue
  mwdBufferQueue.shutdown(); // Shut down MWD buffer queue

  std::cout << "[Main] All threads completed successfully.\n"; // Print final success message
  return 0; // Exit program
}
