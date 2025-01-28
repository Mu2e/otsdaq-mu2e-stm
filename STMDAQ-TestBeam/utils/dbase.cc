#include <memory>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <iostream>   
#include <iomanip>   
#include <string> 
#include <algorithm>
#include <pqxx/pqxx> 
#include "STMDAQ-TestBeam/utils/dbase.hh"
#include "STMDAQ-TestBeam/utils/DateTime.hh"

const int DBase::READ  = 0;
const int DBase::WRITE = 1;

DBase::DBase(std::string host, int port, int read_write){
    if (read_write = READ){
      _username = "stm_reader";
      _password = "stm_reader";
    }
    else if (read_write = WRITE){
      _username = "stm_writer";
      _password = "stm_writer";
    }
    _host = host;
    _port = port;

    std::stringstream ss;
    ss << "dbname = stm user = " << _username << " password = " << _password << " hostaddr = " << _host << " port = " << _port;
    std::cout << "Connecting to database with connection string: " << ss.str() << std::endl;
    try {
      _c = new pqxx::connection(ss.str());
      if (_c->is_open()) {
	std::cout << "Opened database successfully: " << _c->dbname() << std::endl;
      } 
      else {
	std::cout << "Can't open database" << std::endl;
      }
    }
    catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
}

pqxx::connection* DBase::con(){
    return _c;
}

void DBase::write_hpge_baseline(int run, int subrun, float mean, float rms){

  std::map<std::string, float> results = read_hpge_baseline(run, subrun);
  if (results.size() == 0){
    std::stringstream ss;
    ss << "insert into hpge_baseline(run,subrun,mean,rms) values (" << run << "," << subrun << "," << mean << "," << rms << ")";

    try {
      pqxx::work W(*_c);
      std::cout << "Running SQL query: "  << ss.str() << std::endl;
      W.exec( ss.str() );
      W.commit();
    } 
    catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }
  else {
    update_hpge_baseline(run, subrun, mean, rms);
  }
}

void DBase::update_hpge_baseline(int run, int subrun, float mean, float rms){

    std::stringstream ss;
    ss << "update hpge_baseline set mean = " << mean << ", rms = " << rms << " where run = " << run << " and subrun =  " << subrun;

    try {
      pqxx::work W(*_c);
      std::cout << "Running SQL query: "  << ss.str() << std::endl;
      W.exec( ss.str() );
      W.commit();
    } 
    catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
}

std::map<std::string, float> DBase::read_hpge_baseline(int run, int subrun){
    
    std::stringstream ss; ss << "select * from hpge_baseline where run = " << run << " and subrun = " << subrun;

    // Create a non-transactional object.
    pqxx::nontransaction N(*_c);
      
    // Execute SQL query 
    pqxx::result R( N.exec( ss.str() ));

    std::map<std::string, float> results;

    // List the records 
    for (pqxx::result::const_iterator rr = R.begin(); rr != R.end(); ++rr) {
      results["mean"] = rr[2].as<float>(); 
      results["rms"]  = rr[3].as<float>(); 
    }
    return results;
}

void DBase::read_slow_control_data(){
    // Example SQL query 
    std::string sql = "select * from slow_control_data";

    // Create a non-transactional object.
    pqxx::nontransaction N(*_c);
      
    // Execute SQL query 
    pqxx::result R( N.exec( sql ));
      
    // List the records 
    for (pqxx::result::const_iterator rr = R.begin(); rr != R.end(); ++rr) {
      std::cout << "SDID  = "  << rr[0].as<int>() << std::endl;
      std::cout << "SCID  = "  << rr[1].as<int>() << std::endl;
      std::cout << "Value = "  << rr[2].as<float>() << std::endl;
      std::cout << "Time  = "  << rr[3].as<std::string>() << std::endl;
    }
    std::cout << "Operation done successfully" << std::endl;

}

void DBase::write_run_subrun_time(int run, int subrun, int timex, int mode){
  std::cout << "Write sub run time for run = " << run << " subrun = " << subrun << " time = " << timex << " mode = " << mode << std::endl;
  std::stringstream ss;

  DateTime dt(timex);
  //2022-04-11 10:40:52.448487
  
  std::string timestamp = dt.Format(7);
  if (mode == 0){
    ss << "insert into run_subrun_times(run,subrun,start_time) values (" << run << "," << subrun << ",'" << timestamp << "')";
  }
  if (mode == 1) {
    ss << "update run_subrun_times set end_time = '" << timestamp << "' where run = " << run << " and subrun = " << subrun;
  }


  try {
    pqxx::work W(*_c);
    std::cout << "Running SQL query: "  << ss.str() << std::endl;
    W.exec( ss.str() );
    W.commit();
  } 
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

}

void DBase::write_runinfo_start(int run, std::string comment){
  DateTime dt; //current time
  std::string timestamp = dt.Format(7);
  // Remove any quotes from comment string so it doesn't shaft the SQL quoting...
  comment.erase(std::remove(comment.begin(),comment.end(),'\"'),comment.end());
  comment.erase(std::remove(comment.begin(),comment.end(),'\''),comment.end());
  std::stringstream ss;  ss << "insert into runinfo(run,start_time,comment) values (" << run << ",'" << timestamp << "','" << comment << "')";
  try {
    pqxx::work W(*_c);
    std::cout << "Running SQL query: "  << ss.str() << std::endl;
    W.exec( ss.str() );
    W.commit();
  } 
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

void DBase::write_runinfo_stop(int run){
  DateTime dt; //current time
  std::string timestamp = dt.Format(7);
  std::stringstream ss;  ss << "update runinfo set end_time = '" << timestamp << "' where run = " << run;
  try {
    pqxx::work W(*_c);
    std::cout << "Running SQL query: "  << ss.str() << std::endl;
    W.exec( ss.str() );
    W.commit();
  } 
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}
void DBase::close(){
  delete _c; // required for libpqxx 7 since disconnect method discontinued after libpqxx 5
  // _c->disconnect();
}



