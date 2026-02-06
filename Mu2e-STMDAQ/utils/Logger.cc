#include <stdlib.h> 

#include "Mu2e-STMDAQ/utils/DateTime.hh"
#include "Mu2e-STMDAQ/utils/Logger.hh"

// Must constructorialise the singletons/static variables here
STMLogger* STMLogger::_instance = 0;
unsigned int STMLogger::_logLevel = 0;
unsigned int STMLogger::_style = 0;
DateTime STMLogger::_time;
std::mutex STMLogger::_mutex;
string STMLogger::_opFileName;
std::ofstream STMLogger::_opFile;
bool STMLogger::_logToFile;
bool STMLogger::_throwCriticalErrors;
bool STMLogger::_useColor;
Color::Modifier STMLogger::_red(Color::FG_RED);
Color::Modifier STMLogger::_yel(Color::FG_YELLOW);
Color::Modifier STMLogger::_green(Color::FG_GREEN);
Color::Modifier STMLogger::_def(Color::FG_DEFAULT);
std::string STMLogger::_prevMsg;

// Define template
Destroyer<STMLogger> STMLogger::_destroyer;

STMLogger* STMLogger::Instance () {
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  constructor(STMLogger::INFO); //Default log level
  return _instance;
}

STMLogger* STMLogger::Instance (const unsigned int logLevel_) {
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  constructor(logLevel_);
  return _instance;
}

//Common function called by the overloaded "Instance" functions
void STMLogger::constructor (const unsigned int logLevel_) {

  if (!_instance) {
    _instance = new STMLogger;
    _destroyer.SetDoomed(_instance);
    _logLevel = logLevel_;
    _logToFile = false;
    _style = 0;;
   _throwCriticalErrors = true;
   _useColor = true;
  }

  return;
}


void STMLogger::write(unsigned int level_, const std::string & message_) {

  //Write doesn't lock mutex itself by default in case it is called by
  //other functions that have already locked the mutex.
  //This public overload does lock it.
  std::lock_guard<std::mutex> lock(_mutex);
  this->write(level_,message_,lock);

}


void STMLogger::write(unsigned int level_, const std::string & message_, const std::lock_guard<std::mutex> & lock_) {

  // Only print if level is above the set log level threshold
  if ( level_ <= _logLevel ) {

    std::stringstream msg;

    string levelStr = "UNKNOWN";
    if(level_ == STMLogger::ERROR) levelStr = "ERROR";
    else if(level_ == STMLogger::INFO) levelStr = "INFO";
    else if(level_ == STMLogger::WARNING) levelStr = "WARNING";
    else if(level_ == STMLogger::NOTE) levelStr = "NOTE";
    else if(level_ == STMLogger::DEBUG) levelStr = "DEBUG";
    else {
      std::stringstream ss; ss << "[ STMLogger :: write ] Invalid log level " << level_ << " chosen, must be ERROR, INFO, WARNING, NOTE or DEBUG";
      this->write(STMLogger::ERROR,ss.str(),lock_);
      return;
    }

    //Store as new "most recent" message
    _prevMsg = message_;

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
      if( level_ == STMLogger::ERROR ) colorMsg << _red;
      else if( level_ == STMLogger::WARNING ) colorMsg << _yel;
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
      if( level_ == STMLogger::ERROR ) stop::trigger_critical_stop();//throw CriticalError();
    }

  }//if within debug level

}

unsigned int STMLogger::getLogLevel() {
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  return _logLevel;
}

void STMLogger::setLogLevel(const unsigned int logLevel) {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check for valid debug level and set if OK
  if( logLevel > 4 ) {
    std::stringstream ss; ss << "[ STMLogger :: setLogLevel ] Invalid log level " << logLevel << " chosen, must be ERROR, INFO, WARNING, NOTE or DEBUG";
    this->write(STMLogger::ERROR,ss.str(),lock);
    _logLevel = 0;
  }
  else {
    _logLevel = logLevel;
  }


}

void STMLogger::setStylePlain() {
  this->setStyle(0);
}

void STMLogger::setStyle(unsigned int style_) {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check for valid style level and set if OK
  if( style_ > 2 ) {
    std::stringstream ss; ss << "[ STMLogger :: setStyle ] Invalid style " << style_ << " chosen, must be in range 0-2";
    this->write(STMLogger::ERROR,ss.str(),lock);
    _style = 0;
  }
  else {
    _style = style_;
  }

}

//Enable logging to file (as well as stdout)
void STMLogger::LogToFile(string fileName_) {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check if already logging to file
  if(_logToFile) {
    std::stringstream ss; ss << "[ STMLogger :: LogToFile ] Already logging to file";
    CloseLogFile();
    //this->write(STMLogger::ERROR,ss.str(),lock);
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
    std::stringstream ss; ss << "[ STMLogger :: LogToFile ] Logging to file: " << _opFileName;
    this->write(STMLogger::INFO,ss.str(),lock);
  }
  else {
    std::stringstream ss; ss << "[ STMLogger :: LogToFile ] Failed to open file: " << _opFileName << std::endl;
    this->write(STMLogger::ERROR,ss.str(),lock);
  }

}


//Write the file stream to the file
void STMLogger::CloseLogFile() {

  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function

  //Check that we are logging to file
  if(_logToFile) {

    _logToFile = false;

    std::stringstream ss; ss << "[ STMLogger :: CloseLogFile ] Closing log file: " << _opFileName;
    write(STMLogger::INFO,ss.str(),lock);

    _opFile.close();

  }
  else {
    std::stringstream ss; ss << "[ STMLogger :: CloseLogFile ] Cannot close log file as not currently logging to file";
    write(STMLogger::ERROR,ss.str(),lock);
    return;
  }

}


std::string STMLogger::getMostRecentMsg() const { 
  std::lock_guard<std::mutex> lock(_mutex); //Lock mutex for duration of function
  return _prevMsg;
}

