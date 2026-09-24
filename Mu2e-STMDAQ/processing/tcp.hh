#ifndef TCP_hh_
#define TCP_hh_

#include <memory>
#include <string>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <poll.h>

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// Signal handler header
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"
// Event builder code
#include "Mu2e-STMDAQ/processing/event_builder.hh"

// TCP class
class TCP : public OperationMap {

private:

  // Logger
  const std::shared_ptr<AsyncLogger> logger;

  // STMdata 
  const std::shared_ptr<STMdata>& stm;

  // Signal handler
  const std::shared_ptr<SignalHandler>& signal;

  // Event Builder
  const std::shared_ptr<EventBuilder> eb;
  
  // Socket IP address
  const std::string ip;
  // Socket port
  const uint16_t port;

  // Current socket FD (recreated on reconnect)
  int socket_fd{-1};

  // Remote address
  sockaddr_in address{};

  // Linux kernel max buffer sizes
  size_t WMEM_MAX{0};

  // Is server connected?
  std::atomic<bool> is_connected{false};

  // Poll timeout in milliseconds
  const int poll_timeout_ms;
  
  // How often we attempt reconnect while disconnected
  const std::chrono::milliseconds reconnect_period;
  // How often we print "still disconnected / dropping" logs
  const std::chrono::seconds disconnected_log_period;

  // Internal timestamps for rate limiting
  std::chrono::steady_clock::time_point last_reconnect_attempt{};
  std::chrono::steady_clock::time_point last_disconnected_log{};

  // Stats (optional)
  std::atomic<uint64_t> dropped_EWTs{0};
  std::atomic<uint64_t> dropped_bytes{0};
  std::atomic<uint64_t> total_bytes_sent{0};

  // Event to write for events data case
  std::vector<int16_t> event;

  // Get the max buffer kernel memory
  void get_mem_max();

  // Create + configure a new socket_fd 
  bool create_socket();

  // Close current socket_fd if open
  void close_socket();

  // Connect attempt with timeout (ms)
  bool connect_socket();

  // Ensure we have a live connection.
  void check_for_server(bool wait_forever);

  // Send all bytes
  bool send_all(const uint8_t* src, size_t nbytes);

  // Log no server connection 
  void log_disconnect();

  // Check errno for disconnect
  static bool disconnect_errno(int e);

public:

  // Constructuro
  TCP(const std::shared_ptr<AsyncLogger>& logger_,
                  const std::shared_ptr<STMdata>& stm_,
                  const std::shared_ptr<SignalHandler>& signal_);

  // Destructor
  ~TCP(){
    // Close socket
    close_socket();
    // Log stats to user
    logger->log("TCP: total bytes sent = " + std::to_string(total_bytes_sent.load()), 1);
    logger->log("TCP: dropped EWTs  = " + std::to_string(dropped_EWTs.load()), 1);
    logger->log("TCP: dropped bytes    = " + std::to_string(dropped_bytes.load()), 1);
    std::cout << "TCP destructor called.\n";
  }

  // Send payload
  void send_data(std::shared_ptr<DataStruct>& buffer);

  // Read stats/status
  bool connected() const { return is_connected.load(); }
  
  // How often to print throughput
  const std::chrono::seconds rate_log_period{2};
  
  // Bookkeeping for rate calculation
  std::chrono::steady_clock::time_point last_rate_log{};
  uint64_t bytes_since_rate_log{0};
  
};

#endif
