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

// Include multi file handler code
#include "multi_file_handler.hh"
// Include MWD code
#include "MWD.hh"

// Filesystem namespace
namespace fs = std::filesystem;

// Main function (takes in argmuments)
int main(int argc, char *argv[]){

  // Check for correct number of arguments
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <directory-or-file>" << std::endl;
    return 1;
  }
  
  // Parse command-line arguments
  // The input file or file directory
  std::string path = std::string(argv[1]);
  
  std::cout << "Input path: " << path << std::endl;

  // Vector of files to process
  std::vector<std::string> files_to_process;

  // Wrap path as a fs::path
  fs::path fpath(path);
  
  // If path argument is a directory
  if (fs::is_directory(path)) {
    // Log to user
    std::cout << "Detected directory. Scanning for files..." << std::endl;
    // Load the files using the MultiFileHandler
    MultiFileHandler::load_files_from_directory(path, files_to_process);
    // If no files found, log to user and exit
    if (files_to_process.empty()) {
      std::cerr << "No files found in directory: " << path << std::endl;
      return 1;
    }
  }
  // Else if a single file
  else if (fs::is_regular_file(path)) {
    // Log to user
    std::cout << "Detected single file. Adding: " << path << std::endl;
    // Push back single file
    files_to_process.push_back(path);
  }
  // Else, log error and exit
  else {
    std::cerr << "Error: Path is neither a file nor a directory: " << path << std::endl;
    return 1;
  }

  // Initialize the provider with the list of files
  MultiFileHandler provider(files_to_process);

  // ************************************************************  

  //----------------MWD----------------

  // Create MWD class
  MWD mwd("mwd_config.txt");

  // Peak data buffer
  mwd_peaks peak_data = mwd.calc_mwd(provider);

  // Get the file path directory
  std::filesystem::path dir = fpath.parent_path();
  std::string dir_str = dir.string();
  
  // Output text file containing peaks
  std::ofstream myfile1;
  myfile1.open(dir_str+"/peaks.txt");

  // Open binary file for output
  std::ofstream myfile2;
  //  myfile2.open(dir_str+"/peaks_heights.bin", std::ios::binary);
  myfile2.open("peaks_heights.bin", std::ios::binary);
  
  // Get number of peaks found
  int npeaks = peak_data.npeaks;
  
  // Loop over all peaks
  for(int i = 0 ; i < npeaks ; i++){    
    
    // Get the peak time and  height in ADC counts
    double time = peak_data.peak_times.at(i);
    double height = peak_data.peak_heights.at(i);    
    // Cast to int16_t
    int16_t height_int16 = static_cast<int16_t>(height);
        
    // Write to file (peak no, peak time, peak height(ADCs)
    myfile1 << i << ","<< time << "," << height <<std::endl;
    
    // Write binary data
    //    myfile2.write(reinterpret_cast<const char*>(&height_int16), sizeof(int16_t));
    myfile2.write(reinterpret_cast<const char*>(&time), sizeof(double));
    myfile2.write(reinterpret_cast<const char*>(&height), sizeof(double));

  }
  
  // Close file2
  myfile1.close();
  myfile2.close();

  return 0;
}

