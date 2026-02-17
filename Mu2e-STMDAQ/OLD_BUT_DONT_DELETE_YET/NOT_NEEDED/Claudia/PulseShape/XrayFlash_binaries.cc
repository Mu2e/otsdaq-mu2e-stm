#include <iostream>
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

#include "STMDAQ-TestBeam/Claudia/PulseShape/ehDriftFunctions_setseed.h"

int main(int argc, char *argv[]){

  std::string  csv_path = std::string(argv[1]);
  std::string  bin_path = std::string(argv[2]);
  std::string  bin_rate = std::string(argv[3]);
  unsigned int seedoption = std::stod(argv[4]);
   

  unsigned int seed;
  if(seedoption==0){seed = setseed_chrono();}
  else{seed = setseed_man(seedoption);}

  std::default_random_engine gen(seed);
  
  std::vector<int16_t> ADC_Flash;
  ADC_Flash.clear();
    
  //for 1-10kHz Flash
  //int NFlash_per_Xray = 10;
  //for 20-200kHz Flash
  int NFlash_per_Xray = 100;
  

  std::ifstream myFileXrays;

  std::ifstream myFileFlash;
  std::string filenameFlash;

  //fot 50 and 100kHz
  int ncsv = 0;
  int nbinfiles = 0;

  //0-75kHz
  int limit = 10; //ncsvs from Xrays
  //20, 200kHz
  //int limit = 1;

  limit =limit + ncsv;
  
  for(int i = ncsv; i < limit ; i++){

    ADC_Flash.clear();
    
    //Read 10 or 100 Flash files and fill a vector
    for(int j=0 ; j < NFlash_per_Xray; j++){
       
      if(nbinfiles<10){
	filenameFlash = bin_path+"genData0keV_"+bin_rate+"kHz_job0"+std::to_string(nbinfiles)+".bin";
      }
      else{ filenameFlash = bin_path+"genData0keV_"+bin_rate+"kHz_job"+std::to_string(nbinfiles)+".bin";}
      std::cout<<j<<" "<<filenameFlash<<std::endl;
      
      myFileFlash.open(filenameFlash, std::ios::in | std::ios::binary);
      
      if(!myFileFlash.is_open()) throw std::runtime_error("Could not open file");
      //std::cout<<j<<" "<<filenameFlash<<std::endl;
      struct stat st;
      stat(filenameFlash.c_str(), &st);
      unsigned long int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each value is 2 bytes)           
      std::cout<<"size: "<<n<<std::endl;

      int16_t* ADC = new int16_t[n];
      
      //read file
      myFileFlash.read( (char*) ADC, n*sizeof(ADC[0]));

      for(unsigned long int b = 0 ; b <n; b++){
	ADC_Flash.push_back(ADC[b]);
      }

      myFileFlash.close();
      delete ADC;
      nbinfiles++;
  
      
      } //j

    std::cout<<"size flash files: "<<ADC_Flash.size()<<std::endl;
    //for(unsigned long int g = 0; g<ADC_Flash.size();g++){ if(g>30783260){std::cout<<"gpos "<<g<<" val "<<ADC_Flash.at(g)<<std::endl; }}
    
    //With the ADC values from the 10 Flash bin files
    //read the csv position and add to the vector the Xray value

    /*
    std::string  filename  = csv_path+"12Hzxrays_positions_notnule_ADCvalues_job0"+std::to_string(i)+".csv";
    //std::string  filename  = csv_path+"30Hzxrays_positions_notnule_ADCvalues_job0"+std::to_string(i)+".csv";
    myFileXrays.open(filename);
    
    if(!myFileXrays.is_open()) throw std::runtime_error("Could not open file");
    std::cout<<i<<" "<<filename<<std::endl;
   
    std::string line;
    unsigned long int pos;

    int16_t ADC_xray;
    
    std::string val;
    
    // Read data, line by line
    while(std::getline(myFileXrays, line))
    {
        // Create a stringstream of the current line
        std::stringstream ss(line);
	ss >> val;
	ss >> pos;
	ss >> val;
	ss >> ADC_xray;
	ADC_Flash.at(pos) = ADC_Flash.at(pos) + ADC_xray ;
    }
      
    myFileXrays.close();
    */
    double Sizetotal = ADC_Flash.size();
    std::cout<<"size of ADC Flash+Xray: "<<Sizetotal<<std::endl;

    //Need to add the noise 
    /*
    double noiseSD = 0.32; //in mV
    double sigma_noise_ADC=noiseSD*38.5;
    std::cout<<"NoiseSD: "<<noiseSD<<" mV = "<<sigma_noise_ADC<<" ADC Counts"<<std::endl;
    std::normal_distribution<double> distribution(0,sigma_noise_ADC);
    
    for(unsigned long int i=0; i<Sizetotal;i++){
      double noise=distribution(gen);
      ADC_Flash.at(i) = ADC_Flash.at(i) + noise ;
    }
    */
    //Generate 10 new binary files with the sum of Flash and Xrays:
    std::ofstream output_file;
    //std::string   output_filename = bin_rate+"kHz_FlashANDXrays_job0"+std::to_string(i)+"_0.32mVNOISE.bin";
    std::string   output_filename = bin_rate+"kHz_Flash_job0"+std::to_string(i)+".bin";
    output_file.open(output_filename, std::ios::out | std::ios::binary);  

    for (int16_t element : ADC_Flash){
      output_file.write((char *) &element, sizeof(element));
    }
    
    output_file.close();
    
  }//i
  
}
