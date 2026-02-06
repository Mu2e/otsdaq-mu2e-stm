#ifndef TCP_UTILS_H
#define TCP_UTILS_H

#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// Config
constexpr size_t RAW_HEADER_LEN = 32; // length in words of RAW header
constexpr size_t ZS_HEADER_LEN  = 6; // length in words of ZS header
constexpr size_t MWD_HEADER_LEN = 4; // length in words of MWD header
constexpr int16_t RAW_DATA_LEN   = 30000; // Tunable guess at length of raw payload (per event)
constexpr int16_t ZS_DATA_LEN    = 5000; // .. of zs payload
constexpr int16_t MWD_DATA_LEN   = 200; // .. of mwd payload
constexpr size_t MAX_RING_BUFFER = 100000000; // Max size of ring buffer
constexpr size_t MAX_BYTE_BUFFER = 18000000; // Max number of bytes to recv per call
constexpr size_t QUEUE_CAPACITY = 360000; // Max number of bytes in queue
constexpr size_t MAX_EVENT_WORDS = 30032;   // maximum expected words per dataset for reserve
constexpr size_t WRITE_BUFFER_LIMIT = 10;   // number of events to buffer before flushing
// Event header constants
constexpr size_t EVENT_HEADER_WORDS = 3;                   // 3 int16_t words
constexpr size_t EVENT_HEADER_BYTES = EVENT_HEADER_WORDS * sizeof(int16_t);

// ------------------ Spill Sequence ------------------
// Spill lengths
constexpr uint RAW_ON_LEN  = 510;
constexpr uint ZS_ON_LEN   = 510;
constexpr uint MWD_ON_LEN  = 100;

constexpr uint RAW_OFF_LEN = 30000;
constexpr uint ZS_OFF_LEN  = 5000;
constexpr uint MWD_OFF_LEN = 200;

constexpr size_t MAX_ON_SPILL_BYTES  = ((RAW_HEADER_LEN + ZS_HEADER_LEN + MWD_HEADER_LEN) + (3 * RAW_ON_LEN)) * sizeof(int16_t);
constexpr size_t MAX_OFF_SPILL_BYTES = ((RAW_HEADER_LEN + ZS_HEADER_LEN + MWD_HEADER_LEN) + (3 * RAW_OFF_LEN)) * sizeof(int16_t);

// Sequence parameters
constexpr size_t ON_SPILL_COUNT = 7;
constexpr size_t OFF_SPILL_COUNT = 1;

// Total duration
constexpr double DURATION_MIN = 1.0; // 1 minute
constexpr double TARGET_GBPS = 0.1;
const int SPILL_SEQUENCE[8] = {1,1,1,1,1,1,1,0}; // 7 on, 1 off

// Anchor words
constexpr uint16_t RAW_ANCHOR = 0xBEEF;
constexpr uint16_t ZS_ANCHOR = 0xDEAD;
constexpr uint16_t MWD_ANCHOR = 0xFEED;

int PORT = 10025; // TCP socket port
const char* IP = "127.0.0.2";
int N_EVENTS=100; // Number of generated events

// Flag for printout
bool verbose = true;

// New HDR vars
constexpr size_t EVENT_HEADER_LEN = 21;   // indices 0 .. 20 inclusive
constexpr uint16_t EVENT_ANCHOR = 0xCAFE;
// Index positions
constexpr uint16_t anchor_start = 0;
constexpr uint16_t EWT_0 = 1;
constexpr uint16_t EWT_1 = 2;
constexpr uint16_t EWT_2 = 3;
constexpr uint16_t ADCclk_0 = 4;
constexpr uint16_t ADCclk_1 = 5;
constexpr uint16_t ADCclk_2 = 6;
constexpr uint16_t ADCclk_3 = 7;
constexpr uint16_t Ch_DTCclk_0 = 8;
constexpr uint16_t DTCclk_1 = 9;
constexpr uint16_t DTCclk_2 = 10;
constexpr uint16_t DTCclk_3 = 11;
constexpr uint16_t EM_0 = 12;
constexpr uint16_t EM_1 = 13;
constexpr uint16_t EM_2_DRTDC = 14;
constexpr uint16_t PRESCALE = 15;
constexpr uint16_t RAW_LEN = 16;
constexpr uint16_t ZS_REGIONS = 17;
constexpr uint16_t ZS_LEN = 18;
constexpr uint16_t PH_NUM = 19;
constexpr uint16_t anchor_end = 20;

// Event struct with members for each dataset
// Adjusted Event struct to allow per-event lengths
struct Event {
  int64_t event_num;
  uint16_t raw_len;
  uint16_t zs_len;
  uint16_t mwd_len;
  int16_t spill_flag;  // 1 = on, 0 = off
  int16_t template_id;
  std::vector<int16_t> raw;
  std::vector<int16_t> zs;
  std::vector<int16_t> mwd;
};

// BufferView + Eventx (For Receiver)
struct BufferView {
  size_t offset; // word offset into base vector (int16_t words)
  size_t size;   // number of words

};

struct Eventy {
  int64_t event_num;
  std::shared_ptr<std::vector<int16_t>> base; // holds full payload
  BufferView raw;
  BufferView zs;
  BufferView mwd;
};

struct ChunkRef {
  std::shared_ptr<std::vector<uint8_t>> chunk;
  size_t offset;
  size_t size;
};

struct DatasetView {
  size_t offset;
  size_t size;
};

struct EventView {
  std::vector<ChunkRef> buffer_chunks;
  size_t size_bytes;
  const uint16_t* header = nullptr;
  int64_t event_num;
  int16_t spill_flag;
  int16_t template_id;
  
  DatasetView raw;
  DatasetView zs;
  DatasetView mwd;
};

// Receiver variables
extern std::atomic<uint64_t> total_bytes_received;
extern std::atomic<size_t> event_count;
extern std::atomic<size_t> total_words_parsed;
extern std::atomic<uint64_t> total_events_parsed;
extern std::atomic<bool> done;

extern bool write_enabled;
extern bool metrics_enabled;
extern bool debug_logging;

// Create Sender TCP socket and connect to receiver
inline int setup_sender_socket(const char* server_ip, int port) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return -1;
  }

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    perror("inet_pton");
    close(sockfd);
    return -1;
  }

  if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("connect");
    close(sockfd);
    return -1;
  }

  // Increase TCP send buffer
  int bufsize = 128 * 1024 * 1024; // 128 MB
  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize)) < 0) {
    perror("setsockopt(SO_SNDBUF)");
  } else {
    std::cout << "TCP send buffer set to "
	      << bufsize / (1024 * 1024) << " MB" << std::endl;
  }

  std::cout << "Connected to receiver at " << server_ip << ":" << port << std::endl;
  return sockfd;
}

// Create Receiver TCP socket and bind it
inline int setup_tcp_socket(const char* server_ip, int port) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) { perror("socket"); return -1; }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  // Convert input IP string to binary form
  if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
    perror("inet_pton"); 
    close(server_fd); 
    return -1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(server_fd); return -1; }
  if (listen(server_fd, 1) < 0) { perror("listen"); close(server_fd); return -1; }

  std::cout << "Receiver listening on port " << port << std::endl;

  int client_fd = accept(server_fd, nullptr, nullptr);
  if (client_fd < 0) { perror("accept"); close(server_fd); return -1; }

  // Increase TCP receive buffer
  int bufsize = 128 * 1024 * 1024; // 128 MB
  if (setsockopt(client_fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize)) < 0) {
    perror("setsockopt(SO_RCVBUF)");
  } else {
    std::cout << "TCP receive buffer set to " << bufsize / (1024 * 1024) << " MB" << std::endl;
  }
  std::cout << "Client connected" << std::endl;
  close(server_fd); // only need client socket now
  return client_fd;
}

struct EventSizes {
  uint32_t raw_len;
  uint32_t zs_len;
  uint32_t mwd_len;
};

// Precompute offsets for template_id + event_num updates
struct BufferTemplate {
  std::vector<int16_t> buffer;
  std::vector<size_t> event_num_offsets; // byte offsets to event_num words
  int16_t template_id;
  std::vector<EventSizes> event_sizes;
};

// Convert int64_t event_num into three network-order int16_t words
inline void event_num_to_words(int64_t event_num, int16_t& w0, int16_t& w1, int16_t& w2) {
  w0 = static_cast<int16_t>(event_num & 0xFFFF);
  w1 = static_cast<int16_t>((event_num >> 16) & 0xFFFF);
  w2 = static_cast<int16_t>((event_num >> 32) & 0xFFFF);
}

// Convert back three network-order int16_t words into a uint64_t
static inline uint64_t build_len_from_3_words(uint16_t w0, uint16_t w1, uint16_t w2) {
  return static_cast<uint64_t>(w0) |
    (static_cast<uint64_t>(w1) << 16) |
    (static_cast<uint64_t>(w2) << 32);
}

// make header with first word = anchor and last word = data length
std::vector<int16_t> make_header(size_t header_size, int16_t data_len, int16_t anchor = 0xBEEF) {
  std::vector<int16_t> header(header_size, 0);

  // First word is the anchor
  header[0] = anchor;

  // Last word stores payload length
  header[header_size - 1] = data_len;

  return header;
}

std::vector<int16_t> build_buffer(const std::vector<Event>& events) {
  std::vector<int16_t> buffer;

  for (const auto& ev : events) {

    /* ---------------- Event payload size (bytes) ---------------- */
    int64_t payload_bytes =
      static_cast<int64_t>(
			   (EVENT_HEADER_LEN +
			    ev.raw.size() +
			    ev.zs.size() +
			    ev.mwd.size()) * sizeof(int16_t));

    /* ---------------- Build event header ---------------- */
    std::vector<int16_t> hdr(EVENT_HEADER_LEN, 0);

    hdr[anchor_start] = EVENT_ANCHOR;
    hdr[anchor_end]   = EVENT_ANCHOR;

    /* ---- Event / timing metadata (synthetic but consistent) ---- */
    hdr[EWT_0] = static_cast<int16_t>( ev.event_num        & 0xFFFF);
    hdr[EWT_1] = static_cast<int16_t>((ev.event_num >> 16) & 0xFFFF);
    hdr[EWT_2] = static_cast<int16_t>((ev.event_num >> 32) & 0xFFFF);

    // Fake but monotonic clocks
    hdr[ADCclk_0] = static_cast<int16_t>( ev.event_num        & 0xFFFF);
    hdr[ADCclk_1] = static_cast<int16_t>((ev.event_num >> 16) & 0xFFFF);
    hdr[ADCclk_2] = 0;
    hdr[ADCclk_3] = 0;

    hdr[Ch_DTCclk_0] = static_cast<int16_t>(ev.spill_flag & 0x1);
    hdr[DTCclk_1]    = hdr[ADCclk_0];
    hdr[DTCclk_2]    = hdr[ADCclk_1];
    hdr[DTCclk_3]    = 0;

    /* ---- Event mode ---- */
    hdr[EM_0] = ev.template_id;
    hdr[EM_1] = 0;
    hdr[EM_2_DRTDC] = 0;

    /* ---- Prescale word ---- */
    // Bit 0: RAW kept
    hdr[PRESCALE] = ev.raw.empty() ? 0 : 1;

    /* ---- Dataset sizes ---- */
    hdr[RAW_LEN]    = static_cast<int16_t>(ev.raw.size());
    hdr[ZS_REGIONS] = static_cast<int16_t>(ev.mwd.size());
    hdr[ZS_LEN]     = static_cast<int16_t>(ev.zs.size());
    hdr[PH_NUM]     = static_cast<int16_t>(ev.mwd.size());

    /* ---------------- Append to buffer ---------------- */
    buffer.insert(buffer.end(), hdr.begin(), hdr.end());
    buffer.insert(buffer.end(), ev.raw.begin(), ev.raw.end());
    buffer.insert(buffer.end(), ev.zs.begin(),  ev.zs.end());
    buffer.insert(buffer.end(), ev.mwd.begin(), ev.mwd.end());
  }

  return buffer;
}

#endif // TCP_UTILS_H
