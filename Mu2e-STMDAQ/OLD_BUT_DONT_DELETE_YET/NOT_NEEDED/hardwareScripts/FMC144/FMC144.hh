///////////////////////////////////////////////////////////////////////
/// This module configures the FMC144 chips using FMC144_x sub-modules.
///////////////////////////////////////////////////////////////////////

#ifndef FMC144_hh
#define FMC144_hh

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC144 {

public:

  FMC144();

  // Initialize both ADC chips on the FMC144. 
  int init(IPBusManager* hw, int clockmode, int adc_mode);  
  // Initialise clocktree
  void clocktree_init(IPBusManager* hw, int clockmode);  
  // Initialise ADC0 and ADC1
  int adc_init(IPBusManager* hw, int clockmode);
  // Initialise DAC0
  int dac_init(IPBusManager* hw, int odelay, int clockmode);

  // Get clockmode ID for FMC144
  int getClockMode(std::string mode){

    int clockmode = -1;

    if (mode == "INTERNAL"){
	clockmode = CLOCKTREE_CLKSRC_INTERNAL;
    }
    else if (mode == "EXTERNAL"){
	clockmode = CLOCKTREE_CLKSRC_EXTERNAL;
    }
    else if (mode == "EXTERNAL_REF"){
      clockmode = CLOCKTREE_CLKSRC_EXTERNAL_REF;
    }
    else if (mode == "DDS"){
      clockmode = CLOCKTREE_CLKSRC_DDS;
    }
    else{
      std::cout << "Error in FMC144::getClockMode, clockmode cannot be " << mode << std::endl;
      exit(0);
    }
    if (clockmode < 0 or clockmode > 3){
      std::cout << "Error in FMC144::getClockMode, no clockmode acquiredd" << std::endl;
      exit(0);
    }

    return clockmode;  
    
  }

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

  // Get ADC part ID
  uint64_t getADCpartID(){
    return FMC144_ADC_PART_ID;
  }

  // Get DAC part ID
  uint64_t getDACpartID(){
    return FMC144_DAC_PART_ID;
  }

  // Get test mode ADC0
  bool testADC0(){
    return testModeADC0;
  }

  // Get test mode ADC1
  bool testADC1(){
    return testModeADC1;
  }

  // Get DACPLL_ENABLE flag
  bool enableDACPLL(){
    return DACPLL_ENABLE;
  }

      											   
private : 

  // FMC144_init() configure the FMC144 for internal clock operations
  int CLOCKTREE_CLKSRC_INTERNAL = 0;
  // FMC144_init() configure the FMC144 for external clock operations
  int CLOCKTREE_CLKSRC_EXTERNAL = 1;
  // FMC144_init() configure the FMC144 for internal clock \ external ref operations
  int CLOCKTREE_CLKSRC_EXTERNAL_REF = 2;
  // FMC144_init() configure the FMC144 for internal clock with DDS clock chip
  int CLOCKTREE_CLKSRC_DDS = 3;

  int ADC_MODE_2CH_8LANE = 0;
  int ADC_MODE_4CH_8LANE = 1;
  int ADC_MODE_2CH_4LANE = 2;
  int ADC_MODE_4CH_4LANE = 3;

  uint64_t FMC144_ADC_PART_ID = 0x02;
  uint64_t FMC144_DAC_PART_ID = 0x0A;

  // Boolean switches to enable test mode ADC
  bool testModeADC0 = false;
  bool testModeADC1 = false;

  bool DACPLL_ENABLE = false;

};

#endif
