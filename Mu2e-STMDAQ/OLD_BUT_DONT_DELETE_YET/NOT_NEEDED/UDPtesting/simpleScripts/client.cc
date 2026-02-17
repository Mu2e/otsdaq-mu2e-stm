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

using namespace std;

static const uint16_t maxGbit = 10; // Gbits
static const double maxbytes = maxGbit*1e9*0.125; // bytes  

static const uint16_t packetSize = 8198; // bytes
static const uint16_t BUFLEN = packetSize/2; 

struct packet{
  uint size = 2*BUFLEN;
  int16_t data[BUFLEN] = {};
};

struct sockaddr_in servaddr, cliaddr;

int slen = sizeof(cliaddr);

void die(char *s){
  perror(s);
  exit(1);
}

int createClient(){

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
  servaddr.sin_port = htons(51872);   
  servaddr.sin_addr.s_addr = inet_addr("192.168.34.12");

  return sock;

}

// Send packet to socket 
int sendPacket(struct packet& p, int socket){

  usleep(1);

  sendto(socket, p.data, p.size,
	 MSG_CONFIRM, (const struct sockaddr *) &servaddr,
	 sizeof(servaddr));

  return 1;

}

int main(){

  int socket = createClient();

  packet dataPacket;

  uint packetNum = int(maxbytes)/packetSize;
  
  uint packetCount = 0;

  for (int i = 0; i < packetNum; i++) packetCount += sendPacket(dataPacket,socket);

  cout << "Sent " << packetCount << " packets\n" << endl;

  return 1;

}
