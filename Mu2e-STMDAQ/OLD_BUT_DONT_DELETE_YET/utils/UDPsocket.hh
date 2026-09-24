/////////////////////////////////////////////////////////////////////////////////// 
/// This module creates a UDP socket for 10G readout (header).
///////////////////////////////////////////////////////////////////////////////////

#ifndef UDPSOCKET_hh
#define UDPSOCKET_hh

// Hex reader  
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// Logger
#include "STMDAQ-TestBeam/utils/Logger.hh"

#include<iostream>
#include<fstream>
#include<vector>

#include<string.h> //memset                                                                            
#include<arpa/inet.h>
#include <sys/types.h>
#include<sys/socket.h>
#include <vector>

#include <fcntl.h> // for open
#include <unistd.h> // for close 

#include <time.h>
#include <math.h>

class UDPsocket {

private:

  // The maximum udp datagram size
  // Theoretical max = 65,535.
  // Actual IPv4 max = (65,535 bytes − 8-byte UDP header − 20-byte IP header)
  static const uint16_t MAX_UDP_SIZE = 65504;

  // SET THIS ON INITIALISATION LATER

  // The optimised SO_RCVBUF size
  static const uint64_t RCVBUF_SZ = 268435456;
  // The optimised SO_SNDBUF size
  static const uint64_t SNDBUF_SZ = 268435456/2;

  // -------------

  // Recvfrom non-blocking timeout time
  static const uint TIMEOUT_SECS = 0; // Secs
  static const uint TIMEOUT_USECS = 500; // usecs 
  // The number of non-blocking timeouts to wait before exit
  static const uint TIMEOUT_MAX = 5e4;

  // The last packet number
  uint32_t last_packet[CHNUM] = {};

public:

  // Standard constructor - should'nt be used
  UDPsocket();

  // Kernel buffer sizes
  uint64_t sock_mem = 0; // Total allocated socket memory
  static const uint64_t SO_SNDBUF_SIZE = 0; // The set SO_SNDBUF size
  //  The number of packets to receive with sendmmsg
  //  The number of packets to receive with recvmmsg
  static const int RECVMMSG_NUM = 65536;
  //static const int RECVMMSG_NUM = RCVBUF_SZ/MAX_PACKET_SIZE;

  //  static const int SENDMMSG_NUM = 512;
  static const int SENDMMSG_NUM = RECVMMSG_NUM/1e2;

  // The maximum udp datagram length
  static const uint16_t MAX_UDP_LEN = MAX_UDP_SIZE/2;

  // // The UDP packet to send/receice
  int16_t packet[MAX_PACKET_LEN] = {};

  // The socket sevrer / client address
  struct sockaddr_in servaddr, cliaddr;
  fd_set readfds;
  // The size of the client address
  int slen = sizeof(cliaddr);
  // The recv_from return value
  int ret[CHNUM] = {};

  // The time value of the UDP timeout
  struct timeval read_timeout;
  // The UDP timeout counter
  uint timeout_counter = 0;
  // Boolean to signal if the UDP server has timed out
  bool timeout = false;

  // Boolean to signal receiving multiple UDP datagrams
  bool rcv_multiple = true;

  // Boolean whether this is the first packet of the data run
  bool firstPacket = true;

  // Boolean whether each channel is receiving data
  bool receiving_data[CHNUM] = {};

  // Check channel is only either 0 or 1
  void check_channel(int CHAN){   
    if (CHAN != HPGE && CHAN != LABR){
      // Kill program
      Logger::Instance()->write(0,"Error! In UDPsocket::getIPaddress. ");
    }    
  }

  // Check FW_or_SW is only either 0 (FW) or 1 (SW)
  void check_fw_or_sw(int FW_or_SW){
    if (FW_or_SW != FW && FW_or_SW != SW){
      // Kill program
      Logger::Instance()->write(0,"Error! In UDPsocket::check_recv_send. ");
    }    
  }

  // Return the channel name
  string get_channel_name(int CHAN){
    // Check the channel number
    check_channel(CHAN);
    string chan;
    if (CHAN==0){
      chan = "HPGe";
    }
    else{
      chan = "LaBr";
    }
    return chan;
  }

  // Get the max UDP datagram size
  uint16_t get_max_udp_size(){
    return MAX_UDP_SIZE;
  }

  // Set the stored last packet number
  void setLastPacketNum(int chan, uint32_t lastPacket){
    last_packet[chan] = lastPacket;
  }

  // Get the stored last packet number
  uint32_t getLastPacketNum(int chan){
    return last_packet[chan];
  }

  
  // Get the maximum number of allowed UDP timeouts
  uint getTimeoutMax(){
    return TIMEOUT_MAX;
  }

  // Socket error message function
  void die(char *s){
    perror(s);
    exit(1);
  }
    
  // Get socket IP address from xml file
  std::string getIPaddress(int CHAN, int FW_or_SW);

  // Get socket port number from xml file
  int getPort(int CHAN, int FW_or_SW);

  // Setup server to receive packets
  int setupServer(int CHAN, int FW_or_SW);

  // Create UDP socket
  int createSocket(int CHAN, int FW_or_SW);  

  // Create UDP client
  int setupClient(int CHAN, int FW_or_SW);  

  // Bind socket to port
  int bindSocket(int sock);

  // Set SO_RCVBUF size
  int set_SO_RCVBUF(int sock, uint64_t size);

  // Set SO_SNDBUF size
  int set_SO_SNDBUF(int sock, uint64_t size);

  // Set recvfrom non-blocking timeout
  int setTimeout(int secs,double usecs);

  // Set buffer length
  int setBufferLength();

  // Send SINGLE UDP datagram
  int sendOne(int16_t* data, uint16_t size, int sock);
  
  // Send MULTIPLE UDP datagrams
  int send(int16_t** data, uint16_t* size, int MSG_NUM, int sock);

  // Get SINGLE UDP datagram
  int recvOne(int16_t* data, int sock, int CHAN);

  // Get MULTIPLE UDP datagrams
  int recv(int16_t* data, int sock, int CHAN);

};

#endif
