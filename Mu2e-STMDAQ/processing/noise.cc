// Noise code
#include "Mu2e-STMDAQ/processing/noise.hh"
#include <numeric>
#include <fftw3.h>

// Constructor
Noise::Noise(Config& cfg_,
	   const std::shared_ptr<AsyncLogger>& logger_,
	   const std::shared_ptr<STMdata>& stm_):
  cfg(cfg_), logger(logger_), stm(stm_),
  before_peak(stm->zs_config.before_peak),
  total_peak(stm->zs_config.total_peak),
  decay_len(stm->mwd_config.tau * stm->fw_config.fADC),
  max_noise_len(stm->buffer_config.baseline_len)

{

  // Initialise function map for operation manager
  register_operation("get_noise_data", [this](auto& b){ get_noise_data(b); });
  
}



// Get noise data
void Noise::get_noise_data(std::shared_ptr<DataStruct>& buffer){

  // The adc length
  uint64_t n = buffer->raw_len;

  // The data index
  uint64_t j = 0;

  // Set initial parameters
  uint64_t noise_len = 0;
  uint64_t first_peak = true;

  // The number of peaks in the data
  uint64_t peak_num = buffer->zs_data.size(); 
  //if (peak_num == 0) std::cout << "no peaks" << std::endl;

  // Loop over number of peaks in data struct
  for (int i = 0; i < peak_num; i++){
    
    // Get the peak location
    size_t peak_start = buffer->zs_data[i].start;
    
    // The index location to copy from...
    int64_t loc_copy_from = 0;
    int64_t loc_copy_to = 0;
    
    // The data length to copy
    uint64_t data_len = 0;

    // If first peak
    if(first_peak){
      // If start is zero it likely started in previous buffer, skip
      if(peak_start = 0){	  
	 continue;
      }
      
      // Else if noise exists before the start of the first peak
      else{
	
	// Copy start to before peak
	loc_copy_from = 0;
	loc_copy_to = peak_start;	 

	// Store the noise length
        data_len = loc_copy_to - loc_copy_from;
	
      }
      
      // Set first peak to false
      first_peak = false;
      
    } // End if first peak
    
    // Else if not first peak
    else{
      
      // Find the number of values between this peak
      // and the previous one
      int peak_sep = peak_start - prev_peak_start;
      
      // If the peak separation is within the peak window, no noise
      if(peak_sep < before_peak+decay_len){      
	continue;	
      }
      // Else if this new peak is not overlapping
      else{

	// Copy from end of last peak (after detector decay) to start of this one
	loc_copy_from = prev_peak_start + before_peak + decay_len;
	loc_copy_to = peak_start;

	// Store the noise length
	data_len = loc_copy_to - loc_copy_from;

      } // End if (peak_sep < total_peak)

    } // End if/not first peak

    // Store peak time to compare against next peak
    prev_peak_start = peak_start;	    
    
    // If at max noise length or last peak,
    // break out of loop
    uint64_t new_len = data_len + noise_len;
    if (new_len > max_noise_len || i == peak_num-1) {
      // Copy the remaining data to fill the noise
      uint16_t copy_len = max_noise_len - noise_len;
      std::copy(buffer->raw.begin() + loc_copy_from,
		buffer->raw.begin() + loc_copy_from + copy_len,
		std::back_inserter(buffer->noise_data));
      break;
    }

    // If not, copy data chunk and continue
    std::copy(buffer->raw.begin() + loc_copy_from,
	      buffer->raw.begin() + loc_copy_to,
	      std::back_inserter(buffer->noise_data));

    // Increase total noise data size
    noise_len += data_len;

  } // End loop over peaks

  //check_noise(buffer);
  //noise_fft(buffer);

}

void Noise::check_noise(std::shared_ptr<DataStruct>& buffer){  

  // Define a pointer to the noise data
  std::vector<int16_t>* noise_data = &buffer->noise_data;

  // If there is no noise data cannot calculate anything
  if (noise_data == nullptr) {
    //logger->log("Noise::check_noise has no noise data to check, noise_len = "
	//+ std::to_string(noise_len), 1);
    return;
  }

  int64_t noise_len = noise_data->size();
  
  // Get baseline average in buffer
  double sum = std::accumulate(noise_data->begin(), noise_data->end(), 0.0);
  avg_baseline = sum/noise_len;

  // Get baseline rms in buffer
  double square_sum = std::inner_product(noise_data->begin(), noise_data->end(), noise_data->begin(), 0.0);
  double mean_of_squares = square_sum/noise_len;
  baseline_rms = std::sqrt(mean_of_squares);

  std::cout<< "Noise "<< std::to_string(avg_baseline) 
	   << " RMS " << std::to_string(baseline_rms) <<std::endl;

  // If baseline out of bounds, throw errors
  if (abs(avg_baseline) > max_allowed_baseline){
    std::string error = "ERROR in Noise::check_noise. Mean baseline not within +/-"
      + std::to_string(max_allowed_baseline) + ", baseline_avg = "
      + std::to_string(avg_baseline);
    logger->log(error,1);
  }
  if (abs(baseline_rms) > max_allowed_noise_rms){
    std::string error = "ERROR in Noise::check_noise. Noise RMS greater than "
      + std::to_string(max_allowed_noise_rms) + ", baseline_rms = "
      + std::to_string(baseline_rms);
    logger->log(error,1);
  }

  return;
  
}

void Noise::noise_fft(std::shared_ptr<DataStruct>& buffer){
  // Define a pointer to the noise data
  std::vector<int16_t>* noise_data = &buffer->noise_data;

  // Define a pointer to the noise fft
  std::vector<int16_t>* noise_data_fft = &buffer->noise_data_fft;

  // If there is no noise data cannot calculate anything
  if (noise_data == nullptr) {
    //logger->log("Noise::check_noise has no noise data to check, noise_len = "
	//+ std::to_string(noise_len), 1);
    return;
  }

  int64_t noise_len = noise_data->size();
  std::vector<double> noise_for_fft(noise_len);
  for (int i=0; i<noise_len; i++) noise_for_fft[i] = static_cast<double>((*noise_data)[i]);

  fftw_complex* fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (noise_len/2+1));
  fftw_plan plan = fftw_plan_dft_r2c_1d(noise_len, noise_for_fft.data(), fft_out, FFTW_ESTIMATE);

  fftw_execute(plan);

  int64_t fft_len = noise_len/2 + 1;
  std::vector<double> fft_power(fft_len);
  for (int k=0; k<noise_len/2; k++) {
    double real = fft_out[k][0];
    double imag = fft_out[k][1];
    fft_power[k] = real*real + imag*imag;
  }

  // Write FFT to buffer?
  std::copy(fft_power.begin(), fft_power.end(),
	    noise_data_fft->begin());

}
