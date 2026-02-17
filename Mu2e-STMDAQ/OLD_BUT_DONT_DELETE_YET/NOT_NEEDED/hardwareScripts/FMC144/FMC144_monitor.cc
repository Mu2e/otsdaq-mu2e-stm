/////////////////////////////////////////////////////////////////////////////////// 
/// This module is in charge of controlling the monitoring device populated  
/// on the FMC144 printed circuit board.
/////////////////////////////////////////////////////////////////////////////////// 

#include <iomanip>

// FMC144_monitor
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_monitor.hh"

// FMC144_spi read/write
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_spi.hh"

//Standard constructor - shouldn't be used
FMC144_monitor::FMC144_monitor() {
}

// Configure monitoring device on the FMC144.   
void FMC144_monitor::init(IPBusManager* hw){

  std::cout << "Entered FMC144_monitor::init" << std::endl;

  // Build the command word 
  uint64_t commandword;
  commandword = 1<<15;            // Read operation                                                          
  commandword |= (1<<12);         // Page 1                                                                  
  commandword |= (0x1E<<6);       // Start address is 0x1E  

  std::cout << HEX(commandword,8) << std::endl;

  uint64_t t1;

  // Check <Part Revision Number> register  
  t1 = spi_144.spi_read(hw,"AMC7823_CTRL0",commandword);
  std::cout << HEX(t1,8) << std::endl;
  
  // Power-Down Register  
  // Set PADC to '1' to enable ADC operation
  // Set PDACn to '0' to disable DAC operation
  // Set PTS to '0' to disable the precision current source 
  // Set PREFB to '0' disable the reference buffer amplifier
  spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0D<<6)),0x8000);
  std::cout << HEX(t1,8) << std::endl;
  
  // AMC Status/Configuration Register 
  // Set SREF to '0' to select internal refence 
  // Set GREF to '0' to select +1.25V reference  
  // set ECNVT to '0' to enable internal trigger mode    
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0A<<6)),0x0000);
  std::cout << HEX(t1,8) << std::endl;

  // ADC Control Register  
  // Set CMODE to '1' for auto mode (continuous conversion)    
  // Set SA to 0 to start measure at channel 0  
  // Set EA to 8 to end measure at channel 8     
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0B<<6)),0x8080);
  std::cout << HEX(t1,8) << std::endl;

  
}

// Configure the voltage threshold level for the first analogue input (AN0-AN3). 
// This function configures the chipset in order.
// A LED turns on if voltage is out of the target given by arguments. 
uint64_t FMC144_monitor::setthreshold(IPBusManager* hw,float ANA0,float ANA1,float ANA2,float ANA3){

  std::cout << "Entered FMC144_monitor::setthreshold" << std::endl;

  // Calculate threshold register values for voltages plus 5%   
  uint64_t THRH0 = uint64_t(float(ANA0)*1.05 / 2.0 * 4096.0 / 2.5);
  uint64_t THRH1 = uint64_t(float(ANA0)*1.05 / 1.0 * 4096.0 / 2.5);
  uint64_t THRH2 = uint64_t(float(ANA0)*1.05 / 1.0 * 4096.0 / 2.5);
  uint64_t THRH3 = uint64_t(float(ANA0)*1.05 / 1.0 * 4096.0 / 2.5);
  
  // Calculate threshold register values for voltages minus 5%  
  uint64_t THRL0 = uint64_t(float(ANA0)*0.95 / 2.0 * 4096.0 / 2.5);
  uint64_t THRL1 = uint64_t(float(ANA0)*0.95 / 1.0 * 4096.0 / 2.5);
  uint64_t THRL2 = uint64_t(float(ANA0)*0.95 / 1.0 * 4096.0 / 2.5);
  uint64_t THRL3 = uint64_t(float(ANA0)*0.95 / 1.0 * 4096.0 / 2.5);

  // Clip hi threshold   
  if (THRH0 > 0xFFF) THRH0 = 0xFFF;
  if (THRH1 > 0xFFF) THRH1 = 0xFFF;
  if (THRH2 > 0xFFF) THRH2 = 0xFFF;
  if (THRH3 > 0xFFF) THRH3 = 0xFFF;

  // THRH0;
  uint64_t t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0E<<6)),THRH0);
  if (t1 == 0) {
    std::cout << "THRH0 write error!\nExiting...\n" << std::endl;
    exit(0);
  }
  // THRL0
  t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0F<<6)),THRL0);
  if (t1 == 0) {
    std::cout << "THRL0 write error\nExiting...\n" << std::endl;
    exit(0);
  }
  // THRH1
  t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x10<<6)),THRH1);
  if (t1 == 0) {
    std::cout << "THRH1 write error\nExiting...\n" << std::endl;
    exit(0);
  }
  // THRL1
  t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x11<<6)),THRL1);
  if (t1 == 0) {
    std::cout << "THRL1 write error\nExiting...\n" << std::endl;
    exit(0);
  }
  // THRH2
  t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x12<<6)),THRH2);
  if (t1 == 0) {
    std::cout << "THRH2 write error\nExiting...\n" << std::endl;
    exit(0);
  }
  // THRL2
  t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x13<<6)),THRL2);
  if (t1 == 0) {
    std::cout << "THRL2 write error\nExiting...\n" << std::endl;
    exit(0);
  }
  // THRH3
  t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x14<<6)),THRH3);
  if (t1 == 0) {
    std::cout << "THRH3 write error\nExiting...\n\n" << std::endl;
    exit(0);
  }
  // THRL3
  t1 = 0;
  t1 = spi_144.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x15<<6)),THRL3);
  if (t1 == 0) {
    std::cout << "THRL3 write error\nExiting...\n" << std::endl;
    exit(0);
  }

  std::cout << "Exiting FMC144_monitor::setthreshold" << std::endl;

  return 1;
		  
}

// Check if the voltage read from the monitoring device matches the value passed as argument. 
// There are 8 voltages available from the monitoring device and 8 arguments to this function. 
// The value passed gives the expected voltage. 
// The function returns with error if the real value is outside +5/-5%.
int FMC144_monitor::checkvoltages(IPBusManager* hw, std::vector<float> r){

  // Initialize the values to compare against  
  float ANA0 = 3.0;
  float ANA1 = 0.9;
  float ANA2 = 1.2;
  float ANA3 = 1.8;
  float ANA4 = 5.0;
  float ANA5 = -5.0;
  float ANA6 = 3.3;
     
  // Calculate voltages plus 5%   
  float THRH0 = ANA0 * 1.05;
  float THRH1 = ANA1 * 1.05;
  float THRH2 = ANA2 * 1.05;
  float THRH3 = ANA3 * 1.05;
  float THRH4 = ANA4 * 1.05;
  float THRH5 = ANA5 * 0.95;
  float THRH6 = ANA6 * 1.05;
  float THRH7 = 5.5; // VADJ allows for a range from 1.65V to 5.5V

  // Calculate voltages minus 5% 
  float THRL0 = ANA0 * 0.95;
  float THRL1 = ANA1 * 0.95;
  float THRL2 = ANA2 * 0.95;
  float THRL3 = ANA3 * 0.95;
  float THRL4 = ANA4 * 0.95;
  float THRL5 = ANA5 * 1.05;
  float THRL6 = ANA6 * 0.95;
  float THRL7 = 1.65; // VADJ allows for a range from 1.65V to 5.5V

  // Flag to indicate failure as we want to cycle through all the voltages before returning     
  int iserror = 0;

  //Read ADC registers    
  for (int i = 0; i < 8; i++){

    uint64_t dword = spi_144.spi_read(hw,"AMC7823_CTRL0",(i<<6));
    std::cout << HEX(dword,8) << std::endl;
    float result = 2.5*float(dword & 0xFFF) / 4096.0;
    if (i==0){
      result *= 2.0;
      r.push_back(result);
      if (result > THRL0 and result < THRH0){
	printf("3.0V ANALOG : OK %.2f\n",result);
      }
      else{
	printf("3.0V ANALOG : ERROR %.2f\n",result);
	iserror = 1;;
      }
    }
    else if (i == 1){
      result *= 1.0;
      r.push_back(result);
      if (result > THRL1 and result < THRH1){
	printf("0.9V ANALOG : OK %.2f\n",result);
      }
      else{
	printf("0.9V ANALOG : ERROR %.2f\n",result);
	iserror = 1;
      }
    }
    else if (i == 2){
      result *= 1.0;
      r.push_back(result);
      if (result > THRL2 and result < THRH2){
	printf("1.2V ANALOG : OK %.2f\n",result);
      }
      else{
	printf("1.2V ANALOG : ERROR %.2f\n",result);
	iserror = 1;
      }
    }
    else if (i == 3){
      result *= 1.0;
      r.push_back(result);
      if (result > THRL3 and result < THRH3){
	printf("1.8V ANALOG : OK %.2f\n",result);
      }
      else{
	printf("1.8V ANALOG : ERROR %.2f\n",result);
	iserror = 1;
      }
    }
    else if (i == 4){
      result *= 3.0;
      r.push_back(result);
      if (result > THRL4 and result < THRH4){
	printf("5V ANALOG : OK %.2f\n",result);
      }
      else{
	printf("5V ANALOG : ERROR %.2f\n",result);
	iserror = 1;
      }
    }
    else if (i == 5){
      result = (4095.0 - (float(dword & 0xFFF) + 34.0)) * -2.4533;
      result /= 1000.0; // Convert to V
      r.push_back(result);
      if (result > THRL5 and result < THRH5){
	printf("-5V ANALOG : OK %.2f\n",result);
      }
      else{
	printf("-5V ANALOG : ERROR %.2f\n",result);
	iserror = 1;
      }
    }
    else if (i == 6){
      result *= 2.0;
      r.push_back(result);
      if (result > THRL6 and result < THRH6){
	  printf("3.3V ANALOG : OK %.2f\n",result);
      }
      else{
	printf("3.3V ANALOG : ERROR %.2f\n",result);
	iserror = 1;
      }
    }
    else if (i == 7){
      result *= 2.0;
      r.push_back(result);
      if (result > THRL7 and result < THRH7){
	printf("VADJ : OK %.2f\n",result);
      }
      else{
	printf("VADJ ANALOG : ERROR %.2f\n",result);
	iserror = 1;
      }
    }
   
  }
  
  // If error with voltage, return error flag 
  return iserror;

}

// Display temperature of the AMT on the console. 
int FMC144_monitor::displayamctemp(IPBusManager* hw, double temperature){

  uint64_t dword = spi_144.spi_read(hw,"AMC7823_CTRL0",((0x0<<12)|(0x08<<6)));
  std::cout << HEX(dword,8) << std::endl;

  // Calculate deltaVbe in mV 
  double result = 0.61 * float(dword & 0xFFF);

  // Calculate temperature in C 
  result = 2.60 * result - 273.0;
  printf("Temperature  : %f\n",result);
  temperature = result;

  return 1;

}

// Configure and readout monitoring device on the FMC144.
void FMC144_monitor::getdiags(IPBusManager* hw){

  std::cout << "Entered FMC144_monitor::getdiags" << std::endl;
  
  // Check <Part Revision Number> register  
  // Power-Down Register 
  // AMC Status/Configuration Register  
  // ADC Control Register 
  init(hw);

  // Set Threshold Registers 
  // If one of the first 4 voltages is out of the threshold range, 
  // the PWR OK led on the FMC module will go off.  
  setthreshold(hw,3.0,0.9,1.2,1.8);

  // Check all voltages by software  
  // If one of the voltages is out of range (either > +5% or < -5%),
  // the function will return an error. 
  std::vector<float> r;
  checkvoltages(hw,r);

  // Read out internal temp sensor    
  double temperature = 0;
  displayamctemp(hw,temperature);

}
