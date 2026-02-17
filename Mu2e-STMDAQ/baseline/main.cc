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
#include <filesystem> 

// Include baseline code
#include "baseline.hh"

// Filesystem namespace
namespace fs = std::filesystem;

// Main function (takes in argmuments)
int main(int argc, char *argv[]){

  // Check for correct number of arguments
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }
  
  // Get the filename from the argument
  std::string filename  = std::string(argv[1]);
  
  // Print for user
  std::cout << "filename = " << filename << std::endl;
  
  // Get the size of the binary file
  struct stat st;
  stat(filename.c_str(), &st);
  uint64_t file_size = st.st_size; // bytes

  // Get the number of ADC values
  size_t n = file_size / sizeof(int16_t);

  // Print for user
  std::cout << "File size = " << static_cast<double>(file_size) * 1e-9 << " Gbytes" << std::endl;
  std::cout << "Number of ADC values = " << n << std::endl;

  // Open the binary file
  std::ifstream myFile(filename, std::ios::binary);
  if (!myFile) {
    std::cerr << "Error: cannot open file " << filename << std::endl;
    return 1;
  }

  // Create vector and read data
  std::vector<int16_t> ADC(n);
  myFile.read(reinterpret_cast<char*>(ADC.data()), n * sizeof(int16_t));
  myFile.close();

  // Optional: check read size
  if (myFile.gcount() != static_cast<std::streamsize>(n * sizeof(int16_t))) {
    std::cerr << "Warning: read " << myFile.gcount() << " bytes (expected "
              << n * sizeof(int16_t) << ")" << std::endl;
  }
  
  // Data volume in *bits* attributed to each function (your requirement).
  const double data_bits = static_cast<double>(ADC.size()) * sizeof(int16_t) * 8.0; // 16 bits/sample
  

  // ************************************************************  

  //---------------- Baseline ----------------

  // Create baseline class
  Baseline b("baseline_config.txt");
  
  // Histogram data
  // --- Time fill_hist(ADC) ---
  auto t0 = std::chrono::steady_clock::now();
  auto [counts, centres] = b.fill_hist(ADC);
  auto t1 = std::chrono::steady_clock::now();
  double fill_seconds = std::chrono::duration<double>(t1 - t0).count();
  double fill_gbit_per_s = (data_bits / fill_seconds) / 1e9;
  
  // Perform Expectation–Maximization (EM) algorithm
  // --- Time EM_algorithm() ---
  t0 = std::chrono::steady_clock::now();
  auto [baseline, pulses] = b.EM_algorithm();
  t1 = std::chrono::steady_clock::now();
  double em_seconds = std::chrono::duration<double>(t1 - t0).count();
  double em_gbit_per_s = (data_bits / em_seconds) / 1e9;

  // Get components
  auto [w0, mu0, sigma0] = baseline; // Gaussian
  auto [w1, t, lambda] = pulses; // Exponential

  // Log to user
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Data size attributed per function: " << (data_bits / 8.0 / 1e6) << " MB\n";
  std::cout << "fill_hist throughput: " << fill_gbit_per_s << " Gbit/s  (time: " << fill_seconds << " s)\n";
  std::cout << "EM_algorithm throughput: " << em_gbit_per_s << " Gbit/s  (time: " << em_seconds << " s)\n";
  
  std::cout << "Baseline Gaussian: weight = " << w0 << ", mean = " << mu0 << " sigma = " << sigma0 << std::endl;
  std::cout << "Pulses Exponential: weight = " << w1 << ", t = " << t << ", lambda = " << lambda << std::endl;
  
  // Output text file containing peaks
  std::ofstream myfile1;
  myfile1.open("histogram.txt");
  
  // Loop over bins
  for(int i = 0; i < counts.size(); i++){    
    
    // Write to file 
    myfile1 << centres[i] << "," << counts[i] << std::endl;
    
  }

  // Write baseline gaussian parameters
  myfile1 << w0 << "," << mu0 << "," << sigma0 << std::endl;
  
  // Write pulse exponential paramaters
  myfile1 << w1 << "," << t << "," << lambda << std::endl;
  
  // Close file2
   myfile1.close();

  return 0;
}

