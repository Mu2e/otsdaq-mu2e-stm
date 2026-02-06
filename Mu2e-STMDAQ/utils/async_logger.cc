// Async logger header
#include "Mu2e-STMDAQ/utils/async_logger.hh" 

// Constructor
AsyncLogger::AsyncLogger(const Config& cfg, const std::shared_ptr<cpu_utils>& cpu_) :
  done(false), cfg(cfg), cpu(cpu_)  {

  // Get current date and time
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm,"%Y-%m-%d_%H-%M-%S");
  std::string date_time = oss.str(); 

  // Get the log directory
  std::string log_dir = EnvVars::expand("${LOG_DIR}");
  // Get log file name
  std::string log_file = cfg.getValue<std::string>("stm.write_data.logfile")+"_"+date_time+".log";
  
  // Logger
  STMLogger::Instance(STMLogger::DEBUG);
  STMLogger::Instance()->setStylePlain();
  STMLogger::Instance()->LogToFile(log_dir+log_file);
  STMLogger::Instance()->write(1,"STM DAQ started: " + date_time);
  STMLogger::Instance()->write(1,"Loaded configuration file: " + cfg.getXMLpath());
  STMLogger::Instance()->write(1,"Logger initialised");   

  // Start thread to print to screen
  printerThread = std::thread(&AsyncLogger::printerThreadFunc, this);
  
}

// The printer thread function
void AsyncLogger::printerThreadFunc() {

  // Pin thread to core
  [[maybe_unused]] size_t core = cpu->get_next_core("AsyncLogger");
  
  // Inifinte loop
  while (true) {

    // Mutex lock
    std::unique_lock<std::mutex> lock(queueMutex);
    cv.wait(lock, [this] { return !messageQueue.empty() || done; });

    // If a message is waiting...
    while (!messageQueue.empty()) {
      // Get next message from queue
      Message msg = std::move(messageQueue.front());
      messageQueue.pop();
      // Unlock before printing to minimize mutex hold time
      lock.unlock();
      // Print message to screen (CHANGE TO LOGGER)
      STMLogger::Instance()->write(msg.second,msg.first);
      //      std::cout << msg << std::endl;
      // Reacquire lock for next iteration
      lock.lock();   
    }
    
    // Exit thread if no more messages and done is set
    if (done && messageQueue.empty()) {
      break; 
    }
    
  }
}


// Function to log a message
void AsyncLogger::log(const std::string& msg, unsigned int level) {
  {

    // Signal critical error asap
    if (level == 0) stop::trigger_critical_stop();
    
    // Lock the mutex
    std::lock_guard<std::mutex> lock(queueMutex);

    // Get the current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
    // Format the timestamp
    std::ostringstream timeStream;
    timeStream << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");

    // Construct the log message with the timestamp
    std::string logEntry = "[" + timeStream.str() + "] " + msg;

    // Push the message to be printed to the queue
    messageQueue.emplace(logEntry,level);
  }
  // Notify the printer thread
  cv.notify_one(); 
}
