#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <sys/stat.h>

#include "TGraph.h"
#include "TAxis.h"
#include "TROOT.h"


void Xray_newbinary(std::string binarypath_1, std::string rateHz){

  int NfilesXrays = 10;

  std::ifstream myFileXrays;

  //Generate a csv with bin file, position and ADC value
  std::ofstream output_file;
  
  for(int i = 0; i < NfilesXrays ; i++){

    std::string   output_filename = rateHz+"Hzxrays_positions_notnule_ADCvalues_job0"+std::to_string(i)+".csv";
    output_file.open(output_filename);

    
    std::string  filename  = binarypath_1 + "genData347keV_0.0"+rateHz+"kHz_job0"+std::to_string(i)+".bin";
    myFileXrays.open(filename, std::ios::in | std::ios::binary);

    if(!myFileXrays.is_open()) throw std::runtime_error("Could not open file");
    std::cout<<"file "<<i<<" "<<filename<<std::endl;
    //output_file << i<<" "<<filename << "\n";

    struct stat st;
    stat(filename.c_str(), &st);
    unsigned long int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each value is 2 bytes)           
    std::cout<<"size: "<<n<<std::endl;
    
    int16_t* ADC = new int16_t[n];
    //read file
    myFileXrays.read( (char*) ADC, n*sizeof(ADC[0]));

    for(unsigned long int j = 0 ; j <n; j++){
      if(ADC[j]!=0){
	//std::cout<<"position: "<<j<<std::endl;
	//std::cout<<"value: "<<ADC[j]<<std::endl;
	output_file <<"position: "<< j << " ADC: "<< ADC[j] << "\n";
      }
    }

    
    myFileXrays.close();
    output_file.close();
    delete[] ADC;

  }
}
