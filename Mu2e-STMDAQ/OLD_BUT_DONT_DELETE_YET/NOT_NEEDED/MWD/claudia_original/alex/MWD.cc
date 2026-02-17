#include "MWD.hh"
#include "stats.hh"
#include <chrono>
#include <thread>

MWD::MWD(){
    M = 400;
    L = 200;
    tau = 50000;  // in ns
    nsigma_cut = 1;
    thresholdgrad = -0.5;
    fADC = 370; // in MHz
    T0 = (1000.0/fADC); // in ns
    cut_mode = 2;
    fixed_cut_parameter = -500.0;
}

MWD::MWD(double _M, double _L, double _tau, double _nsigma_cut, double _thresholdgrad, double _fADC, int _cut_mode, double _fixed_cut_parameter){
    M = _M;
    L = _L;
    tau = _tau;
    nsigma_cut = _nsigma_cut;
    thresholdgrad = _thresholdgrad;
    fADC = _fADC;
    T0 = (1000.0/fADC); // in ns 
    cut_mode = _cut_mode;
    fixed_cut_parameter = _fixed_cut_parameter;
}


// MWD algorithm
void MWD::mwd_algorithm(uint64_t n, int16_t* data, double* l){  

  // Deconvolution                                         
  double* D = new double[n];
  double sum = 0;

  double aim1 = data[0];
  double aimMm1 = data[0];
  // Loop over all data
  for(int i = 1; i < n; i++){
    // Deconvolution
    double ai = data[i]-(1-(T0/tau))*data[i-1] + aim1;
    // Differentiation
    if (i < M){
      // First M values are just a[i]
      D[i] = ai;
    }
    else{
      double aimM = data[0];
      if (i - M > 0){
	aimM = data[i-M]-(1-(T0/tau))*data[i-M-1] + aimMm1;
      }
      D[i] = ai - aimM;
      aimMm1 = aimM;
    }
    // Save previous a value
    aim1 = ai;
    // Averaging
    if (i < L){
      sum += D[i];
      if (i == L-1){
	l[i] = sum/L; 
      }
      else{
	// First L-1 values are just D[i]
	l[i] = D[i];
      }
    }
    if (i >= L){
      sum += D[i]-D[i-L];
      l[i] = sum/L;
    }

  }

  delete D;

}

// MWD algorithm
double* MWD::mwd_algorithm_original(uint64_t n, int16_t* data){  

  // Deconvolution                                         
  double* a = new double[n];
  a[0] = data[0];
  for(int i = 1; i < n; i++){
    a[i] = data[i]-(1-(T0/tau))*data[i-1] + a[i-1];
  }

  // Differentiation
  double* D = new double[n];
  memcpy( D, a, M*sizeof(a) );
  for (int i = M; i < n; ++i) {
    D[i] = a[i] - a[i-M];
  }
  delete a;

  // Averaging     
  double* l = new double[n];
  double sum = 0;
  memcpy( l, D, (L-1)*sizeof(D) );
  for (int i = 0; i < L-1; ++i) {
    sum += D[i];
  }
  sum += D[L-1];
  l[L-1] = sum/L;
  for (int i = L; i < n; ++i) {
    sum += D[i]-D[i-L];
    l[i] = sum/L;
  }

  delete D;

  return l;
  
}

// Find peaks
MWD_peaks MWD::find_peaks(uint64_t n, 
			  double* l,
			  double baseline_mean, 
			  double baseline_rms, 
			  double time_offset) {

  // Create MWD peaks struct
  MWD_peaks peak_data;

  // If using a threshold cut based on dynamic basline calculation
  if (cut_mode == 1)
    threshold_cut = baseline_mean - baseline_rms*nsigma_cut;
  // If using a fixed threshold cut
  else if (cut_mode == 2)
    threshold_cut = fixed_cut_parameter;
  else {
    std::cout << "ERROR:: find_peaks() : unknown cut mode .... " << cut_mode << std::endl;
    exit(-1);
  }

  double e2 = 0;
  double energy = 0;
  //4000 so that l values are always lower at the start of the peaks 
  double auxlow = 4000.0;
  double timeaux = 0;
  

  // Loop from window to number of averaged points
  for(int i = M; i < n; i++){
    // If the average is below the threshold cut
    if (l[i] < threshold_cut){
      // If this average is lower than the last and lower than auxlow
      if ((l[i] < l[i-1]) && l[i] < auxlow){
	// Set auxlow to the average
	auxlow = l[i];
	// Adc Time
	timeaux = time_offset + ((double) i)/( fADC  ) ; // in us
	e2 = auxlow;
	
      }
      else {
	continue;
      }
    }
    
    if (auxlow == 4000) {
      continue;
    }
    else if (l[i] > threshold_cut){
      
      // Increase number of peaks
      peak_data.npeaks++;
      // Store peak time
      peak_data.peak_times.push_back(timeaux); //us
      // Calculate energy as subtraction from basline mean
      energy = e2 - baseline_mean;
      // Store peak energy
      peak_data.peak_heights.push_back(energy);
      
      auxlow=4000.;
    }
  }

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
