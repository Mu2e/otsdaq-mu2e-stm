///////////////////////////////////////////////////////////////////////////////////
// This module creates a UDP socket for 10G readout (main).  
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

// Environment variables
//#include "STMDAQ-/utils/EnvVars.hh"

// UDP socket header
//#include "STMDAQ-TestBeam/utils/UDPsocket.hh"
#include "UDPsocket.hh"

// Hex reader
//#include "STMDAQ-TestBeam/utils/Hex.hh"

#include<iostream>
#include<fstream>
#include <vector>

#include<string.h> //memset 
#include<arpa/inet.h>
#include <sys/types.h>
#include<sys/socket.h>

#include<time.h> 

#include <fcntl.h> // for open
#include <unistd.h> // for close 

using namespace std;

int recv_len;
int ret;
packet p;

//uint timeout_counter = 0;
//static const uint timeout_max = 1e7;

struct timeval read_timeout;

/*-- UDP Socket Init -------------------------------------------------*/

//Standard constructor - shouldn't be used
UDPsocket::UDPsocket() {}

int UDPsocket::createSocket(int READWRITE){

  int sock;

  // Creating socket file descriptor
  sock=socket(AF_INET, SOCK_DGRAM,0 );
  if ((sock=socket(AF_INET, SOCK_DGRAM,0 )) == -1){ //IPPROTO_UDP           
    die((char*)"socket");
  }

  close(sock);

  // Creating socket file descriptor
  sock=socket(AF_INET, SOCK_DGRAM,0 );
  if ((sock=socket(AF_INET, SOCK_DGRAM,0 )) == -1){ //IPPROTO_UDP           
    die((char*)"socket");
  }
  
  // zero out the structure                                             
  memset((char *) &servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  //  servaddr.sin_port = htons(getPort(READWRITE));  
  servaddr.sin_port = htons(PORT);  
  //  servaddr.sin_addr.s_addr = inet_addr(getIPaddress(READWRITE));
  servaddr.sin_addr.s_addr = inet_addr(IP);

  return sock;

}

int UDPsocket::createClient(int READWRITE){

  int sock;

  // Creating UDP socket file descriptor                                                       
  if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {                                         
    perror("socket creation failed");                                                          
    exit(EXIT_FAILURE);                                                                        
  }                                                                                            
  
  // Zero UDP server address structure                                                         
  memset(&servaddr, 0, sizeof(servaddr));                                                      
  
  // Fill UDP server information                                                               
  servaddr.sin_family = AF_INET;                                                               
  //  servaddr.sin_port = htons(getPort(READWRITE));   
  servaddr.sin_port = htons(PORT);   
  //  servaddr.sin_addr.s_addr = inet_addr(getIPaddress(READWRITE));
  servaddr.sin_addr.s_addr = inet_addr(IP);

  return sock;

}


// Bind socket to port
int UDPsocket::bindSocket(int socket){

  if( bind(socket , (struct sockaddr*)&servaddr, sizeof(servaddr) ) == -1){
    die((char*)"bind");
  }

  // Zero out the client address structure                                             
  memset((char *) &cliaddr, 0, sizeof(cliaddr));

  fflush(stdout);

  return socket;

}  

// Set SO_RCVBUF size
int UDPsocket::set_SO_RCVBUF(int socket, uint32_t size){
  
  int bufferSize = setsockopt(socket,SOL_SOCKET,SO_RCVBUF,(char*)&size,sizeof(size));
  return bufferSize;
  
}  

// Set SO_SNDBUF size
int UDPsocket::set_SO_SNDBUF(int socket, uint32_t size){
  
  int bufferSize = setsockopt(socket,SOL_SOCKET,SO_SNDBUF,(char*)&size,sizeof(size));
  return bufferSize;
  
}  


// Set recvfrom non-blocking timeout    
int UDPsocket::setTimeout(int secs, double usecs){

  read_timeout.tv_sec = secs;
  read_timeout.tv_usec = usecs;

  return 1;

}  

// Set bugger length
int UDPsocket::setBufferLength(){

  length = getBufferLength();
  //  buffer = new int16_t [length];
  //  p.data = new int16_t [length];

  return 1;

}
 
// Send packet to socket 
int UDPsocket::sendPacket(struct packet& p, int socket){

  sendto(socket, p.data, p.size,
	 MSG_CONFIRM, (const struct sockaddr *) &servaddr,
	 sizeof(servaddr));
  
  return 1;

}

// Request packet from socket and return as vector of 16-bit words
//int UDPsocket::getPacket(struct packet& p, int socket){
packet UDPsocket::getPacket(int socket){

  timeout_count = 0;
  while (timeout_count != timeout_max){
    
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);
    ret = select(socket+1, &readfds, NULL, NULL, &read_timeout);
    // If socket has pending data to read
    if (ret > 0){
      // If recv_len error, throw exception
      if ((p.size = recvfrom(socket, p.data, rcvbufsize, 0,
			       (struct sockaddr*) &cliaddr,
			       (socklen_t *) &slen)) < 0){
	die((char*)"recvfrom()");
      }
      // Return packet
      return p;
    }
    else if (ret == 0){
      // Increment timeout_counter
      timeout_count++;
    }
    else{
      cout << "UDP error in UDPsocket::getPacket" << endl;
      cout << "Error selecting... Exiting." << endl;
      exit(0);
    }
  }
   
  // Packet timeout - exit
  cout << "Packet recvfrom timed out! Exiting..." << endl;
  timeout = true;
  //  exit(0);

  return p;

}


