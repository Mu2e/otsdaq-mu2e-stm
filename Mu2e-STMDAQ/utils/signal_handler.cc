// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"

// Define static members
// Initialize exit request flag
std::atomic<bool> SignalHandler::exit_requested(false);
// Initialize mutex
std::mutex SignalHandler::confirm_mutex;
// Initialize condition variable
std::condition_variable SignalHandler::confirm_cv;
// Initialize signal listening thread
std::thread SignalHandler::signal_thread;
 // Static pointer to store instance
SignalHandler* SignalHandler::instance = nullptr; 
// Define and set the cpu manager static variables
std::shared_ptr<cpu_utils> SignalHandler::cpu = nullptr;


// Constructor
SignalHandler::SignalHandler(const std::shared_ptr<AsyncLogger>& logger_,
			     const std::shared_ptr<cpu_utils>& cpu_) : logger(logger_) {

  // Set the static instance pointer
  instance = this;  
  // Initialize exit request flag
  exit_requested.store(false); 
  // Start listening for signals and handling confirmation
  signal_thread = std::thread(listenForSignals);
  // Set the cpu manager static variable
  cpu = cpu_;
  
}


// Signal handler function (executed immediately on receiving a signal)
void SignalHandler::handleSignal(int signal) {

  // Set the exit request flag to true
  exit_requested.store(true);
  
  // Log the received signal if an instance exists and logging is enabled
  if (instance && instance->logger) {
    instance->logger->log("SignalHandler: Signal " + std::to_string(signal) + " received. Awaiting user confirmation.", 1);
  }
  
}

// Function to continuously listen for signals and process exit confirmation
void SignalHandler::listenForSignals() {

  // NEED TO FIX BACKWARDS COMPATABILITY ISSUE HERE                             
  //ThreadManager::pin_thread_to_core(1,"SignalHandler");  

  // Pin thread to core
  if (cpu) {
    cpu->get_next_core("SignalHandler");
  }
  
  // Struct to define signal behavior
  struct sigaction action;
  // Assign handler function to handle signals
  action.sa_handler = handleSignal;
  // Clear signal mask
  sigemptyset(&action.sa_mask); 
  // Default flags
  action.sa_flags = 0;
  // Handle Ctrl+C (SIGINT)
  sigaction(SIGINT, &action, nullptr);
  // Handle termination signal (SIGTERM)
  sigaction(SIGTERM, &action, nullptr);
  // Handle user-defined signal (SIGUSR1)
  sigaction(SIGUSR1, &action, nullptr); 

  // Log that the signal listener has started (if logging is enabled)
  if (instance && instance->logger) {
    instance->logger->log("SignalHandler: Signal handler initialised.", 1);
  }

  // Continuous loop to handle incoming signals
  while (!stop::should_stop()) {
    // Wait until an exit request is triggered
    while (!exit_requested.load() && !stop::should_stop()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Avoid busy-waiting
    }

    // If stop was requested externally, break the loop
    if (stop::should_stop()) break;

    // Ask for confirmation outside of the signal handler (signal-safe)
    std::cout << "\nCaught interrupt. Are you sure you want to exit? (y/n): " << std::endl;
    
    // Log the confirmation request (if logging is enabled)
    if (instance && instance->logger) {
      instance->logger->log("SignalHandler: Awaiting user confirmation for exit.", 1);
    }
    
    char choice;
    auto start_time = std::chrono::steady_clock::now(); // Start timer for timeout mechanism

    // Wait for user input or timeout after 5 seconds
    while (!(std::cin >> choice) && (std::chrono::steady_clock::now() - start_time) < std::chrono::seconds(5)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small sleep to prevent CPU overuse
    }

    // If user confirms exit, stop the program
    if (choice == 'y' || choice == 'Y') {
      std::cout << "Exiting safely...\n";
      if (instance && instance->logger) {
   	instance->logger->log("SignalHandler: User confirmed exit. Stopping program.", 1);
      }
      
      // Request program stop
      stop::trigger_user_stop();

    } else {
      // If user cancels, reset exit flag and continue execution
      std::cout << "Resuming execution.\n";
      if (instance && instance->logger) {
   	instance->logger->log("SignalHandler: User denied exit. Resuming program.", 1);
      }
      exit_requested.store(false); // Reset flag so future signals can trigger exit confirmation again
    }
  }

  // Log the shutdown of the signal listener
  if (instance && instance->logger) {
    instance->logger->log("SignalHandler: Signal listener shutting down.", 1);
  }

}
  
