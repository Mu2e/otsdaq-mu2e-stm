#ifndef ZS_HH
#define ZS_HH

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <cmath>
#include <sys/stat.h>
#include <cstring>

#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Define zero-supression class
class ZS {

public:
 
  // Standard constructor - shouldn't be used
  ZS();

  // Boolean for printing to screen
  static const bool output = false;  
  
  // Function to set config parameters manually
  void set_params(double _hardwareclockSPILL, double _hardwareclockGAP,
		  double _NRAMblocks, double _ADCRAM,
		  unsigned _headersize, double _fADC,
		  int _window, double _threshold,
		  double _tbefore, double _tafter){

    hardwareclockSPILL = _hardwareclockSPILL;                                                    
    hardwareclockGAP = _hardwareclockGAP;                                                        
    NRAMblocks = _NRAMblocks;                                                                    
    ADCRAM = _ADCRAM;                                                                            
    headersize = _headersize;                                                                    
    fADC = _fADC;                                                                                
    window = _window;                                                                            
    threshold = _threshold;                                                                      
    tbefore = _tbefore;                                                                          
    tafter = _tafter;                                                                            

  }

  // Function to set config parameters from xml file
  void set_params_xml(Xml* xml_file){

    hardwareclockSPILL = (double) xml_file->                                                     
      int_value("stm.hardwareclockSPILL",1.695);                                                 
    hardwareclockGAP = (double) xml_file->                                                       
      int_value("stm.hardwareclockGAP",100);                                                     
    NRAMblocks = (double) xml_file->                                                             
      float_value("stm.NRAMblocks",5);                                                           
    ADCRAM = (double) xml_file->                                                                 
      float_value("stm.ADCRAM",2048);                                                            
    headersize =  xml_file->                                                                     
      float_value("stm.headersize",19);                                                          
    fADC = (double) xml_file->                                                                   
      float_value("stm.adc_time_clock",370.0);                                                   
    window = (double) xml_file-> int_value("stm.window",100);                                    
    threshold = (double) xml_file->                                                              
      float_value("stm.threshold",-100);                                                         
    tbefore = (double) xml_file->                                                                
      float_value("stm.tbefore",1);                                                              
    tafter = (double) xml_file->                                                                 
      float_value("stm.tafter",2);                                                               

  }   

  // ZS method functions

  // Function to print config parameters
  void print_params(){  

    printf("headersize = %d\n",headersize);

    std::cout << "hardwareclockSPILL     = " << hardwareclockSPILL << "\n"
	      << "hardwareclockGAP       = " << hardwareclockGAP << "\n"
	      << "NRAMblocks             = " << NRAMblocks << "\n"
	      << "ADCRAM                 = " << ADCRAM << "\n"
	      << "headersize             = " << headersize << "\n"
	      << "fADC [MHz]             = " << fADC << "\n"
	      << "window                 = " << window << "\n"
	      << "threshold              = " << threshold << "\n"
	      << "tbefore                = " << tbefore << "\n"
	      << "tafter                 = " << tafter << "\n" << std::endl;
    
    return;

  }

  // Functon to form input FIFO
  int16_t* Form_InputFIFO(int16_t* nADC, int16_t _mode,
			  unsigned long int _chunknumber,
			  unsigned long int _chunkstart,
			  unsigned long int _trignumstart,
			  unsigned long int _last_triggerstored,
			  int32_t _numADCtoZS);
  
  // Read the ADC data header 
  void ReadInputHeader (int16_t* inFIFO);

  // The zero-suppression algorithm  
  int16_t* supalg(int16_t* inFIFO);

  // Return the number of ZS triggers - DELETE IF NOT USED
  unsigned long int Return_ZSTriggers(){
    return ntriggers;
  }

  // Form the output FIFO
  //int16_t* Form_OutputFIFO();
  
  // Calls all zero-suppresion functions and returns datax
  int16_t* do_ZS(int16_t* inputFIFO);

  // // Return zero-suppressed data array - DELETE IF NOT USED
  // int16_t* Return_ZSOutputFIFO(){		
  //   return outputFIFO;
  // }

  // Return the zero-suppressed FIFO size - DELETE IT NOT USED
  int Return_ZSFIFOSize(){
    return zp_size;
  }
  
  // ZS Parameters

  // Sampling time of ADC (microsec) 
  double tadc_ (){
    return 1.0/(fADC);
  }
  
  // Store tbefore microseconds of data to the left of the trigger 
  int prenumADCstored_ (double tadc){
    return int(tbefore/tadc);
  }
  
  // Store tafter microseconds of data to the right of the trigger  
  int postnumADCstored_ (double tadc){
    return int(tafter/tadc);
  }

  // Theoretical maxiumum number of ADC values per chunk
  unsigned long int chunk_ (){
    return (NRAMblocks*ADCRAM);
  }
  
  // Number of triggers per chunk to send in Spill
  unsigned long int ntriggers_chunkSPILL_ (){
    double chunkdoub = NRAMblocks*ADCRAM;
    return ((chunkdoub/fADC)/hardwareclockSPILL); 
  }
  
  // Number of ADC values per trigger in spill  
  unsigned long int nADC_triggerSPILL_ (){
    return (fADC*hardwareclockSPILL);
  }
  
  // Number of ADC values per chunk according to triggers in Spill 
  unsigned long int nADC_chunk_(unsigned long int nADC_triggerSPILL,
				unsigned long int ntriggers_chunkSPILL){
    return (nADC_triggerSPILL*ntriggers_chunkSPILL);
  }
  
  // Number of ADC values per trigger in Gap 
   unsigned long int nADC_triggerGAP_ (){
    return (fADC*hardwareclockGAP);
  }

  int16_t word_4x16[4];

  int16_t *split64(uint64_t input){
    
    word_4x16[0] = input & 0x0000FFFF;
    word_4x16[1] = input >> 16;
    word_4x16[2] = input >> 32;
    word_4x16[3] = input >> 48;
    
     return word_4x16;
    
  }

  std::vector<std::vector<int16_t>> split (const std::vector<int16_t>& v, int Nsplit) {
    int n = v.size();
    int size_max = n / Nsplit + (n % Nsplit != 0);
    std::vector<std::vector<int16_t>> split;
    for (int ibegin = 0; ibegin < n; ibegin += size_max) {
      int iend = ibegin + size_max;
      if (iend > n) iend = n;
      split.emplace_back (std::vector<int16_t>(v.begin() + ibegin, v.begin() + iend));
    }
    return split;
  }


 private:

  // Length of on-spill period
  double hardwareclockSPILL = 1.695; // us
  // Length of off-spill period
  double hardwareclockGAP = 100; // us
  // Number of block RAMs
  double NRAMblocks = 5;
  // Number of ADC values per block RAM
  double ADCRAM = 2048;
  // Input header size in ADC values
  unsigned headersize = 19;  
  // ADC sampling frequency
  double fADC = 370; // MHz
  // Zero-suppression window size in number of ADC values
  int window = 100;  
  // Threshold for gradient in ZS algorithm
  double threshold = - 100;
  // Time stored before a trigger
  double tbefore = 1; // us
  // Time stored after a trigger
  double tafter = 2; // us

  // Initialized in ZS::ReadInputHeader
  int16_t mode;
  unsigned long int chunknum,
    chunkstart,
    trignumstart,
    last_triggerstored;
  int32_t numADCtoZS; 

  // Initialized in ZS::supalg
  double tadc; 
  int prenumADCstored, postnumADCstored; 
  unsigned long int ntriggers_chunkSPILL,
    nADC_triggerSPILL,
    nADC_chunk,
    nADC_triggerGAP,
    ntriggers,
    lasttrig;
  //  unsigned long int* realtrigger_vect;
  unsigned long int* realtrigger_abs;

  // Initialized in ZS::ZS_array
  int16_t* suppressed_data;
  int16_t init;
 
  // Initialized in ZS::Form_OutputFIFO
  //  int16_t* outputFIFO;
  int zp_size;

};

#endif
