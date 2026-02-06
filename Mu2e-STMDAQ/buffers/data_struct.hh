#ifndef DATA_STRUCT_HH_
#define DATA_STRUCT_HH_

// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"

// Struct of baseline fit data
struct baseline_fit{
  
  // Baseline Gaussian Values
  double w0 = 0;
  double mu0 = 0;
  double sigma0 = 0;
  // Pulse Tail Exponential Values  
  double w1 = 0;
  double t = 0;
  double lambda = 0;

  // Reinitalise struct
  void reset() {
    w0 = 0;
    mu0 = 0;
    sigma0 = 0;
    w1 = 0;
    t = 0;
    lambda = 0;
  }
};

// Pulse height
struct pulse_height {
  int16_t time = 0; // Peak time
  int16_t height = 0; // Peak height  
  pulse_height(int16_t t, int16_t h)
    : time(t), height(h) {}
};

// Zero suppression
struct zs_region {
  size_t start = 0; // Data start of zs peak data
  size_t len = 0; // Data length of zs peak data
  zs_region(size_t s, size_t l)
    : start(s), len(l) {}  
};

// Zero suppression
struct zs_data {
  size_t adc_count = 0; // Total zs data found in this EWT
  std::vector<zs_region> zs_regions;  // All zs data regions found in this EWT
  bool prescale = false; // Prescale this zs data?
};

// Raw data 
struct raw_info {  
  size_t start = 0; // Buffer index of start of event's ADC data
  size_t len = 0; // Length of event's ADC data in the buffer
  bool prescale = false; // Prescale this raw data?
};

// EWT information
struct EWT_info {
  uint64_t EWT = 0; // The EWT number
  sw_event_header hdr = {}; // The EWTS new header data
  raw_info raw; // The raw data information of this EWT
  zs_data zs;  // All zs data found in this EWT
  std::vector<pulse_height> ph;  // All peak heights found in this EW
  bool bad_data = false; // Is this EWT's data bad (e.g. dropped packets)?
};

// Define vector type
using EWTinfo = std::vector<EWT_info>;

// Data struct buffer
struct DataStruct {

  // The fixed buffer number
  const int buffer_num;

  // Do not use flag
  bool do_not_use = false;
  
  // Data vectors
  alignas(64) std::vector<int16_t> raw; // Raw data
  std::vector<int16_t> zs; // Zero-suppressed data
  std::vector<double> ph; // Pulse height buffer

  // Data lengths
  size_t orig_len = 0, raw_len = 0, zs_len = 0, ph_len = 0;

  // Packet info
  size_t packet_count = 0;
  size_t dropped_packet_count = 0;
  std::vector<std::pair<uint32_t,uint32_t>> dropped_packets;

  // EWT info
  size_t EWT_count = 0;
  size_t lost_EWT_count = 0;
  std::vector<std::pair<uint64_t,uint64_t>> lost_EWTs;
  std::vector<std::pair<uint64_t,uint64_t>> incomplete_EWTs;
  
  // The EWT data
  EWTinfo EWTs;

  // Zero suppression
  size_t zs_overflow_num = 0;
  std::vector<uint64_t> peak_index;
  std::vector<zs_region> zs_data;
  std::atomic<bool> peak_finding_finished{false};

  // Baseline calculation
  std::vector<uint64_t> hist_counts_window; // Sliding window histogram
  std::vector<uint64_t> hist_counts_all; // All data histogram
  uint64_t total_window = 0; // Total Window histogram counts
  uint64_t total_all = 0; // Total all data histrogram counts 
  baseline_fit baseline_all; // All data baseline fit results
  baseline_fit baseline_window; // Sliding window baseline fit results

  // The baseline values averaged since the start of the run
  double baseline_mean_avg = 0;
  double baseline_sigma_avg = 0;
  // The current baseline values of the data in this buffer only
  double baseline_mean_current = 0;
  double baseline_sigma_current = 0;
  // Noise data for DQM
  std::vector<int16_t> noise_data;

  // MWD output
  size_t peak_count = 0;

  // CPU performace/efficiency metrics
  std::vector<std::pair<const std::string, double>> cpu_performance;
  
  // Constructor initializes the buffer with a fixed size
  DataStruct(const std::shared_ptr<STMdata>& stm,
             int buffer_num_,
             std::vector<std::string> op_names) 
    : buffer_num(buffer_num_),
      raw(stm->buffer_config.raw_len),
      zs(stm->buffer_config.zs_len),
      ph(stm->buffer_config.ph_len),
      EWTs(stm->buffer_config.max_event_num),
      hist_counts_window(stm->baseline_config.hist_bin_num),
      hist_counts_all(stm->baseline_config.hist_bin_num),
      noise_data(stm->buffer_config.baseline_len),
      cpu_performance([&]{
        std::vector<std::pair<const std::string,double>> v;
        v.reserve(op_names.size());
        for (const auto& name : op_names) {
          v.emplace_back(name, 0.0);
        }
        return v;
      }())
  {
    reset();  // Ensure clean state on creation
  }

  // Reinitalise the data struct
  void reset() {
    do_not_use = false;
    // Reset vector contents 
    std::fill(raw.begin(), raw.end(), 0);
    std::fill(zs.begin(), zs.end(), 0);
    std::fill(ph.begin(), ph.end(), 0);
    orig_len = raw_len = zs_len = ph_len = 0;
    packet_count = 0;
    dropped_packet_count = 0;
    dropped_packets.resize(0);
    EWT_count = 0;
    lost_EWT_count = 0;
    lost_EWTs.resize(0);
    incomplete_EWTs.resize(0);
    for (auto& e : EWTs) {
      e.EWT = 0;
      e.raw = raw_info{};              // if raw_info is trivially copyable      
      e.zs.adc_count = 0;
      e.zs.zs_regions.clear();
      e.zs.prescale = false;
      e.ph.clear();
    }
    zs_overflow_num = 0;
    peak_index.resize(0);
    zs_data.clear();
    std::fill(hist_counts_window.begin(), hist_counts_window.end(), 0);
    std::fill(hist_counts_all.begin(), hist_counts_all.end(), 0);    
    baseline_mean_avg = 0;
    baseline_sigma_avg = 0;    
    baseline_mean_current = 0;
    baseline_sigma_current = 0;
    baseline_window.reset();
    baseline_all.reset();
    std::fill(noise_data.begin(), noise_data.end(), 0);
    peak_count = 0;
    //    peaks.resize(0);
    for (auto& entry : cpu_performance) entry.second = 0.0;
  }

  // Destructor
  ~DataStruct() = default;
  
};

#endif // DATA_STRUCT_HH_
