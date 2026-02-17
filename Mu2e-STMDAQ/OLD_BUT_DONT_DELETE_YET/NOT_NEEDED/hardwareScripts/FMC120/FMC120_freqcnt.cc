/////////////////////////////////////////////////////////////////////
/// This module obtains various frequencies from the clock tree
/////////////////////////////////////////////////////////////////////

#include <unistd.h>

// FMC120_freqcnt
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_freqcnt.hh"

//Standard constructor - shouldn't be used
FMC120_freqcnt::FMC120_freqcnt() {}

// Obtain a given frequency from the clock tree
// This function can either display the frequency on the console or not. 
int FMC120_freqcnt::getfrequency(IPBusManager* hw, uint32_t clksel){

  // tell the firmware to start a measure on a given clock index
  hw->write("axi_fmc120_8lane.Freq_ctr.clk_sel",clksel);
  sleep(0.5);
  
  // read back the just measured value
  uint32_t dword = hw->read("axi_fmc120_8lane.Freq_ctr.clk_cnt");
  double cmdfreq = 125.0;
  double testClkperiod = 1.0 / 125;
  double tmp = 8192 * testClkperiod;
  tmp = (tmp / (dword + 1));
  tmp = 1.00 / tmp;
  int mult_factor = 4;
  if (clksel == 0) {
    std::cout << "Stellar IP Clock: " << tmp << " MHz" << std::endl;
  } else if (clksel == 1) {
    std::cout << "ADCx PHY Clock: " << tmp << " MHz" << std::endl;
  } else if (clksel == 2) {
    std::cout << "LMK Clock: " << tmp << " MHz" << std::endl;
  } else if (clksel == 3) {
    std::cout << "DACx PHY Clock: " << tmp << " MHz" << std::endl;
  } else if (clksel == 4) {
    std::cout << "External Trigger: " << tmp << " MHz" << std::endl;
  }

  return 0;

}

