///////////////////////////////////////////////////////////
// Repeat module: interface with axi_repeat star (header)  
///////////////////////////////////////////////////////////

#ifndef REPEAT_hh
#define REPEAT_hh

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class repeat {

public:

  repeat();

  // Set the burst size in axi_repeat size
  int setburstlength(IPBusManager* hw, 
		     int repeatnode_nr, 
		     int burstlength);  

  // Prepare the wave form memory (WFM) for getting data
  int prepare_wfm_upload(IPBusManager* hw, int repeatnode_nr);  

  // Arm the unit, start to play the WFM data
  int arm(IPBusManager* hw, int repeatnode_nr);

  // cDisarm the unit
  int disarm(IPBusManager* hw, int repeatnode_nr);  

  // Enable external trigger in the unit
  int rst(IPBusManager* hw, int repeatnode_nr);  

  // Enable repeat star for external trigger
  uint64_t check_status(IPBusManager* hw, int repeatnode_nr);  
      											   
private : 


};

#endif
