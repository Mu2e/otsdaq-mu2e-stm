#ifndef GEN_DATA_hh_
#define GEN_DATA_hh_

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <limits>
#include <iterator>
#include <cmath>
#include <atomic>

// STM DATA code
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Include buffer queue code
#include "Mu2e-STMDAQ/buffers/buffer_queue.hh"
// Signal handler code                                                       
#include "Mu2e-STMDAQ/utils/signal_handler.hh"

// Standard thread functions
class GenData {

private:

  // The ADC data
  std::shared_ptr<std::vector<int16_t>> data;

  // Size of the data vector
  size_t data_size = 0;
  // Length of the data vector
  size_t data_len = 0;
  
  // ADC sampling frequency (MHz)                                          
  static constexpr double fADC = 300;
  // ADC sampling time                                                     
  static constexpr double tADC = 1/fADC;

  // Number of events 
  double event_num;
  // Event period (us)
  const size_t event_period;
  // Event length (ADCs)
  const size_t event_len;
  // Event size (bytes)
  size_t event_size;

public:

  // Constructor 
  GenData(size_t event_num_, size_t event_period_, const std::string& filepath);

  // Generate packets
  void gen_packets(const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& queue,
		   const std::shared_ptr<SignalHandler>& signal,
		   std::atomic<bool>& finished);

  // Generate a new event header
  std::vector<int16_t> form_event_header(size_t EWT_,
					 size_t EM_,
					 size_t len,
					 size_t event_start,
					 size_t event_in_packet);
  
  // Return the filled data vector
  std::shared_ptr<std::vector<int16_t>> genData() const {
    return data;
  }
  
};

#endif 
