#include <chrono>
#include <thread>
#include <iomanip>
#include <iomanip>

// Include MWD code
#include "MWD.hh"
// Include stats code
#include "stats.hh"

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
  std::cout << "M                 = " << M << std::endl;
  std::cout << "L                 = " << L << std::endl;
  std::cout << "tau               = " << tau << std::endl;
  std::cout << "fADC (MHz)        = " << fADC << std::endl;
  std::cout << "tADC (ns)         = " << tADC << std::endl;
  std::cout << "used_fixed_cut    = " << (use_fixed_cut ? "true" : "false") << std::endl;
  std::cout << "baseline_mean     = " << baseline_mean << std::endl;
  if (use_fixed_cut){
    std::cout << "fixed_cut_value   = " << fixed_cut_value << std::endl;
  }
  else{
    std::cout << "baseline_rms      = " << baseline_rms << std::endl;
    std::cout << "nsigma_cut        = " << nsigma_cut << std::endl;
    std::cout << "threshold value   = " << threshold_cut << std::endl;
    std::cout << "thresholdgrad     = " << thresholdgrad << std::endl;
  }
  std::cout << "auxlow_init       = " << auxlow_init << std::endl;
  std::cout << "=====================\n" << std::endl;
  
}



// MWD algorithm and peak finder
mwd_peaks MWD::calc_mwd(MultiFileHandler& provider){

  // Ring buffer for a[i] values
  std::vector<double> a_ring(M, 0.0);
  // Ring buffer for D[i] values
  std::vector<double> D_ring(L, 0.0);  

  // Current sample
  int16_t sample = 0;
  // Previous sample
  double prev_sample = 0.0;
  // Track previous a value
  double prev_a = 0.0;
  // Track previous average
  double prev_avg = 0.0;  
  // Running sum for moving average
  double sum = 0.0;  

  // Counters
  size_t i = 0;
  
  // Prepare peak-finding variables
  double auxlow = auxlow_init; 
  double adc_time = 0.0;
  double peak_min = 0.0;

  // Peak data struct
  mwd_peaks peak_data;

  // Progress tracking
  auto start_time = std::chrono::steady_clock::now();
  auto last_print_time = start_time; 
  const size_t total_samples = provider.total_samples();  
  
  // Loop through all samples
  while (provider.next(sample)) {

    // Cast the value as a double
    double data_i = static_cast<double>(sample);
    
    // Deconvolution: ai = data[i] - (1 - (T0/tau)) * data[i-1] + a[i-1]
    double ai = data_i - (1.0 - (tADC / tau)) * prev_sample + prev_a;
    
    // Get a[i-M] from the ring buffer
    double aim = (i >= M) ? a_ring[i % M] : 0.0;
    
    // Differentiation: D[i] = ai - a[i-M] (for i >= M), else D[i] = ai
    double Di = (i < M) ? ai : (ai - aim);
    
    // Save ai into ring buffer for future use
    a_ring[i % M] = ai;
    
    // Moving average
    double avg = 0.0;
    if (i < L) {
      sum += Di;      
      if (i == L - 1) {
	avg = sum / L;
      }
      else {	
	// First L-1 values are just D[i]
	// Not yet fully averaged
	avg = Di;
      }
    }
    else {
      sum += Di - D_ring[i % L];
      avg = sum / L;
    }
    
    // Save current D[i] into D ring buffer for moving sum
    D_ring[i % L] = Di;
        
    // === Peak finding logic ===
    // Loop from window to number of averaged points 
    if (i >= M) {
      // If the average is below the threshold cut
      if (avg < threshold_cut) {
	// If this average is lower than the last and lower than auxlow
	if ((avg < prev_avg) && (avg < auxlow)) {
	  // Set auxlow to the average
	  auxlow = avg;
	  // Adc Time
	  adc_time = (static_cast<double>(i)) / fADC;  // in us
	  peak_min = auxlow;
	}
      }

      // Store found peak
      if (auxlow != auxlow_init && avg > threshold_cut) {
	 // Increase number of peaks
	peak_data.npeaks++;
	// Store peak time
	peak_data.peak_times.push_back(adc_time);  // us
	// Calculate energy as subtraction from basline mean
	double peak_height = peak_min - baseline_mean;
	// Store peak energy
	peak_data.peak_heights.push_back(peak_height);
	auxlow = auxlow_init;  // Reset for next peak
      }
    }
        
    // Prepare for next sample
    prev_sample = data_i;
    prev_a = ai;
    prev_avg = avg;
    ++i;

    // === Progress reporting ===
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_since_last_print = now - last_print_time;
    if (time_since_last_print.count() >= 5.0) {
      std::chrono::duration<double> elapsed_seconds = now - start_time;
      
      double samples_per_second = static_cast<double>(i) / elapsed_seconds.count();
      double gbit_per_second = (samples_per_second * 16.0) / 1e9;  // 16 bits per sample
      
      size_t samples_remaining = (total_samples > 0) ? (total_samples - i) : 0;
      double seconds_remaining = (samples_per_second > 0) ? (samples_remaining / samples_per_second) : 0;
      
      // Convert to h/m/s
      int elapsed_h = static_cast<int>(elapsed_seconds.count()) / 3600;
      int elapsed_m = (static_cast<int>(elapsed_seconds.count()) % 3600) / 60;
      int elapsed_s = static_cast<int>(elapsed_seconds.count()) % 60;
      
      int eta_h = static_cast<int>(seconds_remaining) / 3600;
      int eta_m = (static_cast<int>(seconds_remaining) % 3600) / 60;
      int eta_s = static_cast<int>(seconds_remaining) % 60;
      
      std::cout << "Processed " << i << " samples"
		<< " - Elapsed: " << elapsed_h << "h " << elapsed_m << "m " << elapsed_s << "s";
      
      if (total_samples > 0) {
	std::cout << " - ETA: " << eta_h << "h " << eta_m << "m " << eta_s << "s";
      }
      
      std::cout << " - Speed: " << std::fixed << std::setprecision(3)
          << gbit_per_second << " Gbit/s"
          << " - Found peaks: " << peak_data.npeaks
          << std::endl;
      
      last_print_time = now;
      
    }
    
  }

  std::cout << "MWD + peak finding complete: processed " << i
	    << " samples, found " << peak_data.npeaks << " peaks." << std::endl;
  
  return peak_data;
}





// Calcuate adc baseline
std::vector<double> MWD::calculate_baseline(uint64_t n, double* l){

  int k = M;

  double* gradient = new double[n];
  double* lvalues = new double[n];

  int ilv = 0;

  //Remove peaks and calculate MWD baseline mean
  while (k < n){
    gradient[k] = l[k+1] - l[k];

    if(gradient[k] < thresholdgrad){
      k = k + (M+2*L);
      continue;
    }
    else {
      lvalues[ilv] = l[k];
      ilv++;
      k++;
    }
  }
  double mean = stats::mean(lvalues,ilv);
  double rms = stats::rms(lvalues,mean,ilv);
  std::vector<double> result;

  result.push_back(mean);
  result.push_back(rms);

  delete lvalues;
  delete gradient;

  return result;
}
