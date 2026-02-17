#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <sys/stat.h>
#include <chrono>

#include "STMDAQ-TestBeam/MWD/MWD.hh"

// Main function (takes in argmuments)
int main(int argc, char *argv[]){

  // The input filename
  std::string  filename = std::string(argv[1]);
  // MWD M-value
  int M = atoi(argv[2]);
  // MWD L-value
  int L = atoi(argv[3]);
  // Decay time constant
  double tau = std::stod(argv[4]);

  std::chrono::high_resolution_clock::time_point t1 ;
  std::chrono::high_resolution_clock::time_point t2 ;

  //************************************************************
  //******************CONFIGURATION VARIABLES******************
  //************************************************************
  // ------------- Options and parameters to change -------------  

  // MWD::MWD
  double nsigma_cut = 1; //Used if cutmode = 1
  double thresholdgrad = -0.5; //Used for calculating the baseline
  double fADC = 370; //MHz
  int cut_mode = 2; //Cutmode (1 using sigma, 2 using a fixed cut)
  double fixed_cut_parameter = -500; //Used if cutmode = 2 

  // MWD::find_peaks
  double mean_baseline = 0;
  double rms_baseline = 0; //This is for cutmode 1
  // Time of the ADC0 value for each trigger, 0 in our case  
  double time_offset = 0; 

  // ************************************************************  

  // Call MWD algorithm
  
  // Get the size of the binary file
  struct stat st;
  stat(filename.c_str(), &st);
  uint64_t file_size = st.st_size; // bytes

  // Get the number of ADC values
  uint64_t n = file_size/sizeof(int16_t);

  // Open the binary file with data
  std::ifstream myFile;
  int16_t* data = new int16_t[n];
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) data, n*sizeof(data[0]));
  myFile.close();
    
  //----------------MWD----------------

  // Create MWD class
  // M, L, tau, nsigma_cut (we dont use this, it is for cutmode 1),
  // thresholdgrad (we dont use this, it is just for calculating the baseline)
  // fadc, cutmode (1 using sigma, 2 using a fixed cut), fixed_cut_parameter 
  //  MWD *mwd = new MWD(M,L,tau,nsigma_cut,thresholdgrad,fADC,cut_mode,fixed_cut_parameter);
  MWD *mwd = new MWD();

  // MWD data buffer
  double* l = new double [n] ();

  // Peak data buffer
  MWD_peaks peak_data;

  // Save algorithm start time
  t1 = std::chrono::high_resolution_clock::now();

  // Do MWD algorithm
  mwd->mwd_algorithm(n,data,l);

  // Calculate MWD_baseline
  std::vector<double> baseline = mwd->calculate_baseline(n,l);
  mean_baseline = baseline[0];
  rms_baseline = baseline[1];
  std::cout << "Baseline = " << mean_baseline << " ± " << rms_baseline << std::endl; 
  
  // Find peaks from MWD output
  peak_data = mwd->find_peaks(n,l,mean_baseline, rms_baseline, time_offset);

  // Save stop time
  t2 = std::chrono::high_resolution_clock::now();

  // Get processing time 
  double time_ms = std::chrono::duration<double, std::milli>(t2-t1).count();

  // Get data processing speed in Gbit/s                                   
  double speed = n*sizeof(int16_t)*8*1e-9/(time_ms*1e-3);

  // Print for user
  std::cout << "MWD Algorithm computing time = " << time_ms << " ms" << std::endl;
  std::cout << "MWD Algorithm computing speed = " << speed << " Gbit/s" << std::endl;
  
  // Output text file containing peaks
  std::ofstream myfile;
  myfile.open ("peaks.txt");

  // Get number of peaks found
  int npeaks = peak_data.npeaks;

  // Loop over all peaks
  for(int i = 0 ; i < npeaks ; i++){    

    // Get the peak height in ADC counts
    double time = peak_data.peak_times.at(i);
    double height = peak_data.peak_heights.at(i);    

    // Write to file (peak no, peak time, peak height(ADCs), peak energy (keV)
    myfile << i << ","<< time << "," << height << "," << height*(-0.57) <<std::endl;

  }

  // Close file
  myfile.close();

  return 0;
}

