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

using namespace std;

static const uint16_t maxGbit = 10; // Gbits
static const double maxbytes = maxGbit*1e9*0.125; // bytes  

static const uint16_t packetSize = 8198; // bytes
static const uint16_t BUFLEN = packetSize/2; 

static const uint64_t rcvbufsize = 64000000000;

struct packet{
  uint size;
  int16_t* data;
};

// static const uint off_spill_len = 30000;
// static const uint off_spill_size = 2*off_spill_len;

// // Data for a single off-spill event
// struct off_event{
//   // Total payload size (kb)
//   uint16_t size = fw_tHdr_Size + off_spill_size;
//   // Event number
//   uint64_t number = 0;
//   // Event length
//   uint16_t length = 0;
//   // Data array
//   int16_t data[fw_tHdr_Len + off_spill_len] = {};
// };

// off_event event;

struct sockaddr_in servaddr, cliaddr;
struct timeval read_timeout;

int slen = sizeof(cliaddr);

void die(char *s){
  perror(s);
  exit(1);
}

int createSocket(){

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
  servaddr.sin_port = htons(51872);  
  servaddr.sin_addr.s_addr = inet_addr("192.168.34.12");

  return sock;

}

// Bind socket to port
int bindSocket(int socket){

  if( bind(socket , (struct sockaddr*)&servaddr, sizeof(servaddr) ) == -1){
    die((char*)"bind");
  }

  return socket;

}  

// Request packet from socket and return as vector of 16-bit words
int getPacket(struct packet& p, int socket){
  
  int16_t* buffer;
  buffer = new int16_t [BUFLEN];

  int recv_len;
  
  // Zero out the client address structure
  memset((char *) &cliaddr, 0, sizeof(cliaddr));
  
  fflush(stdout);

  fd_set readfds; 
  FD_ZERO(&readfds);
  FD_SET(socket, &readfds);

  int ret = select(socket+1, &readfds, NULL, NULL, &read_timeout);
  if (ret > 0){
    // socket has pending data to read
    if (recv_len = recvfrom(socket, buffer, 2*BUFLEN, 0, 
			    (struct sockaddr*) &cliaddr, 
			    (socklen_t *) &slen) < 0){
      die((char*)"recvfrom()");
    }
  }
  else if (ret == 0){
    return 0;
    // todo: resend the same packet again, or abort the transfer
  }
  else{
    cout << "error selecting" << endl;
  }
  
  p.size = recv_len;
  p.data = buffer;

  return 1;
  
}

int main(){

  int socket = bindSocket(createSocket());

  int bufferSize = setsockopt(socket,SOL_SOCKET,SO_RCVBUF,(char*)&rcvbufsize,sizeof(rcvbufsize));
  cout << "SO_RCVBUF = " << bufferSize << endl;

  packet dataPacket;

  uint packetCount = 0;

  uint packetNum = int(maxbytes)/packetSize;

  cout << "Waiting for packets..." <<  endl;

  read_timeout.tv_sec = 15;
  read_timeout.tv_usec = 0; 

  while(1){
    if (getPacket(dataPacket,socket) == 0){
      cout << "Socket timed out!" << endl;
      break;
    }
    else{
<<<<<<< HEAD
      for (int i = 0; i < 50; i++){
	cout << i << " " << dataPacket.data[i] << endl;
      }
      exit(0);
=======
      std::cout << "Recevied packet " << packetCount << std::endl;
>>>>>>> bde1220c1687242e5c4d658ec8637a27ae90fbe5
      packetCount += 1;
      if (packetCount == 1){
       	read_timeout.tv_sec = 0;
       	read_timeout.tv_usec = 100; 
      }
    }

  }

  cout << "\nReceived " << packetCount << "/" << packetNum << " expected packets" << " = " << double(packetCount)/double(packetNum)*100 << " % \n" << endl;


  return 1;

}


