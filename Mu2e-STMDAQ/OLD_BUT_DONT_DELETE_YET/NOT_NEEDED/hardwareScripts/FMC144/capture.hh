////////////////////////////////////////////////////////////
/// Capture module: interface with axi_capture star (header) 
////////////////////////////////////////////////////////////

#ifndef CAPTURE_hh
#define CAPTURE_hh

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class capture {

public:
  
  capture();
  
  // Set the burst size in axi_capture size 
  int setburstlength(IPBusManager* hw, int capturenode_nr, 
		     int numberburst, int burstlength);  

  // Enable uploading
  int enable_upload(IPBusManager* hw, int capturenode_nr);  

  // Enable uploading with hardware trigger
  int enable_upload_hw_trig(IPBusManager* hw, int capturenode_nr);  

  // Disable uploading 
  int disable_upload(IPBusManager* hw, int capturenode_nr); 

  // Arm the Unit
  int arm(IPBusManager* hw, int capturenode_nr);  

  // Get the rx and tx pointers from the FIFOs
  uint64_t fifo_pointer_val(IPBusManager* hw, int capturenode_nr);  

  // Send a software (sw) trigger
  int sw_trigger(IPBusManager* hw, int capturenode_nr);  

  // Enable external trigger in the unit
  int external_trigger_enable(IPBusManager* hw, int capturenode_nr);  

  // Reset FIFOs
  int fifo_rst(IPBusManager* hw, int capturenode_nr);  
      											   
private : 


};

#endif
