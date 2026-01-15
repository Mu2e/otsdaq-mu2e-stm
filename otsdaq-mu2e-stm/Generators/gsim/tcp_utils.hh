#ifndef TCP_UTILS_HH
#define TCP_UTILS_HH

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
constexpr size_t MAX_RAW_WORDS = 35000;
constexpr size_t MAX_ZS_WORDS  = 35000;
constexpr size_t MAX_EVENT_WORDS = 30032;   // maximum expected words per dataset for reserve
constexpr size_t MAX_MWD_WORDS = 35000;
constexpr size_t MAX_BYTE_BUFFER = 18000000; // Max number of bytes to recv per call

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
constexpr double TARGET_GBPS = 10.0;
const int SPILL_SEQUENCE[8] = {1,1,1,1,1,1,1,0}; // 7 on, 1 off

// Flag for printout
bool verbose = true;

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
  int64_t event_num;
  int16_t template_id;
  int16_t spill_flag;

  DatasetView raw;
  DatasetView zs;
  DatasetView mwd;
};


// ------------------ Dummy sender vars/funcs ------------------

// Event struct with members for each dataset
struct Event {
  int64_t event_num;
  uint16_t raw_len;
  uint16_t zs_len;
  uint16_t mwd_len;
  int16_t template_id;
  int16_t spill_flag;  // 1 = on, 0 = off

  std::vector<int16_t> raw;
  std::vector<int16_t> zs;
  std::vector<int16_t> mwd;
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

// Helper: make header with last word = data length
std::vector<int16_t> make_header(size_t header_size, int16_t data_len) {
  std::vector<int16_t> header(header_size, 0);
  header[header_size - 1] = data_len;
  return header;
}

// Build buffer for a batch of events
std::vector<int16_t> build_buffer(const std::vector<Event>& events) {
  std::vector<int16_t> buffer;

  for (const auto& ev : events) {
    // --- 3-word event-level payload length in bytes ---
    int64_t sum_bytes = 2 * (static_cast<int64_t>(RAW_HEADER_LEN) + static_cast<int64_t>(ev.raw.size()) +
			     static_cast<int64_t>(ZS_HEADER_LEN)  + static_cast<int64_t>(ev.zs.size()) +
			     static_cast<int64_t>(MWD_HEADER_LEN) + static_cast<int64_t>(ev.mwd.size()));

    uint16_t part1 = static_cast<uint16_t>(sum_bytes & 0xFFFF);
    uint16_t part2 = static_cast<uint16_t>((sum_bytes >> 16) & 0xFFFF);
    uint16_t part3 = static_cast<uint16_t>((sum_bytes >> 32) & 0xFFFF);

    buffer.push_back(static_cast<int16_t>(part1));
    buffer.push_back(static_cast<int16_t>(part2));
    buffer.push_back(static_cast<int16_t>(part3));

    // --- RAW header ---
    auto raw_header = make_header(RAW_HEADER_LEN, static_cast<int16_t>(ev.raw.size()));

    // Anchor
    raw_header[0] = static_cast<int16_t>(0xBEEF);

    // Event number
    int16_t w0, w1, w2;
    event_num_to_words(ev.event_num, w0, w1, w2);
    raw_header[11] = w0;
    raw_header[12] = w1;
    raw_header[13] = w2;

    // Spill flag at header[19]
    raw_header[19] = static_cast<int16_t>(ev.spill_flag);

    // Append RAW header
    buffer.insert(buffer.end(), raw_header.begin(), raw_header.end());

    // --- RAW data ---
    buffer.insert(buffer.end(), ev.raw.begin(), ev.raw.end());

    // --- ZS header ---
    auto zs_header = make_header(ZS_HEADER_LEN, static_cast<int16_t>(ev.zs.size()));
    buffer.insert(buffer.end(), zs_header.begin(), zs_header.end());

    // ZS data
    buffer.insert(buffer.end(), ev.zs.begin(), ev.zs.end());

    // --- MWD header ---
    auto mwd_header = make_header(MWD_HEADER_LEN, static_cast<int16_t>(ev.mwd.size()));
    buffer.insert(buffer.end(), mwd_header.begin(), mwd_header.end());

    // MWD data
    buffer.insert(buffer.end(), ev.mwd.begin(), ev.mwd.end());
  }

  return buffer;
}

#endif // TCP_UTILS_HH
