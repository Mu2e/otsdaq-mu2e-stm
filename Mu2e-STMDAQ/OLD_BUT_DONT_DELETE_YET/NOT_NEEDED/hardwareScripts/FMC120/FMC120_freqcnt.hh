/////////////////////////////////////////////////////////////////
/// This module obtains various frequencies from the clock tree
/////////////////////////////////////////////////////////////////

#ifndef FMC120_freqcnt_hh
#define FMC120_freqcnt_hh

// IPBusManager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC120_freqcnt {

public:

  FMC120_freqcnt();
  
  // Obtain a given frequency from the clock tree. 
  // This function can either display the frequency on the console or not.
  int getfrequency(IPBusManager* hw, uint32_t clksel);

private : 

};

#endif
