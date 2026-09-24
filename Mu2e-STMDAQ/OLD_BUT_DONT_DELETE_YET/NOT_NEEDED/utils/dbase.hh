#ifndef DBASE_HH
#define DBASE_HH

#include <memory>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <iostream>   
#include <iomanip>   
#include <string> 
#include <pqxx/pqxx> 

// Note multiple instances of this class each create a database socket connection so should limit number of
// times this class is instantiated - else can overload database and then we'll have to write a 
// singleton instance which is a pain: ML

class DBase {


private:

  pqxx::connection*   _c;
  std::string _username;
  std::string _password;
  std::string _host;
  int         _port;

public:


  static const int READ;
  static const int WRITE;

  DBase(std::string host, int port, int read_write);
  void write_hpge_baseline(int run, int subrun, float mean, float rms);
  void update_hpge_baseline(int run, int subrun, float mean, float rms);
  std::map<std::string, float> read_hpge_baseline(int run, int subrun);
  void read_slow_control_data();
  void write_run_subrun_time(int run, int subrun, int timex, int mode);
  pqxx::connection* con();
  void write_runinfo_start(int run, std::string comment);
  void write_runinfo_stop(int run);
  void close();

};

#endif


