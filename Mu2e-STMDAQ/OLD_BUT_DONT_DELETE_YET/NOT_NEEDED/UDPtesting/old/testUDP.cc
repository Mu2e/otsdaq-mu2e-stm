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
#include "queue.hh"

// Boolean to indicate receiving packets from firmware
bool fw = true;

// Instance of UDP socket class
UDPsocket udp;

// Instance of circular buffer queue class
queue cbq;

// Convert Gbits to bytes
static const double bytes2Gbits = 1/(1e9*0.125);

// Maximum number of Gbits to send 
static const uint32_t maxGbit = 10; // Gbits
static const double maxbytes = maxGbit/bytes2Gbits; // byte

// The number of Gbits to send
double numGbits = 0;

// Boolean to signal whether we're sending data
// by total size or by number of packets
static const bool dataBySize = false;

// Total number of packets to send
//static const uint32_t maxPackets = 1e5;
static const uint32_t maxPackets = 1046359;

// The number of packets to send 
uint packetNum = 0;

// The packets to be received
packet *rcvData;

// The data buffer size in number of packets
static const uint maxBuffSize = 1e5;

// Number of times to send numGbits
static const int loopNum = 1;
// Number of powers of 2 to check for SO_RCVBUF/SO_SNDBUF values
static const int powerMax = 1;

// Sent packet counter
uint sendCount = 0;
// Received packet counter
uint putCount = 0;
// Received packet buffer counter
uint getCount = 0;

// Define arrays for values for each loop
double eff[loopNum] = {}; // Efficiency array
double sendSpeed[loopNum] = {}; // Send Time array
double recvSpeed[loopNum] = {}; // Receive time array
double getSpeed[loopNum] = {}; // Get time array

// Define variables for average over loop number
double effAvg[powerMax] = {}; // Efficiency average
double sendSpeedAvg[powerMax] = {}; // Send time average
double recvSpeedAvg[powerMax] = {}; // Receive time average
double getSpeedAvg[powerMax] = {}; // Get time average

using namespace std;

void init_queue(){

  // Get max packet size
  uint16_t packetSize = udp.getBufferSize(); // bytes    

  // Get the buffer size in multiples of bytes in a memory page
  uint pageNum = 16;
  size_t BUFFER_SIZE = pageNum*getpagesize();
  cout << "Buffer size = " << pageNum << " memory pages" << endl;
  cout << "            = " << BUFFER_SIZE << " bytes" << endl;
  cout << "            = " << double(BUFFER_SIZE)/double(packetSize) << " packets" << endl;

  // Initialise the queue with the given buffer size
  cbq.init(BUFFER_SIZE);

  return;
  
}

// Setup server to receive packets
int setupServer(){
  
  // create and bind socket
  int socket = udp.bindSocket(udp.createSocket(READ));

  // Set SO_RCVBUF buffer size
  udp.set_SO_RCVBUF(socket,rcvbufsize);

  // Set recvfrom non-blocking timeout
  udp.setTimeout(0,750); // 30 seconds, 0 msecs, 0 usecs, 0 nsecs
  
  return socket;

}

// Client function to send packets
void client(int loop, packet data, int socket){

  // Notify user
  cout << "In send thread" << endl;

  // Get start time before packet sending
  auto start = chrono::steady_clock::now();
  
  // Loop over all packets to be sent
  for (uint i = 0; i < packetNum; i++) {
    //    cout << "\nsendCount = " << i;
    // Send packet and increment counter
    sendCount += udp.sendPacket(data,socket);
    //this_thread::sleep_for(10ns);
  }

  // Notify user
  cout << "Packet sending complete" << endl;
  cout << "Sent: " << packetNum << endl;
  while(getCount != packetNum) cout << "Put: " << putCount << ". Get: " << getCount << "\r" << flush;
  cout << endl;

  // Get end time after packet sending
  auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Store time as Gbits/s
  sendSpeed[loop] = numGbits/(timeNano*1e-9);
  
}

// Server function to receive packets
void server(int loop, int socket){

  cout << "In read thread" << endl;

  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  // Inifite loop
  while(!udp.timeout){
    //    cout << "\nputCount = " << putCount;
    // Get UDP packet and put into queue
    cbq.put(udp.getPacket(socket));
    // Increment the received packet counter
    putCount++;
    if (putCount == packetNum){
      cout << "Packet receving and putting into queue complete" << endl;
      cout << "put: tail = " << cbq.q.tail << ", head = " << cbq.q.head << ", tail - head = " << cbq.q.tail - cbq.q.head << endl;
      cout << "put: putCount = " << putCount << ", getCount " << getCount << endl;
      break;
    }
  } // End infinite loop

  
  if (udp.timeout){
    cout << "Only put " << putCount << " packets into queue..." << endl;
    cout << "put: tail = " << cbq.q.tail << ", head = " << cbq.q.head << ", tail - head = " << cbq.q.tail - cbq.q.head << endl;
    cout << "put: putCount = " << putCount << ", getCount " << getCount << endl;
  }

  // Get end time after packet receiving
  auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Store speed as Gbits/s
  recvSpeed[loop] = numGbits*putCount/packetNum/(timeNano*1e-9);

  cout << "Leaving put thread" << endl;

}

void queue_get(int loop){

  cout << "In get thread"  << endl;

  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  if (fw){
    while(!udp.timeout){
      // Get data packet from queue
      rcvData[0] = cbq.get();
      getCount++;
      if (getCount == packetNum){
	cout << "Getting packets from queue complete" << endl;
	cout << "get: tail = " << cbq.q.tail << ", head = " << cbq.q.head << endl;
	cout << "get: putCount = " << putCount << ", getCount " << getCount << endl;
	break;
      }      	
    }
  }  
  else{
    while(!udp.timeout){
      rcvData[getCount++] = cbq.get();
      if (getCount == packetNum){
	cout << "Getting packets from queue complete" << endl;
	cout << "get: tail = " << cbq.q.tail << ", head = " << cbq.q.head << endl;
	cout << "get: putCount = " << putCount << ", getCount " << getCount << endl;
	break;
      }     	
    }
  } 
  
  if (udp.timeout){
    cout << "Only got " << getCount << " packets from queue..." << endl;
    cout << "get: tail = " << cbq.q.tail << ", head = " << cbq.q.head << endl;
    cout << "get: putCount = " << putCount << ", getCount " << getCount << endl;      
  }

  // Get end time after packet receiving
  auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Store speed as Gbits/s
  getSpeed[loop] = numGbits*getCount/packetNum/(timeNano*1e-9);

  cout << "Leaving get thread" << endl;

}

// Main function
int main(){
  
  // Initialise circular buffer queue
  init_queue();

  // Get packet size
  uint16_t packetSize = udp.getBufferSize(); // bytes
  // Get packet length (packetNum/2)
  uint16_t packetLength = udp.getBufferLength(); // bytes

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

  // Define data struct to send
  packet sendPacket; 
  if (!fw){
    sendPacket.size = packetSize; // Packet size in bytes
    for (int i = 0; i < packetLength; i++){
      sendPacket.data[i] = packetLength - i;
    }
  }

  // Define receive data arrays
  if (fw){
    rcvData = new packet [1];
  }
  else{
    rcvData = new packet [packetNum];
  }

  // Print email about data to be sent
  cout << "Sending " << loopNum << " * " << numGbits/double(bytes2Gbits) << " bytes (as " << packetNum << " packets of size " << packetSize*1e-3 << " kbytes)" << endl;

  // Setup client
  int sendSock = udp.createClient(WRITE);

  // Setup server
  int recvSock = setupServer();

  // Set buffer length
  udp.setBufferLength();

  // Print waiting notice to user...
  cout << "Waiting for packets..." <<  endl;

  // Loop through variable to optimise
  for (int p = 1; p <= powerMax; p++){
    
    // Set SO_RCVBUF to 2^p
    // uint32_t rcvbuff = pow(2,p);
    uint32_t rcvbuff = pow(2,16);
    udp.set_SO_RCVBUF(recvSock,rcvbuff);    

    // Set SO_SNDBUF to 2^p
    //    uint32_t sndbuff = pow(2,p);
    uint32_t sndbuff = pow(2,21);
    udp.set_SO_RCVBUF(recvSock,sndbuff);    
    
    // Loop over number of iterations to average
    for (int i = 0; i < loopNum; i++){
      
      // Initialise sent packet counter to zero
      sendCount = 0;
      // Initialise received packet counter to zero
      putCount = 0;
      // Initialise received packet buffer counter to zero
      getCount = 0;

      // Define threads
      std::thread *put_thread;
      std::thread *send_thread;
      std::thread *get_thread;

      // Start queue put thread
      put_thread = new std::thread (server,i,recvSock);     
      // If not using firmware, start send thread
      if (!fw) send_thread = new std::thread (client,i,sendPacket,sendSock);
      // Start queue get thread
      get_thread = new std::thread (queue_get,i);      

      // Join server thread
      put_thread->join();
      // Join client thread
      if (!fw) send_thread->join();
      // Join server thread
      get_thread->join();

      // Calculate efficiency
      eff[i] = double(getCount)/double(packetNum)*100;

      // Average efficiency sum
      effAvg[p-1] += eff[i];
      // Send speed average sum
      sendSpeedAvg[p-1] += sendSpeed[i];
      // Receive speed average sum
      recvSpeedAvg[p-1] += recvSpeed[i];
      // Get speed average sum
      getSpeedAvg[p-1] += getSpeed[i];

    } // End i-loop
    
    // Print results for each loop number
    if (fw){
      cout << "Efficiency = " << eff[0] << " %. " 
	   << " Receive + Put speed = " << recvSpeed[0] << " Gbit/s." 
	   << " Get speed = " << getSpeed[0] << " Gbit/s." 
	   << endl;
    }
    else{
      for (int i = 0; i < loopNum; i++) cout << i << ", efficiency = " << eff[i] << " %. " 
					     << "Send speed = " << sendSpeed[i] 
					     << " Gbit/s. Receive speed = " << recvSpeed[i] 
					     << " Gbit/s. Get speed = " << getSpeed[i] << endl;
    }
    
    // Calculate efficiency average
    effAvg[p] /= loopNum;
    // Calculate send speed average
    sendSpeedAvg[p] /= loopNum;
    // Calculate receive speed average
    recvSpeedAvg[p] /= loopNum;
	
  } // End p-loop
  
  // If sending packets in software...
  if (!fw){

    // Print average results
    cout << "-- Average -- " << endl;
    for (int p = 1; p <= powerMax; p++) cout << p << "," << effAvg[p-1] << "," << sendSpeedAvg[p-1] << "," <<recvSpeedAvg[p-1] << endl;   

    // Check published/consumed data is identical                              
    bool success = true;
    for (uint i = 0; i < packetNum; i++){
      for (int j = 0; j < packetLength; j++){
	//      cout << i << " " << j << " " << sendPacket.data[j]<< " " << rcvData[i].data[j] << endl;
	if (rcvData[i].data[j] != sendPacket.data[j]){
	  success = false;
	}
      }
    }
    if (success) std::cout << "SUCCESS: Input/Output data identical." << endl;
    if (!success) std::cout << "ERROR: Input/Output data not identical!" << endl;

  }

  // Destroy cicular buffer queue
  cbq.destroy();

  return 0;
  
}



