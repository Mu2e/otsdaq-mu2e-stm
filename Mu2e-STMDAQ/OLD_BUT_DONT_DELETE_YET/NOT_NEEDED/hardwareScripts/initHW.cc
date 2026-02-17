//////////////////////////////////////////////
/// This module initialises the HW (main).   
//////////////////////////////////////////////

#include<iostream>
#include<fstream>

// HW
#include "STMDAQ-TestBeam/hardwareScripts/initHW.hh"

// ADC
#include "STMDAQ-TestBeam/hardwareScripts/initADC.hh"

// ADC
initADC adc;

// Standard constructor - shouldn't be used
initHW::initHW() {}

// Initialise HW 
void initHW::init(IPBusManager* &hw){

  // Notify user
  printf("Initialising HW...\n");  

  // Print to user
  printf("Connecting to devices through IPBusManager...\n");

  // Get hardware connection
  hw = new IPBusManager(hw->getConnectionFile(),hw->getDeviceID());
  
  // Set UHAL logging level to notice  
  hw->noticeUhalLogging();

  // Initialise ADC
  //  adc.init(hw,"FMC120");
    
}




