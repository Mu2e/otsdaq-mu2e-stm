////////////////////////////////////////////////////////////////////////////////// 
/// This module prescales the data as part of the prescale thread (header).
///////////////////////////////////////////////////////////////////////////////////

#ifndef PRESCALE_hh
#define PRESCALE_hh

// Hex reader  
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// Logger
#include "STMDAQ-TestBeam/utils/Logger.hh"

// UDPsocket
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Circular buffer queue
#include "STMDAQ-TestBeam/utils/queue.hh"

// Random
#include "STMDAQ-TestBeam/utils/Random.hh"

#include<iostream>
#include<fstream>
#include<vector>

class prescaleData {

private:

  // The number to prescale the data by
  uint16_t prescale_num[CHNUM] = {1,1};

  // Signal the first event to be read
  bool first_event[CHNUM] = {true,true};

  // The current prescale selection                     
  uint16_t prescale_selection[CHNUM] = {1,1};

  // The current event
  uint64_t next_event_num[CHNUM] = {};

public:

  // Standard constructor - shouldn't be used
  prescaleData();

  // The event counter
  uint64_t event_counter[CHNUM] = {};

  // The accumulated event size 
  uint64_t acc_event_size[CHNUM] = {};

  // // All complete event data in the loop
  // int16_t* event_data[CHNUM] = {new int16_t [queue_buffer::RING_BUFFER_LEN] (),
  // 				new int16_t [queue_buffer::RING_BUFFER_LEN] ()};
  // All complete event data in the loop
  int16_t* event_data[CHNUM];			

  // Return the private prescale number
  uint16_t get_prescale_num(int chan){
    return prescale_num[chan];
  }

  // Set the first event signal to filrst
  bool is_first_event(int chan){
    return first_event[chan];
  }

  // Get the next event number                                         
  uint64_t get_next_event_num(int chan){
    return next_event_num[chan];
  }
  
  // Set the first event                                                  
  void signal_first_event(int chan, uint64_t event_number){

    // Log the set prescale number
    // Logger::Instance()->write(1,
    //  			      "Prescale for " 
    // 			      + (chan == 0) ? "HPGe events = " : "LaBr events = "
    // 			      + std::to_string(prescale_num[chan]));

    // Store the first event                                              
    next_event_num[chan] = event_number;
    // Update the next event number based on the prescale              
    if (prescale_num[chan] != 1) update_next_event_num(chan);
    // Set first event boolean to false                                   
    first_event[chan] = false;
  }


  // Update the next event number based on the prescale                
  void update_next_event_num(int chan){

    // Reset the next event number to "zero"
    if (!is_first_event(chan)) next_event_num[chan] += prescale_num[chan] - prescale_selection[chan];    
    // Find the next random prescale selection (ziggurat algorithm = fast)
    if (prescale_num[chan] != 1){
      prescale_selection[chan] = Random::Instance()->IntegerValue(0,prescale_num[chan]-1,true);
    }
    else{
      prescale_selection[chan] = 1;
    }       
    // Update the next event number
    next_event_num[chan] += prescale_selection[chan];

  }

  // Prescale the data
  uint64_t prescale_data(int chan, uint64_t data_len, 
			 int16_t* &data,
			 queue_buffer &pushq);

};

#endif
