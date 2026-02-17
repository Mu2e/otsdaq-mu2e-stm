//////////////////////////////////////////////////////////////////////////////////
/// This module creates a UDP socket for 10G readout (main).  
/////////////////////////////////////////////////////////////////////////////////// 
/********************************************************************/

#include<iostream>
#include<fstream>
#include <vector>
#include <string>

#include <thread>
#include <mutex>

#include<string.h> //memset 
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close 

#include <time.h>
#include <math.h>

// DQM
#include "STMDAQ-TestBeam/dqm/dqm.hh"

// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)
static const uint chNum = 2;

// Instance of UDP socket class
UDPsocket udp[chNum];

// Instance of UDP class for DQM
UDPsocket udp_dqm;

// Instance of IPBus Manager
IPBusManager* hw = new IPBusManager();

// Instance of HW class
initHW initHW_;

// Main function
int main(){
  
  // Get current date and time 
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t); 
  std::ostringstream oss;
  oss << std::put_time(&tm,"%Y-%m-%d_%H-%M-%S");
  string date_time = oss.str();

  // Get file name starter
  std::string stm_file = "stm-daq_"+date_time;

  // Get log file name
  std::string stm_log_file = stm_file+".log";
  
  // Logger
  Logger::Instance(Logger::DEBUG);
  Logger::Instance()->setStylePlain();
  Logger::Instance()->LogToFile("log/"+stm_log_file);
  Logger::Instance()->write(1,"Logger initialised");
  
  // Initialise all hardware
  initHW_.init(hw);


  // double pi = 3.1345658;
  // int16_t* pi_int = dqm.convert_double(pi);    
  // Initiliase sockets and get memory used for UDP kernel buffers
  int dqm_sock = 0; // DQM socket
   
  // DQM client clients
  dqm_sock = udp_dqm.setupClient(0,SW);
  
  // Initialise classes
  //Instance of form events data class
  formEvents ev(1000);

  //Instance of form events data class
  daqZS zs(1000);

  prescaleData ps;

  checkData cd;

  // Write event data
  queue_write *writeq;

  // Instance of DQM class
  stmDQM dqm(udp,hw,cd,ev,zs,ps,writeq);
   
  dqm.build_dqm_array();

  exit(0);

  // Start DQM thread first                                              
  std::thread *dqm_thread = new std::thread (&stmDQM::dqm_client, // dqm
  					     ref(dqm), // Frontend threads class
  					     dqm_sock, // UDP socket number
  					     std::ref(udp_dqm)); // DQM socket
 
  // Join dqm thread
  dqm_thread->join(); 
  
  return 0;
  
}

 
