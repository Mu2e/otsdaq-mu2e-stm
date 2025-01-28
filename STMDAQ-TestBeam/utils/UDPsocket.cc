///////////////////////////////////////////////////////////////////////////////////
/// This module creates a UDP socket for 10G readout (main).  
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

// Environment variables
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// UDP socket header
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

/*-- UDP Socket Init -------------------------------------------------*/

//Standard constructor
UDPsocket::UDPsocket() {

  // // Get system set RCVBUF size from system rmem_max
  // string size;
  // uint64_t size_val;
  // ifstream rmem_max ("/proc/sys/net/core/rmem_max");
  // getline (rmem_max, size);
  // std::istringstream iss1(size);
  // iss1 >> size_val;
  // rmem_max.close(); // Close the file  
  // RCVBUF_SZ = size_val;

  // // Get system set SNDBUF size from system wmem_max
  // ifstream wmem_max ("/proc/sys/net/core/wmem_max");
  // getline (wmem_max, size);
  // std::istringstream iss2(size);
  // iss2 >> SNDBUF_SZ;
  // wmem_max.close(); // Close the file  
  // // Half the send size (relative to SO_RCVBUF)
  // SNDBUF_SZ /= 2 ;

}

// Get IP address from setup.sh depending on CHANNEL and FW/SW
//const char* UDPsocket::getIPaddress(int CHAN, int FW_or_SW){
std::string UDPsocket::getIPaddress(int CHAN, int FW_or_SW){

  // Check channel number
  check_channel(CHAN);

  // Check FW_or_SW flag
  check_fw_or_sw(FW_or_SW);
  
  // The ip address
  //  const char* ip_address;
  
  // Get UDP IP address from environment varaibles
  std::string ip;
  
  // If the data channel is HPGe (CH0)
  if (CHAN == HPGE){
    // If reading data...
    if (FW_or_SW == FW){
      ip = EnvVars::expand("${STM_HPGE_FW_IP}");
    }
    // If writing data...
    else if (FW_or_SW == SW){
      ip = EnvVars::expand("${STM_HPGE_SW_IP}");
    }
  }
  // If the data channel is LaBr (CH1)
  else if (CHAN == LABR){
    // If reading data...
    if (FW_or_SW == FW){
      ip = EnvVars::expand("${STM_LABR_FW_IP}");
    }
    // If writing data...
    else if (FW_or_SW == SW){
      ip = EnvVars::expand("${STM_LABR_SW_IP}");
    }
  }

  // // Form IP address as char*
  // ip_address = ip.c_str();
  
  // // Return IP
  // return ip_address;
  return ip;
  
}

// Get IP port from setup.sh depending on CHANNEL and FW/SW
int UDPsocket::getPort(int CHAN, int FW_or_SW){

  // Check channel number
  check_channel(CHAN);

  // Check FW_or_SW flag
  check_fw_or_sw(FW_or_SW);

  // Get UDP port number from environment variables
  string portNum;
  
  // If the data channel is HPGe (CH0)
  if (CHAN == HPGE){
    // If reading data...
    if (FW_or_SW == FW){
      portNum = EnvVars::expand("${STM_HPGE_FW_PORT}");
    }
    // If writing data...
    else if (FW_or_SW == SW){
      portNum = EnvVars::expand("${STM_HPGE_SW_PORT}");
    }
  }
  // If the data channel is LaBr (CH1)
  else if (CHAN == LABR){
    // If reading data...
    if (FW_or_SW == FW){
      portNum = EnvVars::expand("${STM_LABR_FW_PORT}");
    }
    // If writing data...
    else if (FW_or_SW == SW){
      portNum = EnvVars::expand("${STM_LABR_SW_PORT}");
    }
  }
  
  // Form IP port as int
  stringstream port_ss;
  port_ss << portNum;
  int port = 0;
  port_ss >> port;

  // Return port
  return port;

}

// Setup server to receive packets
int UDPsocket::setupServer(int CHAN, int FW_or_SW){
  
  // Check channel number
  check_channel(CHAN);

  // Check FW_or_SW flag
  check_fw_or_sw(FW_or_SW);
  
  // create and bind socket
  int socket = bindSocket(createSocket(CHAN,FW_or_SW));
  
  // Set SO_RCVBUF buffer size
  set_SO_RCVBUF(socket,RCVBUF_SZ);

  // Set recvfrom non-blocking timeout time
  setTimeout(TIMEOUT_SECS,TIMEOUT_USECS); // 0 seconds, 500 usecs
  
  return socket;
  
}
 
// Create and bind UDP socket
int UDPsocket::createSocket(int CHAN, int FW_or_SW){

  // Check channel number
  check_channel(CHAN);

  // Check FW_or_SW flag
  check_fw_or_sw(FW_or_SW);

  // The server socket
  int sock;

  // Creating socket file descriptor
  sock=socket(AF_INET, SOCK_DGRAM,0 );
  if ((sock=socket(AF_INET, SOCK_DGRAM,0 )) == -1){ //IPPROTO_UDP           
    // Kill program
    Logger::Instance()->write(0,"Unable to create socket");
  }

  // Ensure any earlier socket is closed
  close(sock);

  // Creating socket file descriptor
  sock=socket(AF_INET, SOCK_DGRAM,0 );
  if ((sock=socket(AF_INET, SOCK_DGRAM,0 )) == -1){ //IPPROTO_UDP           
    // Kill program
    Logger::Instance()->write(0,"Unable to create socket");
  }
  
  // zero out the structure                                             
  memset((char *) &servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  int port = getPort(CHAN,FW_or_SW);
  servaddr.sin_port = htons(port);

  // Form IP address as char*
  std::string ip = getIPaddress(CHAN,FW_or_SW);
  const char* ip_address = ip.c_str();
  //  string ip_address = ip;
  servaddr.sin_addr.s_addr = inet_addr(ip_address);

  // Log server infomation
  string type = (FW_or_SW == FW) ? " FW " : " SW ";
  Logger::Instance()->write(1,
			    "CREATED"
			    + type
			    + "SERVER: Channel = " 
			    + std::to_string(CHAN)
			    + " [" + get_channel_name(CHAN)
			    + "]: IP = " + ip
			    + ", PORT = " + std::to_string(port)
			    + ", socket = " + std::to_string(sock));

  // Set socket to allow port re-use / to reuse port
  int optval = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

  return sock;

}

// Create UDP client
int UDPsocket::setupClient(int CHAN, int FW_or_SW){

  // Check channel number
  check_channel(CHAN);

  // Check FW_or_SW flag
  check_fw_or_sw(FW_or_SW);

  // The client socket
  int sock;

  // Creating UDP socket file descriptor
  if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    Logger::Instance()->write(0,"Unable to create client");
  }
  
  // Zero UDP server address structure
  memset(&servaddr, 0, sizeof(servaddr));

  // Fill UDP server information
  servaddr.sin_family = AF_INET;
  int port = getPort(CHAN,FW_or_SW);
  servaddr.sin_port = htons(port);

  // Form IP address as char*
  std::string ip = getIPaddress(CHAN,FW_or_SW);
  const char* ip_address = ip.c_str();
  servaddr.sin_addr.s_addr = inet_addr(ip_address);
 
  // Log client information
  string type = (FW_or_SW == FW) ? " FW " : " SW ";
  Logger::Instance()->write(1,
			    "CREATED"
			    + type
			    + "CLIENT: Channel = " 
			    + std::to_string(CHAN)
			    + " [" + get_channel_name(CHAN)
			    + "]: IP = " + ip
			    + ", PORT = " + std::to_string(port)
			    + ", socket = " + std::to_string(sock));

  // Set SO_SNDBUF buffer size
  set_SO_SNDBUF(sock,SNDBUF_SZ);

  return sock;

}


// Bind socket to port
int UDPsocket::bindSocket(int socket){

  if(int retval = bind(socket , (struct sockaddr*)&servaddr, sizeof(servaddr) ) == -1){
    perror("bind");    
    Logger::Instance()->write(0,"Unable to bind socket");
  }

  // Zero out the client address structure
  memset((char *) &cliaddr, 0, sizeof(cliaddr));

  fflush(stdout);

  return socket;

}  

// Set SO_RCVBUF size
int UDPsocket::set_SO_RCVBUF(int socket, uint64_t size){

  // Return and print SO_RCVBUF  
  socklen_t xx = sizeof(xx);
  uint64_t oldsize = 0;
  getsockopt(socket,SOL_SOCKET,SO_RCVBUF,(char*)&oldsize,&xx);

  // Set SO_RCVBUF size
  int returnVal = setsockopt(socket,SOL_SOCKET,SO_RCVBUF,(char*)&size,sizeof(size));

  // Return and print SO_RCVBUF
  xx = sizeof(xx);
  uint64_t newsize = 0;
  getsockopt(socket,SOL_SOCKET,SO_RCVBUF,(char*)&newsize,&xx);

  // Increase total kernel memory allocated
  sock_mem += newsize;

  // Log the change to SO_RCVBUF              
  Logger::Instance()->write(1,
			    "Setting SO_RCVBUF size to "
			    + std::to_string(size) 
			    + ": old SO_RCVBUF = " + std::to_string(oldsize)
			    + ", new SO_RCVBUF = " + std::to_string(newsize));

  // Log the calcuated number of UDP packets to send
  Logger::Instance()->write(1,
			    "Calculated RECVMMSG_NUM =  "
			    + std::to_string(RECVMMSG_NUM) 
			    + " * 8198 byte UDP packets = "
			    + std::to_string((RECVMMSG_NUM * MAX_PACKET_SIZE)*1e-9)
			    + " Gbytes");

  
  return returnVal;
  
}  

// Set SO_SNDBUF size
int UDPsocket::set_SO_SNDBUF(int socket, uint64_t size){
  
  // Return and print SO_SNDBUF
  socklen_t xx = sizeof(xx);
  uint64_t oldsize = 0;
  getsockopt(socket,SOL_SOCKET,SO_SNDBUF,(char*)&oldsize,&xx);

  // Set SO_SNDBUF size
  int returnVal = setsockopt(socket,SOL_SOCKET,SO_SNDBUF,(char*)&size,sizeof(size));

  // Return and print SO_SNDBUF
  xx = sizeof(xx);
  uint64_t newsize = 0;
  getsockopt(socket,SOL_SOCKET,SO_SNDBUF,(char*)&newsize,&xx);

  // Increase total kernel memory allocated
  sock_mem += newsize;

  // Log the change to SO_SNDBUF              
  Logger::Instance()->write(1,
			    "Setting SO_SNDBUF size to "
			    + std::to_string(size) 
			    + ": old SO_SNDBUF = " + std::to_string(oldsize)
			    + ", new SO_SNDBUF = " + std::to_string(newsize));

  // Log the calcuated number of UDP packets to send
  Logger::Instance()->write(1,
			    "Calculated SENDMMSG_NUM =  "
			    + std::to_string(SENDMMSG_NUM) 
			    + " * 8198 byte UDP packets = "
			    + std::to_string((SENDMMSG_NUM * MAX_PACKET_SIZE)*1e-9)
			    + " Gbytes");


  return returnVal;

}  


// Set recvfrom non-blocking timeout    
int UDPsocket::setTimeout(int secs, double usecs){

  read_timeout.tv_sec = secs;
  read_timeout.tv_usec = usecs;

  return 1;

}  

// Send UDP datagram to socket 
int UDPsocket::sendOne(int16_t* data, uint16_t size, int socket){

  sendto(socket, data, size,
	 MSG_CONFIRM, (const struct sockaddr *) &servaddr,
	 sizeof(servaddr));
  
  return 1;

}

// Send MULTIPLE UDP datagrams from socket
int UDPsocket::send(int16_t** data, uint16_t* size, int MSG_NUM, int socket){

  // Define mmsg varaibles as array of number of messages to receive
  struct mmsghdr msgvec[MSG_NUM];
  struct iovec msg_iovec[MSG_NUM];
  memset(msgvec,0,sizeof(msgvec));
  // Loop over number of messages to receive
  for (int i = 0; i < MSG_NUM; i++){
    msg_iovec[i] = {data[i],size[i]};
    msgvec[i].msg_hdr.msg_name = (struct sockaddr *) &servaddr;
    msgvec[i].msg_hdr.msg_namelen = sizeof(servaddr);
    msgvec[i].msg_hdr.msg_iov    = &msg_iovec[i];
    msgvec[i].msg_hdr.msg_iovlen = 1;
  }
  // Receive multiple messages
  int retval = sendmmsg(socket,msgvec,MSG_NUM,0);

  // If no messages received, return zero
  if (retval <= 0){
    perror("sendmmsg()");
    std::cout << errno << std::endl;
    return 0;
  }

  // Return the number of received messages
  return retval;
  
}

 
// Request SINGLE UDP datagram from socket
int UDPsocket::recvOne(int16_t* data, int socket, int CHAN){
  
  // If receiving a single UDP message at a time...
  // Initialise server socket
  int datasize = 0;
  FD_ZERO(&readfds);
  FD_SET(socket, &readfds);
  ret[CHAN] = select(socket+1, &readfds, NULL, NULL, &read_timeout);
  // If socket has pending data to read                                    
  if (ret[CHAN] > 0){
    // Get UDP datagram
    if ((datasize = recvfrom(socket, data, MAX_UDP_SIZE, 0,
			     (struct sockaddr*) &cliaddr,
			     (socklen_t *) &slen)) < 0){
      // If recv_len error, throw exception
      die((char*)"recvfrom()");
    }    
    // Signal we're receiving data
    receiving_data[CHAN] = true;
  }
    // If no packet received
  else if (ret[CHAN] == 0){
    // Signal we're not receiving data
    receiving_data[CHAN] = false;    
    return 0;
  }
  else{
    // Signal we're not receiving data
    receiving_data[CHAN] = false;
    // Else if < 0, error and exit
    cout << "UDP error in UDPsocket::recvOne" << endl;
    cout << "error selecting: ret = " << ret[CHAN] << endl;
    exit(0);
  }
  
  return 1;
  
}

 
// Request MULTIPLE UDP datagrams from socket
int UDPsocket::recv(int16_t *data, int socket, int CHAN){

  // Define mmsg varaibles as array of number of messages to receive
  struct mmsghdr msgvec[RECVMMSG_NUM];
  struct iovec msg_iovec[RECVMMSG_NUM];
  struct msghdr msg[RECVMMSG_NUM];
  memset(msgvec,0,sizeof(msgvec));

  // Set the timeout
  struct timespec timeout = {read_timeout.tv_sec,
			     read_timeout.tv_usec*100000};

  // Loop over number of messages to receive
  for (int i = 0; i < RECVMMSG_NUM; i++){        
    msg_iovec[i] = {&data[i*MAX_PACKET_LEN],MAX_PACKET_SIZE};
    msgvec[i].msg_hdr.msg_iov    = &msg_iovec[i];
    msgvec[i].msg_hdr.msg_iovlen = 1;
  }

  // Receive multiple messages
  int retval = recvmmsg(socket,msgvec,RECVMMSG_NUM,MSG_DONTWAIT,&timeout);
  
  // If no messages received, return zero
  if (retval <= 0){
    // Signal we're not receiving data
    receiving_data[CHAN] = false;
    return 0;
  }

  // Signal we're receiving data
  receiving_data[CHAN] = true;

  // Return the number of received messages
  return retval;
  
}
