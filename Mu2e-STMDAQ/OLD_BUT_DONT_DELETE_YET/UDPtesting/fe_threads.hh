/////////////////////////////////////xs/////////////////////////////////////////////
/// This module contains the threads for the STM frontend (header).  
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

#ifndef FETHREADS_hh
#define FETHREADS_hh

#include<iostream>
#include<fstream>

#include <thread>
#include <mutex>

#include<string.h> //memset 
#include<fcntl.h> // for open
#include<unistd.h> // for close 

#include <time.h>
#include <math.h>

// // UDP socket
// #include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// // Circular buffer queue
// #include "STMDAQ-TestBeam/utils/queue.hh"

// // Write data
// #include "STMDAQ-TestBeam/utils/queue_write.hh"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// // Check data
// #include "STMDAQ-TestBeam/processData/checkData.hh"

// // Form events
// #include "STMDAQ-TestBeam/processData/formEvents.hh"

// // Zero suppress
// #include "STMDAQ-TestBeam/processData/zeroSuppress.hh"

// // Prescale events
// #include "STMDAQ-TestBeam/processData/prescale.hh"

class FEthreads {		   

public:
  
  // Standard constructor - should'nt be used
  FEthreads();
  
  // // DQM client to send data over socket to DQM
  // void dqm_client(int socket, daqZS &zs, UDPsocket &udp, 
  // 		  UDPsocket &udp_dqm);

   // Server function to receive packets and put into queue
  void server(int chan, int socket, UDPsocket &udp, 
	      queue_buffer *&pushq);
    
  // Function to check data packets in queue
  void check_data(int chan, UDPsocket &udp, 
  		  queue_buffer *pullq,
  		  queue_buffer *pushq);

  // Function to process packet data into events
  void form_events(int chan, formEvents &ev,
		   UDPsocket &udp, 
		   queue_buffer *pullq, 
		   queue_buffer *pushq);

  // Function to zero suppress event data
  void zs_data(int chan, daqZS &zs,
	       UDPsocket &udp, 
	       queue_buffer *pullq, 
	       queue_buffer *pushq);

  // Function to prescale data and return as single events
  void prescale_data(int chan, UDPsocket &udp, 
   		     queue_buffer *pullq, 
  		     queue_buffer *pushq);

  // 
  void distribute_raw(int chan, int socket,                                                                                                      
		      UDPsocket &udp, UDPsocket &sw_udp,
		      queue_buffer *pullq,
		      uint64_t buffer_len);

  
  // // Function for final data pull from queue for distribution
  // void distribute_data(int chan, 
  // 		       int socket, 
  // 		       UDPsocket &udp, 
  // 		       UDPsocket &sw_udp,
  // 		       queue_buffer<dataVars::off_event> &pullq,
  // 		       shm_queue<dataVars::on_event> *shmq);  
   
  // Function to push data to queue to write
  void write_data(int chan, 
		  uint64_t buffer_len,
   		  UDPsocket &udp,
   		  queue_buffer *pullq,
   		  queue_write *pushq);  
  
  
};

#endif
