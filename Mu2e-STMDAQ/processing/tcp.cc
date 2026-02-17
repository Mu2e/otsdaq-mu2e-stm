#include "Mu2e-STMDAQ/processing/tcp.hh"

// Construcotr
TCP::TCP(const std::shared_ptr<AsyncLogger>& logger_,
         const std::shared_ptr<STMdata>& stm_,
         const std::shared_ptr<SignalHandler>& signal_)
  : logger(logger_),
    stm(stm_),
    signal(signal_),
    eb(std::make_shared<EventBuilder>(logger,stm)),
    ip(stm->tcp_config.snd_ip),
    port(stm->tcp_config.snd_port),
    poll_timeout_ms(stm->tcp_config.poll_timeout_ms),
    reconnect_period(stm->tcp_config.reconnect_period_ms),
    disconnected_log_period(stm->tcp_config.disconnected_log_period),
    event(3*stm->buffer_config.raw_len,0),
    last_rate_log(std::chrono::steady_clock::now()),
    bytes_since_rate_log(0) {

  // Notify user
  logger->log("TCP: Configuring TCP client for " +
              stm->master_config.ch_name + " channel: IP = " + ip
              + ", PORT = " + std::to_string(port) + ".",1);
  
  // Fill remote address
  std::memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) != 1) {
    logger->log("TCP: inet_pton failed for IP " + ip, 0);
  }

  // Get the max buffer kernel memory
  get_mem_max();

  // Initialize timestamps so rate limiting works immediately
  last_reconnect_attempt = std::chrono::steady_clock::now() - reconnect_period;
  last_disconnected_log  = std::chrono::steady_clock::now() - disconnected_log_period;

  // Constructor policy: wait for server to be reachable, unless stop requested.
  check_for_server(/*wait_forever=*/false);
  
  // Register operation for your pipeline manager
  register_operation("send_data", [this](auto& b) { send_data(b); });
  
}

// Get the max buffer kernel memory
void TCP::get_mem_max() {
  
  // Get system set SNDBUF size from system wmem_max
  std::ifstream wmem("/proc/sys/net/core/wmem_max");
  if (!(wmem >> WMEM_MAX)) WMEM_MAX = 0;
  
  // Log to useer
  if (WMEM_MAX == 0){
    logger->log("TCP:get_mem_max: Error! wmem_max not found in /proc/sys/net/core/wmem_max. Exiting...",0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return;
  }

  // Log to user
  logger->log("TCP: /proc/sys/net/core/wmem_max = " +
              std::to_string(WMEM_MAX) + " bytes.",1);
  
  
}

// Check for server connection
void TCP::check_for_server(bool wait_forever) {

  // While no stop signal
  while (!stop::should_stop()) {

    // If already connected, return immediately
    if (is_connected.load(std::memory_order_acquire)) return;

    // Create and tune a fresh socket for each attempt (simple and robust)
    if (!create_socket()) {
      // Socket creation failed. If not waiting, return...
      if (!wait_forever) return;
      // Sleep to stop CPU spin
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      continue;
    }

    // If client connected to server
    if (connect_socket()) {
      // Connection achieved
      logger->log("TCP: TCP client connected to: IP = " + ip
                  + ", PORT = " + std::to_string(port)              
                  + ", socket = " + std::to_string(socket_fd)
                  // + ", timeout = " + std::to_string(read_timeout.tv_sec)
                  // + " s, " + std::to_string(read_timeout.tv_usec)
                  // + " us."
                  ,1);
      return;
    }

    // Client not connected to server
    log_disconnect();
    
    // If we are not supposed to wait forever, return now.
    if (!wait_forever) return;

    // Sleep to stop CPU spin
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  // stop::should_stop() became true = exit gracefully
  close_socket();
  
}


// Close TCP socket
void TCP::close_socket() {

  // If socket descritor active
  if (socket_fd >= 0) {
    // Close socket
    ::close(socket_fd);
    // Deactivate socket descriptor
    socket_fd = -1;
  }
  
  // Flag socket disconnected
  is_connected.store(false, std::memory_order_release);
  
}

// Create a fresh socket each time we try to reconnect.
bool TCP::create_socket() {

  // Close old socket
  close_socket();

  // Create a TCP socket 
  socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Handle socket creation failure    
  if (socket_fd < 0) {
    log_disconnect();
    return false;
  }

  // Set socket to allow port re-use / to reuse port 
  int optval = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &WMEM_MAX, sizeof(WMEM_MAX));

  // Nagle ON for throughput efficiency (0 means TCP_NODELAY is disabled)
  int nodelay = 0;
  setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

  // Non-blocking for connect timeout + EAGAIN handling
  int flags = fcntl(socket_fd, F_GETFL, 0);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

  // Detect silent half-open connections in idle periods.
  setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
  int idle = 5;   // start keepalives after 5s idle
  int intvl = 1;  // 1s between probes
  int cnt = 5;    // 5 probes -> dead (~10s)
  setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
  setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
  setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));

  // Return succesful socket creation
  return true;
}

// Non-blocking connect with timeout:
bool TCP::connect_socket() {

  // Attempt socket connection
  int rc = ::connect(socket_fd, (sockaddr*)&address, sizeof(address));
  // If successful connection
  if (rc == 0) {
    // Signal socket connection
    is_connected.store(true, std::memory_order_release);
    return true;
  }

  // If connection attempt failed immmediately
  if (rc < 0 && errno != EINPROGRESS) {
    // Signal no connection
    is_connected.store(false, std::memory_order_release);
    return false;
  }
  
  // Create pollfd structure
  pollfd pfd{};
  // Tells poll() what socket to monitor
  pfd.fd = socket_fd;
  
  // Tell poll() we want to know when the socket becomes writable.
  // Non-blocking --> writability = connect success/failure
  pfd.events = POLLOUT;
  
  // Wait until socket writable OR timeout expires OR error
  rc = ::poll(&pfd, 1, poll_timeout_ms);
  
  // poll() == 0: timeout occurred (no connection yet)
  // poll() < 0: error occurred
  if (rc <= 0) {
    // Failed connection
    is_connected.store(false, std::memory_order_release);
    return false;
  }
  
  // If connect succeeded, SO_ERROR = 0.
  // If connect failed, SO_ERROR = reason (e.g. ECONNREFUSED).
  int so_error = 0;
  socklen_t len = sizeof(so_error);
  
  // Query socket for SO_ERROR (clears pending error state)
  if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
    // Failed connection
    is_connected.store(false, std::memory_order_release);
    return false;
  }
  
  // SO_ERROR !- nonzero, connection failed.
  if (so_error != 0) {
    // Copy into errno
    errno = so_error;
    // Failed connection
    is_connected.store(false, std::memory_order_release);
    return false;
  }
  
  // Connect completed, SO_ERROR == 0,
  // Successful connection
  is_connected.store(true, std::memory_order_release);
  return true;

}


// Log no server connection
void TCP::log_disconnect() {
  // Rate-limit the "no connection" logging.
  auto now = std::chrono::steady_clock::now();
  if (now - last_disconnected_log < disconnected_log_period) return;
  last_disconnected_log = now;
  std::string msg = "TCP: No server connection detected! errono = " +
    std::to_string(errno) + " (" +
    std::string(strerror(errno)) + ")";   
  logger->log(msg, 2);
}

// Check errno for disconnect
bool TCP::disconnect_errno(int e) {
  return (e == EPIPE ||
          e == ECONNRESET ||
          e == ENOTCONN ||
          e == ETIMEDOUT);
}

// Send data from DataStruct buffer
void TCP::send_data(std::shared_ptr<DataStruct>& buffer) {

  // If not connected to server at start of send, try to connect
  if (!is_connected.load(std::memory_order_acquire)){
      check_for_server(/*wait_forever=*/false);
    }
  
  // Bytes successfully sent this call 
  uint64_t sent_this_call = 0;

  // Get number of EWTs in buffer
  const size_t n = buffer->EWT_count;

  // The buffer's EWT data
  EWTinfo& EWTs = buffer->EWTs;
  
  // Loop over EWTs
  for (size_t i = 0; i < n; ++i) {

    // Get EWT
    EWT_info* this_EWT = &EWTs[i];
    uint64_t EWT = this_EWT->EWT;
    
    // Build one packed event from EventBuilder
    const size_t len = eb->build_event(buffer, i, event);

    // Check length
    if (len == 0) {
      logger->log("TCP::send_data: Event builder returned length = 0 for EWT = " +
                  std::to_string(EWT) + ". Skipping EWT...", 2);
      continue;
    }

    // Get pointer and byte count
    const uint8_t* src = reinterpret_cast<const uint8_t*>(event.data());
    const size_t data_bytes = len * sizeof(int16_t);
    
    // Attempt send
    bool success = send_all(src, data_bytes);

    // If data sending not succesful
    if (!success) {
      
      // Connection broke mid-send: mark disconnected and drop this buffer
      close_socket();
      
      // Log dropped data
      dropped_EWTs.fetch_add(1, std::memory_order_relaxed);
      dropped_bytes.fetch_add((uint64_t)data_bytes, std::memory_order_relaxed);
      
      // Log to user
      log_disconnect();
      
      // Ppportunistic reconnect attempt
      auto now = std::chrono::steady_clock::now();
      if (now - last_reconnect_attempt >= reconnect_period) {
        last_reconnect_attempt = now;
        check_for_server(/*wait_forever=*/false);
      }
    }

    // Sent this EWT successfully
    sent_this_call += (uint64_t)data_bytes;
    
  }

  // Account bytes for rate logging
  bytes_since_rate_log += sent_this_call;
  
  // Rate-limited throughput log
  auto now = std::chrono::steady_clock::now();
  auto dt = now - last_rate_log;
  
  if (dt >= rate_log_period) {
    const double seconds = std::chrono::duration<double>(dt).count();
    const double gbps = (8.0 * (double)bytes_since_rate_log) / seconds / 1e9;
    
    // Log to user
    logger->log("TCP_SEND_CLIENT: send rate = " + std::to_string(gbps) +
                " Gbit/s (avg over " + std::to_string(seconds) + " s)", 1);
    
    // reset window
    last_rate_log = now;
    bytes_since_rate_log = 0;
  }
  
}

// Send byte stream
bool TCP::send_all(const uint8_t* src, size_t nbytes) {

  // If not connected, return
  if (socket_fd < 0 || !is_connected.load(std::memory_order_acquire)) return false;
  
  // Sent bytes
  size_t sent = 0;
  // The poll timeout
  const int poll_timeout_ms = 5; // short sleep when backpressured

  // Poll descriptor
  pollfd pfd{};
  pfd.fd = socket_fd;
  pfd.events = POLLOUT;

  // While not all bytes have sent
  //  while (sent < nbytes && !stop::should_stop()) {
  while (sent < nbytes) {

    // Send byte stream
    ssize_t n = ::send(socket_fd, src + sent, nbytes - sent, MSG_NOSIGNAL);

    // If data has been sent
    if (n > 0) {

      // Store how much sent
      sent += (size_t)n;
      // Store total bytes sent
      total_bytes_sent.fetch_add((uint64_t)n, std::memory_order_relaxed);
      continue;
    }

    // If error code
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Backpressure: sleep until writable (CPU-efficient)
        ::poll(&pfd, 1, poll_timeout_ms);
        continue;
      }

      // Check for errno
      if (disconnect_errno(errno)) {
        return false;
      }

      // Other errors: treat as disconnected too (simpler policy)
      return false;
    }

    // n == 0 -> no progress; treat similarly to backpressure
    ::poll(&pfd, 1, poll_timeout_ms);
  }

  // Return number of sent bytes
  return (sent == nbytes);
  
}

 
