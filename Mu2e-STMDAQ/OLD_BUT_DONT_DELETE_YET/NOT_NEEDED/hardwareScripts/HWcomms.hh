/////////////////////////////////////////////////////////////////////////////////// 
/// This module is the main function in sending commands to the firmware/hardware (header).
///////////////////////////////////////////////////////////////////////////////////

#ifndef HWCOMMS_hh
#define HWCOMMS_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class HWcomms {

public:

  // Standard constructor - shouldn't be used 
  HWcomms();

  //**********************************************
  // BELOW ARE USED FOR ELBE
  //**********************************************

  // Initialize HWcomms hardware
  int pingInterfaces();  

  // Set firmware registers from xml config file
  int setRegisters(IPBusManager*);

  // Start and reset the external clock counter 
  int startResetClock(IPBusManager*);

  // Reset trigger FIFOs
  int resetTriggerFIFOs(IPBusManager*); 

  // Set the 10G readout mode
  int start10Greadout(IPBusManager*); 
  int stop10Greadout(IPBusManager*); 

  // Reset and initliaise DDR4 read
  int reset_DDR4(IPBusManager*); 

  // Toggle ADC emulation 
  int emulateADC(IPBusManager*);
  int stopEmulateADC(IPBusManager*);

  // Toggle triggered data-taking
  int enable_trigData(IPBusManager*); 
  int disable_trigData(IPBusManager*); 

  // Toggle continous 10g packet sending
  int start10Gpackets(IPBusManager*);
  int stop10Gpackets(IPBusManager*);

  // Checks whether the DDR4 memory rolls over
  // and checks where the read address is.
  // Bit 31 tells you whether the memory rollover happened (bit 31 = 1)
  // If rollover, it returns the read address in the least 30 bits.
  uint32_t checkMemoryRollover(IPBusManager* hw);

  // Check DDR4 read/write address
  uint32_t getDDR4addr(IPBusManager*); 

  //**********************************************

  // Toggle continuous data-taking
  int enable_contData(IPBusManager*); 
  int disable_contData(IPBusManager*); 


  // Toggle DDR4 overwrite
  int allow_DDR4overwrite(IPBusManager*); 
  int stop_DDR4overwrite(IPBusManager*); 

  // DDR4 read
  uint32_t checkReadAdrr(IPBusManager*); // Get current DDR4 read address
  uint32_t checkBuffer(IPBusManager*); // Check 1kbyte buffer
  std::vector<uint32_t> readDDR4(IPBusManager*); // Read DDR4 block

  // The full DDR4 write address = 0x7ffff000 
  uint32_t fullDDR4(){
    return ddr4Full;
  }

  // The max DDR4 read address = 0x7fffefff 
  uint32_t maxDDR4(){
    return maxRead;
  }

  // 1 kb = 1024 bytes
  int oneKbyte(){
    return oneKb;
  }

  // 8 kb = 8240 bytes
  int eightKbyte(){
    return eightKb;
  }

private:

  uint32_t ddr4Full = 0x7ffff000;
  uint32_t maxRead = 0x7ffff000;
  int oneKb = 1024;
  int eightKb = 8198;

};




#endif
