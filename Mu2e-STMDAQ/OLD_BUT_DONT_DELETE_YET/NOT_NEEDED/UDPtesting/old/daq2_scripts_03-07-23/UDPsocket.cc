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
int ret[2];
struct timeval read_timeout;


/*-- UDP Socket Init -------------------------------------------------*/

//Standard constructor - shouldn't be used
UDPsocket::UDPsocket() {}

// const char* UDPsocket::getIPaddress(int READWRITE){

//   const char* ip_address;

//   // Get UDP IP address from environment varaibles
//   string ip;
//   // If reading data...
//   if (READWRITE == READ){
//     ip = EnvVars::expand("${STM_READ_IP}");
//   }
//   // If writing data...
//   else if (READWRITE == WRITE){
//     ip = EnvVars::expand("${STM_WRITE_IP}");
//   }
//   // Else if read/write is not 0 or 1...
//   else{
//     cout << "Error! In UDPsocket::getIPaddress, READWRITE = " << READWRITE << endl;
//     cout << "\tREADWRITE must equal 0 or 1. Exting...\n" << endl;
//     exit(0);
//   }
//   ip_address = ip.c_str();
//   cout << "IP ADDRESSS IS: " << ip_address << endl;

//   return ip_address;

// }

// int UDPsocket::getPort(int READWRITE){

//   // Get UDP port number from environment variables
//   string portNum;
//   // If reading data...
//   if (READWRITE == READ){
//     portNum = EnvVars::expand("${STM_READ_PORT}");
//   }
//   // If writing data...
//   else if (READWRITE == WRITE){
//     portNum = EnvVars::expand("${STM_WRITE_PORT}");
//   }
//   // Else if read/write is not 0 or 1...
//   else{
//     cout << "Error! In UDPsocket::getPort, READWRITE = " << READWRITE << endl;
//     cout << "\tREADWRITE must equal 0 or 1. Exting...\n" << endl;
//     exit(0);
//   }

//   const int port = stoi(portNum);

//   return port;

// }


int UDPsocket::createSocket(int CHANNEL){

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
  servaddr.sin_port = htons(getPORT(CHANNEL));  
  //  servaddr.sin_addr.s_addr = inet_addr(getIPaddress(READWRITE));
  servaddr.sin_addr.s_addr = inet_addr(getIPaddress(CHANNEL));

  cout << "SERVER: Channel = " << CHANNEL << ": IP = " << getIPaddress(CHANNEL) << ", PORT = " << getPORT(CHANNEL) << ", socket = " << sock << endl;

  // Set socket to allow port re-use / to reuse port
  int optval = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

  return sock;

}

// int UDPsocket::closeSocket(int sock){

//   return sock;

// }


int UDPsocket::createClient(int CHANNEL){

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
  servaddr.sin_port = htons(getPORT(CHANNEL));   
  //  servaddr.sin_addr.s_addr = inet_addr(getIPaddress(READWRITE));
  servaddr.sin_addr.s_addr = inet_addr(getIPaddress(CHANNEL));

  cout << "CLIENT: Channel = " << CHANNEL << ": IP = " << getIPaddress(CHANNEL) << ", PORT = " << getPORT(CHANNEL) << ", socket = " << sock << endl;

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

// Send packet to socket 
int UDPsocket::sendPacket(struct packet& p, int socket){
  
  sendto(socket, p.data, p.size,
	 MSG_CONFIRM, (const struct sockaddr *) &servaddr,
	 sizeof(servaddr));
  
  return 1;

}

// Request packet from socket and return as vector of 16-bit words
int UDPsocket::getPacket(packet &p, int socket, int chan){

  FD_ZERO(&readfds);
  FD_SET(socket, &readfds);
  ret[chan] = select(socket+1, &readfds, NULL, NULL, &read_timeout);
  if (ret[chan] > 0){
    // socket has pending data to read
    if ((p.size = recvfrom(socket, p.data, rcvbufsize, 0,
			 (struct sockaddr*) &cliaddr,
			 (socklen_t *) &slen)) < 0){
      die((char*)"recvfrom()");
    }
  }
  else if (ret[chan] == 0){
    return 0;
    // todo: resend the same packet again, or abort the transfer
  }
  else{
    cout << "error selecting: ret = " << ret[chan] << endl;
  }
  
  return 1;

}

// Flush queueds packet from socket and return 0 when no bytes left to receive
int UDPsocket::flushPackets(int socket){

  uint16_t length = getBufferLength();
  buffer = new int16_t [length];
  int recv_len = 1;

  // Zero out the client address structure                                             
  memset((char *) &cliaddr, 0, sizeof(cliaddr));

  fflush(stdout);

  cout << "Flushing previous packets..." << endl;

  while (1){
    if ((recv_len = recvfrom(socket, buffer, rcvbufsize, MSG_DONTWAIT, 
			     (struct sockaddr*) &cliaddr, 
			     (socklen_t *) &slen)) == -1){
      cout << "No packets in queue!" << endl;
      break;
    }
    else{
      cout << "Flushed packet with size " << recv_len << " bytes." << endl;
    }
  }

  return recv_len;

}

