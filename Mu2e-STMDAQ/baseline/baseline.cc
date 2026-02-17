// Baseline code
#include "baseline.hh"

// Constructor
Baseline::Baseline(const std::string& config_filename){
  
  // Load config file and configure baseline algorithm
  configure_baseline(config_filename);
  
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

// Load config file and configure Baseline algorithm
void Baseline::configure_baseline(const std::string& filename) {

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
    // Histogram parameters
    if (key == "nbins") nbins = std::stoi(value_str);    
    else if (key == "min_adc") min_adc = std::clamp(std::stoi(value_str), // Minimum adc value
                                                    static_cast<int>(std::numeric_limits<int16_t>::min()),
                                                    static_cast<int>(std::numeric_limits<int16_t>::max()));
    else if (key == "max_adc") max_adc = std::clamp(std::stoi(value_str), // Minimum adc value
                                                    static_cast<int>(std::numeric_limits<int16_t>::min()),
                                                    static_cast<int>(std::numeric_limits<int16_t>::max()));
    // Expectation-Maximization (EM) parameters
    else if (key == "max_iters") max_iters = std::stoi(value_str);
    else if (key == "tol") tol = std::stod(value_str);
    else if (key == "var_floor") var_floor = std::stod(value_str);
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
  
  // Initialise histogram
  width = (max_adc - min_adc)/nbins;
  invw = 1/width;      
  hist_counts = std::vector<uint64_t>(nbins, 0.0);
  hist_bin_centres = std::vector<double>(nbins, 0.0);
  for (int i = 0; i < nbins; ++i) hist_bin_centres[i] = min_adc + (i + 0.5) * width;

  std::cout << "\n=== Config loaded ===" << std::endl;
  // Histogram parameters
  std::cout << "--- Histogram parameters ---" << std::endl;
  std::cout << "nbins             = " << nbins << std::endl;
  std::cout << "min_adc           = " << min_adc << std::endl;
  std::cout << "max_adc           = " << max_adc << std::endl;
  std::cout << "width             = " << width << std::endl;
  std::cout << "invw              = " << invw << std::endl;  
  // EM parameters
  std::cout << "--- EM parameters ---" << std::endl;
  std::cout << "max_iters         = " << max_iters << std::endl;
  std::cout << "tol               = " << tol << std::endl;
  std::cout << "var_floor         = " << var_floor << std::endl;
  std::cout << "=====================\n" << std::endl;
  
}


// Fill histogram of ADC data
std::pair<const std::vector<uint64_t>&, const std::vector<double>&> Baseline::fill_hist(const std::vector<int16_t>& ADC) {
  
  // The adc length
  const size_t n = ADC.size();
  
  // Fill histogram
  for (size_t i = 0; i < n; ++i) {
    const double sample = static_cast<double>(ADC[i]);
    
     
    // // Underflow
    // if (sample < min_adc){
    //   std::string out = "Baseline: Error! ADC data has value " +
    //     std::to_string(sample) +
    //     " which is smaller than the config min ADC value of " +
    //     std::to_string(min_adc) + ".";
    //   std::cout << out << std::endl;
    //   exit(0);
    // }
    
    // Overflow
    if (sample > max_adc){
      std::string out = "Baseline: Error! ADC data has value " +
        std::to_string(sample) +
        " which is larger than the config max ADC value of " +
        std::to_string(max_adc) + ".";
      std::cout << out << std::endl;
      exit(0);
    }

    
    int bin = int((sample - min_adc) * invw);
    if (bin < 0) bin = 0;
    if (bin >= nbins) bin = nbins - 1;
    hist_counts[bin] += 1.0;
    total_hist_counts += 1.0;
  }

  return {hist_counts, hist_bin_centres};
  
}

// Expectation–Maximization (EM) Algorithm
// Gaussian (baseline) + one-sided exponential tail (pulses)
std::pair<std::tuple<double,double,double>,std::tuple<double,double,double>> Baseline::EM_algorithm() {

  // Histogram Mode
  const int mode_bin = get_hist_mode_idx(hist_counts); // Mode
  // ADC value at mode (baseline mean guess)
  const double mu0_init = hist_bin_centres[mode_bin];
  // Baseline mean + 1*sigma (clean side)
  const double p84= hist_percentile(0.84);
  // Baseline sigma guess
  double sigma0_init = std::max(1e-3, p84 - mu0_init);
  // Ensure sigma is > 0
  if (!(sigma0_init > 0.0)){
      sigma0_init = std::max(1.0, (max_adc - min_adc) / (2.0 * nbins));
  }
  
  // Guess mixture weights 
  double w0 = 0.6; // baseline weight
  double w1 = 0.4; // tail weight

  // Baseline Gaussian parameters
  double mu0  = mu0_init;
  double var0 = sigma0_init * sigma0_init;

  // One-sided exponential params
  // Cutoff t anchored at the baseline mean - re-anchored each iteration.
  double t = mu0_init;

  // Initial lambda (1/scale). Rough guess from tail reach.
  double lambda = 1.0 / std::max(50.0, (t - min_adc) / 6.0);

  // Parameter floors / bounds for numerical stability
  const double var_floor = this->var_floor; // existing floor
  const double lambda_min = 1e-6; // ~ very long tail
  const double lambda_max = 1e+1; // ~ very short tail

  // Previous log likeliehood
  double prev_ll = -std::numeric_limits<double>::infinity();
  // Convergence bool
  bool convergence = false;

  // Get total counts
  const double total = total_hist_counts;

  // Loop over number of EM iterations
  for (int iter = 0; iter < max_iters; ++iter) {
    
    // Stats
    // Effective counts for gaussian / exponential
    double n0 = 0.0, n1 = 0.0;       
    double sum0 = 0.0; // ∑ r0 c x
    double sumxx0 = 0.0; // ∑ r0 c x^2
    double sum_x_minus_t = 0.0; // ∑ r1 c (x - t) 
    double ll = 0.0; // log-likelihood

    // Expectation step

    // Loop over histogram bins
    for (int i = 0; i < nbins; ++i) {

      // Get counts in bins
      const double c = hist_counts[i];

      // If no counts, continue
      if (c == 0.0) continue;

      // Get bin centre
      const double x = hist_bin_centres[i];

      // Baseline Gaussian likelihood (with variance floor)
      const double p0 = w0 * gaussian_pdf(x, mu0, std::max(var0, var_floor));

      // One-sided exponential likelihood; zero for x > t
      double p1 = 0.0;
      if (x <= t) {
        // f_exp(x|lambda,t) = lambda * exp(lambda * (x - t)), x <= t
        // Multiply by mixture weight
        p1 = w1 * lambda * std::exp(lambda * (x - t));
      }

      // Total mixture density (+tiny epsilon for log/ratio safety)
      const double den = p0 + p1 + 1e-300;

      // Responsibilities 
      const double r0 = p0 / den;
      const double r1 = 1.0 - r0;

      // Convert to effective counts
      const double cr0 = c * r0;
      const double cr1 = c * r1;

      // Accumulate stats
      n0 += cr0;
      n1 += cr1;
      sum0   += cr0 * x;
      sumxx0 += cr0 * x * x;

      // If (x - t) ≤ 0
      if (x <= t) {
        // Sum num is negative --> will produce positive lambda below.
        sum_x_minus_t += cr1 * (x - t);
      }

      // Log-likelihood contribution (histogram-weighted)
      ll += c * std::log(den);
      
    } // End loop over bins

    // Maxmimisationstep
    
    // Guard against degeneracy
    if (n0 < 1e-12 || n1 < 1e-12) {
      n0 = std::max(n0, 1e-12);
      n1 = std::max(n1, 1e-12);
    }

    // Update mixture weights
    w0 = n0 / total;
    w1 = n1 / total;

    // Update Gaussian mean / variance
    mu0  = sum0 / n0;
    var0 = std::max(var_floor, (sumxx0 / n0) - mu0 * mu0);

    // Update exponential rate 
    // lambda = - N1 / Σ r1 c (x - t), with Σ r1 c (x - t) < 0
    {
      const double denom = sum_x_minus_t; // negative or ~0
      if (denom < -1e-12) {
        lambda = - n1 / denom;
      }
      // Avoid runaway
      if (!(lambda > 0.0)) lambda = 1.0 / 300.0;
      lambda = std::min(std::max(lambda, lambda_min), lambda_max);
    }

    // Re-anchor exponential cutoff to the current baseline
    t = mu0;

    // Convergence check 
    if (std::abs(ll - prev_ll) < std::max(1.0, std::abs(ll)) * tol) {
      // If converged, break
      convergence = true;
      break;
    }

    // Store last LL
    prev_ll = ll;
    
  }

  // If not converged, hard exit
  if (!convergence) {
    std::cout << "[Baseline] EM (Gauss+Exp) did not converge in " << max_iters << std::endl;
    exit(0);
  }

  // Convert exponential parameters to (mean, rms)
  // For f(x|lambda,t) on (-inf, t]: mean  = t - 1/lambda, sigma = 1/lambda
  const double sigma0 = std::sqrt(var0);
  const double mu_exp = t - 1.0 / lambda;
  const double sig_exp = 1.0 / lambda;

  // Return outputs
  return {{w0, mu0, sigma0}, {w1, t, lambda}};
}

// Estimate percentile position from histogram
double Baseline::hist_percentile(double p01) {

  // Total number of counts to accumulate to reach the desired percentile.
  const double target = p01 * total_hist_counts;

  // Cumulative count tracker
  double s = 0.0;                        

  // Loop through bins from low to high ADC values
  for (int i = 0; i < nbins; ++i) {

    // Cumulative count before this bin
    double prev = s;

    // Add this bin’s counts
    s += hist_counts[i];                  
 
    // Once the cumulative sum passes the target percentile count,
    // we've found the bin containing the percentile.
    if (s >= target) {
      // Fractional position within this bin between its start and end
      //      const double frac = (target - prev) / std::max(1e-12, hist_counts[i]);
      const double frac = (target - prev) /
        std::max(1e-12, static_cast<double>(hist_counts[i]));
      
      // Compute corresponding ADC value:
      return hist_bin_centres[i] - 0.5 * width + frac * width;
    }
  }

  // If the loop never hits the target (e.g., due to rounding),
  // return the center of the last bin as a fallback.
  return hist_bin_centres.back();

}


