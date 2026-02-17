////////////////////////////////////////////////////////////////
/// Configures all chips mounted on using FMC120_x sub modules. 
////////////////////////////////////////////////////////////////

#ifndef FMC120_hh
#define FMC120_hh

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC120 {

public:

  FMC120();

  // Configure power
  int config_pwr(IPBusManager* hw);  

  // Initialise FMC120
  int init(IPBusManager* hw, uint clockmode);  
  
			   
private : 

};

#endif
