/////////////////////////////////////////////////////
/// This module write data to a binary file (header).
/////////////////////////////////////////////////////
#ifndef BINARY_FILE_hh
#define BINARY_FILE_hh

#include<iostream>
#include<fstream>

#include <thread>
#include <mutex>

#include <time.h>
#include <math.h>

class binaryFile {

private:

  // Number of data channels
  static const int CHNUM = 2;
  // Signal HPGE data as CHANNEL 0
  static const int HPGE = 0;
  // Signal LABR data as CHANNEL 1
  static const int LABR = 1;

  // The run number
  static const uint RUN_NUM = 0;

  // Maximum file size / buffer size (bytes)
  static const uint MAX_FILE_SIZE = 2e9; // 2GB
  // Maximum packet size (bytes)
  static const uint MAX_PACKET_SIZE = 8198; // 
  // Maximum packet length
  static const uint PACKET_LEN = MAX_PACKET_SIZE/2; // int16_t values
  // Actual maximum file size (bytes)
  static const uint PACKETS_PER_FILE = uint(MAX_FILE_SIZE/MAX_PACKET_SIZE); // 2GB 

  // The ring buffer size (multiple of 2GB files)
  static const uint RING_BUFFER_SIZE = 1024;
  
  // The buffer                                                          
  int16_t buffer[CHNUM][PACKETS_PER_FILE][PACKET_LEN] = {{{}}};

  // Atomic put buffer pointer                                                
  std::atomic<int64_t> put[CHNUM];
  // Atomic write data pointer                                                 
  std::atomic<int64_t> write[CHNUM];

  // Current tail pointer                                                
  int64_t current_tail[CHNUM];
  // Next tail pointer                                                   
  int64_t next_tail[CHNUM];
  // Current head pointer                                                
  int64_t current_head[CHNUM];

public:

  // Standard constructor - should'nt be used
  binaryFile(){
    // For each channel, set pointers to zero             
    for (int chan = 0; chan < channels; chan++){
      put[chan].store(0);
      write[chan].store(0);
    }
  }
  
  // Push data to queue                                                  
  void push(int chan, int16_t* data){

    // Try to push data to queue                                         
    while( ! try_push(chan,data) ){};

  }

  // Return the HPGe Channel number
  int HPGe(){
    return HPGE;
  }

  // Return the LaBr Channel number
  int LaBr(){
    return LABR;
  }

  // Check channel is only either 0 or 1
  void check_channel(int CHAN){   
    if (CHAN != HPGE && CHAN != LABR){
      cout << "Error! In UDPsocket::getIPaddress, CHAN = " << CHAN << endl;
      cout << "\tCHAN must equal 0 (HPGe) or 1 (LaBr). Exting...\n" << endl;
      exit(0);
    }    
  }

  // Return the channel name
  string get_channel_name(int CHAN){
    // Check the channel number
    check_channel(CHAN);
    string chan;
    if (CHAN==0){
      chan = "HPGe";
    }
    else{
      chan = "LaBr";
    }
    return chan;
  }
    


};

#endif
