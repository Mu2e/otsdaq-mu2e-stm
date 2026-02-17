#ifndef TESTFUNCS_hh
#define TESTFUNCS_hh

#include<iostream>
#include<fstream>
#include<thread>
#include<chrono>

// UDP socket
#include "UDPsocket.hh"

// Data variables
#include "dataVars.hh"

class test_funcs {		   

public:
  
  // Standard constructor
  test_funcs();
  
  // UDP client to send data over socket 
  void client(UDPsocket &udp, int socket, int16_t* data, uint32_t number_hb);
  
};

#endif
