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

// Instance of UDP socket class
UDPsocket udp;

// Instance of circular buffer queue class
queue cbq;

// Maximum number of Gbits to send 
static const uint16_t maxGbit = 10; // Gbits
static const double maxbytes = maxGbit*1e9*0.125; // byte

// The number of packets in 10 Gbits
uint packetNum = 0;

// Number of times to send maxGbits
static const int loopNum = 1;
// Number of powers of 2 to check for SO_RCVBUF/SO_SNDBUF values
static const int powerMax = 1;

// Received packet counter
uint recvCount = 0;
// Received packet buffer counter
uint buffCount = 0;

uint32_t rcv_timeout = 0;

int16_t* rcvData;

// Define arrays for values for each loop
double eff[loopNum] = {}; // Efficiency array
double recvSpeed[loopNum] = {}; // Receive time array

// Define variables for average over loop number
double effAvg[powerMax] = {}; // Efficiency average
double recvSpeedAvg[powerMax] = {}; // Receive time average

// The maximum number of packets we expect to receive
static const uint maxPackets = 250000;

// Timeout counter
uint timeout_count = 0;

using namespace std;

void init_queue(){

  // Get the buffer size in multiples of bytes in a memory page
  int pageNum = 50;
  size_t BUFFER_SIZE = pageNum*getpagesize();
  cout << "Buffer size = " << pageNum << " memory pages" << endl;
  cout << "            = " << BUFFER_SIZE << " bytes" << endl;

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

// Server function to receive packets
void server(int loop, packet data, int socket){

  // Get start time before packet receiving
  //  auto start = chrono::steady_clock::now();

  std::chrono::time_point<std::chrono::system_clock> a;
  std::chrono::time_point<std::chrono::system_clock> b;

  // Inifinte loop
  while(1){
    cbq.put(udp.getPacket(socket));
    // Increment the received packet counter
    recvCount++;
    // If we've reached the end of the buffer, cycle to the beginning
    if (recvCount == packetNum){
      cout << "Packet receving and putting into queue complete" << endl;
      break;
    }
  } // End infinite loop  

  // // Inifite loop
  // while(1){
  //   // Get packet and check if it times out (returns 0)
  //   if (udp.getPacket(data,socket) == 0){
  //     timeout_count++;
  //     // If the packets have finished being sent...
  //     // if (sendFinished) {
  //     // 	// ... break infinite loop
  //     // 	cout << "Sending complete" << endl;
  //     // 	break;
  //     // } // Else continue..
  //     //      else 
  //     if (timeout_count >= maxPackets*200) {
  //      	// ... break infinite loop
  //      	cout << "rcvfrom timeout received more than " << maxPackets*200 << " times. Exiting..." << endl;
  //      	break;
  //     }
  //   }
  //   // If not timed out...
  //   else{
  //     if (recvCount == 0) a = std::chrono::system_clock::now();
  //     // Increment the received packet counter
  //     //      cout << "Received: " << recvCount << endl;
  //     recvCount++;
  //     // If recevied one packet, change timeout to 1 usec
  //     if (recvCount == maxPackets) b = std::chrono::system_clock::now();
  //     // if (recvCount == 1){
  //     // 	udp.setTimeout(0,750); // 0 seconds, 200 usec
  //     // }
  //     //      timeout_count = 0;
  //     // Set UDP timeout from rcv_timeout in secs and usecs
  //     // rcv_timeout - 500000;
  //     // int secs = int(double(rcv_timeout)*1e-9);
  //     // double usecs = rcv_timeout*1e-3 - secs*1e6;
  //     // udp.setTimeout(secs,usecs); 
  //     //      }
  //   } // End if timeout
  // } // End infinite loop

  // Get end time after packet receiving
  // auto end = chrono::steady_clock::now();
  // Find time taken to receive all packets
  auto diff = b - a;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  cout << "Receive time = " << timeNano*1e-9 << " seconds" << endl;
  // Store speed as Gbits/s
  //  recvSpeed[loop] = maxGbit/(timeNano*1e-9);
  recvSpeed[loop] = recvCount*(4192*8*1e-9)/(timeNano*1e-9); // Packet size [bytes] * Gbits in byte / time in secs

}

void queue_get(int loop){

  // Inifite loop
  while(1){
    rcvData = cbq.get();
    // Increment the received packet buffee counter                            
    buffCount++;
    // If we've reached the end of the buffer, cycle to the beginning
    if (buffCount == packetNum){
      cout << "Getting packets from queue complete" << endl;
      break;
    }
  } // End infinite loop


}

// Main function
int main(){

  // Get packet size
  uint16_t packetSize = udp.getBufferSize(); // bytes
  // Get packet length (packetNum/2)
  uint16_t packetLength = udp.getBufferLength(); // bytes

  // Set the number of packets for the given number of Gbits and packet size
  packetNum = int(maxbytes/packetSize);

  // Define data struct to receive
  packet recvPacket;
  recvPacket.size = packetSize; // Packet size in bytes
  recvPacket.data = new int16_t [packetLength] (); // Packet length

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
    uint32_t rcvbuff = pow(2,31);
    udp.set_SO_RCVBUF(recvSock,rcvbuff);    
    
    // Loop over number of iterations to average
    for (int i = 0; i < loopNum; i++){
      
      // Initialise received packet counter to zero
      recvCount = 0;
      // Initialise queue packet counter to zero
      buffCount = 0;
      
      // Start seever thread
      std::thread server_thread(server,i,recvPacket,recvSock);     
      // Start queue get thread
      std::thread get_thread(queue_get,i);

      // Join server thread
      server_thread.join();
      // Join get thread
      get_thread.join();

      // Calculate efficiency
      eff[i] = double(recvCount)/maxPackets*100;

      // Average efficiency sum
      effAvg[p] += eff[i];
      // Receive speed average sum
      recvSpeedAvg[p] += recvSpeed[i];

    } // End i-loop
    
    // Print results for each loop number
    for (int i = 0; i < loopNum; i++){
      cout << i << ", efficiency = " << eff[i] << " %. " 
	   << "Receive speed = " << recvSpeed[i] << " Gbit/s." << endl;
    }
    
    // Calculate efficiency average
    effAvg[p] /= loopNum;

    // Calculate receive speed average
    recvSpeedAvg[p] /= loopNum;
	
  } // End p-loop

  // Print average results
  cout << "-- Average -- " << endl;
  for (int p = 1; p <= powerMax; p++){
    cout << p << "," << effAvg[p] << "," << recvSpeedAvg[p] << endl;
  }

  // Destroy cicular buffer queue
  cbq.destroy();

  return 0;
  
}



