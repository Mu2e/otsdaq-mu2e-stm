/////////////////////////////////////////////
// This module monitors the HW (header)
/////////////////////////////////////////////

#ifndef MONITORHW_hh
#define MONITORHW_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class monitorHW {

public:

  // Standard constructor - shouldn't be used 
  monitorHW();

  // Get hw monitor values
  std::tuple<uint64_t> get_values(IPBusManager* hw);  

  // Get the ADC (temperature only at this point!!)
  uint64_t get_adc_temp(IPBusManager* hw, std::string adc);
 
private:

};

#endif
