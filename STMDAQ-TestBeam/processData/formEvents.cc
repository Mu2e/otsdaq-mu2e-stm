///////////////////////////////////////////////////////////////////////////////////
/// This module forms event data as part of the form_events thread (main).  
///////////////////////////////////////////////////////////////////////////////////
/********************************************************************/

#include <iostream>
#include <fstream>
#include <vector>

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// Process data header
#include "STMDAQ-TestBeam/processData/formEvents.hh"

// Instance of dataVars class
dataVars dva;

// Standard constructor - shouldn't be used
formEvents::formEvents(uint64_t buffer_len) {

  // Initialise buffers
  for (int i = 0; i < CHNUM; i++){
    event_data[i] = new int16_t [buffer_len];
  }

}

// Form data into events
uint64_t formEvents::get_events(int chan, uint64_t data_len, 
				int16_t *&data, 
				queue_buffer *pushq){

  // First check packet sizes are correct
  if (data_len % MAX_PACKET_LEN != 0){
    Logger::Instance()->write(0,
                                "formEvents::get_events : Packet sizes incorrect! Channel = "
                              + std::to_string(chan)
                              + ". Data size = "
                              + std::to_string(data_len)
                              + " % MAX_PACKET_LEN = "
                              + std::to_string(data_len % MAX_PACKET_LEN));
  }
  
  // Reinitialise variables and buffers
  event_counter[chan] = 0; // event counter
  acc_event_size[chan] = 0;

  // Loop through all elements
  uint64_t count = 0;

  while (count < data_len){

    // Get the packet start location
    uint64_t packet_start = count;
    
    // Calculate how much left in packet after packet header
    uint16_t leftInPacket = MAX_PACKET_LEN - fw_pHdr_Len;
    // While leftinPacket is more than a trigger header length
    while(leftInPacket > fw_tHdr_Len){
      
      // Get header start index location
      uint64_t hdr_start_loc = packet_start + MAX_PACKET_LEN - leftInPacket;

      // Check for 0xDEADBEEF signalling last packet
      if ((uint16_t)(data[hdr_start_loc] & 0xFFFF) == BEEF
          and (uint16_t)(data[hdr_start_loc+1] & 0xFFFF) == DEAD
	  and (uint16_t)(data[hdr_start_loc+2] & 0xFFFF) == BEEF
          and (uint16_t)(data[hdr_start_loc+3] & 0xFFFF) == DEAD){
	
	// Check rest of packet
	leftInPacket = dva.check_dead_beef(data,packet_start,leftInPacket);

	// Store the last event
	store_event(chan);	

	// Push events to the queue
	pushq->push(chan,event_data[chan],acc_event_size[chan]/sizeof(int16_t));

	// Exit loop through packet                                      
	return event_counter[chan];
      
      }

      // Get event in packet length index location
      uint64_t event_len_loc = hdr_start_loc + fw_tHdr::EvInPacket;

      // Get event length
      uint16_t event_len = data[event_len_loc];      

      // Get total event length
      //uint16_t tot_event_len = data[hdr_start_loc + fw_tHdr::EvLen];      
      
      // Get event number
      uint64_t event_num = dva.get_event_number(data,hdr_start_loc);

      // Get EWT
      //uint64_t EWT = dva.get_event_number(data,hdr_start_loc);
      
      //      std::cout << "Event number = " << event_num << ", EWT = " << EWT <<  ", total event length = " << tot_event_len << ", event portion = " << event_len << std::endl;
      
      // Store the event number
      if (chan == 0) event_number = event_num;

      // If this is the first event of the data run, set first event
      if (first_event[chan]){
	
	// Store the current event number
	current_event_num[chan] = event_num;	

	// memcpy first trigger header
	memcpy(current_event_data[chan],
	       &data[hdr_start_loc],
	       fw_tHdr_Size);
	
	// Increase current event sizes
	current_event_size[chan] += fw_tHdr_Size;
	
	// Set first event boolean to false
	first_event[chan] = false;
 	
      }

      // Recalculate left in packet
      leftInPacket -= fw_tHdr_Len;      
      // If a new event
      if (event_num != current_event_num[chan]){

	// // Check we haven't skipped any EWTs
	// if (event_num != current_event_num[chan]+1){
	//   Logger::Instance()->write(2,
	// 			    "ERROR! EWT skipped "
	// 			    + std::to_string(event_num-current_event_num[chan])
	// 			    + " events from "
	// 			    + std::to_string(current_event_num[chan])
	// 			    + " to "
	// 			    + std::to_string(event_num));
	// }

	// Store the current event
	store_event(chan);
	
	// Start copying the new event...

	// Get the size of the header + data portion
	uint32_t size_to_copy = fw_tHdr_Size + event_len*sizeof(int16_t);
	
	// memcpy first trigger header and data
	memcpy(current_event_data[chan],&data[hdr_start_loc],size_to_copy);
	
	// Increase event size buffer variables
	current_event_size[chan] += size_to_copy;
		
	// Increase event length
	current_event_len[chan] += event_len;	      

 	// Store the current event number
	current_event_num[chan] = event_num;
	
      }
      
      // If event number is the same
      else{

	// Get the size of the data portion only
	uint32_t size_to_copy = event_len*sizeof(int16_t);
	
	// Copy ONLY data
	memcpy(&current_event_data[chan][fw_tHdr_Len + current_event_len[chan]],
	       &data[hdr_start_loc + fw_tHdr_Len],
	       size_to_copy);

	// Increase event size buffer variables
	current_event_size[chan] += size_to_copy;
	
        // Increase event length
	current_event_len[chan] += event_len;

      }      

      // Recalculate left in packet
      leftInPacket -= event_len;      	

    } // End while(leftInPacket > fw_tHdr_Len)
    
    // Increase count by packet length
    count += MAX_PACKET_LEN;   

  } // End i-loop over packets

  // If we've stored any complete events
  if (event_counter[chan] > 0){    
    // Push events to the queue
    pushq->push(chan,event_data[chan],acc_event_size[chan]/sizeof(int16_t));
  }  

  return event_counter[chan];

}

// Store the complete event into the buffer 
void formEvents::store_event(int chan){
  
  // Get the stored event length from the summed event size
  uint16_t event_len_check = (current_event_size[chan] 
			      - fw_tHdr_Size)/sizeof(int16_t);
  
  // Check the stored event length corresponds to the given event length
  if (event_len_check != current_event_len[chan]){
    Logger::Instance()->write(0,
			      "001: ERROR in formEvents::get_events! Chan = "
			      + std::to_string(chan)
			      +". Event number = "
			      + std::to_string(current_event_num[chan])
			      +". Stored event length = "
			      + std::to_string(event_len_check)
			      + ". Given event length = "
			      + std::to_string(current_event_len[chan])
			      + ".");
  }

  // Check the stored event length corresponds to the header total event length
  uint16_t hdr_event_len = current_event_data[chan][fw_tHdr::EvLen];  
  if (hdr_event_len != current_event_len[chan]){
    Logger::Instance()->write(0,
			      "002: ERROR in formEvents::get_events! Chan = "
			      + std::to_string(chan)
			      +". Event number = "
			      + std::to_string(current_event_num[chan])
			      +". Stored event length = "
			      + std::to_string(current_event_len[chan])
			      + ". Given event length = "
			      + std::to_string(hdr_event_len)
			      + ".");
  }
  
  // Ensure event start offset in header is zero
  current_event_data[chan][fw_tHdr::EvStart] = 0;

  // Update event in packet length in header to total event length
  current_event_data[chan][fw_tHdr::EvInPacket] = current_event_len[chan];

  // // Store the event number
  // if (chan == 0){
  
  //   // Get the DTC clock (200 MHz)                                            
  //   int16_t DTCclock_0 = (current_event_data[chan][tHdr_vars.Ch_DTCclk_0] >> 8 ) & 0x00FF;
  //   uint64_t DTCclock = dva.make_uint64_t(DTCclock_0,    
  // 					 current_event_data[chan][tHdr_vars.DTCclk_1],
  // 					 current_event_data[chan][tHdr_vars.DTCclk_2],
  // 					 current_event_data[chan][tHdr_vars.DTCclk_3]);
    
  //   // Get the DTC clock (75 MHz)                                             
  //   uint64_t ADCclock = dva.make_uint64_t(current_event_data[chan][tHdr_vars.ADCclk_0],  
  // 					 current_event_data[chan][tHdr_vars.ADCclk_1],
  // 					 current_event_data[chan][tHdr_vars.ADCclk_2],
  // 					 current_event_data[chan][tHdr_vars.ADCclk_3]);
    
  //   std::cout << "Event number = " << current_event_num[chan] << ", event length = " << current_event_len[chan] << ", DTC clock = " << DTCclock << ", ADC clock = " << ADCclock << std::endl;
  // }

  // Store whether on-spill or off-spill event
  if (chan == 0){
    event_number = current_event_num[chan];
  }
  int16_t on_spill_flag = current_event_data[chan][tHdr_vars.EM_2_DRTDC] & 0x1;
  //if (current_event_data[chan][tHdr_vars.EM_2_DRTDC] != 0) std::cout << current_event_data[chan][tHdr_vars.EM_2_DRTDC] << "  " << on_spill_flag << std::endl;
  //std::cout << current_event_data[chan][tHdr_vars.EM_2_DRTDC] << ", " << on_spill_flag << ", " << current_event_len[chan] << std::endl;  
  event_count[chan] += 1;
  if (on_spill_flag == 1 or current_event_len[chan] < 20000){
    on_spill_count[chan] += 1;
  }
  else{
    off_spill_count[chan] += 1;
  }
  
  
  // Accumulate event size
  acc_event_size[chan] += current_event_size[chan];
  
  // Get the location in the event buffer to copy to
  uint64_t copy_loc = (acc_event_size[chan] - 
		  current_event_size[chan])/sizeof(int16_t);
  // memcpy current event array to all event array
  memcpy(&event_data[chan][copy_loc],
	 current_event_data[chan],
	 current_event_size[chan]);

   // Increment event_counter
  event_counter[chan]++;

  // Reinitalise current event variables
  current_event_len[chan] = 0;  
  current_event_size[chan] = 0;
  
}

