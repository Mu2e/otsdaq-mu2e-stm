///////////////////////////////////////////////////////////////////////////////////
/// This module creates a UDP socket for 10G readout (main).  
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

#include<iostream>
#include<fstream>
#include <vector>

#include <thread>
#include <mutex>

#include<string.h> //memset 
#include<arpa/inet.h>
#include <sys/types.h>
#include<sys/socket.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close 

#include <time.h>
#include <math.h>

#include "UDPsocket.hh"


// Convert Gbits to bytes
static const double bytes2Gbits = 1/(1e9*0.125);

// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)
static const uint chNum = 2;

// Instance of UDP socket class
UDPsocket udp[chNum];

// The packets to be received
packet rcvData[chNum];

// Received packet counter
uint rcvCount[chNum] = {};

// Define arrays for values for each chan
double recvSpeed[chNum] = {}; // Receive time array

using namespace std;

// Setup server to receive packets
int setupServer(int chan){

  // create and bind socket
  int socket = udp[chan].bindSocket(udp[chan].createSocket(chan));

  // Set SO_RCVBUF buffer size
  uint32_t rcvbuff = pow(2,16);
  udp[chan].set_SO_RCVBUF(socket,rcvbuff);    

  // Set recvfrom non-blocking timeout
  udp[chan].setTimeout(0,750); // 30 seconds, 0 msecs, 0 usecs, 0 nsecs
  
  return socket;

}

// Server function to receive packets
void server(int chan, int socket){

  // Print waiting notice to user...
  cout << "In channel " << chan << " read thread" << endl;
  cout << "Waiting for packets..." <<  endl;

  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  // Inifite loop
  while(!udp[chan].timeout){
    // Get UDP packet and rcv into queue
    rcvData[chan] = udp[chan].getPacket(socket);
    // Increment the received packet counter
    if (!udp[chan].timeout) rcvCount[chan]++;
  } // End infinite loop
  
  // Get end time after packet receiving
  auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Store speed as Gbits/s
  recvSpeed[chan] = rcvCount[chan]*udp[chan].getBufferSize()*1e-6/(timeNano*1e-9);

}

// Main function
int main(){
  
  // Define threads
  std::thread *rcv_thread[chNum];

  // Setup server
  int recvSock[chNum];
  
  // Start servers for each channel
  for (uint i = 0; i < chNum; i++){
    recvSock[i] = setupServer(i);
    // Set buffer length
    udp[i].setBufferLength();
  }

  
  // Loop over data channels
  for (uint i = 0; i < chNum; i++){
    // Initialise received packet counter to zero
    rcvCount[i] = 0;
    // Start receive threads
    rcv_thread[i] = new std::thread (server,i,recvSock[i]);     
  }
  
  // Join threads threads
  for (uint i = 0; i < chNum; i++){
    rcv_thread[i]->join();  
  }

  // Print results for each chan numbe  
  for (uint i = 0; i < chNum; i++) cout << "Ch " << i << ": packets received = " << rcvCount[i] << ". " 
					 << " Receive speed = " << recvSpeed[i] 
					 << " Gbit/s." << endl;
  
  return 0;
  
}



