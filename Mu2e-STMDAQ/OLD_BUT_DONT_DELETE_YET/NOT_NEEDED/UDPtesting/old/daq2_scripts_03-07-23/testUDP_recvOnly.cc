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
#include "queue_bf.hh"

// Boolean to indicate using circular buffer queue
bool useQueue = false;
// Boolean to indicate using circular buffer queue
bool writeData = false;

// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)
static const uint chNum = 2;

// Istance of UDP socket class
UDPsocket udp[chNum];

// Instance of circular buffer queue class
queue cbq;

// Instance of write buffer queue class
queue_bf_buffer bfq;

// Convert Gbits to bytes
static const double bytes2Gbits = 1/(1e9*0.125);

// Total number of packets to send
static const uint32_t maxPackets = 1e6;

// The number of packets to send
uint packetNum = 1778;

// Get packet size
static const uint16_t packetSize = udp[0].getBufferSize(); // bytes
// // Get packet length (packetSize/2)
static const uint16_t packetLength = packetSize/2; // bytes

// Receive and put packet into queue counter
uint recvCount[chNum] = {};
// get packet from queue counter
uint getCount[chNum] = {};

// Define arrays for values for each loop
double eff[chNum] = {}; // Efficiency array
double recvSpeed[chNum] = {}; // Receive time array
double getSpeed[chNum] = {}; // Get time array 

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
      if (recvCount[chan] == 0) start = chrono::steady_clock::now();
      // If using circular buffer, place packet in queue
      if (useQueue) cbq.push(chan,data.data);
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
  recvSpeed[chan] = packetSize*bytes2Gbits*recvCount[chan]/(timeNano*1e-9);

  // Notify user
  cout << "Channel " << chan << " received " << recvCount[chan] << " packets" << endl;

  // Reconstruct the packet number                                  
  uint32_t lastPacketNum = (uint32_t) data.data[1] << 16 | (uint32_t) data.data[0];  

  // Notify user
  cout << "Channel: " << chan << " last packet = " << lastPacketNum << endl;


}

// Function to get packets from queue queue
void queue_get(int chan){
  
  // Notify user
  cout << "In queue get thread: channel " << chan << endl;
  
  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  // Infinite loop
  while(1){
    // End condition
    if ((recvFinished[chan] && (getCount[chan] == recvCount[chan]))
	or (getCount[chan] == packetNum)){
      // Exit infinite loop
      break;
    }    
    // Else get data packet from queue
    if (writeData){
      bfq.push(chan,cbq.pull(chan));
    }
    else{
      cbq.pull(chan);
    }
    // Increment got packet counter
    getCount[chan]++;
  }
  
  // Notify user
  cout << "Channel " << chan << " got " << getCount[chan] << " packets from queue" << endl;

  // Get end time after packet receiving
  auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Store speed as Gbits/s
  getSpeed[chan] = packetSize*bytes2Gbits*getCount[chan]/(timeNano*1e-9);

}

// Monitor files to open/close
void write_monitor(int chan){
  while(!bfq.check_write_finished(chan)) bfq.monitor_files(chan);
}

// Write data to file
void write_data(int chan){

  cout << "In write data thread: channel " << chan << endl;

  // Infinte loop
  while(1){
    // Try to write file
    bfq.try_write_file(chan);
    // If exit has been signalled
    if (exitFE){
      // Write the remaining data
      bfq.write_end(chan);
      // Signal that the write has finished
      bfq.signal_write_finished(chan);
      // Break infinite loopb
      break;
    }
  }
}

// Main function
int main(){
  
  // Define threads
  std::thread *write_monitor_thread[chNum];
  std::thread *receive_thread[chNum];
  std::thread *get_thread[chNum];      
  std::thread *write_thread[chNum];

  // Send/receive sockets for each channel
  //  int sendSock[chNum] = {};
  int recvSock[chNum] = {};

  // Loop over number of channels and threads
  for (uint i = 0; i < chNum; i++){    

    // // Setup client
    // sendSock[i] = udp[i].createClient(i);

    // Setup server
    recvSock[i] = setupServer(i);

    // Set SO_RCVBUF
    uint32_t rcvbuff = 1000*pow(2,16);
    udp[i].set_SO_RCVBUF(recvSock[i],rcvbuff);    

    // // Set SO_SNDBUF
    // uint32_t sndbuff = pow(2,21);
    // udp[i].set_SO_SNDBUF(sendSock[i],sndbuff);    

  }

  // Loop over number of channels and start receive/getthreads
  for (uint i = 0; i < chNum; i++){   
    // Start queue receive thread
    receive_thread[i] = new std::thread (&server,i,recvSock[i]);
    // Start queue get thread
    if (useQueue) get_thread[i] = new std::thread (&queue_get,i);
    // If writing data to file...
    if (writeData){
      // Start write data monitor thread
      write_monitor_thread[i] = new std::thread(&write_monitor,i);       
      // Start write data thread
      write_thread[i] = new std::thread (&write_data,i);
    }
  }
  
  // Loop over number of channels and join threads
  for (uint i = 0; i < chNum; i++){    
    
    // Join server thread
    receive_thread[i]->join();
    // Join server thread
    if (useQueue) get_thread[i]->join();    

    // Calculate and print efficiencyefficiency
    if (useQueue){ // If using queue...
      eff[i] = double(getCount[i])/double(packetNum)*100;
      cout << "Channel " << i << ": efficiency = " << eff[i] << " %. " 
	   << "Receive + put speed = " << recvSpeed[i] 
	   << " Gbit/s. Get speed = " << getSpeed[i] 
	   << " Gbit/s." << endl;      
    }
    else{ // If not using queue...
      eff[i] = double(recvCount[i])/double(packetNum)*100;
      cout << "Channel " << i << ": efficiency = " << eff[i] << " %. " 
	   << "Receive + speed = " << recvSpeed[i] 
	   << " Gbit/s." << endl;      
    }
  }

  // If writing data to file...
  if (writeData){    
    // Counter to stop write thread
    int count = 0;
    // Wait 5 seconds...
    while(count < 5){
      std::cout << "Stopping write in " << 5 - count << std::endl;
      sleep(1);
      count++;
    }
    // Signal exit
    exitFE = true;
    for (uint i = 0; i < chNum; i++){
      // Join write thread
      write_thread[i]->join();
      // Join the monitor thread
      write_monitor_thread[i]->join();
    }
  exit(0);
  }

  return 0;
  
}


