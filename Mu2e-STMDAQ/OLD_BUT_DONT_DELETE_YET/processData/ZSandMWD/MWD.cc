#include "MWD.hh"
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

    // Allocate array to save last M values from the previous call
    for (int i = 0; i < CHNUM; i++){
      prev_M[i] = new double [M] ();
      prev_L[i] = new double [L] ();
    }
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

// MWD deconvolution thread
void MWD::deconv_thread(int chan, queue_zs<int16_t> &pullq, queue_zs<double> &pushq, bool *timeout){

  std::cout << "In MWD deconvolution thread" << std::endl;

  // The return value
  int retval = 0;

  // Data buffers
  data_struct<int16_t> pull_buffer;
  data_struct<double> push_buffer;

  // While still data to pull
  while(!*timeout){
    // Pull data from queue
    retval = pullq.pull(timeout,chan,pull_buffer);
    if (retval > 0){
      // Perform MWD deconvolution
      push_buffer = deconvolute(chan,pull_buffer);
      // Push data to qeue
      pushq.push(chan,push_buffer);
    }
  }

}

// MWD differentiation thread
void MWD::diff_thread(int chan, queue_zs<double> &pullq, queue_zs<double> &pushq, bool *timeout){

  std::cout << "In MWD differentiation thread" << std::endl;

  // The return value
  int retval = 0;

  // Data buffer
  data_struct<double> buffer;

  // While still data to pull
  while(!*timeout){
    // Pull data from queue
    retval = pullq.pull(timeout,chan,buffer);
    if (retval > 0){
      // Perform MWD differentiation
      differentiate(chan,buffer);
      // Push data to qeue
      pushq.push(chan,buffer);
    }
  }


}

// MWD avergaing thread
void MWD::avg_thread(int chan, queue_zs<double> &pullq, queue_zs<double> &pushq, bool *timeout){

  std::cout << "In MWD averaging thread" << std::endl;

  // The return value
  int retval = 0;

  // Data buffer
  data_struct<double> buffer;

  // While still data to pull
  while(!*timeout){
    // Pull data from queue
    retval = pullq.pull(timeout,chan,buffer);
    if (retval > 0){
      // Perform MWD averaging
      average(chan,buffer);
      // Push data to qeue
      pushq.push(chan,buffer);
    }
  }


}


// MWD Deconvolution
data_struct<double> MWD::deconvolute(int chan, data_struct<int16_t> &datas){  

  // The adc length
  uint64_t n = datas.data_len;

  // Define a pointer to the adc data
  int16_t *data = datas.adc_data;

  // Create data struct
  data_struct<double> deconv_data;
  deconv_data.count = datas.count;
  deconv_data.start_index = datas.start_index;
  deconv_data.pulse_index = datas.pulse_index;
  deconv_data.header_data = datas.header_data;
  deconv_data.data_len = datas.data_len;
  deconv_data.adc_data = new double [n] ();

  // Loop start
  uint start = 0;

  // If the this is the first deconvolution call
  if (first_deconv[chan]){
    // The first value is just the data
    deconv_data.adc_data[0] = data[0];
    // Store this as the last data point
    last_deconv_data[chan] = data[0];
    // Store this as the last deconvolution value
    last_a[chan] = data[0];
    // Start the loop from 1
    start = 1;
    // Set first deconvolution to false
    first_deconv[chan] = false;
  }
  
  // Loop over data
  for(uint64_t i = start; i < n; i++){
    // Calculate deconvolution
    double a_double = data[i]-(1-(T0/tau))*last_deconv_data[chan] + last_a[chan];
    // Round to nearest int
    deconv_data.adc_data[i] = a_double;
    // Store the last deconvolution value
    last_deconv_data[chan] = data[i];
    // Store the last deconvolution value
    last_a[chan] = a_double;
  }    

  std::cout << "End = " << n-1 << std::endl;

  return deconv_data;

}


// MWD Deconvolution
void MWD::differentiate(int chan, data_struct<double> &datas){  

  // The adc length
  uint64_t n = datas.data_len;

  // Define a pointer to the adc data
  double *data = datas.adc_data;

  // Array for differentiated values
  double* D = new double[n];

  // Loop over all data
  for(int i = 0; i < n; i++){
    // If is less than the M value
    if (i < M){
      // If the this is the first differentiation call
      if (first_diff[chan]){
	// First M values are just a[i]
	D[i] = data[i];	
	// Set first deconvolution to false
	if (i == M-1) first_diff[chan] = false;
      }
      // Else if  not first differentation call
      else{
	// Get data from previous call
	D[i] = data[i] - prev_M[chan][i];
      }
    }
    // Else if i >= M
    else{
      D[i] = data[i] - data[i-M];
    }
  }

  // Save last M values for next call
  memcpy(&prev_M[chan][0],&data[n-M],M*sizeof(double));

  // Overwrite data
  memcpy(&datas.adc_data[0],&D[0],n*sizeof(double));

  // Delete D
  delete D;

}


// MWD Averaging
void MWD::average(int chan, data_struct<double> &datas){  

  // The adc length
  uint64_t n = datas.data_len;

  // Define a pointer to the adc data
  double *data = datas.adc_data;

  // Array for averaged values
  double* l = new double[n];

  // Loop over all data
  for(int i = 0; i < n; i++){
    // If is less than the L value
    if (i < L){
      // If the this is the first averagin call
      if (first_avg[chan]){
	// First L values are just D[i]
	avg_sum[chan] += data[i];
	l[i] = data[i];	
	// If the last of the L values
	if (i == L-1){
	  l[i] = avg_sum[chan]/L;
	  // Set first deconvolution to false
	  first_avg[chan] = false;
	}
      }
      // Else if  not first differentation call
      else{
	// Get data from previous call
	avg_sum[chan] += data[i] - prev_L[chan][i];
	l[i] = avg_sum[chan]/L;
      }
    }
    // Else if i >= L
    else{
      avg_sum[chan] += data[i] - data[i-L];
      l[i] = avg_sum[chan]/L;
    }
  }

  // Save last L values for next call
  memcpy(&prev_L[chan][0],&data[n-L],L*sizeof(double));

  // Overwrite data
  memcpy(&datas.adc_data[0],&l[0],n*sizeof(double));

  // Delete l
  delete l;

}

// MWD algorithm
double* MWD::mwd_algorithm_original(uint64_t n, int16_t* data){  

  // MWD array
  double* l = new double[n];

  // Deconvolution                                         
  double* a = new double[n];
  a[0] = data[0];
  for(int i = 1; i < n; i++){
    a[i] = data[i]-(1-(T0/tau))*data[i-1] + a[i-1];
  }

  return a;

  // Differentiation
  double* D = new double[n];
  memcpy( D, a, M*sizeof(a) );
  for (int i = M; i < n; ++i) {
    D[i] = a[i] - a[i-M];
  }
  delete a;

  return D;
  
  // Averaging     
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
  threshold_cut = baseline_mean - baseline_rms*nsigma_cut;

  double e2 = 0;
  double energy = 0;
  //4000 so that l values are always lower at the start of the peaks 
  double auxlow = 4000.0; // XML
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
std::pair<double,double> MWD::calculate_baseline(uint64_t n, double* l){

  // Baseline variables
  double mean = 0;
  double rms = 0;
  int count = 0;

  // Start the loop from M
  int i = M;

  // While i < total
  while (i < n){
    // Calculate the gradient
    double gradient = l[i+1] - l[i];
    // If in a peak region
    if(gradient < thresholdgrad){
      // Advance
      i += (M+2*L);
      continue;
    }
    // If not in a peak region
    else {
      // Sum mean
      mean += l[i];
      // Sum rms
      rms += l[i]*l[i];
      // Sum baseline count
      count++;
      // Increment counter
      i++;
    }
  }
 
  // Calc mean
  mean /= count;
  // Calc rms
  rms = sqrt(rms/count - mean*mean);

  // Return mean and rms
  return {mean,rms};

}
