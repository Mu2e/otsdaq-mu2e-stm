////////////////////////////////////////////////////////////
/// Capture module: interface with axi_capture star (main)
////////////////////////////////////////////////////////////

// Capture
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/capture.hh"

//Standard constructor - shouldn't be used
capture::capture() {}

// Set the burst size in axi_capture size
int capture::setburstlength(IPBusManager* hw, int capturenode_nr, int numberburst, int burstlength){

  std::string node;

  node ="Captures.capture_"+std::to_string(capturenode_nr)+".nb_of_bursts";
  hw->write(node,numberburst);

  node ="Captures.capture_"+std::to_string(capturenode_nr)+".burst_length";
  hw->write(node,burstlength);

  return 1;

}

// Enable uploading  
int capture::enable_upload(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".ctrl";
  hw->write(node,0x01);

  return 1;

}

// Enable uploading with hardware trigger  
int capture::enable_upload_hw_trig(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".ctrl";
  hw->write(node,0x03);

  return 1;

}

// Disable uploading 
int capture::disable_upload(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".ctrl";
  hw->write(node,0x00);

  return 1;

}

// Arm the Unit  
int capture::arm(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".command";
  hw->write(node,0x01);

  return 1;

}

// Get the rx and tx pointers from the FIFOs  
uint64_t capture::fifo_pointer_val(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".status";
  uint64_t value = hw->read(node);

  return value;

}

// Send a software (sw) trigger 
int capture::sw_trigger(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".command";
  hw->write(node,0x08);

  return 1;

}

// Enable external trigger in the unit 
int capture::external_trigger_enable(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".ctrl";
  hw->write(node,0x02);

  return 1;

}

// Reset FIFOs  
int capture::fifo_rst(IPBusManager* hw, int capturenode_nr){

  std::string node ="Captures.capture_"+std::to_string(capturenode_nr)+".command";
  hw->write(node,0x10);

  return 1;

}
