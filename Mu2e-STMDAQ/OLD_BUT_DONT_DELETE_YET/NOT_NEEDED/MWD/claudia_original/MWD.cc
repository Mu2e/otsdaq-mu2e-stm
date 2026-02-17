#include "STMDAQ-TestBeam/MWD/claudia_original/MWD.h"
#include "STMDAQ-TestBeam/MWD/claudia_original/stats.hh"
#include <chrono>
#include <thread>

MWD::MWD(){
    M = 1000;
    L = 500;
    tau = 50000;  // in ns
    nsigma_cut = 9;
    thresholdgrad = -0.3;
    fADC = 320.0520833313; // in MHz
    cut_mode = 1;
    fixed_cut_parameter = -1000.0;
}

MWD::MWD(double _M, double _L, double _tau, double _nsigma_cut, double _thresholdgrad, double _fADC, int _cut_mode, double _fixed_cut_parameter){
    M = _M;
    L = _L;
    tau = _tau;
    nsigma_cut = _nsigma_cut;
    thresholdgrad = _thresholdgrad;
    fADC = _fADC;
    cut_mode = _cut_mode;
    fixed_cut_parameter = _fixed_cut_parameter;
}

MWD::MWD(Xml* xml_file){
  M = xml_file->int_value("stm.mwd_m_parameter",1000);
  L = xml_file->int_value("stm.mwd_l_parameter",500);
  tau = (double) xml_file->float_value("stm.mwd_tau_parameter",50000);
  nsigma_cut = (double) xml_file->float_value("stm.mwd_nsigma_cut_parameter",9.0);
  thresholdgrad = (double) xml_file->float_value("stm.mwd_gradient_parameter",-0.3);
  fADC = (double) xml_file->float_value("stm.adc_time_clock",320.0);
  cut_mode = xml_file->int_value("stm.mwd_cut_mode",500);
  fixed_cut_parameter = (double) xml_file->float_value("stm.mwd_fixed_cut_parameter",-1000.0);
}

std::string MWD::print() {
  std::stringstream ss;
  ss << "M             = " << M << "\n" 
     << "L             = " << L << "\n" 
     << "tau           = " << tau << "\n" 
     << "nsigma_cut    = " << nsigma_cut << "\n" 
     << "fixed_cut     = " << fixed_cut_parameter << "\n" 
     << "cut_mode      = " << cut_mode << "\n" 
     << "nsigma_cut    = " << nsigma_cut << "\n" 
     << "thresholdgrad = " << thresholdgrad << "\n" 
     << "fADC [MHz]    = " << fADC << "\n" ;
  return ss.str();
}

peaks* MWD::find_peaks(double baseline_mean, double baseline_rms, double time_offset) {

  peaks* peak_data = new peaks();
  peak_data->npeaks = 0;

  if (cut_mode == 1)
    threshold_cut = baseline_mean - baseline_rms*nsigma_cut;
  else if (cut_mode == 2)
    threshold_cut = fixed_cut_parameter;
  else {
    std::cout << "ERROR:: find_peaks() : unknown cut mode .... " << cut_mode << std::endl;
    exit(-1);
  }

  if (cut_mode == 1)
    std::cout << "MWD::find_peaks()---SIGMA CUT : baseline mean "<< baseline_mean << " rms = " << baseline_rms << " num_sigmas = " << nsigma_cut 
	    << " threshold cut value = " << threshold_cut << " and M = " << M << " L = " << L << " and MWD cut mode = " << cut_mode << std::endl;
  if (cut_mode == 2)
    std::cout << "MWD::find_peaks()---FIXED CUT : baseline mean "<< baseline_mean << " rms = " << baseline_rms  
	    << " threshold cut value = " << threshold_cut << " and M = " << M << " L = " << L << " and MWD cut mode = " << cut_mode << std::endl;

  int n = nadc;
  double* e2 = new double[n];
  double* energy = new double[n];
  double* timepeak = new double[n];
  double* adc_time = new double[n];
  //4000 so that l values are always lower at the start of the peaks 
  double auxlow = 4000.0 ;
  int counterpeak = 0 ;
  double timeaux = 0;

  for(int i = 0; i < n; i++){
    adc_time[i] = time_offset + ((double) i)/( fADC  ) ; // in us
  }

  for( int i = M; i < n; i++){

    if (l[i] < threshold_cut){

      if ((l[i] < l[i-1]) && l[i] < auxlow){

	auxlow = l[i];
	timeaux = adc_time[i];
	e2[counterpeak] = auxlow;

      }
      else {
	continue;
      }
    }

    if (auxlow == 4000) {
      continue;
    }
    else if (l[i] > threshold_cut){

      peak_data->npeaks++;
      energy[counterpeak] = e2[counterpeak] - baseline_mean;
      peak_data->peak_heights.push_back(energy[counterpeak]);
      peak_data->peak_times.push_back(timeaux); //us

      //std::cout << "Peak[" << counterpeak << "], Time: " << timeaux <<" us" << ", Energy (ADC counts): " << energy[counterpeak] << std::endl;
      auxlow=4000.;
      counterpeak++;
    }
  }
  delete e2; delete energy; delete timepeak; delete adc_time;
  return peak_data;
}

std::vector<double> MWD::calculate_baseline(){

  int k = M;
  int n = nadc;

  //std::cout << "l values in calculate_baseline.... " << l[0] << " " << l[1] << " " << l[2] << std::endl;
  
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
  // std::cout << " Baseline Mean " << mean << " RMS = " << rms << std::endl;
  result.push_back(mean);
  result.push_back(rms);

  delete lvalues;
  delete gradient;

  return result;
}

void MWD::mwd_algorithm(data* adc_values){  // This fill the double[l] array ie sets the private l*
  int n = adc_values->nadc;
  nadc = n;
  //std::cout << "adc_values .... " << adc_values->adc[0] << " " << adc_values->adc[1] << " " << adc_values->adc[2] << std::endl;

  const double T0 = (1000.0/fADC); // in ns 

  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////                 

  //Deconvolution                                                                                                         
  double* a = new double[n];
  a[0] = adc_values->adc[0];
  for(int i=1; i<n; i++){
    a[i] = adc_values->adc[i]-(1-(T0/tau))*adc_values->adc[i-1] + a[i-1];
  }
  //std::cout << "adc_values .... " << adc_values->adc[0] << " " << adc_values->adc[1] << " " << adc_values->adc[2] << std::endl;
  //std::cout << "a values .... " << a[0] << " " << a[1] << " " << a[2] << std::endl;

  //Differentiation                                                                                                       
  double* D = new double[n];
  memcpy( D, a, M*sizeof(a) );

  for (int i = M; i < n; ++i) {
    D[i] = a[i] - a[i-M];
  }
  //std::cout << "adc_values .... " << adc_values->adc[0] << " " << adc_values->adc[1] << " " << adc_values->adc[2] << std::endl;
  //std::cout << "D values[0..2] .... " << D[0] << " " << D[1] << " " << D[2] << std::endl;
  //std::cout << "D values[M..M+2].... " << D[M] << " " << D[M+1] << " " << D[M+2] << std::endl;
  delete a;
  //std::cout << "adc_values .... " << adc_values->adc[0] << " " << adc_values->adc[1] << " " << adc_values->adc[2] << std::endl;

  //Averaging     
  l = new double[n];
  //std::cout << "l values before memcpy.... " << l[0] << " " << l[1] << " " << l[2] << std::endl;
  double sum = 0.;

  //std::cout << "size of l array = " << n << " nbyes being copied = " << (L-1)*sizeof(D) << std::endl;
  memcpy( l, D, (L-1)*sizeof(D) );
  //std::cout << "l values after memcpy.... " << l[0] << " " << l[1] << " " << l[2] << std::endl;
  for (int i = 0; i < L-1; ++i) {
    sum += D[i];
  }
  //std::cout << " sum = " << sum << std::endl;

  sum += D[L-1];
  //std::cout << " sum = " << sum << std::endl;
  l[L-1] = sum/L;
  //std::cout << " l[L-1] = " << l[L-1] << std::endl;

  for (int i = L; i < n; ++i) {
    sum += D[i]-D[i-L];
    l[i] = sum/L;
  }
  //std::cout << " sum = " << sum << std::endl;
  //for (int i = 0; i < 10; i++) {
  //  std::cout << "l[" << i << "] = " << l[i] << std::endl;
  //}
  
  delete D;
}

void MWD::write_l_to_binary_file(std::string filename){
  std::ofstream opfile;
  opfile.open(filename, std::ios::out | std::ios::binary);
  int bytes_to_write = nadc*sizeof(double);
  opfile.write((char *) l, bytes_to_write);
  opfile.close();
}

void MWD::write_adc_to_binary_file(std::string filename, data* adc_values){
  std::ofstream opfile;
  opfile.open(filename, std::ios::out | std::ios::binary);
  int bytes_to_write = nadc*sizeof(adc_values->adc[0]);
  opfile.write((char *) adc_values->adc, bytes_to_write);
}

