/////////////////////////////////////////////////////////////////
/// This module obtains various frequencies from the clock tree
/////////////////////////////////////////////////////////////////

#ifndef FMC144_freqcnt_hh
#define FMC144_freqcnt_hh

// IPBusManager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC144_freqcnt {

public:

  FMC144_freqcnt();
  
  // Obtain a given frequency from the clock tree. 
  // This function can either display the frequency on the console or not.
  void getfrequency(IPBusManager* hw, int clklsel, int adc_mode);  

  // Get ADC ID for FMC144
  int getADCmode(std::string mode){

    int ADCmode = -1;

    if (mode == "2CH_8LANE"){
	ADCmode = ADC_MODE_2CH_8LANE;
    }
    else if (mode == "4CH_8LANE"){
	ADCmode = ADC_MODE_4CH_8LANE;
    }
    else if (mode == "2CH_4LANE"){
      ADCmode = ADC_MODE_2CH_4LANE;
    }
    else if (mode == "4CH_4LANE"){
      ADCmode = ADC_MODE_4CH_4LANE;
    }
    else{
      std::cout << "Error in FMC144::getADCmde, adc mode cannot be " << mode << std::endl;
      exit(0);
    }
    if (ADCmode < 0 or ADCmode > 3){
      std::cout << "Error in FMC144::getADCmode, no adc mode acquiredd" << std::endl;
      exit(0);
    }

    return ADCmode;  
    
  }
      											   
private : 

  // ADC IDs
  int ADC_MODE_2CH_8LANE = 0;
  int ADC_MODE_4CH_8LANE = 1;
  int ADC_MODE_2CH_4LANE = 2;
  int ADC_MODE_4CH_4LANE = 3;

};

#endif
