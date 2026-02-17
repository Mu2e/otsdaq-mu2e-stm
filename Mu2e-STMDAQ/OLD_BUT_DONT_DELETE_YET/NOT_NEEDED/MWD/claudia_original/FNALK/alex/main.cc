#include "processData.hh"

#include <boost/filesystem.hpp>

processData pData;

// Get all filenames in a given directory path
std::vector<std::string> getFileNames(std::string directoryPath)
{
  namespace fs = boost::filesystem ;
  std::vector<std::string> names ;

  if ( fs::exists(directoryPath) )
    {
      fs::directory_iterator it(directoryPath) ;
      fs::directory_iterator end ;

      while ( it != end )
        {
	  names.push_back(it->path().filename().string()) ;
	  ++it ;
        }
    }

  return names ;
}

// Filter strings based on pattern
void filter(std::vector<std::string>& strings, std::string pattern){
  auto pos = std::remove_if(std::begin(strings), std::end(strings), 
			    [&](std::string& s) { return s.find(pattern) == std::string::npos ; }) ; 

  strings.erase(pos, std::end(strings)) ;
}

// Print vector of strings
void print(const std::vector<std::string>& strings){
  for ( auto& s : strings )
    std::cout << s << '\n' ;
}


// Main function
int main (int argc, char *argv[]){

  // The file specifier passed as the argument
  char* fname = argv[1];

  // The data file directory
  std::string directory = "/data1/STM_VST_DATA/HPGeAndSigGen_11-09-23/";

  // Get all filenames in that directory
  auto files = getFileNames(directory) ;

  // Filter filenames based on argument
  filter(files,fname) ;

  // Ensure all files are binary files
  filter(files,".bin") ;

  std::cout << "Binary containing \"" << fname << "\" in " << directory << ":" << std::endl;
  print(files) ;

  // Create an array of strings for output files
  std::string out_file[files.size()] = {}; 

  // Global ADC data vector
  std::vector<int16_t> adcData;
  // Loop over all files found
  for (int i = 0; i < files.size(); i++){
    //  for (uint i = 0; i < 1; i++){
    // Truncate end of string down to remove file number
    files[i] = files[i].substr(0, 33);
    // Get output file name
    out_file[i] = files[i] + (std::to_string(i) + "_ADConly.bin");
    // Add file number based purely on loop number
    files[i] += (std::to_string(i) + ".bin");
    // Append the directory to the beginning of the filename
    files[i] = directory+files[i];
    // Print to screen
    std::cout << "Acquiring data in " << files[i] << std::endl;
    // Conver to char*
    char *file = const_cast<char*>(files[i].c_str());
    // Get data
    std::vector<int16_t> fileData = pData.readFile(file);

    // Write data to output binary file
    // std::ofstream fout(out_file[i], std::ios::out | std::ios::binary);
    // fout.write((char*)&fileData[0], fileData.size() * sizeof(int16_t));
    // fout.close();

    // Insert to global data vector
    //    adcData.insert(std::end(adcData), std::begin(fileData), std::end(fileData));
  }

  //  std::cout << "Total data size = " << adcData.size() << " elements" << std::endl;


  return 0;

}
