// UDP header
#include "Mu2e-STMDAQ/utils/udp.hh" 

// Constructor
UDP::UDP(const Config& cfg_,
	 const std::shared_ptr<AsyncLogger>& logger_,
	 const std::shared_ptr<SignalHandler>& signal_,
	 const int CHAN_,
	 const bool is_client_) :
  cfg(cfg_), logger(logger_), signal(signal_), CHAN(CHAN_), is_client(is_client_), wait_time(0.0) {
  
  // Create a UDP socket
  socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  // Handle socket creation failure
  if (socket_fd < 0) {
    if (logger) logger->log("socket creation failed",0);
    exit(EXIT_FAILURE);
  }

  // If server
  if (!is_client){    
    // If channel 0 (HPGe)
    if (!CHAN){ // 
      ip = cfg.getValue<std::string>("stm.udp.hpge.fw.ip");
      port = cfg.getValue<int>("stm.udp.hpge.fw.port"); 
    }
    // If channel 1 (LaBr)
    else{
      ip = cfg.getValue<std::string>("stm.udp.labr.fw.ip");
      port = cfg.getValue<int>("stm.udp.labr.fw.port"); 
    }
  }
  // If client
  else{
    // If channel 0 (HPGe)
    if (!CHAN){ // 
      ip = cfg.getValue<std::string>("stm.udp.hpge.sw.ip");
      port = cfg.getValue<int>("stm.udp.hpge.sw.port"); 
    }
    // If channel 1 (LaBr)
    else{
      ip = cfg.getValue<std::string>("stm.udp.labr.sw.ip");
      port = cfg.getValue<int>("stm.udp.labr.sw.port"); 
    }
  }

  // Notify user
  std::string type = (is_client) ? "client" : "server";
  std::string detector = (CHAN) ? "LaBr" : "HPGe";
  if (logger) logger->log("Configuring UDP " + type + " for " + detector + " channel...",1);
  
  // Setup socket
  memset((char *) &address, 0, sizeof(address)); // zero out the structure
  address.sin_family = AF_INET; // Set address family to IPv4
  address.sin_addr.s_addr = inet_addr(ip.c_str()); // Set the IP address
  address.sin_port = htons(port); // Convert and set the port number

  // Set socket to allow port re-use / to reuse port
  int optval = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));     
  // Get rmem_max and wmem_max
  get_mem_max();

  // If creating a client
  if (is_client){
    configure_client();
  }
  // If creating a server
  else{
    configure_server();
  }
  
  // Initialise function map for ProcessManager
  functionMap["receive_data"] = [this](std::shared_ptr<DataStruct>& buffer) {
    receive_data(buffer);
  };
  
}

// Get kernel buffer memory values
void UDP::get_mem_max(){

  // Get system set RCVBUF size from system rmem_max
  std::string size;
  uint64_t size_val;
  ifstream rmem_max ("/proc/sys/net/core/rmem_max");
  getline (rmem_max, size);
  std::istringstream iss1(size);
  iss1 >> size_val;
  rmem_max.close(); // Close the file;
  RMEM_MAX = size_val;
  if (logger) logger->log("/proc/sys/net/core/rmem_max = " + std::to_string(RMEM_MAX) + " bytes.",1);

  // Get system set SNDBUF size from system wmem_max
  ifstream wmem_max ("/proc/sys/net/core/wmem_max");
  getline (wmem_max, size);
  iss1 >> size_val;
  wmem_max.close(); // Close the file
  WMEM_MAX = size_val;  
  if (logger) logger->log("/proc/sys/net/core/wmem_max = " + std::to_string(WMEM_MAX) + " bytes.",1);
    
}


// Configure the server
void UDP::configure_server() {

  // Set the receive buffer size based on the system vale for rmem_max
  socklen_t optlen = sizeof(RMEM_MAX);
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &RMEM_MAX, optlen); 
  // Retrieve the actual buffer size set
  int actual_rmem_max;
  getsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &actual_rmem_max, &optlen); 
  // Print the actual buffer size
  if (logger) logger->log("SO_RCVBUF set to: " + std::to_string(actual_rmem_max) +" (" + std::to_string(actual_rmem_max/2) + ") bytes [RMEM_MAX = " + std::to_string(RMEM_MAX) + " byes].",1); 

  // Bind the socket to the server address
  if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { 
    if (logger) logger->log("Unable to bind socket.",0); // Handle binding failure
    exit(EXIT_FAILURE);
  }

  // Get the current socket flags
  int flags = fcntl(socket_fd, F_GETFL, 0);
  // Set the socket to non-blocking mode
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
  
  // Set the raw data buffer size 
  raw_buffer_size = cfg.getValue<int>("stm.raw_buffer_size");
  std::string size_msg = "Setting raw data buffer size user value of " + std::to_string(raw_buffer_size) + " MB.";
  if (logger) logger->log(size_msg,1);

  // Calculate the maximum number of packets per call
  max_packets_per_call = raw_buffer_size*1e6 / MAX_PACKET_SIZE;
  if (logger) logger->log("Maximum UDP receiving = " + std::to_string(max_packets_per_call) + " * 8198 kB packets per call.",1);

  // Allocate memory for mmsghdr structures
  messages = new mmsghdr[max_packets_per_call];
  // Allocate memory for iovec structures
  iovecs = new iovec[max_packets_per_call]; 
  // Set up the UDP server buffers
  setup_buffers();

  // Set the server's polling structure
  pfd.fd = socket_fd;
  pfd.events = POLLIN;

  // Set recvmmsg timeout
  size_t TIMEOUT_SECS = cfg.getValue<int>("stm.udp.recv_timeout_secs");
  size_t TIMEOUT_USECS = cfg.getValue<int>("stm.udp.recv_timeout_usecs");
  read_timeout.tv_sec = TIMEOUT_SECS;
  read_timeout.tv_usec = TIMEOUT_USECS;   
  timeout = {read_timeout.tv_sec,read_timeout.tv_usec};
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  
  // Set the receive poll timeout
  TIMEOUT_MS = cfg.getValue<int>("stm.udp.poll_timeout_ms");

  // Print to user
  if (logger) logger->log("CREATED UDP SERVER. IP = " + ip
	      + ", PORT = " + std::to_string(port)              
	      + ", socket = " + std::to_string(socket_fd)
	      + ", timeout = " + std::to_string(read_timeout.tv_sec)
	      + " s, " + std::to_string(read_timeout.tv_usec)
	      + " us.",1);
   
}

// Set up the UDP server buffers
void UDP::setup_buffers() {

  // Loop over number of packets per call
  for (size_t i = 0; i < max_packets_per_call; ++i) {
    iovecs[i].iov_len = MAX_PACKET_SIZE; // Set the size of each packet buffer
    messages[i].msg_hdr.msg_iov = &iovecs[i]; // Associate the iovec with the corresponding mmsghdr structure
    messages[i].msg_hdr.msg_iovlen = 1; // Indicate that each mmsghdr describes one buffer
  }
  
}
 
// Configure the client
void UDP::configure_client() {
  
  // Set the receive buffer size based on the system vale for wmem_max
  socklen_t optlen = sizeof(WMEM_MAX);
  setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &WMEM_MAX, optlen); 
  // Retrieve the actual buffer size set
  int actual_wmem_max;
  getsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &actual_wmem_max, &optlen);
  // Print the actual buffer size
  if (logger) logger->log("SO_SNDBUF set to: " + std::to_string(actual_wmem_max) +" (" + std::to_string(actual_wmem_max/2) + ") bytes [WMEM_MAX = " + std::to_string(WMEM_MAX) + " byes].",1); 

  // Print to user
  if (logger) logger->log("CREATED UDP CLIENT. IP = " + ip
	      + ", PORT = " + std::to_string(port)              
	      + ", socket = " + std::to_string(socket_fd)
	      + ".",1);
  
}

// Receive data from UDP server
void UDP::receive_data(std::shared_ptr<DataStruct>& buffer){

  // Set up iovec structures to point directly into DataStruct
  for (size_t i = 0; i < max_packets_per_call; ++i) {
    iovecs[i].iov_base = buffer->raw.data() + (i * MAX_PACKET_LEN);
    messages[i].msg_hdr.msg_iov = &iovecs[i];
  }

  // Ensure buffer is zero before call
  buffer->raw_len = 0;
  buffer->orig_len = 0;

  // Track if we're waiting for data transfer
  bool waiting_for_data = false;
  // The number of messages we receive
  int retval = 0;
  // Start and end wait times
  start_wait = std::chrono::high_resolution_clock::now();
  first_wait = start_wait;
  end_wait = std::chrono::high_resolution_clock::now();
  
  // While no stop signal
  while (!signal->shouldStop()) {
    
    // Packet counter
    int received_packets = 0;
    
    // Loop until max_packets_per_call messages are
    // received or stop signal is triggered 
    while (received_packets < max_packets_per_call && !signal->shouldStop()) {
      
      // Poll for available data
      int poll_ret = poll(&pfd, 1, TIMEOUT_MS);
      
      // If poll failed
      if (poll_ret < 0) {
	if (logger) logger->log("UDP::receive_data - poll failed!",0);
      }
      // Else if not data has been received (timeout)
      else if (poll_ret == 0) {
	// If we have previously received data
	if (has_received_data) {
	  // Store waiting start time
	  start_wait = std::chrono::high_resolution_clock::now();
	  // Warn user
	  if (logger) logger->log("Stopped receiving from STM firmware. Waiting for data...",2);
	  
	  // Reset to detect re-establishment
	  has_received_data = false;
	  // Indicate that we are Waiting for data
	  waiting_for_data = true; 
	}
	// If not previously printed "Waiting for data..."
	else if (!waiting_for_data) {
	  if (logger) logger->log("Waiting for data from STM firmware...",1);
	  // Indicate that we are Waiting for data
	  waiting_for_data = true; 
	}
      }
      // Else, data is available
      else {
	// If not previously recevied data
	if (!has_received_data) {
	  // Store waiting end time
	  end_wait = std::chrono::high_resolution_clock::now();
	  if (start_wait != first_wait){
	    wait_time += end_wait - start_wait;
	  }
	  if (logger) logger->log("Receiving data...",1);
  	  // Indicate that we have recevied data
	  has_received_data = true;
	}
	// Indicate that we are no longer waiting for data
	waiting_for_data = false;
	
	// Receive data from socket
	int retval = recvmmsg(socket_fd, &messages[received_packets], 
			      max_packets_per_call - received_packets, MSG_DONTWAIT, &timeout);
	
	// If recvmmsg error
	if (retval < 0) {
	  if (logger) logger->log("UDP::receive_data - recvmmsg failed",2);
	}
       
	// Update number of packets received
	received_packets += retval;
	
      } // End if data available via poll
           
    } // End while (received_packets < max_packets_per_call)

    // Update size of received data in DataStruct
    buffer->raw_len += received_packets * MAX_PACKET_LEN;
    buffer->orig_len = buffer->raw_len;
    
    // Update number of packets received
    packets_received += received_packets;
    
    // Succesful pull, so return
    if (received_packets == max_packets_per_call) return;

    
  } // End while (!signal->shouldStop())
  
  // Store waiting end time
  end_wait = std::chrono::high_resolution_clock::now();
  if (start_wait != first_wait){
    wait_time += end_wait - start_wait;
  }
  
  return;
  
}

// Function to send a single UDP packet
void UDP::send_packet(const std::vector<int16_t>& data_vec) {

  // Prepare the message header
  struct iovec iov;
  iov.iov_base = const_cast<int16_t*>(data_vec.data());  
  iov.iov_len = data_vec.size() * sizeof(int16_t);

  struct msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_name = &address;
  msg.msg_namelen = sizeof(address);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  ssize_t sent_bytes = sendmsg(socket_fd, &msg, 0);

  if (__builtin_expect(sent_bytes == -1, 0)) {
    if (logger) logger->log(std::string("send_packet error: ") + strerror(errno), 0);
  }

}
