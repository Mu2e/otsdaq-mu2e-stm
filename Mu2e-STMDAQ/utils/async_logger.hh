#ifndef ASYNC_LOGGER_hh_
#define ASYNC_LOGGER_hh_

#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility> // For std::pair

// Include CPU utils header
#include "Mu2e-STMDAQ/utils/cpu_utils.hh"
// Logger code
#include "Mu2e-STMDAQ/utils/Logger.hh" 
// Environmental variables header
#include "Mu2e-STMDAQ/utils/EnvVars.hh"
// Timer code
#include "Mu2e-STMDAQ/utils/timer.hh"

// Pre-allocated buffer pool for memory reuse
class AsyncLogger {
  
public:

  using Message = std::pair<std::string, unsigned int>; // Alias for readability

  // Constructor 
  AsyncLogger(const Config& cfg, const std::shared_ptr<cpu_utils>& cpu_);

  // Destructor for logging
  ~AsyncLogger(){
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      done = true;
    }
    cv.notify_all(); // Notify the thread to stop waiting
    if (printerThread.joinable()) {
      printerThread.join(); // Ensure the thread is joined cleanly
    }

    // Close log file
    Logger::Instance()->CloseLogFile();
    
    std::cout << "AsyncLogger destructor called.\n";
  }

  // Function to log a message
  void log(const std::string& msg, unsigned int level);

private:
  
  std::queue<Message> messageQueue; // Thread-safe message queue
  std::mutex queueMutex;                // Mutex to protect the queue
  std::condition_variable cv;           // Condition variable for notifications
  std::atomic<bool> done;               // Flag to signal shutdown
  std::thread printerThread;            // The dedicated printer thread
  
  // The printer thread function
  void printerThreadFunc();

  // Store reference to the Config instance
  const Config& cfg; 

  // Store reference to CPU utils instance
  const std::shared_ptr<cpu_utils>& cpu;
  
};

#endif 
