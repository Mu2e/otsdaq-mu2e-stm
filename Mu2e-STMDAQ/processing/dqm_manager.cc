#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <atomic>

// Include dqm manager code
#include "Mu2e-STMDAQ/processing/dqm_manager.hh"
// Include dqm structs code
#include "Mu2e-STMDAQ/dqm/dqm_structs.hh"
// Operations manager header
#include "Mu2e-STMDAQ/processing/operation_manager.hh"
// Include UDP code
#include "Mu2e-STMDAQ/processing/udp.hh"

// Constructor
DQM::DQM(const Config& cfg_,
         const std::shared_ptr<AsyncLogger>& logger_,
         const std::shared_ptr<STMdata>& stm_,
         const std::shared_ptr<SignalHandler>& signal_,
	 OperationManager* op_man_) :
  cfg(cfg_), logger(logger_),  stm(stm_), signal(signal_), op_man(op_man_),
  baseline_mean_prev(stm->baseline_config.prev_mean),
  baseline_sigma_prev(stm->baseline_config.prev_sigma),
  baseline_nbins(stm->baseline_config.hist_bin_num),
  noise_len(stm->buffer_config.baseline_len),
  raw_len(stm->dqm_config.raw_len),
  peak_nbins(stm->dqm_config.peak_nbins),
  min_ph(stm->dqm_config.min_ph),
  inv_wid(stm->dqm_config.inv_bin_wid),
  peak_hist_window(peak_nbins,0),
  peak_hist_all(peak_nbins,0),
  window_size_buffers(stm->baseline_config.hist_window_buffers),
  per_buffer_hists(window_size_buffers, std::vector<uint64_t>(peak_nbins, 0)),
  num_dropped_packets(0),
  op_num(0),
  running_(false)
{
    
  size_t baseline_bytes = sizeof(dqm_data_baseline) + 
                     ((baseline_nbins - 1) * sizeof(uint64_t)) + // -1 for array initialised with 1
                     (noise_len * sizeof(int16_t)) +
		     7 + 				// Alignment padding
  		     sizeof(uint64_t);			// End counter
  size_t raw_bytes = sizeof(dqm_data_raw) +
	  	     ((raw_len - 1) * sizeof(int16_t)) + // -1 for array initialised with size 1
		     7 + 				// Alignment padding
  		     sizeof(uint64_t);			// End counter
  size_t peak_bytes = sizeof(dqm_data_peak) +
	  	     ( ((2 * peak_nbins) - 1) * sizeof(uint64_t)) + // Two histograms -1 for array initialised with size 1
		     sizeof(uint64_t);			// End counter

  // Register shared memory blocks
  registerBlock<dqm_data_raw>(DQMPageType::RAW, "/dqm_raw_data", raw_bytes);
  registerBlock<dqm_data_baseline>(DQMPageType::BASELINE, "/dqm_adc_baseline", baseline_bytes);
  registerBlock<dqm_data_peak>(DQMPageType::PEAKS, "/dqm_peak_data", peak_bytes); 
  
  // Register operations for OperationManager
  register_operation("update_dqm", [this](auto& b){ update_dqm(b); });
  
}

void DQM::init_shm() {
  // Op num only known after op_manager has run
  this->op_num = op_man->getUseOps().size()-1;

  size_t daq_bytes = sizeof(dqm_data_daq) +
		     ((op_num - 1) * sizeof(op_entry)) + // -1 for array initialised with size 1
		     7 +				// Alignment padding
		     sizeof(uint64_t);			// End counter

  registerBlock<dqm_data_daq>(DQMPageType::SPEEDS, "/dqm_daq_data", daq_bytes);

}

auto t0 = std::chrono::steady_clock::now();
auto prev_baseline = t0;
auto prev_raw = t0;
auto prev_peak = t0;
auto prev_cpu = t0;
auto prev_alarm = t0;

constexpr auto baseline_period = std::chrono::milliseconds{200};
constexpr auto raw_period      = std::chrono::milliseconds{200};
constexpr auto peak_period     = std::chrono::milliseconds{200};
constexpr auto cpu_period      = std::chrono::milliseconds{1000};
constexpr auto alarm_period    = std::chrono::milliseconds{200};

// Main update loop: runs once per second
void DQM::update_dqm(std::shared_ptr<DataStruct>& buffer) {

  // Do the buffer by buffer operations
  alarm_info(buffer);
  histogram_peaks(buffer);

  // Get the time now
  auto now = std::chrono::steady_clock::now();

  // Update baseline DQM (if baseline period has passed)
  if (now - prev_baseline >= baseline_period) {
    // Update baseline DQM
    update_dqm_baseline(buffer);
    // Store last baseline update time
    prev_baseline = now;
  }

  // Update raw data DQM (if raw data period has passed)
  if (now - prev_raw >= raw_period) {
    // Update raw data DQM 
    update_dqm_raw(buffer);
    // Store last raw data update time
    prev_raw = now;
  }
    
  // Update peak data DQM (if peak data period has passed)
  if (now - prev_peak >= peak_period) {
    // Update peak data DQM 
    update_dqm_peak(buffer);
    // Store last peak data update time
    prev_peak = now;
  }
    
  // Update cpu performance DQM (if cpu period has passed)
  if (now - prev_cpu >= cpu_period) {
    // Update cpu performance DQM 
    update_cpu_performance(buffer);
    // Store last raw data update time
    prev_cpu = now;
  }

}

void DQM::alarm_info(std::shared_ptr<DataStruct>& buffer) {

  // Check for dropped packets
  if (buffer->dropped_packet_count > 0) {
    num_dropped_packets += buffer->dropped_packet_count;
  }

  // Check for slow processes ?
  //for (size_t i = 0; i < buffer->cpu_performance.size(); ++i) {

    // Get operation name - DQM speed not updated as running
    //const auto& name = buffer->cpu_performance[i].first;
    //if (name.find("DQM") != std::string::npos)
    //  continue;

    // Get CPU speed
    //double perf = buffer->cpu_performance[i].second;

    //double input_speed = 6;
    //if (perf < input_speed * 1.05) {
    //  alarms->slow_ops = true;
    //}
  //}

}

// Fill histogram of peaks in every buffer
void DQM::histogram_peaks(std::shared_ptr<DataStruct>& buffer) {
  
  // Get updated histogram of peak heights
  // Remove the oldest buffer from the window histogram
  for (int b = 0; b < peak_nbins; b++) {
    peak_hist_window[b] -= per_buffer_hists[window_index][b];
    total_window -= per_buffer_hists[window_index][b];
    per_buffer_hists[window_index][b] = 0; // reset this buffer histogram
  }

  // Loop through EWTs and peaks
  for (const auto& ewt : buffer->EWTs) {
    for (const auto& pulse : ewt.ph) {
      // Get pulse height
      int16_t height = pulse.height;

      if (height < min_ph || height > 0) continue;

      // Find bin
      int bin = int((height - min_ph) * inv_wid);

      // Add to histograms
      ++per_buffer_hists[window_index][bin];
      ++peak_hist_window[bin];
      ++peak_hist_all[bin];
    }
  }

  // Advance circular index
  ++window_index;
  if (window_index == window_size_buffers) window_index = 0;
 
}

// Update the DQM baseline
void DQM::update_dqm_baseline(std::shared_ptr<DataStruct>& buffer){
  
  // Get the DQM baseline shm block
  auto* baseline = get<dqm_data_baseline>(DQMPageType::BASELINE);

  // Begin safe write of shm
  baseline->gen_start.fetch_add(1, std::memory_order_release);  

  // Update baseline values
  baseline->timestamp_ns = get_current_time_ns(); 
  baseline->baseline_mean_prev = baseline_mean_prev;
  baseline->baseline_sigma_prev = baseline_sigma_prev;
  baseline->baseline_mean_avg = buffer->baseline_mean_avg;
  baseline->baseline_sigma_avg = buffer->baseline_sigma_avg;
  baseline->baseline_mean_current = buffer->baseline_mean_current;
  baseline->baseline_sigma_current = buffer->baseline_sigma_current;
  baseline->baseline_fit_mean_all = buffer->baseline_all.mu0;
  baseline->baseline_fit_sigma_all = buffer->baseline_all.sigma0;
  baseline->baseline_fit_mean_wind = buffer->baseline_window.mu0;
  baseline->baseline_fit_sigma_wind = buffer->baseline_window.sigma0;
  baseline->baseline_bins = baseline_nbins; 
  baseline->noise_len = noise_len; 

  // Copy hist data 
  uint64_t* hist_dest = reinterpret_cast<uint64_t*>(baseline->baseline_data);
  std::memcpy(hist_dest,
	      buffer->hist_counts_all.data(),
	      baseline_nbins * sizeof(uint64_t));
  
  // Copy noise data
  size_t copy_len = std::min(buffer->noise_data.size(), static_cast<size_t>(noise_len));
  int16_t* noise_dest = reinterpret_cast<int16_t*>(baseline->baseline_data + (baseline_nbins * sizeof(uint64_t)));
  std::memcpy(noise_dest,
	      buffer->noise_data.data(),
	      copy_len * sizeof(int16_t));
  // Fill rest with zeroes
  if (copy_len < noise_len) std::memset(noise_dest + copy_len, 0, (noise_len - copy_len) * sizeof(int16_t));

  // Align with 8 byte for gen end write
  uintptr_t noise_end_addr = reinterpret_cast<uintptr_t>(noise_dest) + (noise_len * sizeof(int16_t)) ;
  uintptr_t aligned_gen_end_addr = (noise_end_addr + 7) & ~7;
  auto* gen_end_ptr = reinterpret_cast<std::atomic<uint64_t>*>(aligned_gen_end_addr);

  // End safe write of shm
  gen_end_ptr->store(baseline->gen_start.load(std::memory_order_acquire), std::memory_order_release);
  
}


// Update raw data display
void DQM::update_dqm_raw(std::shared_ptr<DataStruct>& buffer) {
  
  // Get raw DQM SHM block
  auto* raw = get<dqm_data_raw>(DQMPageType::RAW);
  
  // Write to shared memory safely
  raw->gen_start.fetch_add(1, std::memory_order_release);
  
  // Timestamp and rate
  raw->timestamp_ns = get_current_time_ns();  // Implement this helper

  raw->raw_len = raw_len;
  
  // Copy raw data
  int16_t* raw_dest = reinterpret_cast<int16_t*>(raw->raw_data);
  size_t copy_len = std::min(buffer->raw_len, static_cast<size_t>(raw_len));
  std::memcpy(raw_dest,
	      buffer->raw.data(),
	      copy_len * sizeof(int16_t));
  // Fill rest with zeroes
  if (copy_len < raw_len) std::memset(raw_dest + copy_len, 0, (raw_len - copy_len) * sizeof(int16_t));

  // Align with 8 byte for gen end write
  uintptr_t raw_end_addr = reinterpret_cast<uintptr_t>(raw_dest) + (raw_len * sizeof(int16_t));
  uintptr_t aligned_gen_end_addr = (raw_end_addr + 7) & ~7; 
  auto* gen_end_ptr = reinterpret_cast<std::atomic<uint64_t>*>(aligned_gen_end_addr);
  
  // Complete safe write
  gen_end_ptr->store(raw->gen_start.load(std::memory_order_acquire), std::memory_order_release);
}


// Update peak peak display
void DQM::update_dqm_peak(std::shared_ptr<DataStruct>& buffer) {
  
  // Get peak DQM SHM block
  auto* peaks = get<dqm_data_peak>(DQMPageType::PEAKS);
  
  // Write to shared memory safely
  peaks->gen_start.fetch_add(1, std::memory_order_release);
  
  // Timestamp and rate
  peaks->timestamp_ns = get_current_time_ns();  // Implement this helper

  peaks->peak_nbins = peak_nbins;
  
  // Copy data
  std::memcpy(peaks->peak_data,
	      peak_hist_all.data(),
	      peak_nbins * sizeof(uint64_t));
  
  std::memcpy(peaks->peak_data + peak_nbins,
	      peak_hist_window.data(),
	      peak_nbins * sizeof(uint64_t));

  // Find gen end write location
  auto* gen_end_ptr = reinterpret_cast<std::atomic<uint64_t>*>(peaks->peak_data + 2 * peak_nbins);
  
  // Complete safe write
  gen_end_ptr->store(peaks->gen_start.load(std::memory_order_acquire), std::memory_order_release);

  //uint64_t* gen_end_ptr = peaks->peak_data + (2 * peak_nbins);
  //uint64_t gen_end = peaks->gen_start.load(std::memory_order_acquire);
  //__atomic_store_n(gen_end_ptr, gen_end, __ATOMIC_RELEASE);
}



// Update CPU performace DQM
void DQM::update_cpu_performance(std::shared_ptr<DataStruct>& buffer) {
  
  auto daq_perf = get<dqm_data_daq>(DQMPageType::SPEEDS);

  // Write to shared memory safely
  daq_perf->gen_start.fetch_add(1, std::memory_order_release);
  
  // Timestamp
  daq_perf->timestamp_ns = get_current_time_ns();

  // Dropped packets counter
  daq_perf->num_dropped_packets = num_dropped_packets;

  // Get ADC temp
  double temp = 60.0;
  daq_perf->adc_temp = temp;

  // Number of operations
  daq_perf->num_ops = op_num;
 
  // Start print line
  std::ostringstream oss;
  oss << "Operation speed (Gbit/s): ";

  bool first_entry = true;
  for (size_t i = 0; i < buffer->cpu_performance.size(); ++i) {
    const auto& name = buffer->cpu_performance[i].first;
    double perf = buffer->cpu_performance[i].second;
    
    // Print
    if (name.find("DQM") != std::string::npos)
      continue;
    
    if (!first_entry)
      oss << " | ";
    
    oss << name << "=" << std::fixed << std::setprecision(2) << perf;
    first_entry = false;

    // Copy name up to 31 characters with null end
    std::memset(daq_perf->ops_data[i].name, 0, 32);
    std::strncpy(daq_perf->ops_data[i].name, name.c_str(), 31);

    // Copy speed
    daq_perf->ops_data[i].speed = perf;
  }
  
  logger->log(oss.str(), 1);
  
  uintptr_t data_end_addr = reinterpret_cast<uintptr_t>(daq_perf->ops_data) + (op_num * sizeof(op_entry));
  uintptr_t aligned_gen_end_addr = (data_end_addr + 7) & ~7;
  auto* gen_end_ptr = reinterpret_cast<std::atomic<uint64_t>*>(aligned_gen_end_addr);

  gen_end_ptr->store(daq_perf->gen_start.load(std::memory_order_acquire), std::memory_order_release);

}


uint64_t DQM::get_current_time_ns() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

