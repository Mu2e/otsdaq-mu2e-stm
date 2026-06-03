// UDP header
#include "Mu2e-STMDAQ/processing/udp.hh" 

// Constructor
UDP::UDP(const std::shared_ptr<AsyncLogger>& logger_,
         const std::shared_ptr<STMdata>& stm_,         
         const std::shared_ptr<SignalHandler>& signal_,
         const bool is_client_) :
  logger(logger_), stm(stm_), signal(signal_), 
  is_client(is_client_), // Server or client
  ip((stm->master_config.ch_num) ? stm->udp_config.rcv_ip : stm->udp_config.snd_ip), // IP address
  port((stm->master_config.ch_num) ? stm->udp_config.rcv_port : stm->udp_config.snd_port), // Port
  max_packet_num(stm->buffer_config.max_packet_num),
  wait_time(0.0), // Time waiting with no packets received
  idle_timeout_ms(stm->udp_config.idle_timeout_ms),
  last_recv_time(std::chrono::steady_clock::now())

{
  
  // Create a UDP socket
  socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  // Handle socket creation failure
  if (socket_fd < 0) {
    if (logger) logger->log("UDP: socket creation failed. Exiting...",0);
    return;
  }

  // Notify user
  std::string type = (is_client) ? "client" : "server";
  if (logger) logger->log("UDP: Configuring UDP " + type + " for " + stm->master_config.ch_name + " channel...",1);
  
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

  // Register operations for OperationManager
  register_operation("receive_data", [this](auto& b){ receive_data(b); });
  
}

// Get kernel buffer memory values
void UDP::get_mem_max(){

  // Get system set RCVBUF size from system rmem_max
  std::string size;
  uint64_t size_val;
  std::ifstream rmem_max ("/proc/sys/net/core/rmem_max");
  getline (rmem_max, size);
  std::istringstream iss1(size);
  iss1 >> size_val;
  rmem_max.close(); // Close the file;
  RMEM_MAX = size_val;
  if (logger) logger->log("UDP: /proc/sys/net/core/rmem_max = " + std::to_string(RMEM_MAX) + " bytes.",1);

  // Get system set SNDBUF size from system wmem_max
  std::ifstream wmem_max ("/proc/sys/net/core/wmem_max");
  getline (wmem_max, size);
  iss1 >> size_val;
  wmem_max.close(); // Close the file
  WMEM_MAX = size_val;  
  if (logger) logger->log("UDP: /proc/sys/net/core/wmem_max = " + std::to_string(WMEM_MAX) + " bytes.",1);
    
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
  if (logger) logger->log("UDP: SO_RCVBUF set to: " +
                          std::to_string(actual_rmem_max) + " (" +
                          std::to_string(actual_rmem_max/2) + ") bytes [RMEM_MAX = "
                          + std::to_string(RMEM_MAX) + " byes].",1); 

  // Bind the socket to the server address
  if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { 
    if (logger) logger->log("UDP: Unable to bind socket to IP: " +
                            std::string(inet_ntoa(address.sin_addr)) + ", Port: " +                            
                            std::to_string(ntohs(address.sin_port)) + ". Exiting....",0); // Handle binding failure
    return;
  }

  // Get the current socket flags
  int flags = fcntl(socket_fd, F_GETFL, 0);
  // Set the socket to non-blocking mode
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
  
  // Allocate memory for mmsghdr structures
  messages = new mmsghdr[max_packet_num];
  // Allocate memory for iovec structures
  iovecs = new iovec[max_packet_num]; 
  // Set up the UDP server buffers
  setup_buffers();

  // Set the server's polling structure
  pfd.fd = socket_fd;
  pfd.events = POLLIN;

  // Set recvmmsg timeout
  read_timeout.tv_sec = stm->udp_config.recv_timeout_s;
  read_timeout.tv_usec = stm->udp_config.recv_timeout_us;   
  timeout = {read_timeout.tv_sec,read_timeout.tv_usec};
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  // Print to user
  if (logger) logger->log("UDP: CREATED UDP SERVER. IP = " + ip
                          + ", PORT = " + std::to_string(port)              
                          + ", socket = " + std::to_string(socket_fd)
                          + ", timeout = " + std::to_string(read_timeout.tv_sec)
                          + " s, " + std::to_string(read_timeout.tv_usec)
                          + " us.",1);
   
}

// Set up the UDP server buffers
// void UDP::setup_buffers() {

//   // Loop over number of packets per call
//   for (size_t i = 0; i < max_packet_num; ++i) {
//     iovecs[i].iov_len = MAX_PACKET_SIZE; // Set the size of each packet buffer
//     messages[i].msg_hdr.msg_iov = &iovecs[i]; // Associate the iovec with the corresponding mmsghdr structure
//     messages[i].msg_hdr.msg_iovlen = 1; // Indicate that each mmsghdr describes one buffer
//   }
  
// }

// #include <cstring> // for std::memset
void UDP::setup_buffers() {
  for (size_t i = 0; i < max_packet_num; ++i) {

    // Clear everything so we don't have garbage in msg_name, msg_control, etc.
    std::memset(&messages[i], 0, sizeof(mmsghdr));
    std::memset(&iovecs[i],   0, sizeof(iovec));

    // Each iovec describes one packet buffer; length is fixed here
    iovecs[i].iov_len = MAX_PACKET_SIZE;

    // Hook the iovec into the msghdr inside mmsghdr
    messages[i].msg_hdr.msg_iov    = &iovecs[i];
    messages[i].msg_hdr.msg_iovlen = 1;

    // We don't care about the sender address → disable it explicitly
    messages[i].msg_hdr.msg_name    = nullptr;
    messages[i].msg_hdr.msg_namelen = 0;

    // No ancillary data
    messages[i].msg_hdr.msg_control    = nullptr;
    messages[i].msg_hdr.msg_controllen = 0;

    // msg_len will be filled in by recvmmsg
    messages[i].msg_len = 0;
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
  if (logger) logger->log("UDP: SO_SNDBUF set to: " + std::to_string(actual_wmem_max) +" (" + std::to_string(actual_wmem_max/2) + ") bytes [WMEM_MAX = " + std::to_string(WMEM_MAX) + " byes].",1); 

  // Print to user
  if (logger) logger->log("UDP: CREATED UDP CLIENT. IP = " + ip
                          + ", PORT = " + std::to_string(port)              
                          + ", socket = " + std::to_string(socket_fd)
                          + ".",1);
  
}

// Receive data from UDP server
void UDP::receive_data(std::shared_ptr<DataStruct>& buffer){

  // Check correct buffer sizing
  size_t required = max_packet_num * MAX_PACKET_SIZE;
  size_t available = buffer->zs.size() * sizeof(int16_t);  
  if (available < required) {
    std::cerr << "ERROR: raw buffer too small: "
              << available << " bytes available, "
              << required << " required.\n";
    std::abort();
  }

  // Set up iovec structures to point directly into DataStruct
  for (size_t i = 0; i < max_packet_num; ++i) {
    // Point each iovec into the correct slice of the shared data buffer
    iovecs[i].iov_base = buffer->zs.data() + (i * MAX_PACKET_LEN); 
    messages[i].msg_hdr.msg_iov = &iovecs[i];
    // Reset per-call output fields
    messages[i].msg_len              = 0;
    messages[i].msg_hdr.msg_flags    = 0;
  }
  
  // Ensure buffer is zero before call
  buffer->zs_len = 0;
  buffer->orig_len = 0;
  
  // Start and end wait times
  if (!waiting_for_data) start_wait = std::chrono::high_resolution_clock::now();
  
  // Packet counter
  int packets_this_call = 0;
  
  // Loop until stop signal is triggered 
  while (!stop::should_stop()) {
    
    // Poll for available data
    int poll_ret = poll(&pfd, 1, stm->udp_config.poll_timeout_ms);
    
    // If poll failed
    if (poll_ret < 0) {
      if (logger) logger->log("UDP: UDP::receive_data - poll failed!",0);
      break;
    }
    
    // If data is available
    else if (poll_ret > 0) {

      // If we haven't yet received data
      if (!has_received_data){
        // Store waiting end time
        end_wait = std::chrono::high_resolution_clock::now();
        // ... and add to wait time counter
        if (packets_this_call > 0){
          wait_time += end_wait - start_wait;
          add_wait(end_wait - start_wait);
        }
      }
      
      // Receive data from socket
      int retval = recvmmsg(socket_fd, &messages[packets_this_call], 
                            max_packet_num - packets_this_call, MSG_DONTWAIT, &timeout);
      // int retval = udp_debug::debug_recvmmsg(socket_fd, &messages[packets_this_call],
      //                                        max_packet_num - packets_this_call, MSG_DONTWAIT, &timeout);
      
      // If recvmmsg error
      if (retval < 0) {
        char buf[256];
        strerror_r(errno, buf, sizeof(buf));
        std::string errorStr(buf);
        if (logger) logger->log("UDP: UDP::receive_data - recvmmsg failed with error code: "
                                + (std::to_string(errno))
                                + " " + errorStr,0);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return;
      }
      
      // Update number of packets received
      packets_this_call += retval;

      // If packets update last receive time for idle timeout
      if (retval > 0) {
        last_recv_time = std::chrono::steady_clock::now();
        idle_timeout_logged = false;
      }

      // Update size of received data in DataStruct
      buffer->zs_len += retval * MAX_PACKET_LEN;
      buffer->orig_len = buffer->zs_len;
      
      // Update total number of packets received
      total_packets_received += retval;
            
      // If first data of call
      if (!has_received_data) has_received_data = true;
      
      // Tell user we're receiving data
      if (logger && waiting_for_data) logger->log("UDP: Receiving data...",1);
      
      // Signal we're not waiting for data
      waiting_for_data = false;
      
      // Reached maximum number of packets per call, so return
      if (packets_this_call == max_packet_num) return;      
      
    }
    
    // Else, if no data available
    else{
      
      // If we were previously receiving data
      if (!waiting_for_data){
        // If we've not still received the first data yet
        if (!has_received_data){
          if (logger) logger->log("UDP: Waiting for data from STM firmware...",1);
        }
        else{
          // Start new waiting time
          start_wait = std::chrono::high_resolution_clock::now();	    
          // Warn user
          if (logger) logger->log("UDP: Stopped receiving from STM firmware. Waiting for data...",2);
        }
      }		
      
      // Signal we're waiting for data
      waiting_for_data = true;

     
      // If we were previously receiving data and have now gone idle
      if (has_received_data && !idle_timeout_logged) {
         auto now = std::chrono::steady_clock::now();
         auto idle_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
             now - last_recv_time).count();

         if (idle_ms > idle_timeout_ms) {
             idle_timeout_logged = true;
             logger->log("UDP: Idle timeout after " + std::to_string(idle_ms) +
                        "ms, releasing buffer. Packets this call = " +
			std::to_string(packets_this_call) + ".", 2);
	     buffer->has_idle_timeout = true;
             return;
         }
      }
       
      
      // Push what has already been pulled to the queue
      if (packets_this_call > 0) return;
      
    }
    
  } // End while (!stop::should_stop())  
   
  // If a signal has been caught and we've been waiting for data
  if (waiting_for_data){
    // Get waiting end time
    end_wait = std::chrono::high_resolution_clock::now();
    // ... and add to wait time counter
    wait_time += end_wait - start_wait;
    add_wait(end_wait - start_wait);
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
