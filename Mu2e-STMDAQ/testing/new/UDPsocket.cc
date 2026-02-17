// UDP socket header
#include "UDPsocket.hh"

/*-- UDP Socket Init -------------------------------------------------*/

//Standard constructor
UDPsocket::UDPsocket() {}

// Get IP address
std::string UDPsocket::getIPaddress(){

  // Get UDP IP address from environment varaibles
  std::string ip = "192.168.34.12";
  
  return ip;
  
}

// Get IP port
int UDPsocket::getPort(){

  // Get UDP port number 
  int port = 51872;

  // Return port
  return port;

}

// Setup server to receive packets
int UDPsocket::setupServer(){
  
  // create and bind socket
  int socket = bindSocket(createSocket());
  
  // Set SO_RCVBUF buffer size
  set_SO_RCVBUF(socket,RCVBUF_SZ);

  // Set recvfrom non-blocking timeout time
  setTimeout(TIMEOUT_SECS,TIMEOUT_USECS); // 0 seconds, 500 usecs
  
  return socket;
  
}
 
// Create and bind UDP socket
 int UDPsocket::createSocket(){

  // The server socket
  int sock;

  // Creating socket file descriptor
  sock=socket(AF_INET, SOCK_DGRAM,0 );
  if ((sock=socket(AF_INET, SOCK_DGRAM,0 )) == -1){ //IPPROTO_UDP           
    // Kill program
    std::cout << "Unable to create socket" << std::endl;
    exit(0);
  }

  // Ensure any earlier socket is closed
  close(sock);

  // Creating socket file descriptor
  sock=socket(AF_INET, SOCK_DGRAM,0 );
  if ((sock=socket(AF_INET, SOCK_DGRAM,0 )) == -1){ //IPPROTO_UDP
    // Kill program
    std::cout << "Unable to create socket" << std::endl;
    exit(0);
  }
  
  // zero out the structure                                             
  memset((char *) &servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  int port = getPort();
  servaddr.sin_port = htons(port);

  // Form IP address as char*
  std::string ip = getIPaddress();
  const char* ip_address = ip.c_str();
  //  string ip_address = ip;
  servaddr.sin_addr.s_addr = inet_addr(ip_address);

  // Log server infomation
  std::cout << "CREATED SERVER: IP = " + ip
    + ", PORT = " + std::to_string(port)
    + ", socket = " + std::to_string(sock) << std::endl;

  // Set socket to allow port re-use / to reuse port
  int optval = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

  return sock;

}

// Create UDP client
int UDPsocket::setupClient(){

  // The client socket
  int sock;

  // Creating UDP socket file descriptor
  if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    std::cout << "Unable to create client" << std::endl;
    exit(0);
  }
  
  // Zero UDP server address structure
  memset(&servaddr, 0, sizeof(servaddr));

  // Fill UDP server information
  servaddr.sin_family = AF_INET;
  int port = getPort();
  servaddr.sin_port = htons(port);


  // Form IP address as char*
  std::string ip = getIPaddress();
  const char* ip_address = ip.c_str();
  servaddr.sin_addr.s_addr = inet_addr(ip_address);
 
  // Log client information
  std::cout << "CREATED CLIENT: IP = " + ip
    + ", PORT = " + std::to_string(port)
    + ", socket = " + std::to_string(sock) << std::endl;

  // Set SO_SNDBUF buffer size
  set_SO_SNDBUF(sock,SNDBUF_SZ);

  return sock;

}


// Bind socket to port
int UDPsocket::bindSocket(int socket){

  if( bind(socket , (struct sockaddr*)&servaddr, sizeof(servaddr) ) == -1){
    // Kill program
    std::cout << "Unable to bind socket" << std::endl;
    exit(0);
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
  std::cout << "Setting SO_RCVBUF size to "
    + std::to_string(size) 
    + ": old SO_RCVBUF = " + std::to_string(oldsize)
    + ", new SO_RCVBUF = " + std::to_string(newsize) << std::endl;
  
  // Log the calcuated number of UDP packets to send
  std::cout << "Calculated RECVMMSG_NUM =  "
    + std::to_string(RECVMMSG_NUM) 
    + " * 8198 byte UDP packets = "
    + std::to_string((RECVMMSG_NUM * MAX_PACKET_SIZE)*1e-9)
    + " Gbytes" << std::endl;
  
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
  std::cout << "Setting SO_SNDBUF size to "
    + std::to_string(size) 
    + ": old SO_SNDBUF = " + std::to_string(oldsize)
    + ", new SO_SNDBUF = " + std::to_string(newsize) << std::endl;
  
  // Log the calcuated number of UDP packets to send
  std::cout << "Calculated SENDMMSG_NUM =  "
    + std::to_string(SENDMMSG_NUM) 
    + " * 8198 byte UDP packets = "
    + std::to_string((SENDMMSG_NUM * MAX_PACKET_SIZE)*1e-9)
    + " Gbytes" << std::endl;

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

  // MSG_NUM = 17;
  // for (int i = 0; i< 5; i++){
  //   std::cout << i << " " << data[0][i] << std::endl;
  // }
  // int len = size[MSG_NUM-1]/sizeof(int16_t);
  // for (int i = len-5; i< len; i++){
  //   std::cout << i << " " << data[MSG_NUM-1][i] << std::endl;
  // }
  
  // Define mmsg varaibles as array of number of messages to receive
  struct mmsghdr msgvec[MSG_NUM];
  struct iovec msg_iovec[MSG_NUM];
  memset(msgvec,0,sizeof(msgvec));
  // Loop over number of messages to receive
  for (int i = 0; i < MSG_NUM; i++){
    msg_iovec[i] = {data[i],size[i]};
    // for (int j= MAX_PACKET_LEN-5; j < MAX_PACKET_LEN; j++){
    //   std::cout << i << " " << j << " " << data[i][j] << std::endl;
    // }
    msgvec[i].msg_hdr.msg_name = (struct sockaddr *) &servaddr;
    msgvec[i].msg_hdr.msg_namelen = sizeof(servaddr);
    msgvec[i].msg_hdr.msg_iov    = &msg_iovec[i];
    msgvec[i].msg_hdr.msg_iovlen = 1;
  }
  // Receive multiple messages
  int retval = sendmmsg(socket,msgvec,MSG_NUM,0);

  //  exit(0);
  
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
int UDPsocket::recvOne(int16_t* data, int socket){
  
  // If receiving a single UDP message at a time...
  // Initialise server socket
  int datasize = 0;
  FD_ZERO(&readfds);
  FD_SET(socket, &readfds);
  ret = select(socket+1, &readfds, NULL, NULL, &read_timeout);
  // If socket has pending data to read                                    
  if (ret > 0){
    // Get UDP datagram
    if ((datasize = recvfrom(socket, data, MAX_UDP_SIZE, 0,
			     (struct sockaddr*) &cliaddr,
			     (socklen_t *) &slen)) < 0){
      // If recv_len error, throw exception
      die((char*)"recvfrom()");
    }    
    // Signal we're receiving data
    receiving_data = true;
  }
    // If no packet received
  else if (ret == 0){
    // Signal we're not receiving data
    receiving_data = false;    
    return 0;
  }
  else{
    // Signal we're not receiving data
    receiving_data = false;
    // Else if < 0, error and exit
    std::cout << "UDP error in UDPsocket::recvOne" << std::endl;
    std::cout << "error selecting: ret = " << ret << std::endl;
    exit(0);
  }
  
  return 1;
  
}

 
// Request MULTIPLE UDP datagrams from socket
int UDPsocket::recv(int16_t *data, int socket){

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
    receiving_data = false;
    return 0;
  }

  // Signal we're receiving data
  receiving_data = true;

  // Return the number of received messages
  return retval;
  
}
