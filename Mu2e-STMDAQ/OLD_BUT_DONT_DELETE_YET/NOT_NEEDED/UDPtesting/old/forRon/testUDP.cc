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

// Boolean to signal whether we're sending data
// by total size or by number of packets
static const bool dataBySize = false;

// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)
static const uint chNum = 2;

// Istance of UDP socket class
UDPsocket udp[chNum];


// Convert Gbits to bytes
static const double bytes2Gbits = 1/(1e9*0.125);

// Maximum number of Gbits to send 
static const uint16_t maxGbit = 6558.4; // Gbits
static const double maxbytes = maxGbit/bytes2Gbits; // byte

// The number of Gbits to send
double numGbits = 0;

// Total number of packets to send
static const uint32_t maxPackets = 5e5;

// The number of packets to send
uint packetNum = 0;

// Get packet size
static const uint16_t packetSize = udp[0].getBufferSize(); // bytes
// // Get packet length (packetSize/2)
static const uint16_t packetLength = packetSize/2; // bytes

// Sent packet counter
uint sendCount[chNum] = {};
// Receive and put packet into queue counter
uint recvCount[chNum] = {};

// Define arrays for values for each loop
double eff[chNum] = {}; // Efficiency array
double sendSpeed[chNum] = {}; // Send Time array
double recvSpeed[chNum] = {}; // Receive time array

// Boolean to indicate receiving packets has finished
bool recvFinished[chNum] = {};

bool exitFE = false;

using namespace std;

// Setup server to receive packets
int setupServer(int chan){
  
  // create and bind socket
  int socket = udp[chan].bindSocket(udp[chan].createSocket(chan));

  // Set SO_RCVBUF buffer size
  udp[chan].set_SO_RCVBUF(socket,rcvbufsize);

  // Set recvfrom non-blocking timeout
  udp[chan].setTimeout(0,500); // 30 seconds, 0 msecs, 0 usecs, 0 nsecs
  
  return socket;

}

// Client function to send packets
void client(int chan, packet *data, int socket){

  // Notify user
  cout << "In send thread: channel " << chan << ", socket " << socket << endl;

  // Get start time before packet sending
  auto start = chrono::steady_clock::now();

  // Loop over all packets to be sent
  for (uint i = 0; i < packetNum; i++) {
    sendCount[chan] += udp[chan].sendPacket(data[0],socket);
  }

  // Get end time after packet sending
  auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Store time as Gbits/s
  sendSpeed[chan] = numGbits/(timeNano*1e-9);

  // Notify user
  cout << "Channel " << chan << " sent " << sendCount[chan] << " packets" << endl;

  
}

// Server function to receive packets and put into queue
void server(int chan, int socket){

  // Notify user
  cout << "In receive thread: channel " << chan << ", socket " << socket << endl;

  // Define data struct to receive
  packet data;
  data.size = packetSize; // Packet size in bytes
  data.data = new int16_t [packetLength] (); // Packet length

  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  // Timeout counter
  int timeout_count = 0;
  // Maximum number of timeouts
  int timeout_max = 1e7;

  // Loop until timeout
  while(timeout_count != timeout_max){
    // Get packet and check if it times out (returns 0)
    if (udp[chan].getPacket(data,socket,chan) <= 0){
      // Increment timeout counters
      timeout_count++;
      // If the packets have finished being sent...
      if (recvCount[chan]==packetNum) {
	// ... break infinite loop
	break;
      } // Else continue..
    }
    // If not timed out...
    else{
      // Increment the received packet counter
      recvCount[chan]++;
      // Reset timeout counter
      timeout_count = 0;
    } // End if timeout
  } // End infinite loop
   
  // Signal receiving data is finished
  recvFinished[chan] = true;

  // Get end time after packet receiving
  auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Store speed as Gbits/s
  recvSpeed[chan] = numGbits*recvCount[chan]/packetNum/(timeNano*1e-9);

  // Notify user
  cout << "Channel " << chan << " received " << recvCount[chan] << " packets" << endl;


}

// Main function
int main(){

  // If sending data by total size
  if (dataBySize){
    // Set the number of Gbits to send as the maximum Gbits
    numGbits = maxGbit;
    // Set the number of packets for the given number of Gbits and packet size
    packetNum = int(maxbytes/packetSize);
  }
  // else if sending data by total number of packets
  else{
    // Set the number of packets to send as the maximum number of packets
    packetNum = maxPackets;
    // Set the number of Gbits for the given number of packets and packet size
    numGbits = double(packetNum)*double(packetSize)*double(bytes2Gbits);
  }

  // Define data structs for each channel
  packet *sendPacket[chNum]; 
  // Only one packet to send on repeat
  sendPacket[0] = new packet [1];
  // Set packet size in bytes
  sendPacket[0][0].size = packetSize; 
  // Initialise packet with length  
  sendPacket[0][0].data = new int16_t [packetLength] (); 
  for (int i = 0; i < packetLength; i++){
    // Fill packet with distinct data
    sendPacket[0][0].data[i] = packetLength - i;
  }
  // Copy data for second channel
  sendPacket[1] = sendPacket[0];

  // Print message about data to be sent
  cout << "Sending " << numGbits/bytes2Gbits*1e-9 << " Gbytes as " << packetNum << " packets of size " << packetSize*1e-3 << " kbytes" << endl;
  
  // Get time now
  auto start = chrono::steady_clock::now();
  // Loop through and touch all packets
  for (uint i = 0; i < packetNum; i++) sendPacket[0][0].data = sendPacket[0][0].data;
  // Get time after loop
  auto end = chrono::steady_clock::now();
  // Find time taken to touch all packets
  auto diff = end - start;
  // Get time in nano seconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  
  // Print time taken to loop through pakcets
  cout << "Time taken to loop through 1 * " << packetNum << " packets = " << timeNano*1e-6 << " ms" << endl;
  // Print speed to loop through pakcets
  cout << "Speed to loop through 1 * " << packetNum << " packets = " << maxGbit/(timeNano*1e-9) << " Gbit/s" << endl;
  

  // Define threads
  std::thread *receive_thread[chNum];
  std::thread *send_thread[chNum];
  
  // Send/receive sockets for each channel
  int sendSock[chNum] = {};
  int recvSock[chNum] = {};

  // Loop over number of channels and threads
  for (uint i = 0; i < chNum; i++){    
    // Setup client
    sendSock[i] = udp[i].createClient(i);
    
    // Setup server
    recvSock[i] = setupServer(i);

    // Set SO_RCVBUF
    uint32_t rcvbuff = 1e4*pow(2,16);
    udp[i].set_SO_RCVBUF(recvSock[i],rcvbuff);    
    
    // Set SO_SNDBUF
    uint32_t sndbuff = pow(2,21);
    udp[i].set_SO_SNDBUF(sendSock[i],sndbuff);    
    
  }

  // Loop over number of channels and start receive/getthreads
  for (uint i = 0; i < chNum; i++){   
    // Start queue receive thread
    receive_thread[i] = new std::thread (&server,i,recvSock[i]);
  }
  
  // Pause and countdown to sending
  for (int i = 0; i < 3; i++){
    cout << "Sending data in " << 3-i << endl;
    sleep(1);
  }	    
  // Loop over number of channels and send threads
  for (uint i = 0; i < chNum; i++){          
      send_thread[i] = new std::thread (&client,i,sendPacket[i],sendSock[i]);
  }    
  

  // Loop over number of channels and join threads
  for (uint i = 0; i < chNum; i++){    
    
    // Join client thread
    send_thread[i]->join();    
    // Join server thread
    receive_thread[i]->join();
    
    eff[i] = double(recvCount[i])/double(packetNum)*100;
    cout << "Channel " << i << ": efficiency = " << eff[i] << " %. " 
	 << "Send speed = " << sendSpeed[i] 
	 << " Gbit/s. Receive + speed = " << recvSpeed[i] 
	 << " Gbit/s." << endl;      
  }

  return 0;
  
}


