////////////////////////////////////////////
/// This module sets the FMC120 clocktree
////////////////////////////////////////////

#ifndef FMC120_CLOCKTREE_hh
#define FMC120_CLOCKTREE_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC120_clocktree {

public:

  // Constructor
  FMC120_clocktree();

  // Reset the clock chip
  int reset_clock_chip(IPBusManager* hw);

  // osc100 enable
  int osc100_enable(IPBusManager* hw, bool enable);

  // osc500 enable
  int osc500_enable(IPBusManager* hw, bool enable);

  // Initialise the clocktree
  int clocktree_init(IPBusManager* hw, uint clockmode);  

private : 

};

#endif
