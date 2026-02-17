///////////////////////////////////////////////////////////////////////////////////
/// This module prescales the data as part of the prescale thread (main).  
///////////////////////////////////////////////////////////////////////////////////
/********************************************************************/

#include <iostream>
#include <fstream>
#include <vector>

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// Process data header
#include "STMDAQ-TestBeam/processData/prescale.hh"

// Instance of dataVars class
dataVars dvars;

// Standard constructor - shouldn't be used
prescaleData::prescaleData() {}

// Form data into events and prescale
uint64_t prescaleData::prescale_data(int chan, uint64_t data_len, 
				 int16_t *&data, 
				 queue_buffer &pushq){
  
  // Reinitialise variables and buffers
  event_counter[chan] = 0; // event counter
  acc_event_size[chan] = 0;
  //  event_data[chan] = empty_data_buffer;

  // Loop through all elements
  uint64_t count = 0;
  while (count < data_len){

    // Get the event start location
    uint64_t event_start = count;
    
    // Get the location to copy to based on the size stored so far
    uint64_t copy_loc = acc_event_size[chan]/sizeof(int16_t);

    // Get event number
    uint64_t event_num = dvars.get_event_number(data,event_start);
    
    // // Check if trigger header end is correct                                                         
    // for (uint i = 0; i < tHdr_vars.HdrEnd_len; i++){
    //   if(data[event_start+tHdr_vars.HdrEnd_start+i] != tHdr_vars.HdrEnd_data[i]){
    // 	Logger::Instance()->write(0,
    //                             "ERROR in prescale! Channel = "
    // 				  +std::to_string(chan)
    // 				  +" Event number "
    // 				  + std::to_string(event_num)
    // 				  + " has event header end value "
    // 				  + std::to_string(data[event_start+tHdr_vars.HdrEnd_start+i])
    // 				  + " instead of "
    // 				  + std::to_string(tHdr_vars.HdrEnd_data[i])
    // 				  + " for header index "
    // 				  + std::to_string(tHdr_vars.HdrEnd_start+i));
    //   }
    // }

    
    // Get event length
    uint16_t event_len = data[event_start+fw_tHdr::EvLen];      

    // Initialise data size to copy
    uint32_t size_to_copy = 0;
    
    // If this is the first event of the data run, set first event
    if (is_first_event(chan)){
      signal_first_event(chan,event_num);      
    }
    
    // If event number = prescale selection
    if (event_num == get_next_event_num(chan)){

      // Get the size of the header AND data
      size_to_copy = fw_tHdr_Size + event_len*sizeof(int16_t);

      // Find the next event to be selected by the prescale
      update_next_event_num(chan);

      // Increment event_counter
      event_counter[chan]++;
      
    }
    // If event number != prescale selection
    else{

      // Get the size of the header ONLY
      size_to_copy = fw_tHdr_Size;
      
      // Update event length in header 
      data[event_start+fw_tHdr::EvLen] = 0;

    }

    // memcpy first trigger header and data
    memcpy(&event_data[chan][copy_loc],&data[event_start],size_to_copy);
    
    // Accumulate event size
    acc_event_size[chan] += size_to_copy;    
    
    // Increase count by event header and size
    count += fw_tHdr_Len + event_len;

  } // End loop over all elements
  
  // If data to push...
  if (acc_event_size[chan] > 0){
    // Push events to queue
    pushq.push(chan,event_data[chan],acc_event_size[chan]/sizeof(int16_t));
  }

  // Store the number of events saved
  return event_counter[chan];

}
