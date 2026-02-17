// HW
#include "STMDAQ-TestBeam/hardwareScripts/initHW.hh"

// Instance of IPBus Manager
IPBusManager* hw = new IPBusManager();

// Instance of HW class
initHW initHW_;

// Main function
int main(){

  // Initialise all hardware
  initHW_.init(hw);
  
  return 0;
  
}

 
