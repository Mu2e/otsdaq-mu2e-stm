///////////////////////////////////////////////////////////
// Repeat module: interface with axi_repeat star (main) 
///////////////////////////////////////////////////////////

// repeat
#include "STMDAQ-TestBeam/hardwareScripts/repeat.hh"

//Standard constructor - shouldn't be used
repeat::repeat() {}

// Set the burst size in axi_repeat size 
int repeat::setburstlength(IPBusManager* hw, int repeatnode_nr, int burstlength){

  std::string node ="Repeats.repeat_"+std::to_string(repeatnode_nr)+".burst_size";
  hw->write(node,burstlength);

  return 1;
  
}

// Prepare the wave form memory (WFM) for getting data     
int repeat::prepare_wfm_upload(IPBusManager* hw, int repeatnode_nr){

  std::string node ="Repeats.repeat_"+std::to_string(repeatnode_nr)+".ctrl";
  hw->write(node,0x08);

  return 1;
  
}

// Arm the unit, start to play the WFM data  
int repeat::arm(IPBusManager* hw, int repeatnode_nr){

  std::string node ="Repeats.repeat_"+std::to_string(repeatnode_nr)+".ctrl";
  hw->write(node,0x01);

  return 1;
  
}

// Disarm the unit 
int repeat::disarm(IPBusManager* hw, int repeatnode_nr){

  std::string node ="Repeats.repeat_"+std::to_string(repeatnode_nr)+".ctrl";
  hw->write(node,0x02);

  return 1;
  
}

// Enable external trigger in the unit   
int repeat::rst(IPBusManager* hw, int repeatnode_nr){

  std::string node ="Repeats.repeat_"+std::to_string(repeatnode_nr)+".ctrl";
  hw->write(node,0x10);

  return 1;
  
}

// Check repeat node status
uint64_t repeat::check_status(IPBusManager* hw, int repeatnode_nr){

  std::string node ="Repeats.repeat_"+std::to_string(repeatnode_nr)+".status";
  uint64_t value = hw->read(node);

  return value;
  
}
