/////////////////////////////////////xs//////////////////////////////////////////
/// This module contains functions for testing the STM frontend (header).  
/////////////////////////////////////////////////////////////////////////////////

/********************************************************************/

#ifndef TESTFUNCS_hh
#define TESTFUNCS_hh

#include<iostream>
#include<fstream>

#include <thread>
#include <mutex>

#include<string.h> //memset 
#include<fcntl.h> // for open
#include<unistd.h> // for close 

#include <time.h>
#include <math.h>

// UDP socket
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Circular buffer queue
#include "STMDAQ-TestBeam/utils/queue.hh"

// Data variables
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

class test_funcs {		   

public:
  
  // Standard constructor - should'nt be used
  test_funcs();
  
  // UDP client to send data over socket 
  void client(int chan, UDPsocket &udp, int socket, int16_t* data, int packet_num);
  
  // Function for final data pull to compare input data for test 
  void pull_data(int chan, UDPsocket &udp, 
		 queue_buffer *pullq, uint64_t bufferer_size,
		 int16_t* data);    
  
  // Function to compare input data to ouput for testing 
  void compare_data(int chNum, uint32_t numberhb, uint64_t* data_len, 
		    int16_t** input, int16_t** output);

};

#endif
