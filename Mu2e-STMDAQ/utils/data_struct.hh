#ifndef DATA_STRUCT_HH_
#define DATA_STRUCT_HH_

// STM data header
#include "Mu2e-STMDAQ/utils/stm_data.hh"

// Header map indices
constexpr int hdrMap_EWT = 0; // EWT
constexpr int hdrMap_adcIndex = 1; // Buffer index of start of event's ADC data
constexpr int hdrMap_dataLen = 2; // Length of event's ADC data in the buffer
constexpr int hdrMap_eventStart = 3; // Offset from start of event data
constexpr int hdrMap_eventLen = 4; // Total event length
constexpr int hdrMap_hdrData = 5; // The actual header data

// Header map tuple type
using hdrMapTuple = std::tuple<
  uint64_t, // EW
  size_t, // Buffer index of start of event's ADC data
  size_t, // Length of event's ADC data in the buffer
  size_t, // Offset from start of event data
  size_t, // Total event length
  std::array<int16_t,eHdr_Len> // The actual header data
  >;

// Initialised header map
constexpr hdrMapTuple init_hdr_map = {
  0, // EW
  0, // Buffer index of start of event's ADC data
  0, // Length of event's ADC data in the buffer
  0, // Offset from start of event data
  0, // Total event length
  std::array<int16_t,eHdr_Len>{}}; // The actual header data
  
// Data struct buffer
struct DataStruct {

  // The fixed buffer number
  const int buffer_num;
  
  // Data vectors
  alignas(64) std::vector<int16_t> raw; // Raw data
  std::vector<int16_t> zs; // Zero-suppressed data
  std::vector<int16_t> mwd; // MWD data

  // Data lengths
  size_t orig_len = 0, raw_len = 0, zs_len = 0, mwd_len = 0;

  // Packet info
  size_t packet_count = 0;
  size_t dropped_packet_count = 0;
  std::vector<std::pair<uint32_t,uint32_t>> dropped_packets;

  // EWT info
  size_t EWT_count = 0;
  size_t lost_EWT_count = 0;
  std::vector<std::pair<uint64_t,uint64_t>> lost_EWTs;
  std::vector<std::pair<uint64_t,uint64_t>> incomplete_EWTs;
  
  // Raw data header map
  size_t raw_header_num = 0;
  std::vector<hdrMapTuple> raw_header_map;

  // Zero suppression
  size_t zs_overflow_num = 0;
  std::vector<uint64_t> peak_index;
  // The baseline values averaged since the start of the run
  double baseline_mean_avg = 0;
  double baseline_rms_avg = 0;
  // The current baseline values of the data in this buffer only
  double baseline_mean_current = 0;
  double baseline_rms_current = 0;
  // Noise data for DQM
  std::vector<int16_t> noise_data;

  // MWD
  std::vector<int16_t> mwd_peak_heights;
  
  // Constructor initializes the buffer with a fixed size
  DataStruct(int buffer_num_, size_t raw_size, double zs_size, size_t noise_len, size_t mwd_size,
	     size_t max_events_per_struct) 
    : buffer_num(buffer_num_),
      raw(static_cast<size_t>(raw_size) / sizeof(int16_t)),
      zs(static_cast<size_t>(zs_size) / sizeof(int16_t)),
      mwd(static_cast<size_t>(mwd_size) / sizeof(int16_t)),
      raw_header_map(max_events_per_struct),
      noise_data(noise_len) {
    reset();  // Ensure clean state on creation
  }

  // Reinitalise the data struct
  void reset() {
    // Reset vector contents 
    std::fill(raw.begin(), raw.end(), 0);
    std::fill(zs.begin(), zs.end(), 0);
    std::fill(mwd.begin(), mwd.end(), 0);
    orig_len = raw_len = zs_len = mwd_len = 0;
    packet_count = 0;
    dropped_packet_count = 0;
    dropped_packets.resize(0);
    EWT_count = 0;
    lost_EWT_count = 0;
    lost_EWTs.resize(0);
    incomplete_EWTs.resize(0);
    raw_header_num = 0;
    std::fill(raw_header_map.begin(), raw_header_map.end(), init_hdr_map);
    zs_overflow_num = 0;
    peak_index.resize(0);
    baseline_mean_avg = 0;
    baseline_rms_avg = 0;    
    baseline_mean_current = 0;
    baseline_rms_current = 0;
    std::fill(noise_data.begin(), noise_data.end(), 0);
    mwd_peak_heights.resize(0);
  }

  // Destructor
  ~DataStruct() {
    // Ensure memory is cleared before destruction
    if (!raw.empty()) {
      raw.clear();  
    }
    if (!zs.empty()) {
      zs.clear();
    }
    if (!mwd.empty()) {
      mwd.clear();
    }
    if (!dropped_packets.empty()) {
      dropped_packets.clear();
    }
    if (!lost_EWTs.empty()) {
      lost_EWTs.clear();
    }
    if (!incomplete_EWTs.empty()) {
      incomplete_EWTs.clear();
    }
    if (!raw_header_map.empty()) {
      raw_header_map.clear();
    }
    if (!peak_index.empty()) {
      peak_index.clear();
    }
  }
  
};

#endif // DATA_STRUCT_HH_
