#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <cstring>
#include <array>
#include <functional>
#include <chrono>
#include <iomanip>
#include <sched.h>
#include <unistd.h>

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
template <typename T>
class RingBuffer {
private:
  std::vector<std::shared_ptr<T>> buffer;
  size_t head = 0;
  size_t tail = 0;
  size_t capacity;
  std::atomic<size_t> size = 0;
  std::mutex mutex;
  std::condition_variable cv_push;
  std::condition_variable cv_pop;
  bool stop = false;

public:
  explicit RingBuffer(size_t max_size) : capacity(max_size), buffer(max_size) {}

  ~RingBuffer() { // Destructor for logging
    std::cout << "RingBuffer destructor called.\n";
  }

  void push(const std::shared_ptr<T>& item) {
    std::unique_lock<std::mutex> lock(mutex);
    cv_push.wait(lock, [this]() { return stop || size < capacity; });
    if (stop) return;
    buffer[head] = item;
    head = (head + 1) % capacity;
    ++size;
    cv_pop.notify_one();
  }

  std::shared_ptr<T> pop() {
    std::unique_lock<std::mutex> lock(mutex);
    cv_pop.wait(lock, [this]() { return stop || size > 0; });
    if (size == 0) return nullptr;
    auto item = buffer[tail];
    tail = (tail + 1) % capacity;
    --size;
    cv_push.notify_one();
    return item;
  }

  void shutdown() {
    {
      std::lock_guard<std::mutex> lock(mutex);
      stop = true;
    }
    cv_push.notify_all();
    cv_pop.notify_all();
  }

  bool is_empty() {
    std::lock_guard<std::mutex> lock(mutex);
    return size == 0;
  }
};

// Pre-allocated buffer pool for memory reuse
class BufferPool {
private:
  std::vector<std::shared_ptr<DataStruct>> pool;
  std::mutex mutex;

public:
  BufferPool(size_t pool_size, size_t buffer_size) {
    for (size_t i = 0; i < pool_size; ++i) {
      pool.emplace_back(std::make_shared<DataStruct>(buffer_size));
    }
  }

  ~BufferPool() { // Destructor for logging
    std::cout << "BufferPool destructor called.\n";
  }

  // std::shared_ptr<DataStruct> acquire() {
  //     std::lock_guard<std::mutex> lock(mutex);
  //     if (pool.empty()) {
  //         return std::make_shared<DataStruct>(60 * 1024 * 1024 / sizeof(int16_t));
  //     } else {
  //         auto buffer = pool.back();
  //         pool.pop_back();
  //         return buffer;
  //     }
  // }

  std::shared_ptr<DataStruct> acquire() {
    std::lock_guard<std::mutex> lock(mutex);
    if (pool.empty()) {
      return nullptr; // Return nullptr to signal no available buffer
    } else {
      auto buffer = pool.back();
      pool.pop_back();
      return buffer;
    }
  }

  void release(std::shared_ptr<DataStruct> buffer) {
    std::lock_guard<std::mutex> lock(mutex);
    pool.push_back(buffer);
  }
};

// Global total data tracker
std::atomic<uint64_t> globalTotalBytes{0};
const uint64_t targetTotalBytes = 10L * 1024 * 1024 * 1024; // 10 GB
std::atomic<bool> kernelDone{false}; // Signal when kernelBufferWorker is finished
std::vector<std::atomic<bool>> threadDone(7); // Flags to signal when each thread has finished

// Pin thread to specific CPU core
void pin_thread_to_core(size_t core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

// General worker function for processing stages
void workerThread(RingBuffer<DataStruct>& inputQueue, RingBuffer<DataStruct>& outputQueue, BufferPool& pool, const char* stage, std::atomic<uint64_t>& totalBytesProcessed, size_t core_id, std::function<void(std::shared_ptr<DataStruct>&)> operation, size_t threadIndex) {
  pin_thread_to_core(core_id);
  auto start_time = std::chrono::high_resolution_clock::now();

  while (!kernelDone || !inputQueue.is_empty() || !threadDone[threadIndex - 1].load()) {
    auto buffer = inputQueue.pop();
    if (buffer) {
      operation(buffer); // Perform the operation on the buffer

      // Append stage-specific metadata
      strncat(buffer->metadata, stage, sizeof(buffer->metadata) - strlen(buffer->metadata) - 1);

      uint64_t bytesProcessed = buffer->size * sizeof(int16_t);
      totalBytesProcessed += bytesProcessed;

      outputQueue.push(buffer); // Push the processed buffer to the next stage
    }
  }

  // Signal completion to the next thread
  threadDone[threadIndex].store(true);

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;
  double gbits = totalBytesProcessed.load() * 8.0 / 1e9; // Convert to Gbits
  std::cout << "[" << stage << "] Processing speed: " << std::fixed << std::setprecision(2) << (gbits / elapsed.count()) << " Gbit/s\n";
}

// Kernel buffer simulation function
void kernelBufferWorker(RingBuffer<DataStruct>& outputQueue, BufferPool& pool, size_t core_id) {
  pin_thread_to_core(core_id);

  while (globalTotalBytes < targetTotalBytes) {
    auto buffer = pool.acquire();
    if (!buffer) { // Wait for an available buffer
      std::this_thread::yield();
      continue;
    }
    buffer->size = (rand() % (60 * 1024 * 1024 / sizeof(int16_t))) + 1000; // Simulate variable size
    memset(buffer->data.data(), rand() % 100, buffer->size * sizeof(int16_t));
    strncpy(buffer->metadata, "Generated by kernelBufferWorker", sizeof(buffer->metadata));

    uint64_t bytesGenerated = buffer->size * sizeof(int16_t);
    globalTotalBytes += bytesGenerated;

    outputQueue.push(buffer);
  }
  kernelDone = true; // Signal that kernel is done producing data

  // Signal the first worker thread
  threadDone[0].store(true);
}

// Consumer thread to ensure the final buffer is emptied
void consumerThread(RingBuffer<DataStruct>& inputQueue, BufferPool& pool, size_t core_id, size_t threadIndex) {
  pin_thread_to_core(core_id);
  while (!kernelDone || !inputQueue.is_empty() || !threadDone[threadIndex - 1].load()) {
    auto buffer = inputQueue.pop();
    if (!buffer) continue; // Wait for remaining data
    pool.release(buffer);
  }

  // Signal completion
  threadDone[threadIndex].store(true);

  std::cout << "[Consumer] Final buffer emptied.\n";
}

int main() {
  const size_t max_queue_size = 20; // Increased maximum size for each ring buffer
  const size_t buffer_pool_size = 30; // Increased pre-allocated buffer pool size
  const size_t buffer_size = 60 * 1024 * 1024 / sizeof(int16_t); // Buffer size in elements

  // Create buffer pool
  BufferPool pool(buffer_pool_size, buffer_size);

  // Define ring buffers for each step
  RingBuffer<DataStruct> kernelBufferQueue(max_queue_size);
  RingBuffer<DataStruct> checkDataQueue(max_queue_size);
  RingBuffer<DataStruct> formEventsQueue(max_queue_size);
  RingBuffer<DataStruct> rawBufferQueue(max_queue_size);
  RingBuffer<DataStruct> zsBufferQueue(max_queue_size);
  RingBuffer<DataStruct> mwdBufferQueue(max_queue_size);

  // Track total bytes processed for each thread
  std::atomic<uint64_t> totalBytesKernel{0};
  std::atomic<uint64_t> totalBytesCheck{0};
  std::atomic<uint64_t> totalBytesForm{0};
  std::atomic<uint64_t> totalBytesRaw{0};
  std::atomic<uint64_t> totalBytesZS{0};
  std::atomic<uint64_t> totalBytesMWD{0};

  // Initialize thread completion flags
  //    threadDone = std::vector<std::atomic_bool>(7, false);
  // threadDone.resize(7);
  for (auto& flag : threadDone) {
    flag.store(false);
  }

  // Example operations for each worker thread
  auto operationExample = [](std::shared_ptr<DataStruct>& buffer) {
    for (size_t i = 0; i < buffer->size; ++i) {
      buffer->data[i] += 1; // Increment each data element by 1
    }
  };

  // zsProcessThread operation function                                         
  auto zsOperation = [](std::shared_ptr<DataStruct>& buffer) {                  
    buffer->data2.resize(buffer->data.size());                                  
    buffer->size2 = buffer->size;                                               
    for (size_t i = 0; i < buffer->size; ++i) {                                 
      buffer->data2[i] = buffer->data[i] * 2;                                    
    }                                                                           
  };                                                                            
                                                                                   
  // mwdProcessThread operation function                                        
  auto mwdOperation = [](std::shared_ptr<DataStruct>& buffer) {                 
    buffer->data3.resize(buffer->data2.size());                                 
    buffer->size3 = buffer->size2;                                              
    for (size_t i = 0; i < buffer->size2; ++i) {                                
      double temp = static_cast<double>(buffer->data2[i]) / 2.0;                 
      buffer->data3[i] = static_cast<int16_t>(temp);                             
    }                                                                           
  }; 
    
  // Launch worker threads for each stage
  std::thread kernelThread(kernelBufferWorker, std::ref(kernelBufferQueue), std::ref(pool), 0);
  std::thread checkDataThread(workerThread, std::ref(kernelBufferQueue), std::ref(checkDataQueue), std::ref(pool), " | Checked", std::ref(totalBytesCheck), 1, operationExample, 1);
  std::thread formEventsThread(workerThread, std::ref(checkDataQueue), std::ref(formEventsQueue), std::ref(pool), " | Events Formed", std::ref(totalBytesForm), 2, operationExample, 2);
  std::thread rawProcessThread(workerThread, std::ref(formEventsQueue), std::ref(rawBufferQueue), std::ref(pool), " | Algorithm Processed", std::ref(totalBytesRaw), 3, operationExample, 3);
  std::thread zsProcessThread(workerThread, std::ref(rawBufferQueue), std::ref(zsBufferQueue), std::ref(pool), " | ZS Processed", std::ref(totalBytesZS), 4, zsOperation, 4);
  std::thread mwdProcessThread(workerThread, std::ref(zsBufferQueue), std::ref(mwdBufferQueue), std::ref(pool), " | MWD Processed", std::ref(totalBytesMWD), 5, mwdOperation, 5);
  std::thread finalConsumerThread(consumerThread, std::ref(mwdBufferQueue), std::ref(pool), 6, 6);

  // Wait for threads to complete
  kernelThread.join();
  checkDataThread.join();
  formEventsThread.join();
  rawProcessThread.join();
  zsProcessThread.join();
  mwdProcessThread.join();
  finalConsumerThread.join();

  // Shutdown all queues
  kernelBufferQueue.shutdown();
  checkDataQueue.shutdown();
  formEventsQueue.shutdown();
  rawBufferQueue.shutdown();
  zsBufferQueue.shutdown();
  mwdBufferQueue.shutdown();

  return 0;
}
