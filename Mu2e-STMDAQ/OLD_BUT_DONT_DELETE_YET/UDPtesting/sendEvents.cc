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

// UDPsocket
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Gen data
#include "STMDAQ-TestBeam/UDPtesting/genData.hh"

// !!!! CHANGE THESE ONLY !!! //
// Number of heartbeats
static const uint32_t numberhb = 0xFFF;
// Heartbeat length (multiple of 8ns) 
static const uint32_t deltahb = 0x30D4; 
// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)
static const uint chNum = 1;
// !!!!!!!!!!!!!!!!!!!!!!!!!! //

// Event length (ns)
static const uint32_t deltahb_ns = deltahb*8;
// Event length (s)
static const double deltahb_s = deltahb_ns*1e-9;
// 300 Msamples per second
static const uint64_t sampsPerSec = 3e8;
// Event length (counts)
static const uint64_t deltahb_len = deltahb_s*sampsPerSec;

// The first event number
uint32_t start_event_num = 99;

// Instance of UDP socket class
UDPsocket udp[chNum];

// Total number of packets with data to generate
static const uint64_t genNum = 0xFFFF;

// Generate data
genData gen(genNum,deltahb);

// Convert Gbits to bytes
static const double bytes2Gbits = 8e-9;

// Initialise events
uint64_t event_array_len[chNum] = {};
int16_t* event_data[chNum];

// Firmware trigger header struct from dataVars.hh
fw_tHdr tHdr;

dataVars dvs;

// Client function to send packets
void client(int chan, int socket){

  // Notify user
  cout << "In send thread: channel " << chan << ", socket " << socket << endl;

  // Send data counter
  uint64_t sendCount = 0;

  // Get the total event length (with header)
  uint16_t event_len = fw_tHdr_Len + deltahb_len;

  // Define receive buffer as array of events
  int16_t *snd_buffer[udp[chan].SENDMMSG_NUM];
  uint16_t *size_buffer = new uint16_t [udp[chan].SENDMMSG_NUM] ();
  for (int i = 0; i < udp[chan].SENDMMSG_NUM; i++){
    // Initalise send buffer to zero
    snd_buffer[i] = new int16_t [event_len] ();
    size_buffer[i] = event_len;
  }
  
  // Get start time before packet sending
  auto start = chrono::steady_clock::now();

  // Define a message counter
  int msgNum = 0;

  // Number of cycles through the generated data
  int genCount = 0;

  // Loop over all packets to be sent 
  for (uint i = 0; i < numberhb; i++) {
    // Memcpy packet to buffer
    memcpy(snd_buffer[msgNum],&event_data[chan][(i%genNum)*event_len],event_len*sizeof(int16_t));
    // Increment message counter
    msgNum++;
    // Check if SENDMMMSG_NUM has been reached
    if (msgNum == udp[chan].SENDMMSG_NUM){
      // Sleep to mimic fw sending
      //      this_thread::sleep_for(5ms);
      // Send buffer
      udp[chan].send(snd_buffer,size_buffer,udp[chan].SENDMMSG_NUM,socket);
      // Increase counter by number of sent messages
      sendCount += udp[chan].SENDMMSG_NUM;
      // Reset message counter
      msgNum = 0;
    }
    // Send any leftover packets at end of loop
    else if (i == numberhb-1){
      // Send remaining buffer
      for (int j = 0; j < msgNum; j++){
	uint64_t enumb = dvs.get_event_number(snd_buffer[j],0);
	std::cout << i << "   " << j << "   " << enumb << std::endl;
      }
      udp[chan].send(snd_buffer,size_buffer,msgNum,socket);
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
  double data_size = sendCount*(fw_tHdr_Size + 2*deltahb_len)*1e-9;

  // Store speed as Gbits/s
  double sendSpeed = data_size*8/(timeNano*1e-9);

  // Notify user
  cout << "Channel = " << chan 
       << " [" << udp[chan].get_channel_name(chan)
       << "] sent " << sendCount
       << " *  " << deltahb_ns 
       << " ns events [" << data_size
       << " Gbytes] at "<< sendSpeed << " Gbit/s" << endl;

  // Pause to timeout
  for (int i = 0; i < 5; i++){
    cout << "Timing out in " << 5-i << endl;
    sleep(1);
  }	    
  // Timeout
  udp[chan].timeout = true;  
  
}

// Main function
int main(){

  std::cout << "Generating data..." << std::endl;
  // Loop over channels
  for (uint c = 0; c < chNum; c++){
    // Generate venets
    std::pair<uint64_t,int16_t*> events = gen.genEvents(c,genNum);
    // Store event array length
    event_array_len[c] = events.first;
    // Store event data
    event_data[c] = events.second;
  }
  
  // Define threads
  std::thread *send_thread[chNum];

  // Send/receive sockets for each channel
  int sendSock[chNum] = {};

  // Loop over number of channels and threads
  for (uint c = 0; c < chNum; c++){    
    
    // Setup clients
    sendSock[c] = udp[c].setupClient(c,SW);

  }

  // Pause and countdown to sending
  for (int i = 0; i < 3; i++){
    cout << "Sending data in " << 3-i << endl;
    sleep(1);
  }	    

  // Print message about data to be sent
  cout << "Sending " << numberhb*(fw_tHdr_Size + 2*deltahb_len)*1e-9 
       << " Gbytes as " << numberhb << " * " << deltahb_ns 
       << " ns events of size " << fw_tHdr_Size + 2*deltahb_len 
       << " bytes" << endl;
  
  // Loop over number of channels and send threads
  for (uint c= 0; c < chNum; c++){          
    send_thread[c] =  new std::thread (&client,c,sendSock[c]);
  }    
  
  // Loop over number of channels and join threads
  for (uint c = 0; c < chNum; c++){
    // Join send thread
    send_thread[c]->join();      
  }

  return 0;
  
}

