// Zero suppress code
#include "Mu2e-STMDAQ/processing/zero_suppress.hh"

// Constructor
ZeroSuppress::ZeroSuppress(Config& cfg_,
                           const std::shared_ptr<AsyncLogger>& logger_,
                           const std::shared_ptr<STMdata>& stm_,
                           const std::shared_ptr<SignalHandler>& signal_) : 
  cfg(cfg_), logger(logger_), stm(stm_), signal(signal_),
  before_peak(stm->zs_config.before_peak),
  after_peak(stm->zs_config.after_peak),
  total_peak(stm->zs_config.total_peak),
  threshold(stm->zs_config.grad_threshold),
  window(stm->zs_config.grad_window),
  n_average(stm->zs_config.grad_n_avg),
  prev_data(std::vector<int16_t>(window+n_average, 0.0))
{

  // Register operations for OperationManager
  register_operation("find_peaks", [this](auto& b, auto& prev){ find_peaks(b, prev); });
  register_operation("suppress_data", [this](auto& b){ suppress_data(b); });
  
}

// Find peaks in data
void ZeroSuppress::find_peaks(std::shared_ptr<DataStruct>& buffer,
                              std::shared_ptr<DataStruct>& prev_buffer){

  // Store peak info from previous call first
  for (int i = 0; i < next_peak_data.size(); i++){
    buffer->zs_data.emplace_back(next_peak_data[i]);
  }
  // Set size of next peak data buffer to zero for this call
  next_peak_data.clear();
  
  // The adc length
  size_t n = buffer->raw_len;
  
  // Define a pointer to the raw data buffer
  int16_t* data_ptr = buffer->raw.data();

  // The data index
  size_t j = 0;

  // The total data to span = this buffer + previous leftoer
  size_t total = n+prev_num;

  // This buffer's leftover
  size_t leftover = window + ((total - window) % n_average);
  
  // Loop over all ADC values up to the window
  while(j < total-leftover){

    // Initialise average gradient / time variables
    double av_gradient = 0;
    double av_ADCtime = 0;    

    // Loop over all average entries                                     
    for(int k = 0; k < n_average; k++){

      // Low/high window indices
      size_t idx_low  = j;
      size_t idx_high = idx_low + window;      
      
      // Gradient values
      int16_t grad_low = 0;
      int16_t grad_high = 0;

      // If first buffer, take data straight from buffer
      if (first_buffer){
        grad_low  = data_ptr[idx_low];
        grad_high = data_ptr[idx_high];
      }
      // If not first buffer, span leftover data first
      else{
        grad_low  = (idx_low  < prev_num) ? prev_data[idx_low]  : data_ptr[idx_low  - prev_num];
        grad_high = (idx_high < prev_num) ? prev_data[idx_high] : data_ptr[idx_high - prev_num];
      }
      
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
    int64_t peak_loc = av_ADCtime/n_average;
  
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

      // Get the peak loc in this buffer's boundary indices
      peak_loc -= prev_num;

      // Get the peak start location
      int64_t peak_start = (int64_t)peak_loc - before_peak;
      // If first buffer or no prev buffer, set peak start to 0
      if ((first_buffer || !prev_buffer) && peak_start < 0) peak_start = 0;

      // Set the data length as total peak length
      size_t data_len = total_peak;

      // If the peak starts in the previous buffer
      if (peak_start < 0) {
	// How many samples of this peak belong to the previous buffer?
	// peak_start is negative (int64_t), so -peak_start is positive.
	const size_t need_prev = static_cast<size_t>(-peak_start);

	// Previous buffer length (size_t)
	const size_t prev_n = prev_buffer->raw_len;

	// Clamp so we never underflow
	const size_t prev_len = (need_prev <= prev_n) ? need_prev : prev_n;

	// Tail start index in previous buffer
	const size_t prev_start = prev_n - prev_len;

	// Record the tail segment INSIDE buffer0 boundaries
	prev_buffer->zs_data.emplace_back(prev_start, prev_len);

	// Now the remaining part in current buffer begins at 0
	peak_start = 0;

	// Reduce how much we still need to store in this/current/next buffer
	if (data_len >= prev_len) data_len -= prev_len;
	else data_len = 0;
      }

      // If data start + data len in this buffer is larger than the buffer
      if (peak_start + data_len > n){
        // Calculate data length from start of next buffer
        size_t next_buffer_len = peak_start + data_len - n;
        // Store peak info to this-> next peak buffer for next call
        next_peak_data.emplace_back(0,next_buffer_len);
        // Calculate remaining data length in this buffer
        data_len -= next_buffer_len;
      }
        
      // Store peak info in this buffer
      buffer->zs_data.emplace_back(peak_start,data_len);      
      
    } // End if peak found
  
  } // End loop over ALL event data

  // Store prev_num as this buffer's leftover
  prev_num = leftover;
    
  // Index to copy overflow data
  size_t copy_from = n - prev_num;                                                

  // Copy overflow data
  std::copy(buffer->raw.begin() + copy_from,
            buffer->raw.begin() + n,                                              
            prev_data.begin());       

  // Set first buffer to false
  first_buffer = false;

  return;
  
}


// Suppress data
void ZeroSuppress::suppress_data(std::shared_ptr<DataStruct>& buffer){

  // Define a pointer to the adc data
  std::vector<int16_t>* raw_data = &buffer->raw;

  // Define a pointer to the adc data
  std::vector<int16_t>* zs_data = &buffer->zs;

  // Ensure suppressed data length is zero
  buffer->zs_len = 0;

  // Store the end point of the last data copy
  size_t last_copy_end = 0;
  
  // EWT counter
  size_t EWT_count = 0;
  
  // The buffer's EWT data
  EWTinfo& EWTs = buffer->EWTs;

  // Counter of ADC data in EWTs
  size_t EWT_adc_count = 0;

  // Keep track of last EWT index
  size_t last_EWT_j = 0;

  // Loop over all zs data regions in buffer
  for (int i = 0; i < buffer->zs_data.size(); i++){
    // ZS data region start index
    size_t start = buffer->zs_data[i].start;
    // Zs data region length
    size_t len = buffer->zs_data[i].len;
    // Copy end point
    size_t copy_end = start + len;
    // If start being is before the last copy end
    if (last_copy_end > start){
      // Start from the last copy end
      start = last_copy_end;
      // Adjust length
      len = copy_end - start;
    }

    // If we've already copied up to the end of the buffer, skip
    if (start == buffer->raw_len) continue;
    
    // Copy the data
    std::copy(raw_data->begin() + start,
              raw_data->begin() + start + len,
              zs_data->begin() + buffer->zs_len);
    // Increase zs data length in buffer
    buffer->zs_len += len;

    // Bool to check EWT has been found
    bool found_EWT = false;  

    // Remaining region to assign (may be split across EWTs)
    size_t cur_start = start;
    size_t remaining = len;
    
    // Loop over all EWTs
    for (size_t j = last_EWT_j; j < static_cast<size_t>(buffer->EWT_count); j++) {

      // Get the EWT info
      EWT_info* this_EWT = &EWTs[j];
      // Get starting data index of this EWT
      const size_t EWT_start = this_EWT->raw.start;
      // Get data length of this EWT
      const size_t EWT_len   = this_EWT->raw.len;
      // Get start loc of next EWT
      const size_t new_EWT_loc = EWT_start + EWT_len;

      // We haven't reached the EWT that contains cur_start yet
      if (cur_start < EWT_start) {
        // With your ordering guarantees, this usually means something is inconsistent,
        // so stop searching.
        break;
      }
      
      // // If the ZS data region starts in this EWT
      // if (start >= EWT_start && start < new_EWT_loc) {
      // If the current sub-region starts inside this EWT, store as much as fits
      if (cur_start >= EWT_start && cur_start < new_EWT_loc) {

        // Amount we can store in this EWT without crossing the boundary
        const size_t chunk_len = std::min(remaining, new_EWT_loc - cur_start);        
        
        // If this EWT already has ZS regions... 
        if (!this_EWT->zs.zs_regions.empty()) {
          // Get last ZS region in this EWT
          auto &last = this_EWT->zs.zs_regions.back();  // pair<start,len>
          // Get last ZS region region point
          const size_t last_end = last.start + last.len;
          // If the new region starts exactly where the last one ended
          if (cur_start == last_end) {
            // Extend previous region instead of adding a new entry.
            last.len += chunk_len;       
          }
          // Else add a new ZS region to this EWT
          else {
            this_EWT->zs.zs_regions.emplace_back(cur_start, chunk_len);
          }
        }
        // If not ZS regions in this EWT, add first entry
        else {
          this_EWT->zs.zs_regions.emplace_back(cur_start, chunk_len);
        }
                
        // Increase stored ZS data
        this_EWT->zs.adc_count += chunk_len;
        // Signal found EWT
        found_EWT = true;
        // Store last EWT index
        last_EWT_j = j;

        // Advance
        cur_start += chunk_len;
        remaining -= chunk_len;
        
        // Done if we've assigned the whole original region
        if (remaining == 0) {
          break;
        }
        
      }
    }    
    
    // If not EWT found for ZS data 
    if (!found_EWT){
      // Critical Error
      logger->log("ZeroSupress::suppress_data: Error! zs data located outside of EWTs in buffer!",0);
      std::cout << i << " " << buffer->zs_data.size() << " " << start << " " << len << " " << copy_end << " " << buffer->raw_len << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;            
    }
    
    // Update last copy end
    last_copy_end = copy_end;
    
  }
  
  // Loop over all EWTs
  for (int j = 0; j < buffer->EWT_count; j++){
    // Get the EWT info
    EWT_info* this_EWT = &EWTs[j];
    //    uint64_t EWT = this_EWT->EWT;
    // Get number of ZS regions
    int16_t zs_regions = this_EWT->zs.zs_regions.size();
    // Get EWT header
    sw_event_header& header = this_EWT->hdr;
    // Store number of ZS regions in EWT header
    header[sw_eHdr.ZS_REGIONS] = zs_regions;
    //    std::cout << EWT << " " << Athis_EWT->zs.zs_regions.size() << " " << header[sw_eHdr.ZS_REGIONS] << std::endl;
    // Store total ZS length in EWT header
    header[sw_eHdr.ZS_LEN] = this_EWT->zs.adc_count;
    // Calculate total EWT adc data
    EWT_adc_count += this_EWT->zs.adc_count;
  }
  
    
  // If unequal, throw errors
  if (buffer->zs_len != EWT_adc_count){
    std::string error = "ZeroSuppress::suppress_data: ERROR. Total ZS data len in buffer = "
      + std::to_string(buffer->zs_len) + ", total ZS data len in EWTs ="
      + std::to_string(EWT_adc_count) + "!";
    logger->log(error,0);
    return;
  }


  // buffer->zs_len = 0;  
  // for (int i = 0; i < buffer->EWT_count; i++){
  //   EWT_info* this_EWT = &EWTs[i];
  //   uint64_t EWT = this_EWT->EWT;
  //   size_t EWT_start = this_EWT->raw.start;
  //   size_t EWT_len = this_EWT->raw.len;
  //   size_t new_EWT_loc = EWT_start + EWT_len; // The next EWT location
  //   for (int j = 0; j < this_EWT->zs.zs_regions.size(); j++){
  //     size_t zs_start = this_EWT->zs.zs_regions[j].start;
  //     size_t zs_len = this_EWT->zs.zs_regions[j].len;
  //     //      std::cout << EWT << " " << EWT_start << " " << zs_start << " " << zs_len << " " << new_EWT_loc << std::endl;
  //   // Copy the data
  //   std::copy(raw_data->begin() + zs_start,
  //             raw_data->begin() + zs_start + zs_len,
  //             zs_data->begin() + buffer->zs_len);
  //   // Increase zs data length in buffer
  //   buffer->zs_len += zs_len;
  //   }
  // }

  // Increase total suppressed data length
  tot_sup_len += buffer->zs_len;
  
  return;
  

}

