#ifndef THREAD_MANAGER_hh_
#define THREAD_MANAGER_hh_

#include <iomanip>
#include <cstring>
#include <functional> 
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include <iomanip>

// Include CPU utils header                                                                       
#include "Mu2e-STMDAQ/utils/cpu_utils.hh"
// Buffer queue code
#include "Mu2e-STMDAQ/buffers/buffer_queue.hh"
// Buffer pool code
#include "Mu2e-STMDAQ/processing/buffer_pool.hh"
// Operation Manager code
#include "Mu2e-STMDAQ/processing/operation_manager.hh"
// Hardware Manager code
#include "Mu2e-STMDAQ/hardware/hw_manager.hh"
// DQM code
#include "Mu2e-STMDAQ/processing/dqm_manager.hh"

// Standard thread functions
class ThreadManager {

private:
  

  // Store reference to CPU utils instance
  const std::shared_ptr<cpu_utils>& cpu;
  
  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;
  
  // Buffer pool
  const std::shared_ptr<BufferPool>& pool;

  // Hardware manager
  const std::shared_ptr<HardwareManager> hw;
  
  // UDP class
  const std::shared_ptr<UDP> udp;

  // The threads
  std::vector<std::thread> threads;

  // The thread queues
  std::vector<std::shared_ptr<BufferQueue<DataStruct>>> queues;

  // Number of threads
  const size_t thread_num;

  // Thread Name
  std::vector<std::string> thread_name;  

  // Flags to signal when each thread has finished
  std::atomic<bool> stop_requested{false};
  std::vector<std::atomic<bool>> finished;

  // Number of buffers processed for each thread
  std::vector<uint64_t> buffer_count;
  
  // Track total bytes processed for each thread
  std::vector<uint64_t> tot_bytes_processed;

  // Average thread processing speed
  std::vector<double> avg_speed_num;
  std::vector<double> avg_speed_den;

  // Number of baseline sliding window buffers
  const size_t baseline_window_buffers;

public:
  
  // Constructor to initialize threads
  explicit ThreadManager(const std::shared_ptr<cpu_utils>& cpu_,
                         const std::shared_ptr<AsyncLogger>& logger_,
                         const std::shared_ptr<STMdata>& stm_,
                         const std::shared_ptr<SignalHandler>& signal_,
                         const std::shared_ptr<BufferPool>& pool_,
                         const std::shared_ptr<OperationManager>& om,
                         const std::shared_ptr<HardwareManager> hw_);

  // Get UDP wait time
  std::chrono::duration<double> wait() const { return udp->get_wait(); }

  // Emergency shutdown: signal stop and wait briefly, then detach stuck threads
  void shutdown(std::chrono::seconds timeout) {
    stop_requested.store(true);

    // Wait up to timeout for threads to set their finished flags
    auto deadline = std::chrono::steady_clock::now() + timeout;
    bool all_done = false;
    while (std::chrono::steady_clock::now() < deadline) {
      all_done = true;
      for (size_t i = 0; i < finished.size(); ++i) {
        if (!finished[i].load()) { all_done = false; break; }
      }
      if (all_done) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (size_t i = 0; i < threads.size(); ++i) {
      if (!threads[i].joinable()) continue;
      if (finished[i].load()) {
        threads[i].join();
      } else {
        logger->log("ThreadManager::shutdown: detaching stuck thread " + std::to_string(i), 2);
        threads[i].detach();
      }
    }
    logger->log("ThreadManager timed shutdown complete", 1);
  }

  // Destructor
  ~ThreadManager() {
    for (auto& thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    logger->log("ThreadManager destructor called", 1);
  }
  
  // General worker thread function 
  void worker_thread(const size_t thrd_idx,
                    const std::shared_ptr<BufferQueue<DataStruct>>& inq,
                    const std::shared_ptr<BufferQueue<DataStruct>>& outq,
                    const op_any operation);

  // Perform operation on buffer
  void op_on_buffer(const size_t thrd_idx,
                    const op_any operation,
                    std::shared_ptr<DataStruct>& buffer,
                    std::shared_ptr<DataStruct>& prev_buffer);

  // Push/release outgoing buffer
  void push_buffer(const size_t thrd_idx,
                   const std::shared_ptr<BufferQueue<DataStruct>>& outq,
                   std::shared_ptr<DataStruct>& buffer);
  
  // Log thread performance 
  void log_performance(const size_t thrd_idx,
                       const std::chrono::time_point<
                       std::chrono::high_resolution_clock> thread_start_time);

  // Data source thread function 
  void data_source_thread(const size_t thrd_idx);

  
};

#endif 



