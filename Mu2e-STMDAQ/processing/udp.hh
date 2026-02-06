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
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh" 
// UDP debug code
#include "Mu2e-STMDAQ/debug/udp_debug.hh" 


// UDP Class for socket setup and data reception
class UDP : public OperationMap {
  
private:

  // Async Logger
  const std::shared_ptr<AsyncLogger> logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Signal Handler
  const std::shared_ptr<SignalHandler>& signal;
  
  // Is this socket a client (if not, then a server)
  const bool is_client;

  // Socket IP address
  const std::string ip;
  // Socket port
  const uint16_t port;
  
  // File descriptor for the UDP socket
  int socket_fd;
  // Structure to hold the socket's address and port
  sockaddr_in address; 

  // System rmem_max
  size_t RMEM_MAX;
  // System wmem_max
  size_t WMEM_MAX;

  // The time value of the UDP timeout
  struct timeval read_timeout;
  struct timespec timeout;

  // Server polling structure
  struct pollfd pfd;
  
  // Pointer to an array of mmsghdr structures for recvmmsg
  struct mmsghdr* messages;
  // Pointer to an array of iovec structures describing the buffers
  struct iovec* iovecs; 

  // Maximum number of packets in buffer
  const size_t max_packet_num;
  
  // Have we received the first packet?
  bool has_received_data = false;

  // Are we waiting for data from the socket
  bool waiting_for_data = false;

  // The total number of packets received in this run
  size_t total_packets_received = 0;
  
  // Timing
  std::chrono::duration<double> wait_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_wait =
    std::chrono::high_resolution_clock::now();
  std::chrono::time_point<std::chrono::high_resolution_clock> end_wait = start_wait;

  std::atomic<double> wait_secs{0.0};
  void add_wait(std::chrono::duration<double> delta){
    double old_val = wait_secs.load(std::memory_order_relaxed);
    double new_val;
    do {
      new_val = old_val + delta.count();
    } while (!wait_secs.compare_exchange_weak(
                                              old_val, new_val,
                                              std::memory_order_release,
                                              std::memory_order_relaxed));
  }
    
public:

  // Constructor
  UDP(const std::shared_ptr<AsyncLogger>& logger_,
      const std::shared_ptr<STMdata>& stm_,
      const std::shared_ptr<SignalHandler>& signal_,      
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
    if (logger) logger->log("UDP server received a total of " + std::to_string(total_packets_received) + " packets.",1);
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

  // Function to send single UDP packet
  void send_packet(const std::vector<int16_t>& data_vec);
  
  // Return the address of the total wait time
  std::chrono::duration<double>* get_wait_time(){
    return &wait_time;
   }  
  
  std::chrono::duration<double> get_wait() {
    return std::chrono::duration<double>(wait_secs.load(std::memory_order_acquire));
  }
  
};

#endif //UDP_hh_ 
