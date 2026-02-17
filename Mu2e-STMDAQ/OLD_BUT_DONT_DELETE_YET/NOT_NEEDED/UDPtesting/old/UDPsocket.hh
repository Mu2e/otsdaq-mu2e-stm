/////////////////////////////////////////////////////////////////////////////////// 
/// This module creates a UDP socket for 10G readout (header).
///////////////////////////////////////////////////////////////////////////////////

#ifndef UDPSOCKET_hh
#define UDPSOCKET_hh

// Hex reader  
//#include "STMDAQ-TestBeam/utils/Hex.hh"

#include<iostream>
#include<fstream>
#include <vector>

#include<string.h> //memset                                                                            
#include<arpa/inet.h>
#include <sys/types.h>
#include<sys/socket.h>
#include <vector>

#include <fcntl.h> // for open
#include <unistd.h> // for close 

#include <time.h>

using namespace std;

//static const char* IP = "192.168.34.18";

const static int READ = 0;
const static int WRITE = 1;

static const uint32_t rcvbufsize = 65536;

struct packet{
  int16_t size;
  int16_t data[8198/2] = {};
};

class UDPsocket {

public:

  UDPsocket();

  //  const char* IP = "127.0.0.1";
  const char* IP = "192.168.34.12";
  const int PORT = 51872;  

  uint timeout_counter = 0;
  static const uint timeout_max = 5e7;
  bool timeout_wait = false;
  bool timeout = false;

  uint16_t getBufferSize(){
    return BUFSIZE;
  }

  uint16_t getBufferLength(){
    return BUFLEN;
  }

  struct sockaddr_in servaddr, cliaddr;

  int slen = sizeof(cliaddr);

  int timeout_count = 0;

  uint16_t length;
   
  int16_t *size;
  int16_t *buffer;

  fd_set readfds;
 
  // Socket error message function
  void die(char *s){
    perror(s);
    exit(1);
  }

  // Get socket IP address from xml file
  const char* getIPaddress(int READWRITE);

  // Get socket port number from xml file
  int getPort(int READWRITE);

  // Create UDP socket
  int createSocket(int READWRITE);  

  // Create UDP client
  int createClient(int READWRITE);  

  // Bind socket to port
  int bindSocket(int sock);

  // Set SO_RCVBUF size
  int set_SO_RCVBUF(int sock, uint32_t size);

  // Set SO_SNDBUF size
  int set_SO_SNDBUF(int sock, uint32_t size);

  // Set recvfrom non-blocking timeout
  int setTimeout(int secs,double usecs);

  // Set bugger length
  int setBufferLength();

  // Send 10G packet
  int sendPacket(struct packet& p, int sock);

  // Get 10G packet
  //  int getPacket(struct packet& p, int sock);
  packet getPacket(int sock);

  // Flush any previously sent packets
  int flushPackets(int sock);

private:

  uint16_t BUFSIZE = 8198; //Max size of buffer    
 //  uint16_t BUFSIZE = 20; //Max size of buffer    
  uint16_t BUFLEN = BUFSIZE/2; //Max length of buffer  


};




#endif
