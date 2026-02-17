#include <chrono>
#include <thread>
#include <iomanip>
#include <iomanip>
#include <type_traits>  // for std::is_integral
#include <fstream>      // for std::ofstream
#include <cstdint>      // for int16_t
#include <vector>
#include <cmath>        // for std::round
#include <type_traits>  // for std::is_integral
#include <stdexcept>    // for std::runtime_error

// Include MWD code
#include "MWD.hh"
// Include stats code
#include "stats.hh"

int offset = 0;

// Constructor
MWD::MWD(const std::string& config_filename){

  // Load config file and configure MWD algorithm
  configure_mwd(config_filename);
  
  // Confirmation loop
  std::string response;
  while (true) {
    std::cout << "Do you want to continue with this configuration? (Y/N): ";
    std::getline(std::cin, response);
  
    // Remove leading/trailing whitespace and convert to uppercase/lowercase
    if (response.empty()) continue;
    char c = std::toupper(response[0]);
  
    if (c == 'Y') {
      std::cout << "Continuing with the current configuration.\n" << std::endl;
      break;
    } else if (c == 'N') {
      std::cerr << "Aborted by user." << std::endl;
      exit(1);
    } else {
      std::cerr << "Invalid response. Please type Y or N." << std::endl;
    }
  }

  // Create necessary buffers
  a_ring = std::vector<double>(M, 0.0); // Ring buffer for 'a'
  D_ring = std::vector<double>(L, 0.0); // Ring buffer for 'D'

  // Variables for deconvolution, differentiation, moving average calculation
  m_mask = M - 1;
  l_mask = L - 1;
  inv_L = 1.0 / static_cast<double>(L);  // precompute reciprocal of L  

  // Normalised tau
  tau_norm = 1.0 - tADC / tau;

  // Period
  inv_fADC = 1.0 / fADC;

  // Calculate baseline offet
  if (use_fixed_cut){
    baseline_offset = 0;
  }
  else{
    baseline_offset = 0;
    //    baseline_offset = INT16_T_MAX - (baseline_mean + nsigma_baseline_offset*baseline_rms);
    //    threshold_cut += baseline_offset;
    std::cout << "Baseline offset = " << baseline_offset << ". Physical baseline = " << baseline_mean << ", offset baseline = " << baseline_mean + baseline_offset << ", new threshold value = " << threshold_cut << "." << std::endl;
  }
  
}

// Load config file and configure MWD algorithm
void MWD::configure_mwd(const std::string& filename) {

  // Get the config file
  std::ifstream infile(filename);
  if (!infile) {
    std::cerr << "ERROR: Cannot open config file: " << filename << std::endl;
    exit(1);
  }

  // Read file line by line
  std::string line;
  while (std::getline(infile, line)) {
    // Remove inline comments
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }
    
    // Skip blank lines
    if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

    // Now find the keyes
    std::istringstream iss(line);
    std::string key;
    if (std::getline(iss, key, '=')) {
      std::string value_str;
      if (std::getline(iss, value_str)) {
	// Remove whitespace
	key.erase(key.find_last_not_of(" \t\r\n") + 1);
	key.erase(0, key.find_first_not_of(" \t\r\n"));
	value_str.erase(value_str.find_last_not_of(" \t\r\n") + 1);
	value_str.erase(0, value_str.find_first_not_of(" \t\r\n"));

	// And store the values
	try {
	  if (key == "M") M = std::stoi(value_str);
	  else if (key == "L") L = std::stoi(value_str);
	  else if (key == "tau") tau = std::stod(value_str);
	  else if (key == "fADC") fADC = std::stod(value_str);
	  else if (key == "use_fixed_cut") use_fixed_cut = std::stoi(value_str);
	  else if (key == "baseline_mean") baseline_mean = std::stod(value_str);
	  else if (key == "baseline_rms") baseline_rms = std::stod(value_str);
	  else if (key == "nsigma_cut") nsigma_cut = std::stod(value_str);
	  else if (key == "thresholdgrad") thresholdgrad = std::stod(value_str);
	  else if (key == "nsigma_baseline_offset") nsigma_baseline_offset = std::stod(value_str);
	  else if (key == "fixed_cut_value") fixed_cut_value = std::stod(value_str);
	  else if (key == "auxlow_init") auxlow_init = std::stod(value_str);
	  else {
	    std::cerr << "WARNING: Unknown config key: " << key << std::endl;
	    exit(0);
	  }
	} catch (const std::exception& e) {
	  std::cerr << "ERROR: Failed to parse value for key '" << key << "': " << value_str << std::endl;
	}
      }
    }
  }

  // Calculate ADC period
  tADC = 1e3/fADC;
  
  // Calculate threshold_cut value based on cut_mode
  // If using a fixed, user value for the threshold cut
  if (use_fixed_cut) {
    threshold_cut = fixed_cut_value;
  }
  // If using a threshold cut based on baseline calculation
  else {
    threshold_cut = baseline_mean - baseline_rms * nsigma_cut;
  }

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "\n=== Config loaded ===" << std::endl;
  std::cout << "M                      = " << M << std::endl;
  std::cout << "L                      = " << L << std::endl;
  std::cout << "tau                    = " << tau << std::endl;
  std::cout << "fADC (MHz)             = " << fADC << std::endl;
  std::cout << "tADC (ns)              = " << tADC << std::endl;
  std::cout << "used_fixed_cut         = " << (use_fixed_cut ? "true" : "false") << std::endl;
  std::cout << "baseline_mean          = " << baseline_mean << std::endl;
  if (use_fixed_cut){
    std::cout << "fixed_cut_value        = " << fixed_cut_value << std::endl;
  }
  else{
    std::cout << "baseline_rms           = " << baseline_rms << std::endl;
    std::cout << "nsigma_cut             = " << nsigma_cut << std::endl;
    std::cout << "threshold value        = " << threshold_cut << std::endl;
    std::cout << "thresholdgrad          = " << thresholdgrad << std::endl;
  }
  std::cout << "nsigma_baseline_offset = " << nsigma_baseline_offset << std::endl;
  std::cout << "auxlow_init            = " << auxlow_init << std::endl;
  std::cout << "=====================\n" << std::endl;
  
  
}



// MWD algorithm and peak finder
mwd_peaks MWD::calc_mwd(MultiFileHandler& provider){

  // Peak data struct
  mwd_peaks peak_data;

  // Check M and L values are powers of 2
  if ((M & (M - 1)) != 0 || (L & (L - 1)) != 0) {
    std::cerr << "Error: M and L must be powers of 2 for optimized ring buffer access." << std::endl;
    std::exit(1);
  }

  // Current sample
  int16_t sample = 0;
  // Create and fill a vector of all samples from file
  std::vector<int16_t> samples;  
  while (provider.next(sample)) {
    samples.push_back(sample+offset);
  }

  // Print ADC baseline results
  // std::pair<double, double> base = calculate_baseline(samples);
  // std::cout << "Calcuated baseline = " << base.first << " ± " << base.second << std::endl;
  // exit(0);
      
  // Store stotal number of sample
  const size_t total_samples = provider.total_samples();

  // Create vector for differentiated values
  std::vector<int16_t> D(total_samples);
  
  // Create vector of averages after MWD algorithm
  std::vector<int16_t> avgs(total_samples);
  
  // Progress tracking
  auto start_time = std::chrono::steady_clock::now();

  // Deconvolution and differentiation
  deconv_and_diff(samples, D);

  //  mwd_algorithm(samples,avgs);
  //  do_all(samples,peak_data);
  
  // === Progress reporting ===
  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = now - start_time;  
  double samples_per_second = static_cast<double>(samples.size()) / elapsed_seconds.count();
  double gbit_per_second = (samples_per_second * 16.0) / 1e9;  // 16 bits per sample    
  std::cout << " - Deconvolution and differentiation speed: " << std::fixed << std::setprecision(3) << gbit_per_second << " Gbit/s" << std::endl;
  
  // Restart timer
  start_time = std::chrono::steady_clock::now();

  // Averaging
  averaging(D, avgs);
  
  // === Progress reporting ===
  now = std::chrono::steady_clock::now();
  elapsed_seconds = now - start_time;
  samples_per_second = static_cast<double>(samples.size()) / elapsed_seconds.count();
  gbit_per_second = (samples_per_second * 16.0) / 1e9;  // 16 bits per sample
  std::cout << " - Averaging speed: " << std::fixed << std::setprecision(3)
  	    << gbit_per_second << " Gbit/s" << std::endl;
  
  // Restart timer
  start_time = std::chrono::steady_clock::now();

  // Find peaks in averaged data
  find_peaks(avgs, peak_data);
  
  // === Progress reporting ===
  now = std::chrono::steady_clock::now();
  elapsed_seconds = now - start_time;
  samples_per_second = static_cast<double>(samples.size()) / elapsed_seconds.count();
  gbit_per_second = (samples_per_second * 16.0) / 1e9;  // 16 bits per sample    
  std::cout << " - Peak finding speed: " << std::fixed << std::setprecision(3)
  	    << gbit_per_second << " Gbit/s"
  	    << " - Found peaks: " << peak_data.npeaks
  	    << std::endl;
  
  // Log outcome
  std::cout << "MWD + peak finding complete: processed " << samples.size()
	    << " samples, found " << peak_data.npeaks << " peaks." << std::endl;
  
  return peak_data;
  
}

// Deconvolution and differentiation
template <typename T>
void MWD::deconv_and_diff(const std::vector<int16_t>& samples,
			  std::vector<T>& D) {  

  // Variables for deconvolution
  double prev_sample = 0.0;
  double prev_a = 0.0;

  // Open binary output file for ai values as doubles
  const std::string ai_output_filename = "ai_"+std::to_string(offset)+".bin";
  std::ofstream ai_file(ai_output_filename, std::ios::binary);
  if (!ai_file.is_open()) {
    throw std::runtime_error("Failed to open output file for ai values: " + ai_output_filename);
  }

  // Open binary output file for Di values as doubles
  const std::string Di_output_filename = "Di_"+std::to_string(offset)+".bin";
  std::ofstream Di_file(Di_output_filename, std::ios::binary);
  if (!Di_file.is_open()) {
    throw std::runtime_error("Failed to open output file for ai values: " + Di_output_filename);
  }

  
  // Loop over all data
  const size_t n = samples.size(); 
  for (size_t i = 0; i < n; ++i) {
    
    // Cast the value as a double
    const double data_i = static_cast<double>(samples[i]) - baseline_mean;
    
    // Deconvolution: ai = data[i] - (1 - (T0/tau)) * data[i-1] + a[i-1]
    const double ai = data_i - tau_norm * prev_sample + prev_a;

    // Write ai to binary file as double
    ai_file.write(reinterpret_cast<const char*>(&ai), sizeof(double));

    // Get the m index
    const size_t m_index = i & m_mask;
    
    // Get a[i-M] from the ring buffer
    const double aim = (i >= M) ? a_ring[m_index] : 0.0;
    
    // Differentiation: D[i] = ai - a[i-M] (for i >= M), else D[i] = ai
    const double Di = (i < M) ? ai : (ai - aim);
    D[i] = round_if_integral<T>(Di);

    // Write Di to binary file as double
    Di_file.write(reinterpret_cast<const char*>(&Di), sizeof(double));
    
    // Save ai into ring buffer for future use
    a_ring[m_index] = ai;
    
    // Prepare for next sample
    prev_sample = data_i;
    prev_a = ai;
    
  }

  // Close the file after loop
  ai_file.close();
  Di_file.close();
  
}


// Averaging
template <typename T_D, typename T_avg>
void MWD::averaging(const std::vector<T_D>& D,
		    std::vector<T_avg>& avgs) {

  // Open binary output file for avgs values as doubles
  const std::string avg_output_filename = "avg_"+std::to_string(offset)+".bin";
  std::ofstream avg_file(avg_output_filename, std::ios::binary);
  if (!avg_file.is_open()) {
    throw std::runtime_error("Favgled to open output file for avg values: " + avg_output_filename);
  }
  
  
  // Avergaing sum
  double sum = 0.0;

  // Loop over all data
  const size_t n = D.size();
  for (size_t i = 0; i < n; ++i) {

    // Get the l index
    const size_t l_index = i & l_mask;
    
    // Cast the value as a double
    const double Di = static_cast<double>(D[i]);
    
    // Moving average
    double val;
    if (i < L) {
      sum += Di;
      val = (i == L - 1) ? (sum * inv_L) : Di;
    } else {
      sum += Di - D_ring[l_index];
      val = sum * inv_L;
    }

    // Write Di to binary file as double
    avg_file.write(reinterpret_cast<const char*>(&val), sizeof(double));
    
    // Round if necessary
    avgs[i] = round_if_integral<T_avg>(val);
    
    // Save current D[i] into D ring buffer for moving sum
    D_ring[l_index] = Di;
    
  }

  // Close the file after loop
  avg_file.close();

  
}

// Find peaks
template <typename T>
void MWD::find_peaks(const std::vector<T>& avgs, mwd_peaks& peak_data) {
  
  // Initialise parameters to find peaks
  double auxlow = auxlow_init;
  double adc_time = 0.0;
  double peak_min = 0.0;
  double prev_avg = 0.0;
  
  // Loop over all data
  const size_t n = avgs.size();
  for (size_t i = 0; i < n; ++i) {

    // Get average value
    const double avg = static_cast<double>(avgs[i]);

    // Peak finding logic only kicks in after M samples
    // [[likely]] helps branch prediction
    if (i >= M) [[likely]] {  
      if (avg < threshold_cut) {
        if ((avg < prev_avg) && (avg < auxlow)) {
          auxlow = avg;
          adc_time = static_cast<double>(i) * inv_fADC; // in µs
          peak_min = auxlow;
        }
      }

      // Check if a peak has been finalized
      if (auxlow != auxlow_init && avg > threshold_cut) {
        peak_data.npeaks++;
        peak_data.peak_times.push_back(adc_time);  // in µs
        const double peak_height = peak_min - baseline_mean;
        peak_data.peak_heights.push_back(peak_height);
        auxlow = auxlow_init; // Reset for next peak   
      }
    }
    
    // Store this after for next call
    prev_avg = avg;
    
  }
  
}

// The MWD algorithm
template <typename T>
void MWD::mwd_algorithm(const std::vector<int16_t>& samples,
			std::vector<T>& avgs) {  

  // Variables for deconvolution, differentiation, moving average calculation
  double prev_sample = 0.0;
  double prev_a = 0.0;
  double sum = 0.0;
 
  // Loop over all data
  const size_t n = samples.size(); 
  for (size_t i = 0; i < n; ++i) {

    // Cast the value as a double
    const double data_i = static_cast<double>(samples[i]);
    
    // Deconvolution: ai = data[i] - (1 - (T0/tau)) * data[i-1] + a[i-1]
    const double ai = data_i - tau_norm * prev_sample + prev_a;

    // Get the m index and l index
    const size_t m_index = i & m_mask;
    const size_t l_index = i & l_mask;
    
    // Get a[i-M] from the ring buffer
    const double aim = (i >= M) ? a_ring[m_index] : 0.0;
    
    // Differentiation: D[i] = ai - a[i-M] (for i >= M), else D[i] = ai
    const double Di = (i < M) ? ai : (ai - aim);
    
    // Save ai into ring buffer for future use
    a_ring[m_index] = ai;
    
    // Prepare for next sample
    prev_sample = data_i;
    prev_a = ai;

    // Moving average
    double val;
    if (i < L) {
      sum += Di;
      val = (i == L - 1) ? (sum * inv_L) : Di;
    } else {
      sum += Di - D_ring[l_index];
      val = sum * inv_L;
    }

    // Round if necessary
    avgs[i] = round_if_integral<T>(val);
    
    // Save current D[i] into D ring buffer for moving sum
    D_ring[l_index] = Di;
    
  }
  
}

// Find peaks
void MWD::do_all(const std::vector<int16_t>& samples, mwd_peaks& peak_data) {

  // Variables for deconvolution, differentiation, moving average calculation
  double prev_sample = 0.0;
  double prev_a = 0.0;
  double sum = 0.0;
  
  // Initialise parameters to find peaks
  double auxlow = auxlow_init;
  double adc_time = 0.0;
  double peak_min = 0.0;
  double prev_avg = 0.0;
  
  // Loop over all data
  const size_t n = samples.size(); 
  for (size_t i = 0; i < n; ++i) {

    // Cast the value as a double
    const double data_i = static_cast<double>(samples[i]);
    
    // Deconvolution: ai = data[i] - (1 - (T0/tau)) * data[i-1] + a[i-1]
    const double ai = data_i - tau_norm * prev_sample + prev_a;

    // Get the m index and l index
    const size_t m_index = i & m_mask;
    const size_t l_index = i & l_mask;
    
    // Get a[i-M] from the ring buffer
    const double aim = (i >= M) ? a_ring[m_index] : 0.0;
    
    // Differentiation: D[i] = ai - a[i-M] (for i >= M), else D[i] = ai
    const double Di = (i < M) ? ai : (ai - aim);
    
    // Save ai into ring buffer for future use
    a_ring[m_index] = ai;
    
    // Prepare for next sample
    prev_sample = data_i;
    prev_a = ai;

    // Moving average
    double avg;
    if (i < L) {
      sum += Di;
      avg = (i == L - 1) ? (sum * inv_L) : Di;
    } else {
      sum += Di - D_ring[l_index];
      avg = sum * inv_L;
    }

    // Save current D[i] into D ring buffer for moving sum
    D_ring[l_index] = Di;

    // Peak finding logic only kicks in after M samples
    // [[likely]] helps branch prediction
    if (i >= M) [[likely]] {  
      if (avg < threshold_cut) {
        if ((avg < prev_avg) && (avg < auxlow)) {
          auxlow = avg;
          adc_time = static_cast<double>(i) * inv_fADC; // in µs
          peak_min = auxlow;
        }
      }

      // Check if a peak has been finalized
      if (auxlow != auxlow_init && avg > threshold_cut) {
        peak_data.npeaks++;
        peak_data.peak_times.push_back(adc_time);  // in µs
        const double peak_height = peak_min - baseline_mean;
        peak_data.peak_heights.push_back(peak_height);
        auxlow = auxlow_init; // Reset for next peak   
      }
    }
    
    // Store this after for next call
    prev_avg = avg;
    
  }
  
  
}


// Calcuate adc baseline
std::pair<double, double> MWD::calculate_baseline(const std::vector<int16_t>& samples) {

  const int n = samples.size();

  int i = M;
  int count = 0;
  double sum = 0.0;
  double sum_sq = 0.0;

  while (i < n) {
    double grad = static_cast<double>(samples[i]) - samples[count];

    if (grad < thresholdgrad) {
      i += (M + 2 * L);
    } else {
      double val = samples[i];
      sum += val;
      sum_sq += val * val;
      ++count;
      ++i;
    }
  }

  if (count == 0) {
    return {0.0, 0.0};  // No valid values found
  }

  double mean = sum / count;
  double rms = std::sqrt((sum_sq / count) - (mean * mean));
  return {mean, rms};
}
