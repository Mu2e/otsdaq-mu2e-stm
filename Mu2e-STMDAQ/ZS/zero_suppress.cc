// Zero suppress code
#include "zero_suppress.hh"

// Constructor
ZeroSuppress::ZeroSuppress(){
  
  // ADC sampling frequency (MHz)
  fADC = cfg.getValue<double>("stm.fw.fADC");
  // ADC sampling time
  tADC = 1.0/(fADC);
  logger->log("ZeroSuppress: ADC sampling frequency = " + std::to_string(fADC)
	      + " MHz (" + std::to_string(tADC) + " us per ADC value).",1);

  // Time to store before peak (us)
  before_peak_time = cfg.getValue<double>("stm.zs.hpge.store_before_peak");
  // Number of ADC values to store before peak
  before_peak = before_peak_time*fADC;
  logger->log("ZeroSuppress: Storing " + std::to_string(before_peak_time)
	      + " us / " + std::to_string(before_peak) +
	      " ADC values / " + std::to_string(before_peak*sizeof(int16_t)) +
	      " bytes before peak.",1);   

  // Time to store after peak (us)
  after_peak_time = cfg.getValue<double>("stm.zs.hpge.store_after_peak");
  // Number of ADC values to store after peak
  after_peak = after_peak_time*fADC;
  logger->log("ZeroSuppress: Storing " + std::to_string(after_peak_time)
	      + " us / " + std::to_string(after_peak) +
	      " ADC values / " + std::to_string(after_peak*sizeof(int16_t)) +
	      " bytes after peak.",1);   

  // Total number of ADC values stored per peak
  total_peak = before_peak + after_peak;
  logger->log("ZeroSuppress: Storing a total of " + std::to_string(total_peak/fADC)
	      + " us / " + std::to_string(total_peak) +
	      " ADC values / " + std::to_string(total_peak*sizeof(int16_t)) +
	      " bytes per peak.",1);   
  
  // HPGe decay time constant (us)
  hpge_decay_time = cfg.getValue<double>("stm.zs.hpge.decay_time");
  // HPGe decay time constant (ADC values)
  hpge_decay = int(hpge_decay_time*fADC);
  logger->log("ZeroSuppress: HPGe decay time constant = " + std::to_string(hpge_decay_time)
	      + " us / " + std::to_string(hpge_decay) + " ADC values.",1);
  
  // Gradient threshold
  threshold = cfg.getValue<int>("stm.zs.hpge.grad_threshold");
  logger->log("ZeroSuppress: Gradient threshold = " + std::to_string(threshold) + ".",1);   

  // Gradient window
  window = cfg.getValue<int>("stm.zs.hpge.grad_window");
  logger->log("ZeroSuppress: Gradient window = " + std::to_string(window) + " ADC values.",1);   

  // Gradient average
  n_average = cfg.getValue<int>("stm.zs.hpge.n_avg");
  logger->log("ZeroSuppress: Gradient average number = " + std::to_string(n_average) + " ADC values.",1);     
  
  // Maximum amount of overflow data to store from previous buffer
  prev_data_max = before_peak + window + n_average;

  // Allocate the overflow data vector to that size
  prev_data.resize(prev_data_max);
  std::fill(prev_data.begin(), prev_data.end(), 0);

  // Resize previous peak data vector to before peak length
  prev_peak_data.resize(before_peak);
  
  // Maximum noise length to store (us)
  double max_noise_time = cfg.getValue<double>("stm.dqm.baseline.noise_length");
  max_noise_len = int(max_noise_time*fADC);
  logger->log("ZeroSuppress: Storing " + std::to_string(max_noise_time)
              + " us / " + std::to_string(max_noise_len) + " ADC values of noise data per buffer.",1);
  
}


// Find peaks in data
void ZeroSuppress::find_peaks(std::shared_ptr<DataStruct>& buffer){

  // The adc length
  uint64_t n = buffer->raw_len;

  // The adjusted adc length
  uint64_t n_overflow = n-buffer->zs_overflow_num;

  // Define a pointer to the adc data
  std::vector<int16_t>* data = &buffer->raw;

  // The data index
  uint64_t j = 0;

  // Initialize baseline statistics
  double baseline_sum = 0;
  double baseline_sum2 = 0;
  uint64_t baseline_count = 0;
  
  // Rolling sums for the last n_average baseline values
  double recent_sum = 0;
  double recent_sum2 = 0;
  int recent_count = 0;

  // Loop over all ADC values 
  while(j < n_overflow){

    // Initialise average gradient / time variables
    double av_gradient = 0;
    double av_ADCtime = 0;    

    // Reset recent tracker
    recent_sum = 0;
    recent_sum2 = 0;
    recent_count = 0;
        
    // Loop over all average entries                                     
    for(int k = 0; k < n_average; k++){
   
      // Gradient values
      int16_t grad_low = 0;
      int16_t grad_high = 0;
   
      // Calculate gradient
      grad_low = (*data)[j];
      grad_high = (*data)[j+window];
   
      // Sum average "time" in counts
      av_ADCtime += j;

      // Only accumulate baseline if clearly not in a peak
      if (j > peak_end && peak_found == false) {
	baseline_sum += grad_low;
	baseline_sum2 += grad_low * grad_low;
	baseline_count++;	
	// Track for rollback
	recent_sum += grad_low;
	recent_sum2 += grad_low * grad_low;
	recent_count++;
      }
      
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
      peak_found=false;
      continue;
    }
  
    // Skip the rest indexes of the peak after                           
    // the trigger that has already been stored                          
    if(avg_gradient < threshold && peak_found == true){
      continue;
    }
  
    // If within the threshold and no peak previously found...
    if(avg_gradient < threshold && peak_found == false){
   
      // Peak found! 
      peak_found = true;
      // Increment the peak countera
      peak_count++;
      // Store peak location in adc data only
      buffer->peak_index.push_back(peak_loc);

      // Roll back most recent baseline additions (they belong to the peak!)
      baseline_sum -= recent_sum;
      baseline_sum2 -= recent_sum2;
      baseline_count -= recent_count;
            
      // Define peak region end
      peak_end = peak_loc + hpge_decay;
      
    } // End if peak found

    
  } // End loop over ALL event data

  // Calculate peak end in preparation for next buffer
  if (peak_end >= n_overflow){
    peak_end -= (n_overflow);
  }
  else{
    peak_end = -1;
  }
  
  // Calculate and storecurrent baseline mean and RMS
  double inv_count = 1.0 / baseline_count;
  double baseline_mean = baseline_sum * inv_count;
  buffer->baseline_mean_current = baseline_mean;
  buffer->baseline_rms_current = std::sqrt((baseline_sum2 * inv_count) - baseline_mean * baseline_mean);
  
  // Calculate avg baseline mean and RMS
  avg_baseline_sum += baseline_sum;
  avg_baseline_sum2 += baseline_sum2;
  avg_baseline_count += baseline_count;
  double avg_inv_count = 1.0 / avg_baseline_count;
  // Store average baseline mean and rms
  avg_baseline_mean = avg_baseline_sum * avg_inv_count;
  avg_baseline_rms = std::sqrt((avg_baseline_sum2 * avg_inv_count)
				- avg_baseline_mean * avg_baseline_mean);  
  buffer->baseline_mean_avg = avg_baseline_mean;
  buffer->baseline_rms_avg = avg_baseline_rms;

  // Remove the overflow number from the data length
  buffer->raw_len -= buffer->zs_overflow_num;

  // How many headers to remove from this buffer
  size_t buffers_fewer = 0;

  // adc counter
  size_t adc_count = 0;
  
  // Check how many headers to copy (loop in reverse)
  for (size_t i = buffer->raw_header_num; i-- > 0; ) {
    auto& map = buffer->raw_header_map[i];
    auto& header_data = std::get<hdrMap_hdrData>(map);
    // Get the start index of the event
    size_t event_start = std::get<hdrMap_adcIndex>(map);
    // Get the event length
    size_t event_length = std::get<hdrMap_dataLen>(map);
    // If overflow copy index falls inside this event, copying only a portion
    if (buffer->raw_len >= event_start && buffer->raw_len < event_start + event_length){
      // Adjust event length
      size_t new_event_length = buffer->raw_len - event_start;
      std::get<hdrMap_dataLen>(map) = new_event_length;
      header_data[eHdr.EvInPacket] = new_event_length;
      break;
    }
    else{
      // Increase the adc counter
      adc_count += event_length;
      // Reinitialise header map
      buffer->raw_header_map[i] = init_hdr_map;
      // Increment the number of buffers to remove      
      buffers_fewer++;
   }    
  }

  // Adjust header map number
  buffer->raw_header_num -= buffers_fewer;

  // Check the data total and headers total match  
  std::tuple last = buffer->raw_header_map[buffer->raw_header_num-1];
  uint64_t last_EWT = std::get<hdrMap_EWT>(last);
  size_t last_event_start = std::get<hdrMap_adcIndex>(last);
  size_t last_event_length = std::get<hdrMap_dataLen>(last);
  size_t total =  last_event_start + last_event_length;
  // If unequal, throw errors
  if (buffer->raw_len != total){
    std::string error = "ERROR in ZeroSuppress::find_peaks. For last EWT in buffer = "
      + std::to_string(last_EWT) + ", total data size ("
      + std::to_string(buffer->raw_len) + ") != accumulated data size in header map ("
      + std::to_string(total) + ")";
    logger->log(error,0);
  }
  
  return;
  
}


// Suppress data
void ZeroSuppress::suppress_data(std::shared_ptr<DataStruct>& buffer){

  // The adc length
  uint64_t n = buffer->raw_len;

  // Define a pointer to the adc data
  std::vector<int16_t>* data = &buffer->raw;

  // The data index
  uint64_t j = 0;

  // Define a pointer to the peak locations
  std::vector<uint64_t>* peaks = &buffer->peak_index;
  
  // The number of peaks in the data
  uint64_t peak_num = buffer->peak_index.size();  

  // Define a pointer to the adc data
  std::vector<int16_t>* suppressed_data = &buffer->zs;

  // Largest peak separation (to store fraction of baseline data)
  uint64_t max_sep = 0;
  size_t max_sep_index = 0;
  
  // Ensure suppressed data length is zero
  buffer->zs_len = 0;  

  // First, store any data needed from peak found in previous call
  if (left_to_copy > 0){
    // Copy the data
    std::copy(data->begin(),
    	      data->begin() + left_to_copy,
    	      suppressed_data->begin() + buffer->zs_len);

    // Increase suppressed data size
    buffer->zs_len += left_to_copy;

    // Set left to copy to zero
    left_to_copy = 0;
 
  }   

  // Loop over number of peaks in data struct
  for (int i = 0; i < peak_num; i++){
    
    // Get the peak location
    uint64_t peak_loc = (*peaks)[i];
    
    // The index location to copy from...
   int64_t loc_copy_from = 0;
    
    // The data length to copy
    uint64_t data_len = 0;

    // If first peak
    if(first_peak){
      
      // Check whether there is 1 us of data before the first peak...
      // If not, the number of ADC values is: peak_loc
      if(peak_loc - before_peak < 0){	  
	
	// Copy from first element in ADC array
	loc_copy_from = 0;
	
	// Store the data from data[0] --> peak_loc+after_peak
	data_len = peak_loc + after_peak;
	
      }
      
      // Else if 1 us or more exists before the start of the first peak
      else{
	
	// Copy from 1 us before peak
	loc_copy_from = peak_loc-before_peak;	 
	
	// Store the whole peak
	data_len = total_peak;
	
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
      if(peak_sep < total_peak){      
	
	// Copy from end of previous peak
	loc_copy_from = prev_peak_loc + after_peak;
	
	// Store just the new peak separation
	data_len = peak_sep;

      }
      // Else if this new peak is outside the 3us window of the previous peak
      else{

	// Copy from 1 us before peak
	loc_copy_from = peak_loc - before_peak;

	// Store the total peak
	data_len = total_peak;

	// If the peak starts in the previous call
	if (loc_copy_from < 0){
	  // Get the amount to copy
	  int to_copy = std::abs(loc_copy_from);
	  // Copy the data
	  std::copy(prev_peak_data.begin() + (before_peak - to_copy),
	  	    prev_peak_data.begin() + before_peak,
	  	    suppressed_data->begin() + buffer->zs_len);
	  // Increase the suppressed data length
	  buffer->zs_len += to_copy;
	  // Set the new location to copy to the start of the data call
	  loc_copy_from = 0;
	  // Update the amount left to store
	  data_len -= to_copy;
	}

	// Recalculate max peak seperation for baseline DQM
	if (i > 0){ // Avoid overlapping peaks for simplicity
	  if (peak_sep > max_sep) {
	    max_sep = peak_sep;
	    max_sep_index = i;
	  }
	}

      } // End if (peak_sep < total_peak)

    } // End if/not first peak

    // Store peak time to compare against next peak
    prev_peak_loc = peak_loc;	    
    
    // If the data to copy goes beyond the data length
    if(loc_copy_from + data_len > n){
      
      // Store the amount left to copy from the previous call
      left_to_copy = data_len - (n - loc_copy_from);
      
      // Store only until end of data
      data_len -= left_to_copy;
      
    }
    else{
      
      // No data left to copy in next call
      left_to_copy = 0;
      
    }
    
    // If last peak, store distance from last peak to end
    if (i == peak_num - 1) prev_peak_loc -= n;
      
    // If the location to copy from is in the next call, skip
    if (loc_copy_from > n) continue;

    // Copy the data
    std::copy(data->begin() + loc_copy_from,
    	      data->begin() + loc_copy_from + data_len,
    	      suppressed_data->begin() + buffer->zs_len);

    // Increase suppressed data size
    buffer->zs_len += data_len;

  } // End loop over peaks

  // Store the end of this call
  std::copy(data->begin() + (n - before_peak),
  	    data->begin() + n,
  	    prev_peak_data.begin());

  // Accumulate the total suppressed data len
  tot_sup_len += buffer->zs_len;

  // Store portion of baseline data for the DQM
  // Calcluate noise length to copy (peak separation - time stored before peak_n - time stored after peak_(n-1)
  int noise_len = int(max_sep) - int(before_peak) - int(hpge_decay);
  if (noise_len != 0){
    // Get the data length
    size_t data_len = std::min<size_t>(noise_len,max_noise_len);
    // The index location to copy from (go back start of new of peak)
    uint64_t loc_copy_from = (*peaks)[max_sep_index] - before_peak - data_len;
    // Copy the noise data
    std::copy(data->begin() + loc_copy_from,
	      data->begin() + loc_copy_from + data_len,
	      buffer->noise_data.begin());
  }

}

