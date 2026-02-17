/////////////////////////////////////////////////////////////////////////////////// 
/// This module is in charge of controlling the monitoring device populated 
/// on the FMC144 printed circuit board.
/////////////////////////////////////////////////////////////////////////////////// 

#ifndef FMC144_monitor_hh
#define FMC144_monitor_hh

#include <iomanip>

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC144_monitor {

public:

  FMC144_monitor();

  // Configure and monitoring device on the FMC144.
  void init(IPBusManager* hw);  

  // Configure and readout monitoring device on the FMC144.
  void getdiags(IPBusManager* hw);  

  // Configure the voltage threshold level for the first analogue input (AN0-ANA3).
  // This function configures the chipset in order.
  // A LED turns on if voltage is out of the target given by arguments.  
  uint64_t setthreshold(IPBusManager* hw,float ANA0,float ANA1,float ANA2,float ANA3);

  // Check if the voltage read from the monitoring device matches the value passed as argument.
  // There are 8 voltages available from the monitoring device and 8 arguments to this function.
  // The value passed gives the expected voltage.
  // The function returns with error if the real value is outside +5/-5%.   
  int checkvoltages(IPBusManager* hw, std::vector<float> r);  

  // Display temperature of the AMT on the console.
  int displayamctemp(IPBusManager* hw, double temperature);  

private : 

  std::shared_ptr<IPBusManager> _ipbus;

};

#endif
