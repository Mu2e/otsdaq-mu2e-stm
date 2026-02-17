////////////////////////////////////////////////////////////////////////
/// This module initialises the FMC120 analogue to digital (ADC) converter
////////////////////////////////////////////////////////////////////////

#ifndef FMC120_ADC_hh
#define FMC120_ADC_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC120_adc {

public:

  // Constructor
  FMC120_adc();

  // Initialise ADC
  int adc_init(IPBusManager* hw);

private : 

};

#endif
