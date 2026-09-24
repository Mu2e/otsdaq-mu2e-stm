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
  
  // Standard constructor - should'nt be used
  test_funcs();
  
  // UDP client to send data over socket 
  void client(UDPsocket &udp, int socket, int16_t* data,
	      uint64_t packet_num, uint32_t number_hb, uint32_t send_num);

  // Update event headers to send more than generated in memory
  bool update_headers(int16_t* &data, uint64_t packet_num,
		      uint i, int offset, uint32_t number_hb, uint32_t send_num);

  
};

#endif
