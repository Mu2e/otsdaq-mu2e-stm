#include <stdlib.h> 

#include "Mu2e-STMDAQ/utils/DateTime.hh"
#include "Mu2e-STMDAQ/utils/Logger.hh"
// Include dqm structs code
#include "Mu2e-STMDAQ/dqm/dqm_structs.hh"

// Must constructorialise the singletons/static variables here
LoggerSTM* LoggerSTM::_instance = 0;
unsigned int LoggerSTM::_logLevel = 0;
unsigned int LoggerSTM::_style = 0;
DateTime LoggerSTM::_time;
std::mutex LoggerSTM::_mutex;
string LoggerSTM::_opFileName;
std::ofstream LoggerSTM::_opFile;
bool LoggerSTM::_logToFile;
bool LoggerSTM::_logToSHM;
bool LoggerSTM::_throwCriticalErrors;
bool LoggerSTM::_useColor;
Color::Modifier LoggerSTM::_red(Color::FG_RED);
Color::Modifier LoggerSTM::_yel(Color::FG_YELLOW);
Color::Modifier LoggerSTM::_green(Color::FG_GREEN);
Color::Modifier LoggerSTM::_def(Color::FG_DEFAULT);
std::string LoggerSTM::_prevMsg;

// Define template
Destroyer<LoggerSTM> LoggerSTM::_destroyer;

LoggerSTM* LoggerSTM::Instance () {
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  constructor(LoggerSTM::INFO); //Default log level
  return _instance;
}

LoggerSTM* LoggerSTM::Instance (const unsigned int logLevel_) {
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  constructor(logLevel_);
  return _instance;
}

//Common function called by the overloaded "Instance" functions
void LoggerSTM::constructor (const unsigned int logLevel_) {

  if (!_instance) {
    _instance = new LoggerSTM();
    _destroyer.SetDoomed(_instance);
    _logLevel = logLevel_;
    _logToFile = false;
    _logToSHM = false;
    _style = 0;
    _throwCriticalErrors = true;
    _useColor = true;
  }

  return;
}

template <typename T>
void LoggerSTM::registerBlock(DQMPageType type, const std::string& shm_name, size_t size, bool persist) {

  // Register in manager
  SHMmanager::Instance().registerBlock<T>(type, shm_name, size, persist);
}

template <typename T>
T* LoggerSTM::get(DQMPageType type) {
  // Retrieve the pointer from the global registry
  T* ptr = SHMmanager::Instance().get<T>(type);
  if (!ptr) {
      throw std::runtime_error("Requested LoggerSTM SHM block not found in SHMmanager.");
  }
  return ptr;
}


void LoggerSTM::write(unsigned int level_, const std::string & message_) {

  //Write doesn't lock mutex itself by default in case it is called by
  //other functions that have already locked the mutex.
  //This public overload does lock it.
  std::lock_guard<std::mutex> lock(_mutex);
  this->write(level_,message_,lock);

}


void LoggerSTM::write(unsigned int level_, const std::string & message_, const std::lock_guard<std::mutex> & lock_) {

  // Only print if level is above the set log level threshold
  if ( level_ <= _logLevel ) {

    std::stringstream msg;
    std::stringstream shm_msg;

    string levelStr = "UNKNOWN";
    if(level_ == LoggerSTM::ERROR) levelStr = "ERROR";
    else if(level_ == LoggerSTM::INFO) levelStr = "INFO";
    else if(level_ == LoggerSTM::WARNING) levelStr = "WARNING";
    else if(level_ == LoggerSTM::NOTE) levelStr = "NOTE";
    else if(level_ == LoggerSTM::DEBUG) levelStr = "DEBUG";
    else {
      std::stringstream ss; ss << "[ LoggerSTM :: write ] Invalid log level " << level_ << " chosen, must be ERROR, INFO, WARNING, NOTE or DEBUG";
      this->write(LoggerSTM::ERROR,ss.str(),lock_);
      return;
    }

    //Store as new "most recent" message
    _prevMsg = message_;

    // Shared memory just needs message
    if (_logToSHM && (level_ == LoggerSTM::ERROR || level_ == LoggerSTM::WARNING)) {
      shm_msg << message_;
      logToSHM(level_, shm_msg.str());
    }

    //Compose message based on style
    if (_style > 2) _style = 0; //Default to 0 if invalid style set
    if (_style == 0) {
      msg << message_; //Message only
    }
    else if (_style == 1)  {
      msg << levelStr << " : " << message_; //Add message level
    }
    else if (_style == 2)  {
      string date = _time.Format() ;
      msg << levelStr << " : " << date << " : " << message_; //Add message level and date
    }

    //Send msg to stdout
    if( _useColor ) { //Add color case
      std::stringstream colorMsg;
      if( level_ == LoggerSTM::ERROR ) colorMsg << _red;
      else if( level_ == LoggerSTM::WARNING ) colorMsg << _yel;
      else colorMsg << _def;
      colorMsg << msg.str() << _def << std::endl;
      std::cout << colorMsg.str();
    }
    else { //No color case
      std::cout << msg.str() << std::endl;
    }
    
    //Send same message to file (no color)
    if (_logToFile) {
      _opFile << msg.str() << std::endl;
      _opFile.flush(); //Write the stream to the file
    }

    //Throw CriticalError if required
    if(_throwCriticalErrors) {
      if( level_ == LoggerSTM::ERROR ) stop::trigger_critical_stop();//throw CriticalError();
    }

  }//if within debug level

}

unsigned int LoggerSTM::getLogLevel() {
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  return _logLevel;
}

void LoggerSTM::setLogLevel(const unsigned int logLevel) {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check for valid debug level and set if OK
  if( logLevel > 4 ) {
    std::stringstream ss; ss << "[ LoggerSTM :: setLogLevel ] Invalid log level " << logLevel << " chosen, must be ERROR, INFO, WARNING, NOTE or DEBUG";
    this->write(LoggerSTM::ERROR,ss.str(),lock);
    _logLevel = 0;
  }
  else {
    _logLevel = logLevel;
  }


}

void LoggerSTM::setStylePlain() {
  this->setStyle(0);
}

void LoggerSTM::setStyle(unsigned int style_) {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check for valid style level and set if OK
  if( style_ > 2 ) {
    std::stringstream ss; ss << "[ LoggerSTM :: setStyle ] Invalid style " << style_ << " chosen, must be in range 0-2";
    this->write(LoggerSTM::ERROR,ss.str(),lock);
    _style = 0;
  }
  else {
    _style = style_;
  }

}

//Enable logging to file (as well as stdout)
void LoggerSTM::LogToFile(string fileName_) {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check if already logging to file
  if(_logToFile) {
    std::stringstream ss; ss << "[ LoggerSTM :: LogToFile ] Already logging to file";
    CloseLogFile();
    //this->write(LoggerSTM::ERROR,ss.str(),lock);
    //return;
  }

  //Check if stream already exists
  //TODO

  //Open file
  _opFileName = fileName_;
  _opFile.open(_opFileName.c_str()); //TODO Catch exceptions

  //If open was successful, set flag
  if(_opFile) {
    _logToFile = true;
    std::stringstream ss; ss << "[ LoggerSTM :: LogToFile ] Logging to file: " << _opFileName;
    this->write(LoggerSTM::INFO,ss.str(),lock);
  }
  else {
    std::stringstream ss; ss << "[ LoggerSTM :: LogToFile ] Failed to open file: " << _opFileName << std::endl;
    this->write(LoggerSTM::ERROR,ss.str(),lock);
  }

}


//Write the file stream to the file
void LoggerSTM::CloseLogFile() {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check that we are logging to file
  if(_logToFile) {

    _logToFile = false;

    std::stringstream ss; ss << "[ LoggerSTM :: CloseLogFile ] Closing log file: " << _opFileName;
    write(LoggerSTM::INFO,ss.str(),lock);

    _opFile.close();

  }
  else {
    std::stringstream ss; ss << "[ LoggerSTM :: CloseLogFile ] Cannot close log file as not currently logging to file";
    write(LoggerSTM::ERROR,ss.str(),lock);
    return;
  }

}

void LoggerSTM::initSHM(unsigned int max_alarms) {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  // Calculate necessary size of SHM
  size_t alarm_bytes = sizeof(dqm_data_alarm) +
		   (max_alarms - 1) * sizeof(alarm_entry) + // -1 for array initialised with size 1
		   sizeof(uint64_t);  

  // Create persistent SHM block
  registerBlock<dqm_data_alarm>(DQMPageType::ALARMS, "/dqm_alarm_data", alarm_bytes, true);
  _logToSHM = true;

  std::stringstream ss; ss << "[ LoggerSTM::initSHM ] Registered SHM block for alarms" <<
	 " with size " << alarm_bytes;
  this->write(LoggerSTM::INFO,ss.str(),lock);

  auto* alarm_data = get<dqm_data_alarm>(DQMPageType::ALARMS);

  alarm_data->max_alarms = max_alarms;
  alarm_data->write_idx = 0;
  alarm_data->num_alarms = 0;

}

void LoggerSTM::logToSHM(unsigned int level_, const std::string & message_) {
  // Get pointer to SHM block
  auto* alarm_data = get<dqm_data_alarm>(DQMPageType::ALARMS);

  // Write to shared memory safely
  alarm_data->gen_start.fetch_add(1, std::memory_order_release);

  // Latest alarm time 
  alarm_data->timestamp_ns = get_current_time_ns();

  // Get next alarm write location
  int write_idx = alarm_data->write_idx;
  alarm_entry* next_alarm = &alarm_data->alarms[write_idx];

  // Write all bytes to 0 then write message up to 127 bytes
  std::memset(next_alarm->text, 0, 128);
  std::strncpy(next_alarm->text, message_.c_str(), 127);

  // Set alarm level
  next_alarm->level = level_;

  // Set alarm time 
  next_alarm->time_ns = get_current_time_ns();

  // Increment number
  if (alarm_data->num_alarms != alarm_data->max_alarms) alarm_data->num_alarms += 1;
  
  // Increment write index
  alarm_data->write_idx = (alarm_data->write_idx + 1) % alarm_data->max_alarms;

  // Find gen end write location
  auto* gen_end_ptr = reinterpret_cast<std::atomic<uint64_t>*>(alarm_data->alarms 
		  			+ alarm_data->max_alarms);

  // Complete safe write
  gen_end_ptr->store(alarm_data->gen_start.load(std::memory_order_acquire), std::memory_order_release);

}


std::string LoggerSTM::getMostRecentMsg() const { 
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  return _prevMsg;
}

uint64_t LoggerSTM::get_current_time_ns() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

