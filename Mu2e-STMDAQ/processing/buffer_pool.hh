#ifndef BUFFER_POOL_HH_
#define BUFFER_POOL_HH_

#include <boost/lockfree/stack.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Buffer queue code
#include "Mu2e-STMDAQ/buffers/buffer_queue.hh"
// Operation Manager code
#include "Mu2e-STMDAQ/processing/operation_manager.hh"  

// Pre-allocated buffer pool for memory reuse
class BufferPool {

private:

  // Store reference to CPU utils instance                                                            
  const std::shared_ptr<cpu_utils>& cpu;

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;
  
  // STM data info
  const std::shared_ptr<STMdata>& stm;

  // Lock-free stack for buffer storage
  std::unique_ptr<boost::lockfree::stack<std::shared_ptr<DataStruct>>> pool;

  // Single-producer, single-consumer queue for buffers needing reset
  std::unique_ptr<boost::lockfree::spsc_queue<std::shared_ptr<DataStruct>>> resetQueue;

  // Atomic flag for background processing
  std::atomic<bool> running{true};
  std::thread resetThread;
  
  // Atomic flag to prevent multiple threads from clearing the pool simultaneously
  std::atomic<bool> clearing {false};

  // Data buffer sizes and lengths
  const size_t raw_size, raw_len, zs_size, zs_len, ph_size, ph_len, baseline_size, baseline_len;
  
    // Maximum number of mu2e events in buffer
  const size_t max_event_num;

  // Additional of the total allocated buffer size for extras
  const double extra;

  // Queue capacity per operation
  const size_t queue_capacity;

  // Buffer pool size per operation
  const size_t buffer_pool_size;
  
  // The number of threads of initialise (number of operations from OperationManager)
  const size_t thread_num;
  
  // The total size of the buffer pool 
  const size_t buffer_pool_total;

  // Background worker for resetting buffers
  void resetWorker();
  
public:
  
  // Constructor
  BufferPool(const std::shared_ptr<cpu_utils>& cpu_,
             const std::shared_ptr<AsyncLogger>& logger,
             const std::shared_ptr<STMdata>& stm_,
             const std::shared_ptr<OperationManager>& om);
  
  // Destructor
  ~BufferPool() {
    // Signal that the reset thread is not running
    running = false;
    // Join reset buffer thread
    if (resetThread.joinable()) {
        resetThread.join();
    }
    // Clear buffer pool
    clearPool();
    // Ensure reset queue memory cleanup
    resetQueue.reset();
    std::cout << "BufferPool destructor complete.\n";
    //if (logger) logger->log("BufferPool destructor complete.",1);
  }
  
  // Acquire a buffer from the pool (Non-blocking)
  std::shared_ptr<DataStruct> acquire();
  
  // Return a buffer to the pool (Non-blocking)
  void release(std::shared_ptr<DataStruct>& buffer);

  // Clear buffer pool to ensure safe memory release
  void clearPool();

};

#endif // BUFFER_POOL_HH_
