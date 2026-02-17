////////////////////////////////////////////////////////////////////////
/// Configures all chips mounted on using FMC120_x sub modules.
////////////////////////////////////////////////////////////////////////

// FMC120
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_spi.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_clocktree.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_adc.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_dac.hh"

// FMC120 functions
FMC120_clocktree f;
FMC120_adc fa;
FMC120_dac fad;

//Standard constructor - shouldn't be used
FMC120::FMC120() {}

// Configure power
int FMC120::config_pwr(IPBusManager* hw){
  
  //Set I2C Port Expander default values
  spi_120.i2c_write(hw,0x3A03,0xFF); // All pins are inputs
  
  spi_120.i2c_write(hw,0x3A02,0x00); // No polarity inversion
  
  //Check for Module Present
  uint32_t dword = spi_120.i2c_read(hw,0x3A00);
  if ((dword & 0x8) == 0){ // Pin 3 is FMC+ PRSNT_N signal, check for low value
    spi_120.i2c_write(hw,0x3A01,0x80); //Set bit 7 to high when output
    spi_120.i2c_write(hw,0x3A03,0x7F); //Output for only bit 7
    uint32_t dword2 = spi_120.i2c_read(hw,0x3A00); // Check PG status 
    if ((dword2 & 0x1) == 0x1){
      std::cout << "OK" << std::endl; // Do nothing
    }
    else{
      std::cout << "Power is NOT good on FMC+" << std::endl;
    }
  }
  else{
    std::cout << "No FMC found" << std::endl;
  }

  sleep(1);
  
  spi_120.i2c_write(hw,0x3A03,0xFF); // All pins are inputs
  sleep(1);
  spi_120.i2c_write(hw,0x3A02,0x00); // No Polarity Inversion
  
  //Check for Module Present
  dword = spi_120.i2c_read(hw,0x3A00);
  if ((dword & 0x8) ==0){
    std::cout << "Enabling VADJ with 1.8V on FMC+" << std::endl;
    spi_120.i2c_write(hw,0x3A01,0x80); //Set bit 7 to high when output
    spi_120.i2c_write(hw,0x3A03,0x7F); //Output for only bit 7
    uint32_t dword2=spi_120.i2c_read(hw,0x3A00); // Check PG status 
    if ((dword2 & 0x1) == 0x1){
      std::cout << "Power is good on FMC+" << std::endl;
    }
    else{
      std::cout << "Power is NOT good on FMC+" << std::endl;
    }
  }
  else{
    std::cout << "No FMC found" << std::endl;
  }
  
  sleep(1);

  return 0;

}

// Initialise FMC120
int FMC120::init(IPBusManager* hw, uint clockmode){

  uint32_t  dword = 0;
  int ila_count = 0;
  int i = 0;
  
  f.clocktree_init(hw,CLOCKTREE_CLKSRC_INTERNAL);
  
  while (i <=10){
    // Configure Transceiver 
    // Assert transceiver reset
    hw->write("axi_fmc120_8lane.FMC120_ctrl.transceiver",0x01);
    
    sleep(0.4);
    
    // Release transceiver reset
    hw->write("axi_fmc120_8lane.FMC120_ctrl.transceiver",0x00);
    std::cout << "Wait for QPLLs to lock" << std::endl;
    sleep(1);
    
    dword = hw->read("axi_fmc120_8lane.FMC120_ctrl.status");
    std::cout << HEX(dword,4) << std::endl;
    dword &= 0xc00;
    if (dword != 0xc00){
      std::cout << "QPLLs not locked" << std::endl;
    }
    else{
      std::cout << "QPLLs are locked" << std::endl;
    }
    
    //Configure ADC0 and ADC1
    std::cout << "Configuring ADCs ..." << std::endl;
    
    fa.adc_init(hw);
    
    // Enable manual bit alignment
    hw->write("axi_fmc120_8lane.FMC120_ctrl.transceiver",0x10);
    
    //Configure DAC0
    std::cout << "Configuring DAC ..." << std::endl;
    fad.dac_init(hw,0);
    
    //Check for JESD ADC to be stable
    dword = hw->read("axi_fmc120_8lane.FMC120_ctrl.status");
    if (dword >> 8 & 0x1){
      std::cout << "ADC0 aligned" << std::endl;
    }
    else{
      std::cout << "ADC0 failed bit alignment" << std::endl;
    }    
    if (static_cast<uint16_t>(dword) >> 9 & 0x1){
      std::cout << "ADC1 aligned" << std::endl;
    }
    else{
      std::cout << "ADC1 failed bit alignment" << std::endl;
    }
    
    // Check for JESD multiframe alignment
    dword = hw->read("axi_fmc120_8lane.FMC120_ctrl.adc_valid");
    if (dword == 0x0F){
      std::cout << "ADC JESD204B Initial Lane Alignment Complete" << std::endl;
      break;
    }
    else{
      std::cout << "ADC JESD204B Initial Lane Alignment Failed" << std::endl;
    }
    
    i++;
    
  }
  
  if (i==10){
    std::cout << "FMC120 Init failed" << std::endl;
    return -1;
  }
  else{
    return 0;
  }
  
}
