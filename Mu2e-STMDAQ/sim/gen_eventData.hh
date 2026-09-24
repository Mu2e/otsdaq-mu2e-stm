#ifndef GEN_EVENTS_hh_
#define GEN_EVENTS_hh_

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
// Include ring buffer code
#include "Mu2e-STMDAQ/buffers/buffer_queue.hh"
// Signal handler code                                                       
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
// Firmware simulation code
//#include "Mu2e-STMDAQ/simfw/gen_data.hh"
//#include "Mu2e-STMDAQ/simdaq/sim_params.hh"


// Standard thread functions
class GenEvents {

private:

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // ADC sampling frequency (MHz)                                          
  const double fADC;
  // ADC sampling time                                                     
  const double tADC;

  // Number of events 
  double event_num;
  // Event period (us)
  const size_t event_period;
  // Event length (ADCs)
  const size_t event_len;
  // Event size (bytes)
  const size_t event_size;
  // Raw prescsle value
  const size_t prescale;

  const int adcCountsAroundPeakZS;


  // Length of the data vector
  const size_t data_len;  
  // Size of the data vector
  const size_t data_size;
  
  // Maximum number of event per struct
  const size_t max_events_per_struct = 10;

  // Buffer array size
  const size_t buffer_arr_len;

//  const int adcCountsAroundPeakZS;

public:

  // Constructor 
  GenEvents(const std::shared_ptr<STMdata>& stm_, size_t event_num_, size_t event_period_, size_t prescale_, int adcCountsAroundPeakZS_);

  // Generate events
  void gen_events(const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& queue,
		  const std::shared_ptr<SignalHandler>& signal,
		  std::atomic<bool>& finished);

  // Peak shape function 
  double calculate_peak(uint16_t time, uint16_t Amp, uint16_t tshift, uint16_t tfall, uint16_t tdecay);

  // Generate a new event header
  std::vector<int16_t> form_event_header(size_t EWT_,
					size_t EM_,
					size_t len,
					size_t event_start,
					size_t event_in_packet);  // Return the filled data vector

};

#endif 
