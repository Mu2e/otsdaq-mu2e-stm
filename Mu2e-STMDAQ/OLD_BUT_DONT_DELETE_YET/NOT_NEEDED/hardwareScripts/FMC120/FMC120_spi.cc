////////////////////////////////////////////////////////////////////////
/// This module implements the FMC120 spi AND i^2c read/write
////////////////////////////////////////////////////////////////////////

// FMC120_spi
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_spi.hh"

//Standard constructor - shouldn't be used
FMC120_spi::FMC120_spi() {}

// i^2c write  
void FMC120_spi::i2c_write(IPBusManager* hw, 
			   uint32_t slaveaddr, uint32_t data){
  
  int i2c_busy = 1;
  int rc = 0;
  hw->write("i2c_master.i2c.address",slaveaddr);  
  hw->write("i2c_master.i2c.wdat",data);  
  hw->write("i2c_master.i2c.cmd",0x1);  
  while (i2c_busy & 0x1) {
    i2c_busy = hw->read("i2c_master.i2c.cmd");    
    sleep(0.1);
  }

}

// i^2c read
uint32_t FMC120_spi::i2c_read(IPBusManager* hw, uint32_t slaveaddr){

  int i2c_busy = 1;
  int rc = 0;
  hw->write("i2c_master.i2c.address",slaveaddr);  
  hw->write("i2c_master.i2c.cmd",0x02);  
  while (i2c_busy & 0x1) {
    i2c_busy = hw->read("i2c_master.i2c.cmd");
    
    sleep(0.1);
  }
  rc = hw->read("i2c_master.i2c.read");
  
  return rc;

}

// spi read
uint32_t FMC120_spi::spi_read(IPBusManager* hw, 
			      uint32_t spi_select, uint32_t spi_addr) {

  uint32_t spi_value = 0;
  hw->write("i2c_master.i2c.byte",0x00);
  if (spi_select == LMK_SELECT) {
    // read enable
    spi_value += 0x1 << 23;
    // Address is in bits 20 down to 8
    spi_value += (spi_addr & 0x1FFF) << 8; 
  } 
  else if (spi_select == DAC_SELECT) {
    // read enable
    spi_value += 0x1 << 23;
    // Address is in bits 22 down to 16
    spi_value += (spi_addr & 0x7F) << 16;
  } 
  else if (spi_select == ADC0_SELECT || spi_select == ADC1_SELECT) {
    // read enable
    spi_value += 0x1 << 23;
    // Address is in bits 22 down to 8 
    spi_value += (spi_addr & 0x7FFF) << 8;
  } 
  else {
    std::cout << "Unsupported SPI write access" << std::endl;
    return -1;
  }
  // Write 1st byte
  i2c_write(hw, 0x1C06, spi_value >> 0);
  // Write 1st byte
  i2c_write(hw, 0x1C07, spi_value >> 8);
  // Write 1st byte
  i2c_write(hw, 0x1C08, spi_value >> 16);
  // Initiate SPI cycle
  i2c_write(hw, 0x1C00, spi_select);
  uint32_t dword0 = i2c_read(hw, 0x1C0E);
  uint32_t dword1 = i2c_read(hw, 0x1C0F);
  uint32_t data = (dword1 << 8) | dword0;

  return data;

}

// spi write
uint32_t FMC120_spi::spi_write(IPBusManager* hw, uint32_t spi_select, 
	      uint32_t spi_addr, uint32_t spi_value) {

  int rc = 0;
  int dword = 0;
  hw->write("i2c_master.i2c.byte",0x00);  
  if (spi_select == LMK_SELECT) {
    // Address is in bits 20 down to 8
    dword += (spi_addr & 0x1FFF) << 8;
    // 8 bit data
    dword += spi_value & 0xFF;
  } 
  else if (spi_select == DAC_SELECT) {
    // Address is in bits 22 down to 16
    dword += (spi_addr & 0x7F) << 16;
    // 16  bit data
    dword += spi_value & 0xFFFF;
  } 
  else if (spi_select == ADC0_SELECT || spi_select == ADC1_SELECT || spi_select == ADC_SELECT_BOTH) {
    // Address is in bits 22 down to 8 
    dword += (spi_addr & 0x7FFF) << 8;
    // 8 bit data
    dword += spi_value & 0xFF;
  } 
  else {
    std::cout << "Unsupported SPI write access" << std::endl;
    return -1;
  }
  
  // Write 1st byte
  i2c_write(hw, 0x1C06, dword >> 0);
  // Write 1st byte
  i2c_write(hw, 0x1C07, dword >> 8);
  // Write 1st byte
  i2c_write(hw, 0x1C08, dword >> 16);
  // Initiate SPI cycle
  i2c_write(hw, 0x1C00, spi_select);

  return rc;

}

