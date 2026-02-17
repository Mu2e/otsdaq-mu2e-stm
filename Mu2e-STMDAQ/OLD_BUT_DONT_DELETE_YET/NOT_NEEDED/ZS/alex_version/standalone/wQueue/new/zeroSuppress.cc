////////////////////////////////////////////////////////////////////////////////
/// This module performs the online zero-suppresion (main).       
////////////////////////////////////////////////////////////////////////////////
/********************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <cmath>
#include <math.h>

// Zero-supress data header
#include "zeroSuppress.hh"

#include "genData.hh"


// Standard constructor - shouldn't be used
daqZS::daqZS() {

  // Check that the max data stored from the previous event
  // isn't larger than an on-spill event
  std::cout << "Max data stored in previous event is " << prev_data_max << " ADC values = " << (double)prev_data_max*tadc << " us" << std::endl;
  if ((double)prev_data_max*tadc > 1.675){
    std::cout << "ERROR in ZS.hh! Maximum data stored from previous event is larger than an on-spill event" << std::endl;
    exit(0);
  }
    
}

// Pulse finding thread function
void daqZS::pulse_thread(int chan, queue_zs &pullq, queue_zs &pushq, bool *timeout){

  std::cout << "In pulse finding thread" << std::endl;

  // The return calue
  int retval = 0;

  // The total data count                                                 
  uint64_t data_count = 0;
  
  // The total pulse count                                                 
  uint64_t pulse_count = 0;

  // The total pulse count                                                 
  //  double pulse_rate = 0;
  
  // Data buffer                                                           
  data_struct buffer;

  // While still data to pull                                              
  while(!*timeout){
    // Pull data from queue                                                
    retval = pullq.pull(timeout,chan,buffer);
    if (retval > 0){
      // Find pulses
      pulse_count += find_peaks(chan,buffer);
      // Update rate
      //      pulse_rate = (double)buffer.pulse_index.size() / ((double)buffer.data_len * (double)tadc) * 1e3;
      //      std::cout << "Pulse rate = " << pulse_rate << " kHz" << std::endl;
      // Push data to queue
      pushq.push(chan,buffer);
    }
  }
  
  //  std::cout << "Found " << pulse_count << " pulses" << std::endl;

}

// Zero suppresseing thread function
void daqZS::suppress_thread(int chan, queue_zs &pullq, queue_zs &pushq, bool *timeout){

  std::cout << "In suppress data thread" << std::endl;

  // The return calue
  int retval = 0;

  // The total pulse count                                                 
  uint64_t data_count = 0;

  // Data buffer                                                           
  data_struct buffer;

  // While still data to pull                                              
  while(!*timeout){
    // Pull data from queue                                                
    retval = pullq.pull(timeout,chan,buffer);
    if (retval > 0){
      // Find pulses
      data_count += suppress_data(chan,buffer);
      // Push data to queue
      pushq.push(chan,buffer);
    }
  }
  
}


// ZS algorithm
data_struct daqZS::split_headers(int chan, uint64_t n, int16_t* &data){
  
  // The header int16_t counter
  uint64_t header_count = 0;

  // Get the number to average
  int n_average = get_n_average();
  
  // Create data struct
  data_struct event_data;
  event_data.count = 0;
  event_data.data_len = 0;
  event_data.header_data = new int16_t [n] ();
  event_data.adc_data = new int16_t [n + prev_num[chan]] ();

  // If not first pass, copy in the data from the previous call
  if (!first_pass[chan]){
    // Copy in the header
    memcpy(event_data.header_data,prev_header[chan],fw_tHdr_Size);
    // Account for header in count
    header_count += fw_tHdr_Len;
    // Store the start of the event in the adc data
    event_data.start_index.push_back(0);
    // Copy in the data
    memcpy(event_data.adc_data,
	   prev_data_end[chan],prev_num[chan]*sizeof(int16_t));
    // Increase the event count
    event_data.count = 1;
    // Increase the data length
    event_data.data_len = prev_num[chan];
  }

  // The start index of each event
  int64_t i = 0;

  // Header start index location                                    
  uint64_t hdr_start_loc = 0;
  // event length                                                   
  uint16_t event_len = 0;
    
  // Loop over all ADC values 
  while(i<n){
    
    // Get header start index location                                    
    hdr_start_loc = i;
    
    // Get event length index location                                    
    uint64_t event_len_loc = hdr_start_loc + fw_tHdr::EvLen;
    // Get event length                                                   
    event_len = data[event_len_loc];           
    
    // Memcpy event header
    memcpy(&event_data.header_data[header_count],
	   &data[hdr_start_loc],fw_tHdr_Len*sizeof(int16_t));

    // Skip past event header
    i += fw_tHdr_Len;      

    // Account for header
    header_count += fw_tHdr_Len;

    // Memcpy adc data
    memcpy(&event_data.adc_data[event_data.data_len],
	   &data[i],event_len*sizeof(int16_t));
     
    // Update i to be same as j
    i += event_len;

    // Store the start of the event in the adc data
    event_data.start_index.push_back(event_data.data_len);

    // Account for header
    event_data.data_len += event_len;
   
    // Increment the event counter
    event_data.count++;                

  } // End loop over ALL event data

  // Calculate the amount of overflow data to find the avg gradient
  //  std::cout << window + (event_data.data_len - prev_num[chan] - window) % n_average << " = " << window << " + (" << event_data.data_len << " - " << prev_num[chan] <<" - " << window << ") % " << n_average << std::endl; 
  prev_num[chan] = window + (event_data.data_len - prev_num[chan] - window) % n_average;

  //  std::cout << "prev_num = " << prev_num[chan] << std::endl;
  // Store the last header for the next call
  memcpy(prev_header[chan],&data[hdr_start_loc],fw_tHdr_Size);
  // Change the event start offset
  prev_header[chan][fw_tHdr::EvStart] = event_len - prev_num[chan];
  // Change the event length
  prev_header[chan][fw_tHdr::EvLen] = prev_num[chan];  
  // Store the overflow data from last event
  memcpy(prev_data_end[chan],&data[n-prev_num[chan]],
	 prev_num[chan]*sizeof(int16_t));        

  // Make sure first pass is false
  first_pass[chan] = false;

  return event_data;

}


// ZS algorithm
uint64_t daqZS::find_peaks(int chan, data_struct &datas){
  
  // The adc length
  uint64_t n = datas.data_len;

  // Define a pointer to the adc data
  int16_t *data = datas.adc_data;

  // The data index
  int64_t j = 0;

  // Counter for number of peaks found
  uint64_t peak_count = 0;

  // Get the number to average
  int n_average = get_n_average();

  // Loop over all ADC values 
  while(j<(n-window)){
    
    // Initialise average gradient / time variables
    double av_gradient = 0;
    double av_ADCtime = 0;    
    
    // Loop over all average entries                                     
    for(int k = 0; k < n_average; k++){
      
      // Gradient values
      int16_t grad_low = 0;
      int16_t grad_high = 0;
      
      // Calculate gradient
      grad_low = data[j];
      grad_high = data[j+window];
      
      // Sum average "time" in counts
      av_ADCtime += j;
      
      // Increment j
      j++;
      
      // Calculate gradient
      int16_t gradient = grad_high - grad_low;
      // Sum average gradient
      av_gradient += gradient;
      
    }
    
    // Calculate average gradient                                        
    double avg_gradient = av_gradient/n_average;
    // Calculate average trigger time                                    
    uint64_t peak_loc = av_ADCtime/n_average;

    // If the averaged gradient is beyond the threshold
    if(avg_gradient > threshold){
      // No peak found
      peak_found[chan]=false;
      continue;
    }
    
    // Skip the rest indexes of the peak after                           
    // the trigger that has already been stored                          
    if(avg_gradient < threshold && peak_found[chan]==true){
      continue;
    }
    
    // If within the threshold and no peak previously found...
    if(avg_gradient < threshold && peak_found[chan]==false){
      
      // Peak found! 
      peak_found[chan] = true;
      
      // Increment the peak countera
      peak_count++;
      
      // Store peak location in adc data only
      datas.pulse_index.push_back(peak_loc);
      
    } // End if peak found
    
  } // End loop over ALL event data

  // Remove the upper window from the data length
  datas.data_len -= window;
  
  return peak_count;

}

// Suppress data
uint64_t daqZS::suppress_data(int chan, data_struct &datas){

  // The adc length
  uint64_t n = datas.data_len;

  // Define a pointer to the adc data
  int16_t *data = datas.adc_data;

  // The number of peaks in the data
  uint64_t peak_num = datas.pulse_index.size();  

  // Store suppressed data
  int16_t* suppressed_data = new int16_t[n];

  // The suppressed data length
  uint64_t sup_data_len = 0;  

  // First, store any data needed from peak found in previous call
  if (left_to_copy[chan] > 0){
    // Memcpy the data
    memcpy(&suppressed_data[sup_data_len],&data[0],
	   left_to_copy[chan]*sizeof(int16_t));       

    // Increase suppressed data size
    sup_data_len += left_to_copy[chan];

    // Set left to copy to zero
    left_to_copy[chan] = 0;
 
  }   

  // Loop over number of peaks in data struct
  for (int i = 0; i < peak_num; i++){
    
    // Get the peak location
    uint64_t peak_loc = datas.pulse_index[i];

    //    std::cout << tot_peak_count << " " << prev_peak_loc[chan] << " " << peak_loc << " " << peak_loc - prev_peak_loc[chan] << std::endl;
    
    // The index location to copy from...
    int64_t loc_copy_from = 0;
    
    // The data length to copy
    uint64_t data_len = 0;

    // If first peak
    if(first_peak[chan]){
      
      // Check whether there is 1 us of data before the first peak...
      // If not, the number of ADC values is: peak_loc
      if(peak_loc - before_peak_max < 0){	  
	
	// Copy from first element in ADC array
	loc_copy_from = 0;
	
	// Store the data from data0] --> peak_loc+after_peak_max
	data_len = peak_loc + after_peak_max;
	
      }
      
      // Else if 1 us or more exists before the start of the first peak
      else{
	
	// Copy from 1 us before peak
	loc_copy_from = peak_loc-before_peak_max;	 
	
	// Store the whole peak
	data_len = total_peak_max;
	
      }
      
      // Set first peak to false
      first_peak[chan] = false;
      
    } // End if first peak
    
      // Else if not first peak
    else{
      
      // Find the number of values between this peak
      // and the previous one
      int peak_sep = peak_loc - prev_peak_loc[chan];
      
      // If the peak separation is within the peak window
      if(peak_sep < total_peak_max){      

	// Copy from end of previous peak
	loc_copy_from = prev_peak_loc[chan] + after_peak_max;
	
	// Store just the new peak separation
	data_len = peak_sep;
	
      }
      // Else if this new peak is outside the 3us window of the previous peak
      else{
	
	// Copy from 1 us before peak
	loc_copy_from = peak_loc - before_peak_max;

	// Store the total peak
	data_len = total_peak_max;

	// If the peak starts in the previous call
	if (loc_copy_from < 0){
	  // Get the amount to copy
	  int to_copy = std::abs(loc_copy_from);
	  // Memcpy the data
	  memcpy(&suppressed_data[sup_data_len],&prev_peak_overlap[chan][before_peak_max - to_copy],
		 to_copy*sizeof(int16_t));
	  // Increase the suppressed data length
	  sup_data_len += to_copy;
	  // Set the new location to copy to the start of the data call
	  loc_copy_from = 0;
	  // Update the amount left to store
	  data_len -= to_copy;
	}

      } // End if (peak_sep < total_peak_max)

    } // End if/not first peak

    // Store peak time to compare against next peak
    prev_peak_loc[chan] = peak_loc;	    
    
    // If the data to copy goes beyond the data length
    if(loc_copy_from + data_len > n){
      
      // Store the amount left to copy from the previous call
      left_to_copy[chan] = data_len - (n - loc_copy_from);
      
      // Store only until end of data
      data_len -= left_to_copy[chan];
      
    }
    else{
      
      // No data left to copy in next call
      left_to_copy[chan] = 0;
      
    }
    
    // If the last peak in this call
    if (i == peak_num - 1){      
      
      // Store the distance from the last peak to the end
      prev_peak_loc[chan] -= n;
    
    } // End if last peak

    // If the location to copy from is in the next call, skip
    if (loc_copy_from > n){
      continue;
    }	

    // Memcpy the data
    memcpy(&suppressed_data[sup_data_len],&data[loc_copy_from],
	   data_len*sizeof(int16_t));

    // Increase suppressed data size
    sup_data_len += data_len;

    //    std::cout << i << " " << data_len << " " << sup_data_len << std::endl;
    
    // Increment total peak counter
    tot_peak_count++;
    
  } // End loop over peaks

  // Overwrite data
  datas.adc_data = suppressed_data;

  // Store new data length
  datas.data_len = sup_data_len;

  // Store the end of this call
  memcpy(prev_peak_overlap[chan],&data[n-before_peak_max],
	 before_peak_max*sizeof(int16_t));        

  
  return sup_data_len;

}



// // ZS algorithm
// std::tuple<int16_t*,uint64_t,uint64_t,
// 	   uint64_t,int64_t,int64_t> daqZS::do_ZS(int chan,
// 						  uint64_t n, int16_t* &data){
  
//   // Store suppressed data
//   int16_t* suppressed_data = new int16_t[n-window];

//   // The start index of each event
//   uint64_t i = 0;

//   // Counter for number of peaks found
//   uint64_t peak_count = 0;

//   // The suppressed data length
//   uint64_t sup_data_len = 0;  

//   // Get the number to average
//   int n_average = get_n_average();

//   // The counter of data points left from previous event
//   int prev_count = 0;

//   // ADC counter
//   uint64_t adc_count = 0;
  
//   // Baseline ADC counter
//   int64_t baseline_count = 0;

//   // Baseline mean
//   int64_t baseline_mean = 0;

//   // Baseline RMS
//   int64_t baseline_rms = 0;
  
//   // Loop over all ADC values 
//   while(i<(n-window)){
    
//     // Get header start index location                                    
//     uint64_t hdr_start_loc = i;
    
//     // Get event length index location                                    
//     uint64_t event_len_loc = hdr_start_loc + fw_tHdr::EvLen;
//     // Get event length                                                   
//     uint16_t event_len = data[event_len_loc];           

//     // Skip past event header
//     i += fw_tHdr_Len;      
     
//     // Event index counter
//     uint64_t j = i;

//     // Find the loop end index
//     uint64_t event_end = i + event_len;

//     // Loop through event 
//     while(j < event_end){
   	
//       // If reaching the end of the data window
//       if((j + n_average) > (n - window)){
//        	// Get how many points are left
// 	prev_num[chan] = event_end-j;
//        	// Store the end of this data event
// 	memcpy(prev_data_end[chan],&data[event_end-prev_data_max],
// 	       prev_data_max*sizeof(int16_t));
// 	// Break event loop
// 	break;
//       }
      
//       // Initialise average gradient / time variables
//       double av_gradient = 0;
//       double av_ADCtime = 0;    
      
//       // Loop over all average entries                                     
//       for(int k = 0; k < n_average; k++){
	
// 	// Gradient values
// 	int16_t grad_low = 0;
// 	int16_t grad_high = 0;
	
// 	// If there is data from the previous call
// 	if (prev_count < prev_num[chan]){

// 	  // Get the lower gradient index
// 	  uint64_t low_index = prev_data_max - prev_num[chan];
// 	  // Get the low data from the previous window
// 	  grad_low = prev_data_end[chan][low_index + prev_count];
// 	  // If the window max is inside the previous data
// 	  if (prev_count + window < prev_num[chan]){
// 	    // Get the high data from the previous window
// 	    grad_high = prev_data_end[chan][low_index + prev_count + window];

// 	  }
// 	  // If the window max is in he new data
// 	  else{
// 	    // Get the high data from the previous window
// 	    grad_high = data[i + prev_count + window - prev_num[chan]];
// 	  }
	  
// 	  // Sum average "time" in counts
// 	  av_ADCtime += (j + prev_count - prev_num[chan]);
	  
// 	  // Increment prev_count
// 	  prev_count++;

// 	}
// 	// Else if no data from previous call
// 	else{
	  
// 	  // Calculate gradient
// 	  grad_low = data[j];
// 	  if (j + window >= event_end){
// 	    grad_high = data[j+window+fw_tHdr_Len];
// 	  }
// 	  else{
// 	    grad_high = data[j+window];
// 	  }

// 	  // Sum average "time" in counts
// 	  av_ADCtime += j;
	  	 	  
// 	  // Increment j
// 	  j++;
	  
// 	}

// 	// Incremdent distance since previous peak
// 	peak_separation[chan]++;
// 	// Increment adc_count
// 	adc_count++;	 

// 	// If reached average max and out of previous call region
// 	if (prev_count >= prev_num[chan] and k == n_average-1){
// 	  // Reset previous call variables
// 	  prev_count = 0;
// 	  prev_num[chan] = 0;
// 	}
	
// 	// If there is data after the last peak found left to store
// 	if (left_to_store[chan] > 0){
// 	  // Store data
// 	  suppressed_data[sup_data_len] = grad_low;
// 	  // Increment sup_data_len
// 	  sup_data_len++;
// 	  // Decrement left_to_store
// 	  left_to_store[chan]--;
// 	}
// 	// Else if in baseline (no-peak) region
// 	else{
// 	  // If the distance since the last peak is longer than the return to baseline
// 	  if (peak_separation[chan] > hpge_decay_adc){
// 	    // Calculate baseline mean
// 	    baseline_mean += grad_low;
// 	    // Calculate baseline mean
// 	    baseline_rms += grad_low*grad_low;
// 	    // Increment baseline counter
// 	    baseline_count++;
// 	  }
// 	}	
	
// 	// Calculate gradient
// 	int16_t gradient = grad_high - grad_low;
// 	// Sum average gradient
// 	av_gradient += gradient;
	
//       }

//       // Calculate average gradient                                        
//       double avg_gradient = av_gradient/n_average;
//       // Calculate average trigger time                                    
//       uint64_t peak_loc = av_ADCtime/n_average;
      
//       // If the averaged gradient is beyond the threshold
//       if(avg_gradient > threshold){
// 	// No peak found
// 	peak_found[chan]=false;
// 	continue;
//       }
      
//       // Skip the rest indexes of the peak after                           
//       // the trigger that has already been stored                          
//       if(avg_gradient < threshold && peak_found[chan]==true){
// 	continue;
//       }
      
//       // If within the threshold and no peak previously found...
//       if(avg_gradient < threshold && peak_found[chan]==false){
	
// 	// Peak found! 
// 	peak_found[chan] = true;

// 	// Increment the peak countera
// 	peak_count++;

// 	// The index location to copy from...
// 	int64_t loc_copy_from = 0;

//  	// The data length to copy before the peak
// 	uint64_t data_len_before = 0;

// 	// If first peak
// 	if(first_peak[chan]){

// 	  // Check whether there is 1 us of data before the first peak...
// 	  // If not, the number of ADC values is: peak_loc
// 	  if(before_peak_max > peak_separation[chan]){
	    
// 	    // Copy from first element after event header in ADC array
// 	    loc_copy_from = fw_tHdr_Len;
	    
// 	    // Store data len to copy before peak
// 	    data_len_before = peak_loc;
	    
// 	  }       
// 	  // Else if 1 us or more exists before the start of the first peak
// 	  else{
	    
// 	    // Copy from 1 us before peak
// 	    loc_copy_from = peak_loc-before_peak_max;
	    
// 	    // Store data len to copy before peak
// 	    data_len_before = before_peak_max;
	    
// 	  }
	  
// 	  // Set first peak to false
// 	  first_peak[chan] = false;
	  
// 	} // End if first peak
	
// 	// Else if not first peak
// 	else{
	  
// 	  // If the peak separation is within the peak window
// 	  if(peak_separation[chan] < total_peak_max){

// 	    // Store data len to copy before peak
// 	    if (peak_separation[chan] > after_peak_max){
// 	      // If overlap is before peak_loc
// 	      data_len_before = peak_separation[chan] - after_peak_max;
// 	    }
// 	    // Else overlap is after peak_loc
// 	    else{
// 	      data_len_before = 0;
// 	    }

// 	    // Copy from end of previous peak
// 	    loc_copy_from = peak_loc - data_len_before;

// 	    // Subtract the points already stored from averaging
// 	    if (left_to_store[chan] > 0){
// 	      sup_data_len -= (j - peak_loc + prev_count - prev_num[chan]);
// 	    }
	    
// 	  }
	  
// 	  // Else if this new peak is outside the 3us window of the previous peak
// 	  else{

// 	    // Copy from 1 us before peak
// 	    loc_copy_from = peak_loc-before_peak_max;
	    
// 	    // Store data len to copy before peak
// 	    data_len_before = before_peak_max;
	    
// 	  } // End if (peak_separation[chan] < total_peak_max)
	  
// 	} // End if first peak	
	
// 	// The amount left to store is always after_after_peak_max
// 	left_to_store[chan] = after_peak_max;
	
// 	// If the peak has been found in the previous data
// 	if (peak_loc < i){
// 	  // Get the location to copy from in the previous data
// 	  uint32_t peak_loc_in_prev = prev_data_max - (i - peak_loc);
// 	  loc_copy_from = peak_loc_in_prev - data_len_before;
// 	  // Copy all the data before the peak from the previous data
// 	  memcpy(&suppressed_data[sup_data_len],
// 		 &prev_data_end[chan][loc_copy_from],
// 		 data_len_before*sizeof(int16_t));
// 	  // Increase sup_data_len
// 	  sup_data_len += data_len_before;
// 	  // Get number of points already averaged after peak_loc
// 	  int navg_after_peak_loc = i - peak_loc + prev_count - prev_num[chan];
// 	  // Loop over number of points averaged after peak
// 	  for (int k = 0; k < navg_after_peak_loc; k++){
// 	    // If still inside prev data
// 	    if (peak_loc_in_prev + k < prev_data_max){
// 	      // Store data from previous event
// 	      suppressed_data[sup_data_len] = prev_data_end[chan][peak_loc_in_prev+k];
// 	    }
// 	    // Else if drifted into new event	     
// 	    else{
// 	      // Store data from new event
// 	      suppressed_data[sup_data_len] = data[peak_loc_in_prev - prev_data_max+k];
// 	    }
// 	    // Increment sup_data_len
// 	    sup_data_len++;
// 	    // Decrement left_to_store
// 	    left_to_store[chan]--;
// 	  } // End k-loop	  
// 	}	
// 	// Else if the peak is in the current event
// 	else{
// 	  // If thedata before the peak starts in the previous event
// 	  if (loc_copy_from < i){
// 	    int copy_prev_len = fabs(i - loc_copy_from);
// 	    int remainder = data_len_before - copy_prev_len;
// 	    int16_t* copy_addr = 0;
// 	    if (i == fw_tHdr_Len){	      
// 	      // Get the location to copy from in the previous data
// 	      loc_copy_from = prev_data_max - copy_prev_len;
// 	      copy_addr = &prev_data_end[chan][loc_copy_from];
// 	    }
// 	    else{
// 	      loc_copy_from = hdr_start_loc - copy_prev_len;
// 	      copy_addr = &data[loc_copy_from];
// 	    }	     	    
// 	    // Copy all the data before the peak up until start of this event
// 	    memcpy(&suppressed_data[sup_data_len],
// 		   copy_addr,
// 		   copy_prev_len*sizeof(int16_t));
// 	    // Increase sup_data_len
// 	    sup_data_len += copy_prev_len;
// 	    // Now copy remainder from this event
// 	    memcpy(&suppressed_data[sup_data_len],
// 		   &data[i],
// 		   remainder*sizeof(int16_t));
// 	    // Increase sup_data_len
// 	    sup_data_len += remainder;
// 	  } 
// 	  // Else if the data to copy before starts in this event
// 	  else{
// 	    // Copy all data before the peak from this event
// 	    memcpy(&suppressed_data[sup_data_len],
// 		   &data[loc_copy_from],
// 		   data_len_before*sizeof(int16_t));
// 	    // Increase sup_data_len
// 	    sup_data_len += data_len_before;	    
// 	  } // End if loc_copy_from <i	  
// 	  // Account for distance between average peak location and j
// 	  for (int k = 0; k < (j-peak_loc); k++){
// 	    // Store data
// 	    suppressed_data[sup_data_len] = data[peak_loc+k];
// 	    // Increment sup_data_len
// 	    sup_data_len++;
// 	    // Decrement left_to_store
// 	    left_to_store[chan]--;
// 	  } // End k-loop	  
// 	} // End if peak is found in previous data
	
// 	// Reset peak_separation to zero
// 	peak_separation[chan] = 0;

//       } // End if peak found
          
//     } // End loop over single event data

//     // Update i to be same as j
//     i = event_end;
   
//   } // End loop over ALL event data

//   // Ensure not dividing by zero
//   if (baseline_count != 0) {
//     // Calculate basline mean
//     baseline_mean /= baseline_count;
//     // Calculate basline mean
//     baseline_rms = sqrt(baseline_rms/baseline_count - baseline_mean*baseline_mean);
//   }
//   // Calculate rate
//   uint64_t rate = peak_count / (adc_count * tadc * 1e-6);

//   return {suppressed_data,sup_data_len,peak_count,rate,baseline_mean,baseline_rms};
  
// }

std::tuple<int16_t*,uint64_t,uint64_t> daqZS::ZS_safe(int chan,
						      uint64_t n,
						      int16_t* &ADC){
  // Store suppressed data
  int16_t* suppressed_data = new int16_t[n-window];

  // Calculate the average of the gradient each
  // n_average ADC values to avoid fluctuations
  uint64_t j = 0;

  // Boolean if peak found                                               
  bool found_peak = false;

  // Counter for number of peaks found
  uint64_t peak_count = 0;

  // The suppressed data length
  uint64_t sup_data_len = 0;  

  // Boolean to signal the first peak
  bool first_peak = true;

  // The time of the previous peak
  uint64_t prev_peak_loc = 0;

  // Get the number to average
  int n_average = get_n_average();

  // Loop over all ADC values                                                   
  while(j<(n-window)){
    
    // Each point of the gradient and ADCtime
    // averaged with the (n_average-1) following
    // points, each point is the mean of n_average points of the gradient       
    if((j + n_average) > (n - window)){
      n_average= (n-window)-j;
    }

    // Initialise average gradient / time variables
    double av_gradient = 0;
    double av_ADCtime = 0;

    // Loop over all average gradients
    for(int k = 0; k < n_average; k++){
      // Calculate gradient
      int16_t grad_high = ADC[j+k+window];
      int16_t grad_low = ADC[j+k];
      int16_t gradient = grad_high - grad_low;
      // Sum average gradient
      av_gradient += gradient;
      // Sum average time
      double time = j+k;
      av_ADCtime += time;
    }
    
    // Calculate average gradient                                        
    double avg_gradient = av_gradient/n_average;
    // Calculate average trigger time                                    
    uint64_t peak_loc = av_ADCtime/n_average;
    
    // Increase loop counter                                             
    j += n_average;

    // If the averaged gradient is beyond the threshold
    if(avg_gradient > threshold){
      // No peak found
      found_peak=false;
      continue;
    }
    
    // Skip the rest indexes of the peak after                           
    // the trigger that has already been stored                          
    if(avg_gradient < threshold && found_peak==true){
      continue;
    }
    
    // If within the threshold and no peak previously found...
    if(avg_gradient < threshold && found_peak==false){

      // Peak found! 
      found_peak=true;
      
      // Increment the peak counter
      peak_count++;
      
      // The index location to copy from...
      uint64_t loc_copy_from = 0;
      // The data length to copy
      uint64_t data_len = 0;
      
      // If first peak
      if(first_peak){
	
	// Check whether there is 1 us of data before the first peak...
	// If not, the number of ADC values is: peak_loc
	if(peak_loc - before_peak_max < 0){	  
	  
	  // Copy from first element in ADC array
	  loc_copy_from = 0;
	  
	  // Store the data from data0] --> peak_loc+after_peak_max
	  data_len = peak_loc + after_peak_max;
	  
	}
	
	// Else if 1 us or more exists before the start of the first peak
	else{
	  
	  // Copy from 1 us before peak
	  loc_copy_from = peak_loc-before_peak_max;	 
	  
	  // Store the whole peak
	  data_len = total_peak_max;
 	}
	
	// Set first peak to false
	first_peak = false;
	
      } // End if first peak
      
      // Else if not first peak
      else{
	
	// Find the number of values between this peak
	// and the previous one
	int peak_sep = peak_loc - prev_peak_loc;

	// If the peak separation is within the peak window
	if(peak_sep < total_peak_max){
	  
	  // Copy from end of previous peak
	  loc_copy_from = prev_peak_loc + after_peak_max;
	  
	  // Store just the new peak separation
	  data_len = peak_sep;
	  
	}
	// Else if this new peak is outside the 3us window of the previous peak
	else{
	  
	  // Copy from 1 us before peak
	  loc_copy_from = peak_loc-before_peak_max;
	  
	  // Store the total peak
	  data_len = total_peak_max;
	  
	} // End if (peak_sep < total_peak_max)
	
	// If the data to copy goes beyond the data length
	if(loc_copy_from + data_len > n){
	 
	  // Store only until end of data
	  data_len = n - loc_copy_from;
	  
	}
	
      } // End if first peak
      
      // Memcpy the data
      memcpy(&suppressed_data[sup_data_len],&ADC[loc_copy_from],
	     data_len*sizeof(int16_t));
      
      // Increase suppressed data size
      sup_data_len += data_len;

      // Store peak time to compare against next peak
      prev_peak_loc = peak_loc;
      
    } // End if peak found
    
  } // End loop over ADC values

  return {suppressed_data,sup_data_len,peak_count};
  
}
