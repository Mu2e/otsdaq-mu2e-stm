///////////////////////////////////////////////////////////////////////////
/// This module contains functions for testing the STM frontend (main).
////////////////////////////////////////////////////////////////////////////********************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <time.h>
#include <math.h>

// Test functions
#include "STMDAQ-TestBeam/frontEnd/test_funcs.hh"

// Data variables class
dataVars dvar;

//Standard constructor
test_funcs::test_funcs() {}

// Client function to send packets
void test_funcs::client(int chan, UDPsocket &udp ,int socket, 
			int16_t* data, int packet_num){

  // Notify user
  cout << "In send thread: channel " << chan << ", socket " << socket << endl;

  // Send data counter
  uint64_t sendCount = 0;

  // Define receive buffer as array of packets
  int16_t *snd_buffer[UDPsocket::SENDMMSG_NUM];
  uint16_t *size_buffer = new uint16_t [UDPsocket::SENDMMSG_NUM] ();
  for (int i = 0; i < UDPsocket::SENDMMSG_NUM; i++){
    snd_buffer[i] = new int16_t [MAX_PACKET_LEN] ();
    size_buffer[i] = MAX_PACKET_SIZE;
  }

  // Get start time before packet sending
  auto start = chrono::steady_clock::now();

  // Define a message counter
  int msgNum = 0;

  // Loop over all packets to be sent 
  for (int i = 0; i < packet_num; i++) {
    // Memcpy packet to buffer
    memcpy(snd_buffer[msgNum],&data[i*MAX_PACKET_LEN],MAX_PACKET_SIZE);
    // Increment message counter
    msgNum++;
    // Check if SENDMMMSG_NUM has been reached
    if (msgNum == UDPsocket::SENDMMSG_NUM){
      // Sleep to mimic fw sending
      //      this_thread::sleep_for(5ms);
      // Send buffer
      int retval = udp.send(snd_buffer,size_buffer,UDPsocket::SENDMMSG_NUM,socket);
      std::cout << "Sending: " << retval << "\n";
      // Increase counter by number of sent messages
      sendCount += udp.SENDMMSG_NUM;
      // Reset message counter
      msgNum = 0;
    }
    // Send any leftover packets at end of loop
    else if (i == packet_num-1){
      // Send remaining buffer
      int retval = udp.send(snd_buffer,size_buffer,msgNum,socket);
      std::cout << "Sending: " << retval << "\n";
      // Increase counter by number of sent messages
      sendCount += msgNum;
    }
  }

  // Get end time after packet sending
  auto end = chrono::steady_clock::now();
  // Find time taken to send all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = sendCount*MAX_PACKET_SIZE*1e-9;

  // Store speed as Gbits/s
  double sendSpeed = data_size*8/(timeNano*1e-9);

  // Notify user
  cout << "Channel = " << chan 
       << " [" << udp.get_channel_name(chan)
       << "] sent " << sendCount
       << " packets [" << data_size
       << " Gbytes] at "<< sendSpeed << " Gbit/s" << endl;
  
}

// Function for final data pull to compare input data for test
void test_funcs::pull_data(int chan, UDPsocket &udp, 
			   queue_buffer *pullq, uint64_t buffer_size,
			   int16_t* data){
  
  // Notify user
  cout << "In pull / compare thread: channel " << chan << endl;
  
  // Pull data counters                                                
  uint64_t count = 0;
  
  // Data buffer
  int16_t *data_buffer = new int16_t [buffer_size] ();
  
  // Define the return value of number of messages got from queue
  uint64_t pull_len = 0; // Total length of data
  
  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();
  // Get end time after event distributions
  auto end = chrono::steady_clock::now();
  
  // Infinite loope
  while(1){
    // End condition
    if (udp.timeout){
      // Exit infinite loop
      break;
    }
    // Else pull data from queue                                      
    pull_len = pullq->pull(&udp.timeout,chan,data_buffer);
    // Restart the timing clock when first messages is pulled
    if (count == 0) start = chrono::steady_clock::now();
    // Memcpy data to output arrays
    memcpy(&data[count],data_buffer,pull_len*sizeof(int16_t));
    // Increment distribute event counters
    count += pull_len;
    // Store new end time
    if (pull_len > 0) end = chrono::steady_clock::now();  
  }
  // Find time taken to distribute all events
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = count*sizeof(int16_t)*1e-9;
  
  // Delay cout
  this_thread::sleep_for(4s);
  
  // Store speed as Gbits/s
  double speed = data_size*8/(timeNano*1e-9);
  
  // Notify user
  Logger::Instance()->write(1,"Channel = "
			    + std::to_string(chan)
			    + " [" + udp.get_channel_name(chan)
			    + "] pulled "
			    + std::to_string(data_size)
			    + " Gbytes at "
			    + std::to_string(speed)
			    + " Gbit/s");
  
}

// Function to compare input data to ouput for testing
void test_funcs::compare_data(int chNum, uint32_t numberhb, uint64_t* data_len, 
			      int16_t** input, int16_t** output){
  
  // Boolean to signal success
  bool* success = new bool[chNum];
  // Loop over channels
  for (int c = 0; c < chNum; c++){
    // Initialise as success
    success[c] = true;
    // Error counter
    uint64_t false_count = 0;
    // Counters
    uint64_t out_count = 0;
    uint64_t count = 0;      
    uint64_t event_count = 0;
    // Loop over all elements in input array
    while(count < data_len[c]){
      // Input array
      uint64_t event_start = count; // Event start index
      uint64_t event_num = dvar.get_event_number(input[c],count);
      uint16_t event_len = input[c][event_start+fw_tHdr::EvLen];
      // Output array
      uint64_t out_event_start = out_count;
      uint64_t out_event_num = dvar.get_event_number(output[c],out_count);
      uint16_t out_event_len = output[c][out_event_start+fw_tHdr::EvLen];
      // Check event numbers are the same
      if (out_event_num != event_num){
	std::cout << "Error in compare output data!! Event numbers do not match in  channel " << c << std::endl;
	std::cout << "Input event number = " << event_num << std::endl; 
	std::cout << "Output event number = " << out_event_num << std::endl; 
	std::cout << "Exiting..." << std::endl;
	exit(0);
      }
      // Loop over all header elements before length
      for (uint i = 0; i < fw_tHdr::EvLen; i++){
	// If not equal...
	if (input[c][count+i] != output[c][out_count+i]){
	  // Unsuccesful...
	  false_count++;
	  success[c] = false;
	}
      }
      // Check event lengths are the same (if non-zero)
      if (out_event_len != event_len && out_event_len != 0){
	std::cout << "Error in compare output data!! Event lengths do not match for event " << out_event_num << " in  channel " << c << std::endl;
	std::cout << "Input data event length = " << event_len << std::endl; 
	std::cout << "Output data event length = " << out_event_len << std::endl; 
	std::cout << "Exiting..." << std::endl;
      }
      // Loop over all header elements after length
      for (uint i = fw_tHdr::EvLen+1; i < fw_tHdr_Len; i++){
	// If not equal...
	if (input[c][count+i] != output[c][out_count+i]){
	  // Unsuccesful...
	  false_count++;
	  success[c] = false;
	}
      }
      // Loop over output event length
      if (out_event_len != 0){
	for (uint i = fw_tHdr_Len; i < fw_tHdr_Len+event_len; i++){
	  // If not equal...
	  if (input[c][count+i] != output[c][out_count+i]){
	    // Unsuccesful...
	    false_count++;
	    success[c] = false;
	  }
	}
	// Increment event counter
	event_count++;
      }
      // Increase counters
      count += fw_tHdr_Len + event_len;
      out_count += fw_tHdr_Len + out_event_len;
    }
    // Print results for each channel
    if (success[c]) std::cout << "CHANNEL " << c 
			      << " SUCCESS: Identical input/output for " << event_count << " events (" << numberhb << " headers). Prescale ~ " << round((double)numberhb/(double)event_count) << std::endl;
    if (!success[c]) std::cout << "CHANNEL " << c 
			       << " ERROR: Input/Output data not identical! " 
			       << false_count << "/" << data_len[c] 
			       << " elements = " << (double)false_count/(double)data_len[c]*100 
			       << "% incorrect..." << endl;
    
  }
  
}

