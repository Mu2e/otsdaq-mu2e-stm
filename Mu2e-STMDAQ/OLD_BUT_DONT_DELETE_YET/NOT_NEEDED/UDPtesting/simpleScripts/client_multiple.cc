///////////////////////////////////////////////////////////////////////////////////
/// This module creates a UDP socket for 10G readout (main).  
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

#include<iostream>
#include<fstream>
#include <vector>

#include<string.h> //memset 
#include<arpa/inet.h>
#include <sys/types.h>
#include<sys/socket.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close 

#include <math.h>       /* pow */

#include <thread>

using namespace std;

static const uint16_t maxGbit = 10; // Gbits
static const double maxbytes = maxGbit*1e9*0.125; // bytes  

static const uint16_t packetSize = 8198; // bytes
static const uint16_t BUFLEN = packetSize/2; 

//static const uint packetNum = int(maxbytes)/packetSize;
static const uint packetNum = int(2e9)/packetSize;
//static const uint packetNum = 1e6;

static const uint chNum = 2;

//static const char* IP[chNum] = {"127.0.0.1","127.0.0.2"};
static const char* IP[chNum] = {"192.168.34.12","192.168.34.14"};
static const int PORT[chNum] = {51872,51874};

int client[chNum] = {};

uint packetCount[chNum] = {};

struct packet{
  uint size = 2*BUFLEN;
  int16_t data[BUFLEN] = {};
};

// Initialise data packet and number of packets
packet *dataPacket[chNum];

struct sockaddr_in servaddr[chNum], cliaddr[chNum];

int slen = sizeof(cliaddr);

void die(char *s){
  perror(s);
  exit(1);
}

int createClient(uint chan){

  int sock;

  // Creating UDP socket file descriptor
  if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }
  
  // Zero UDP server address structure
  memset(&servaddr[chan], 0, sizeof(servaddr[chan]));

  // Fill UDP server information
  servaddr[chan].sin_family = AF_INET;            
  servaddr[chan].sin_port = htons(PORT[chan]);   
  servaddr[chan].sin_addr.s_addr = inet_addr(IP[chan]);

  cout << "Channel = " << chan << ": IP = " << IP[chan] << ", PORT = " << PORT[chan] << ", socket = " << sock << endl;

  static const uint32_t sndbufsize = pow(2,21);
  setsockopt(sock,SOL_SOCKET,SO_SNDBUF,
	     (char*)&sndbufsize,sizeof(sndbufsize));

  return sock;

}

// Send packets to socket 
int sendPacket(uint chan, int socket, packet *p){
  
    // Loop over packets to send
  for (uint i = 0; i < packetNum; i++){
    
    // Sleep
    //    usleep(1);
    
    // Send packet
    sendto(socket, p[i].data, p[i].size,
    	   MSG_CONFIRM, (const struct sockaddr *) &servaddr[chan],
    	   sizeof(servaddr[chan]));

    // Increment sent packet counter
    packetCount[chan]++;

  }

  return 1;

}

int main(){

  cout << "Creating distinct packets..." << endl;
  
  // Loop over packets to send
  dataPacket[0] = new packet [packetNum];
  dataPacket[1] = new packet [packetNum];
  for (uint j = 0; j < packetNum; j++){
    for (uint k = 0; k < BUFLEN; k++){      
      dataPacket[0][j].data[k] = j + k;
      dataPacket[1][j].data[k] = 1 + j + k;
    }
  }
    
  for (int i = 0; i < 5; i++){
    cout << "Sending packets in " << 5-i << "..." << endl;
    sleep(1);
  }

  // Loop over channels
  // Create client for each channel
  for (uint i = 0; i < chNum; i++){
    client[i] = createClient(i);
  }

  // Create client thread for each channels
  std::thread *client_thread[chNum];
  
  // Execute client thread for each channel
  for (uint i = 0; i < chNum; i++){
    client_thread[i] = new std::thread(sendPacket,i,client[i],dataPacket[i]);
  }

  // Join client threads
  for (uint i = 0; i < chNum; i++){
    client_thread[i]->join();
    cout << "Channel " << i << " sent " << packetCount[i] << " packets" << endl;
  } 

  return 1;

}
