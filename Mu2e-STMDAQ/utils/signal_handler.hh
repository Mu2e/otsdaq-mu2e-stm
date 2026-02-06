#ifndef SIGNAL_HANDLER_HH_
#define SIGNAL_HANDLER_HH_

#include <iostream> 
#include <csignal> 
#include <atomic> 
#include <mutex> 
#include <thread> 
#include <condition_variable> 
#include <chrono>
#include <fstream> // File I/O for logging

// Stop signal
#include "Mu2e-STMDAQ/utils/stop_signal.hh"

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"

// Signal handler class
class SignalHandler {
  
private:

  // Async Logger
  const std::shared_ptr<AsyncLogger> logger;

  // Store reference to CPU utils instance
  static std::shared_ptr<cpu_utils> cpu;
  
  // Flag to request exit confirmation
  static std::atomic<bool> exit_requested;
  
  // Mutex for synchronizing confirmation input
  static std::mutex confirm_mutex;
  
  // Condition variable for confirmation handling
  static std::condition_variable confirm_cv; 

  // Thread for listening to signals
  static std::thread signal_thread; 

  // Signal handler function (
  static void handleSignal(int signal);

  // Continuously listen for signals and process exit confirmation
  static void listenForSignals();

  // Pointer to store the singleton instance
  static SignalHandler* instance;  
  
public:
  
  // Constructor
  explicit SignalHandler(const std::shared_ptr<AsyncLogger>& logger_,
			 const std::shared_ptr<cpu_utils>& cpu_);

  // Destructor
  ~SignalHandler() {
    // Ensure stop flag is set before cleanup
    stop::trigger_user_stop();
    // Wait for signal thread to complete execution
    if (signal_thread.joinable()) signal_thread.join();
    // Reset static instance pointer
    instance = nullptr;
    // Notify user
    std::cout << "SignalHandler destructor called.\n";
  }
  
};


#endif // SIGNAL_HANDLER_HH_
