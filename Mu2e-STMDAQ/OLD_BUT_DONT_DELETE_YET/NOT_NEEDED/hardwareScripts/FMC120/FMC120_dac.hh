////////////////////////////////////////////////////////////////////////
/// This module initialises the FMC120 digital to analogue (DAC) converter
////////////////////////////////////////////////////////////////////////

#ifndef FMC120_DAC_hh
#define FMC120_DAC_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC120_dac {

public:

  // Constructor
  FMC120_dac();

  static const uint16_t FMC120_DAC_PART_ID = 0x0A;

  // Reset and wake DAC
  int reset_and_wake_dac(IPBusManager* hw);

  // Short patten test
  int short_pattern_test(IPBusManager* hw);

  // Initialise DAC
  int dac_init(IPBusManager* hw, uint32_t odelay);

private : 

};

#endif
