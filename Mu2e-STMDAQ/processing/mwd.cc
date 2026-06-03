// MWD code
#include "Mu2e-STMDAQ/processing/mwd.hh"

// Constructor
MWD::MWD(const std::shared_ptr<AsyncLogger>& logger_,
         const std::shared_ptr<STMdata>& stm_) :
  logger(logger_), stm(stm_),
  // Baseline subtracion
  use_manual_baseline(stm->mwd_config.use_manual_baseline), 
  manual_baseline_mean(stm->mwd_config.manual_mean), 
  manual_baseline_sigma(stm->mwd_config.manual_sigma),
  use_window_baseline(stm->mwd_config.use_window_baseline),
  // Deconvolution
  tADC(stm->fw_config.tADC), tau(stm->mwd_config.tau), tau_norm(stm->mwd_config.tau_norm),
  //  Differentiation
  M(stm->mwd_config.M), m_mask(stm->mwd_config.m_mask), a_ring(std::vector<double>(M, 0.0)),
  // Averaging
  L(stm->mwd_config.L), inv_L(stm->mwd_config.inv_L), l_mask(stm->mwd_config.l_mask),
  D_ring(std::vector<double>(L, 0.0)),
  // Peak finding   
  peak_init(stm->mwd_config.peak_min_init), peak_height(peak_init), peak_time(0.0),
  peak_zs_start(0), peak_zs_len(0), 
  use_fixed_cut(stm->mwd_config.use_fixed_cut), fixed_cut_value(stm->mwd_config.fixed_cut_value), 
  nsigma_cut(stm->mwd_config.nsigma_cut)
{
  
  // Register operations for OperationManager
  register_operation("subtract_baseline", [this](auto& b){ subtract_baseline(b); });
  register_operation("deconv", [this](auto& b){ deconvolution(b); });
  register_operation("diff", [this](auto& b){ differentiation(b); });
  register_operation("averaging", [this](auto& b){ averaging(b); });
  //  register_operation("find_peaks", [this](auto& b){ find_peaks(b); });
  register_operation("find_peaks", [this](auto& b, auto& prev){ find_peaks(b, prev); });
  
}


// Subtract baseline 
void MWD::subtract_baseline(std::shared_ptr<DataStruct>& buffer) {

  // The adc length
  size_t n = buffer->raw_len;

  // Define a pointer to the raw data buffer
  int16_t* data_ptr = buffer->raw.data();
 
  // Define a pointer to the pulse height data buffer
  double* ph_ptr = buffer->ph.data();

  // Initialise baseline value for buffer
  double baseline = 0;

  // If using config manual baseline
  if (use_manual_baseline){
    baseline = manual_baseline_mean;
  }
  // Else if not manual baseline
  else{
    // If using dyanmic window baseline
    if (use_window_baseline){
      baseline = buffer->baseline_window.mu0;
    }
    // Else use baseline since start of run
    else{
      baseline = buffer->baseline_all.mu0;
    }
  }
    
  // Loop over all data
  for (size_t i = 0; i < n; ++i) {
    
    // Cast the value as a double
    const double data_i = static_cast<double>(data_ptr[i]);
    
    // Store sample with baseline subtracted
    ph_ptr[i] = data_i - baseline;
        
  }

  // Save mwd length
  buffer->ph_len = n;

}


// Deconvolution 
void MWD::deconvolution(std::shared_ptr<DataStruct>& buffer) {

  // The adc length
  size_t n = buffer->raw_len;

  // Define a pointer to the pulse height data buffer
  double* ph_ptr = buffer->ph.data();

  // Loop over all data
  for (size_t i = 0; i < n; ++i) {
    
    // Cast the value as a double
    const double data_i = static_cast<double>(ph_ptr[i]);
    
    // Deconvolution: ai = data[i] - (1 - (T0/tau)) * data[i-1] + a[i-1]
    const double ai = data_i - tau_norm * prev_sample + prev_a;

    // Store deconvolved sample into ph (temporary workspace)
    ph_ptr[i] = ai;
        
    // Prepare for next sample
    prev_sample = data_i;
    prev_a = ai;

  }

  // Save mwd length
  buffer->ph_len = n;

}

// Differentiation
void MWD::differentiation(std::shared_ptr<DataStruct>& buffer) {

  // The deconvoluted data length
  size_t n = buffer->ph_len;

  // Define a pointer to the pulse height data buffer
  double* data_ptr = buffer->ph.data();
  
  // Loop over all data
  for (size_t i = 0; i < n; ++i) {
    
    // Get the deconvolution value
    const double ai = data_ptr[i];
    
    // Get the m index
    const size_t m_index = diff_index & m_mask;
    
    // Get a[i-M] from the ring buffer
    const double aim = (diff_index >= M) ? a_ring[m_index] : 0.0;
    
    // Differentiation: D[i] = ai - a[i-M] (for i >= M), else D[i] = ai
    const double Di = ai - aim;

    // Save to buffer
    data_ptr[i] = Di;

    // Save ai into ring buffer for future use
    a_ring[m_index] = ai;
    
    // Increment the sample index
    ++diff_index;
    
  }

}

// Averaging
void MWD::averaging(std::shared_ptr<DataStruct>& buffer) {

  // The deconvoluted data length
  size_t n = buffer->ph_len;
  if (n == 0) return;

  // Non-aliasing hints 
  double* __restrict data_ptr = buffer->ph.data();
  double* __restrict D_ptr    = D_ring.data();

  // Keep running state in locals for better register use
  size_t idx = avg_index;
  double s   = sum;

  // Pointer loop over data
  double*       p   = data_ptr;
  double* const end = data_ptr + n;

  for (; p != end; ++p, ++idx) {

    // Get ring-buffer index
    const size_t l_index = idx & l_mask;

    // Current sample
    const double Di = *p;

    // Moving average 
    double val;
    if (idx < L) {
      s += Di;
      val = (idx == L - 1) ? (s * inv_L) : Di;
    } else {
      s += Di - D_ptr[l_index];
      val = s * inv_L;
    }

    // Save current D[i] into ring buffer for moving sum
    D_ptr[l_index] = Di;
    
    // Store averaged value back into ph
    *p = val;

  }

  // Write back updated state
  avg_index = idx;
  sum       = s;
}

// Find peaks in data
void MWD::find_peaks(std::shared_ptr<DataStruct>& buffer,
                     std::shared_ptr<DataStruct>& prev_buffer){
  
    // If prev buffer is null, we can't use it
    if(!prev_buffer) peak_in_prev_buffer = false;

    // Define a pointer to the pulse height data buffer
    double* data_ptr = buffer->ph.data();    

    // The data length
    size_t n = buffer->ph_len;    

    // The peak finding threshold cut
    double threshold_cut = -1.0*nsigma_cut;

    bool below_threshold = false;
    
    // If using fixed threshold cut
    if (use_fixed_cut) {
      // Use fixed value
      threshold_cut = fixed_cut_value;
    }
    // Else use dyanmic sigma from basline cut
    else{
      // If using config manual baseline
      if (use_manual_baseline){
        threshold_cut *= manual_baseline_sigma;
      }
      // Else if not manual baseline
      else{
        // If using dyanmic window baseline
        if (use_window_baseline){
          threshold_cut *= buffer->baseline_window.sigma0;
        }
        // Else use baseline since start of run
        else{
          threshold_cut *= buffer->baseline_all.sigma0;
        }
      }
    }

    // Number of peaks found in this buffer
    size_t peak_count = 0;

    // EWT counter
    size_t EWT_count = 0;

    // The buffer's EWT data
    EWTinfo& EWTs = buffer->EWTs;
    
    // Get the first EWT header of the buffer
    EWT_info* this_EWT = &EWTs[EWT_count];
    uint64_t EWT = this_EWT->EWT;
    size_t EWT_start = this_EWT->raw.start;
    size_t EWT_len = this_EWT->raw.len;
    size_t new_EWT_loc = EWT_start + EWT_len; // The next EWT location

    // Peak finding only starts after M samples
    for (size_t i = 0; i < n; ++i){

      // If we've reached the end of the EWT and skip missing
      while (i == new_EWT_loc && EWT_count < buffer->EWT_count - 1){
        ++EWT_count; // Increment the EWT counter (in buffer)
        this_EWT = &EWTs[EWT_count];
        EWT_start = this_EWT->raw.start;
        EWT_len = this_EWT->raw.len;
        new_EWT_loc = EWT_start + EWT_len; // The next EWT location
      }

      // Get the averaged MWD value for this sample
      const double avg = data_ptr[i];
      
      // Only perform peak-finding logic after the warm-up period (M samples)
      if (pf_index >= M) [[likely]] {
        
        // Detect peak region below threshold
        if (avg < threshold_cut) {

          // Signal below threshold
          below_threshold = true;
          
          // Need to find peak mininum...
          // If signal is still falling and reaches a new minimum, update candidate
          if ((avg < prev_avg) && (avg < peak_height)) {

            // If peak region started in prev buffer, signal peak is now in this buffer
            peak_in_prev_buffer = false;
            
            // Record new minimum
            peak_height = avg;

            // Record time of that minimum from start of EWT
            peak_time = i-EWT_start;

            // Record EWT of minimum
            peak_EWT = this_EWT;

            // Record the index EWT containing the peak
            peak_EWT_idx = EWT_count;
            
          }
        }
        
        // Detect when the signal has risen back above the threshold 
        // Pulse has ended → finalize the peak
        if (peak_height != peak_init && avg > threshold_cut) {

          // Declare EWT to store
          EWT_info* EWT_to_store;
          
          // Did this peak region start in the prev buffer
          if (peak_in_prev_buffer){
            // The prev buffer's EWT data
            EWTinfo& prev_EWTs = prev_buffer->EWTs;
            // Find the EWT in the prev buffer
            EWT_to_store = &prev_EWTs[prev_buffer_EWT_idx];
            // Prev EWT header
            sw_event_header& prev_header = EWT_to_store->hdr; 
            // Incrememt number of pulse heights in EWT header
            ++prev_header[sw_eHdr.PH_NUM];

            // Define a pointer to the prev data buffer
            double* prev_data_ptr = prev_buffer->ph.data();
            // Get prev peak count
            size_t prev_peak_count = prev_buffer->peak_count;
            // Store the peak time and height in the same buffer
            prev_data_ptr[2*prev_peak_count] = peak_time; // Peak time index
            prev_data_ptr[2*prev_peak_count+1] = peak_height; // Peak height
            // Increment prev buffer peak count
            ++prev_buffer->peak_count;
            // Increase prev buffer ph length
            prev_buffer->ph_len += 2;
            // Increase total mwd length
            tot_mwd_data_len += 2;
          }
          // If peak is in this EWT
          else{
            // EWT to store is this EWT
            EWT_to_store = peak_EWT;
            // Incrememt number of pulse heights in EWT header
	    ++peak_EWT->hdr[sw_eHdr.PH_NUM];
            // Increment counters
            peak_count++; 
          }
          
          // Add a new peak time and height entry to the peak's EWT info
          EWT_to_store->ph.emplace_back(static_cast<int16_t>(std::lround(peak_time)),
                                        static_cast<int16_t>(std::lround(peak_height)));

          // Increment all peak counters
          peak_count_all++;
                             
          // Reset for next peak search
          peak_height = peak_init;

          // Ensure peak in prev buffer is false
          peak_in_prev_buffer = false;
          
          // Signal above threshold
          below_threshold = false;
          
        }
      }      
      
      // Store this after for next call
      prev_avg = avg;
      
      // Increment the sample index
      ++pf_index;
      
    }    

    // Now we need to update the ph vector with
    // the correct pulses for binary file writing    
    // Initialise counter
    size_t count = 0;    
    // Loop over all EWTs
    for (int i = 0; i < buffer->EWT_count; i++){
      // Get the EWT info
      const EWT_info* this_EWT = &EWTs[i];
      // Loop through all pulses in this EWT
      for (int j = 0; j < this_EWT->ph.size(); j++){
        // Get time and height of pulse
        const double time = static_cast<double>(this_EWT->ph[j].time);
        const double height = static_cast<double>(this_EWT->ph[j].height);
        // Store the peak time and height in the same buffer
        data_ptr[count] = time; // Peak time index (from start of EWT)
        ++count;
        data_ptr[count] = height; // Peak height
        ++count;
      }
    }
    
    // Record number of peaks and data length
    buffer->peak_count = peak_count;
    buffer->ph_len = 2*peak_count;
    tot_mwd_data_len += buffer->ph_len;

    // If unequal, throw errors
    if (buffer->ph_len != count){
      std::string error = "MWD::find_peaks: ERROR. Total ph data len in buffer = "
        + std::to_string(buffer->ph_len) + ", total ph data len in EWTs ="
        + std::to_string(count) + "!";
      logger->log(error,0);
      return;
    }
    
    // If still in a below threshold region
    if (below_threshold){
      // Signal peak in prev buffer
      peak_in_prev_buffer = true;
      // Store the EWT where the peak was found
      prev_buffer_EWT_idx = peak_EWT_idx;
    }
    
    return;
}


