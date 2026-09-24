////////////////////////////////////////////////////////////////////////////////// 
/// This module forms event data as part of the form_events thread (header).
///////////////////////////////////////////////////////////////////////////////////

#ifndef FORMEVENTS_hh
#define FORMEVENTS_hh

// Hex reader  
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// Logger
#include "STMDAQ-TestBeam/utils/Logger.hh"

// UDPsocket
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Circular buffer queue
#include "STMDAQ-TestBeam/utils/queue.hh"

#include<iostream>
#include<fstream>
#include<vector>

class formEvents {

private:

public:

  // Standard constructor - shouldn't be used
  formEvents(uint64_t buffer_len);
  
  // Signal the first event 
  bool first_event[CHNUM] = {true,true};

  // The event counter
  uint64_t event_counter[CHNUM] = {};

  // The current event number
  uint64_t current_event_num[CHNUM] = {};

  // The current event length (ADC data only)
  uint16_t current_event_len[CHNUM] = {};

  // The current event size (including headers)
  uint64_t current_event_size[CHNUM] = {};

  // The accumulated event size (including headers)
  uint64_t acc_event_size[CHNUM] = {};

  // The current event data
  int16_t *current_event_data[CHNUM] = {new int16_t [UDPsocket::MAX_UDP_LEN] (),
					new int16_t [UDPsocket::MAX_UDP_LEN] ()};

  // All complete event data in the loop
  // int16_t* event_data[CHNUM] = {new int16_t [queue_buffer::RING_BUFFER_LEN] (),
  // 				new int16_t [queue_buffer::RING_BUFFER_LEN] ()};
  int16_t* event_data[CHNUM];
  
  // Store the current event window tag
  uint64_t event_number = 0;
  uint64_t event_count[CHNUM] = {};
  uint64_t on_spill_count[CHNUM] = {};
  uint64_t off_spill_count[CHNUM] = {};

  // Get events from packet data
  uint64_t get_events(int chan, uint64_t data_len, 
		   int16_t* &data,
		   queue_buffer *pushq);
  
  // Store the complete event into the buffer
  void store_event(int chan);


};

#endif
