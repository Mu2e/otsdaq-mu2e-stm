/////////////////////////////////////////////
// This module initialises the hardware.
/////////////////////////////////////////////

#ifndef INITHW_hh
#define INITHW_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class initHW {

public:

  // Standard constructor - shouldn't be used 
  initHW();

  // Initialise HW 
  void init(IPBusManager* &hw);  

private:

};

#endif
