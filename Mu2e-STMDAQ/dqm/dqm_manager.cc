#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>  

// Include dqm manager code
#include "Mu2e-STMDAQ/dqm/dqm_manager.hh"
// Include dqm structs code
#include "Mu2e-STMDAQ/dqm/dqm_structs.hh"

// Constructor
DQM::DQM(const Config& cfg_,
         const std::shared_ptr<AsyncLogger>& logger_,
         const std::shared_ptr<STMdata>& stm_,
         const std::shared_ptr<SignalHandler>& signal_) :
  cfg(cfg_), logger(logger_),  stm(stm_), signal(signal_),  
  baseline_mean_prev(stm->baseline_config.prev_mean),
  baseline_sigma_prev(stm->baseline_config.prev_sigma), running_(false)
{
    
  // Maximum raw data length to store (us)
  double cfg_raw_time = cfg.getValue<double>("stm.dqm.raw_data.raw_length");
  int cfg_raw_len = int(cfg_raw_time*stm->fw_config.fADC);
  if (cfg_raw_len != raw_len){
    logger->log("Error in DQM: SHM allocation for raw data vector is  "
		+ std::to_string(raw_len/stm->fw_config.fADC)
		+ " us / " + std::to_string(raw_len) + " ADCs, but is set in cfg to "
		+ std::to_string(cfg_raw_time)
                + " us / " + std::to_string(cfg_raw_len) + " ADCs.",0);
  }
  else{    
    logger->log("DQM: Allocating shm for " + std::to_string(raw_len/stm->fw_config.fADC)
		+ " us / " + std::to_string(raw_len) + " ADC values of raw data.",1);
  }
  
  // Maximum noise length to store (us)
  double cfg_noise_time = cfg.getValue<double>("stm.dqm.baseline.noise_length");
  int cfg_noise_len = int(cfg_noise_time*stm->fw_config.fADC);
  if (cfg_noise_len != noise_len){
    logger->log("Error in DQM: SHM allocation for noise vector is  "
		+ std::to_string(noise_len/stm->fw_config.fADC)
		+ " us / " + std::to_string(noise_len) + " ADCs, but is set in cfg to "
		+ std::to_string(cfg_noise_time)
                + " us / " + std::to_string(cfg_noise_len) + " ADCs.",0);
  }
  else{    
    logger->log("DQM: Allocating shm for " + std::to_string(noise_len/stm->fw_config.fADC)
		+ " us / " + std::to_string(noise_len) + " ADC values of noise data.",1);
  }
    
  // Register shared memory blocks
  registerBlock<dqm_data_raw<raw_len>>(DQMPageType::RAW, "/dqm_raw_data");
  registerBlock<dqm_data_baseline<noise_len>>(DQMPageType::BASELINE, "/dqm_adc_baseline");
  registerBlock<dqm_data_physics>(DQMPageType::PHYSICS, "/dqm_adc_physics");

  // Register operations for OperationManager
  register_operation("update_dqm", [this](auto& b){ update_dqm(b); });
  
}

auto t0 = std::chrono::steady_clock::now();
auto prev_baseline = t0;
auto prev_raw = t0;
auto prev_cpu = t0;

constexpr auto baseline_period = std::chrono::milliseconds{200};
constexpr auto raw_period      = std::chrono::milliseconds{200};
constexpr auto cpu_period      = std::chrono::milliseconds{1000};

// Main update loop: runs once per second
void DQM::update_dqm(std::shared_ptr<DataStruct>& buffer) {

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

  // Update cpu performace DQM (if cpu period has passed)
  if (now - prev_cpu >= cpu_period) {
    // Update cpu performance DQM 
    update_cpu_performance(buffer);
    // Store last raw data update time
    prev_cpu = now;
  }

  
}

// Update the DQM baseline
void DQM::update_dqm_baseline(std::shared_ptr<DataStruct>& buffer){
  
  // Get the DQM baseline shm block
  auto* baseline = get<dqm_data_baseline<noise_len>>(DQMPageType::BASELINE);

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

  // Copy noise data
  size_t copy_len = std::min(buffer->noise_data.size(), static_cast<size_t>(noise_len));
  std::memcpy(baseline->noise_data,
	      buffer->noise_data.data(),
	      copy_len * sizeof(int16_t));

  // End safe write of shm
  baseline->gen_end.store(baseline->gen_start.load(std::memory_order_acquire), std::memory_order_release);
    
}

// Update raw data display
void DQM::update_dqm_raw(std::shared_ptr<DataStruct>& buffer) {
  
  // Get raw DQM SHM block
  auto* raw = get<dqm_data_raw<raw_len>>(DQMPageType::RAW);
  
  // Write to shared memory safely
  raw->gen_start.fetch_add(1, std::memory_order_release);
  
  // Timestamp and rate
  raw->timestamp_ns = get_current_time_ns();  // Implement this helper
  
  // Copy noise data
  size_t copy_len = std::min(buffer->raw_len, static_cast<size_t>(raw_len));
  std::memcpy(raw->samples,
	      buffer->raw.data(),
	      copy_len * sizeof(int16_t));
  
  
    // Complete safe write
  raw->gen_end.store(raw->gen_start.load(std::memory_order_acquire), std::memory_order_release);
}

// Update CPU performace DQM
void DQM::update_cpu_performance(std::shared_ptr<DataStruct>& buffer) {
  
  std::ostringstream oss;
  oss << "Operation speed (Gbit/s): ";

  // Print speeds
  bool first_entry = true;
  for (size_t i = 0; i < buffer->cpu_performance.size(); ++i) {
    const auto& name = buffer->cpu_performance[i].first;
    double perf = buffer->cpu_performance[i].second;
    
    if (name.find("DQM") != std::string::npos)
      continue;
    
    if (!first_entry)
      oss << " | ";
    
    oss << name << "=" << std::fixed << std::setprecision(2) << perf;
    first_entry = false;
  }
  
  logger->log(oss.str(), 1);
  
}


// Get the current time in ns
uint64_t DQM::get_current_time_ns() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

