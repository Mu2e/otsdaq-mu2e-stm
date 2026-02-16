#ifndef TCP_UTILS_HH
#define TCP_UTILS_HH

// Author - George Sweetmore

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

#include "tcp_event_header.hh"  // Header defintion

// Config
constexpr size_t RAW_HEADER_LEN = 32; // length in words of RAW header
constexpr size_t ZS_HEADER_LEN  = 6; // length in words of ZS header
constexpr size_t PH_HEADER_LEN = 4; // length in words of PH header
constexpr int16_t RAW_DATA_LEN   = 30000; // Tunable guess at length of raw payload (per event)
constexpr int16_t ZS_DATA_LEN    = 5000; // .. of zs payload
constexpr int16_t PH_DATA_LEN   = 200; // .. of ph payload
constexpr size_t MAX_RAW_WORDS = 35000;
constexpr size_t MAX_ZS_WORDS  = 35000;
constexpr size_t MAX_EVENT_WORDS = 30032;   // maximum expected words per dataset for reserve
constexpr size_t MAX_PH_WORDS = 35000;
constexpr size_t MAX_BYTE_BUFFER = 18000000; // Max number of bytes to recv per call

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
  const uint16_t* header = nullptr;
  std::array<uint16_t, EVENT_HEADER_WORDS> header_copy{}; 
  int64_t event_num;
  int16_t template_id;
  int16_t spill_flag;

  DatasetView raw;
  DatasetView zs;
  DatasetView ph;
};

struct EventBatch
{
  uint64_t container_seq_id;   // = first event_num in batch
  uint64_t container_frag_id;  // fixed per receiver / stream
  std::vector<EventView> events;
};

#endif // TCP_UTILS_HH
