/////////////////////////////////////////////////////////////////////
/// This module obtains various frequencies from the clock tree
/////////////////////////////////////////////////////////////////////

// FMC144_freqcnt
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_freqcnt.hh"

//Standard constructor - shouldn't be used
FMC144_freqcnt::FMC144_freqcnt() {}

// Obtain a given frequency from the clock tree.                                                                  
// This function can either display the frequency on the console or not. 
void FMC144_freqcnt::getfrequency(IPBusManager* hw, int clksel, int adc_mode){

  uint64_t dword;
  std::string node;

  // Tell the firmware to start a measure on a given clock index 
  node = "axi_fmc144_8lane.Freq_ctr.clk_sel";
  hw->write(node,clksel);
  //  sleep(0.50);
  usleep(5e5); // 0.5 secs

  // Read back the just measured value
  node = "axi_fmc144_8lane.Freq_ctr.clk_cnt";
  dword = hw->read(node);
  std::cout << dword << std::endl;

  // Compute the frequency  
  float testClkPeriod = 1.0/125.0;
  float tmp = 8192.0 * testClkPeriod;
  tmp /= (dword + 1);
  tmp = 1.0/tmp;

  int mult_factor = 4;

  // If we were asked to display to console then we do that  
  if (adc_mode == getADCmode("2CH_4LANE") or adc_mode == getADCmode("4CH_4LANE")){
    mult_factor = 2;
  }
  
  if (clksel==0){
    printf("Eth clock: %6.2f MHz\n",tmp);
  }
  else if (clksel == 1){
    printf("ADCx PHY Clock: %6.2f MHz (Fs = %7.2f)\n",tmp,mult_factor*tmp);
  }
  else if (clksel == 2){
    printf("SYSREF Clock : %6.2f MHz (Fs = %7.2f)\n",tmp,tmp);
  }
  else if (clksel == 3){
    printf("DACx PHY Clock: %6.2f MHz (Fs = %7.2f)\n",tmp,mult_factor*tmp);
  }
  else if (clksel == 4){
    printf("External Trigger : %6.2f MHz\n",tmp);
  }

}
