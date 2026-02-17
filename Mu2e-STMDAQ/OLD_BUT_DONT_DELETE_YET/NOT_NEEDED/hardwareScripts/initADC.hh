/////////////////////////////////////////////
// This module initialises the ADC (main).
/////////////////////////////////////////////

#ifndef INITADC_hh
#define INITADC_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class initADC {

public:

  // Standard constructor - shouldn't be used 
  initADC();

  // Initialise ADC 
  void init(IPBusManager* hw, std::string adc);  

  // Initialise FMC120
  int init_FMC120(IPBusManager* hw);  

  // Initialise FMC144
  int init_FMC144(IPBusManager* hw);  

  // --                                                              
  // Functions to get private variables                              
  // --              

  // Get ADC mode                                                    
  int getADCmode(){
    return adcMode;
  }

  // Get number burst                                                
  int getNumberBurst(){
    return numberBurst;
  }

  // Get Nsamps                                                      
  int getNsamps(){
    return Nsamps;
  }

  // Get Nsize                                                       
  int getNsize(){
    return Nsize;
  }

  // Get burst length                                                
  int getBurstLength(){
    return burstLength;
  }

  // Get burst length capture                                        
  int getBurstLengthCapture(){
    return burstLength_capture;
  }


private:

  // Define variables for device                                     
  int adcMode = 1;
  int numberBurst = 0;
  int Nsamps = 8192;
  int Nsize = 8192;
  int burstLength = 1536; // 512 * 2 * 12                            
  int burstLength_capture = 1280; // 1256; // 628  

};

#endif
