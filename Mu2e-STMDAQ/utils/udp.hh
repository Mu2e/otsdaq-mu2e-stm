#ifndef UDP_hh_
#define UDP_hh_

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <cstring>
#include <array>
#include <functional>
#include <chrono>
#include <iomanip>
#include <sched.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/udp.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/sysinfo.h>
#include <cmath>
#include <signal.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// STM data header
#include "Mu2e-STMDAQ/utils/stm_data.hh"
// Ring buffer code
#include "Mu2e-STMDAQ/utils/ring_buffer.hh" 
// Operations base header
#include "Mu2e-STMDAQ/utils/operations_base.hh" 

// UDP Class for socket setup and data reception
class UDP : public OperationBase {
  
private:

  // Store reference to the Config instance
  const Config& cfg;   
  
  // Async Logger
  const std::shared_ptr<AsyncLogger> logger;

  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;
  
  // The channel number
  const int CHAN;
  
  // Is this socket a client (if not, then a server)
  const bool is_client;

  // Socket IP address
  std::string ip;
  // Socket port
  uint16_t port;
  
  // File descriptor for the UDP socket
  int socket_fd;
  // Structure to hold the socket's address and port
  sockaddr_in address; 

  // System rmem_max
  size_t RMEM_MAX;
  // System wmem_max
  size_t WMEM_MAX;
  // The data struct size
  size_t raw_buffer_size;
  
  // The time value of the UDP timeout
  struct timeval read_timeout;
  struct timespec timeout;
  // The poll timout
  int TIMEOUT_MS = 100; // Default to 0.1s

  // Server polling structure
  struct pollfd pfd;
  
  // Maximum number of packets that can be received in a single call
  size_t max_packets_per_call; 

  // Pointer to an array of mmsghdr structures for recvmmsg
  struct mmsghdr* messages;
  // Pointer to an array of iovec structures describing the buffers
  struct iovec* iovecs; 

  // Have we received the first packet?
  bool has_received_data = false;
  
  // Packets received
  uint64_t packets_received = 0;

  // All thread wait time for paused packet receiving
  std::chrono::duration<double> wait_time;

  // Clock times
  std::chrono::time_point<std::chrono::high_resolution_clock> start_wait =
    std::chrono::high_resolution_clock::now();
  std::chrono::time_point<std::chrono::high_resolution_clock> first_wait = start_wait;
  std::chrono::time_point<std::chrono::high_resolution_clock> end_wait = start_wait;
  
  // Function map for process manager
  std::unordered_map<std::string, std::function<void(std::shared_ptr<DataStruct>&)>> functionMap;
  
public:

  // Constructor
  UDP(const Config& cfg_,
      const std::shared_ptr<AsyncLogger>& logger_,
      const std::shared_ptr<SignalHandler>& signal_,
      const int CHAN_,
      const bool is_client_);
  
  // Destructor
  ~UDP() {
    // Close the socket when the object is destroyed
    close(socket_fd);
    // If server
    if (!is_client){
      delete[] messages; // Free the allocated mmsghdr structures
      delete[] iovecs; // Free the allocated iovec structures
    }
    if (logger) logger->log("UDP server received a total of " + std::to_string(packets_received) + " packets.",1);
    if (logger) logger->log("Total UDP server dead time = " + std::to_string(wait_time.count()) + " seconds.",1);
    std::cout << "UDP destructor called.\n";
  }

  // Get the systems kernel buffer values
  void get_mem_max();
  
  // Configure server
  void configure_server();

  // Set up the UDP server buffers
  void setup_buffers();
  
  // Configure client
  void configure_client();
  
  // Receive data from UDP server
  void receive_data(std::shared_ptr<DataStruct>& buffer);

  // Return the dynamically set struct size
  size_t get_raw_buffer_size() const {
    return raw_buffer_size; 
  }
  
  // Return the maximum packets per call
  size_t get_max_packets_per_call() const {
    return max_packets_per_call; 
  }

  // Function to send single UDP packet
  void send_packet(const std::vector<int16_t>& data_vec);
  
  // Return the address of the total wait time
  std::chrono::duration<double>* get_wait_time(){
    return &wait_time;
  }

  // Execute function for ProcessManager
  void execute(const std::string& methodName, std::shared_ptr<DataStruct>& buffer) override {
    if (functionMap.find(methodName) != functionMap.end()) {
      functionMap[methodName](buffer);
    } else {
      if (logger) logger->log("Error: Invalid method name '" + methodName + "' in UDP\n",0);
    }
  }
  
};

#endif //UDP_hh_ 
