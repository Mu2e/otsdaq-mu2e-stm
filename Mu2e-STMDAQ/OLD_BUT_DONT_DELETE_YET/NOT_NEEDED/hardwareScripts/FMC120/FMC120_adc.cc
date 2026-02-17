////////////////////////////////////////////////////////////////////////
/// This module initialises the FMC120 analogue to digital (ADC) converter 
////////////////////////////////////////////////////////////////////////

// FMC120_adc
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_adc.hh"

// FMC120 spi and i^2c read/write 
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_spi.hh"

// Constructor
FMC120_adc::FMC120_adc() {}

// Initialise ADC                                                         
int FMC120_adc::adc_init(IPBusManager* hw){
  
  uint32_t dword = 0;
  
  // ADC initialization
  std::cout << "Pulsing ADC RESET pins" << std::endl;

  // First Pulse Hardware Reset Line on Both ADC's 
  // this is the equivalent of flushing all the regs in software
  // Read in the Registerread, modify, write 
  // to avoid clobbering any other register settings   
  dword = spi_120.i2c_read(hw,0x1C03);
  
  // force a clear on the ADC Reset Pins  
  // ensures a low on abnormal terminations or restarts
  dword &= 0xF9;
  spi_120.i2c_write(hw,0x1C03,dword);
  
  // Set reset bits active
  dword |= 0x06;
  spi_120.i2c_write(hw,0x1C03,dword);
  
  //Clear reset bit
  dword &= 0xF9;
  spi_120.i2c_write(hw,0x1C03,dword);
  
  sleep(0.02);
  //power up adc input amplifiers
  dword = spi_120.i2c_read(hw,0x1C01);
  dword &= 0xF3; //clear ADCx_Amp_Off bits
  spi_120.i2c_write(hw,0x1C01,dword);
  sleep(0.02);
  
  // Initialize ADCs
  std::cout << "Configuring ADCs" << std::endl;
  
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x0000,0x81); //LMFS = 4211
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x11,0x80); //select master page of analog bank
  
  // set analog input to DC coupling Z5K **********************************
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x004F,0x00); //DC coupling enable Bit 0 = off,  1 = Enabled shifts VCM at adc down ~ 200mV ***
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x0026,0x40); //IGNORE inputs on power down pin
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x0059,0x20); //Set the always write 1 Bit

  // select main digital page 6800
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4003,0x00); //select JESD Digital Page
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4004,0x68); //select JESD Digital Page
 
  //Set Nyquist Zone
  dword = 0;// set dword to nyquist zone,
  
  if (dword==0){ //First Nyquist zone
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6842,dword); //Set nyquist Zone 0 0-500Mhz
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x684E,0x80); //Enable nyquist correction 
    std::cout << "ADC set to First Nyquist Zone DC-500MHz " << std::endl;
  }
   
  if (dword==1){ //Second Nyquist zone
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6842,dword); //Set nyquist Zone  zone 2 500 to 1000MHz
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x684E,0x80); //Enable nyquist correction 
    std::cout << "ADC set to Second Nyquist Zone 500 to 1000MHz" << std::endl;
  }

  if (dword==2){ //Third Nyquist zone
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6842,dword); //Set nyquist Zone 3 = zone 3 1000 to 1500MHz
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x684E,0x80); //Enable nyquist correction
    std::cout << "ADC set to Third Nyquist Zone 1000 to 1500MHz" << std::endl;
  }
   
  // **** Feature Updates for 54Jxx family
  if (1){
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4005,0x01); //enable single channel writes 
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4004,0x68); //Upper byte of page address
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4003,0x00); //middle byte
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4002,0x00); //middle byte
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4001,0x00); //lower byte of 32bit page address
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x604E,0x20); //for CH-A, write to register address 0x4E for a feature Update
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x704E,0x20); //for CH-B, write to register address 0x4E for a feature Update
    //select main digital page 6800, put the ADC select back were we found it
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4005,0x00); //enable broadcast
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4003,0x00); //select JESD Digital Page
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4004,0x68); //select JESD Digital Page
  }
   
  //*** DIGITAL CORE RESET *** 
  // the digital reset must be pulsed for register writes to take effect
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x60F7,0x00); //Digital Reset
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6000,0x01); //assert Digital Reset
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6000,0x00); //clear Digital Reset
  sleep (0.02);
   
  // select 6A00 JESD Anlaog Page
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4003,0x00); //select JESD Digital Page
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4004,0x6A); //select JESD Digital Page
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6016,0x02); //40X pll

  // select 6900 Digital JESD Page
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4003,0x00); //select page lowbyte
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4004,0x69); //select page highbyte
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6001,0x02); //set LMF = 4244 
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6007,0x08); //set internal defaults JESDV and subclass V1 
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6000,0x80); //set control K  
  spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6006,0x07); //set K to 8
  sleep(0.05);
   
  spi_120.spi_write(hw,LMK_SELECT,0x11F,0x05); // Disable Sysref to ADC 0
  spi_120.spi_write(hw,LMK_SELECT,0x117,0x05); // Disable Sysref to ADC 1

  // *** DC-COUPLED PERFORMANCE IMPROVEMENT ***
  // this code should be enabled when the FMC120 is DC Coupled, It greatly improves performance when measuring signals with a DC offset
  // it also improves / removes ADC generated low frequency distortion below shevral hundred kiloherta
  // this code should be disabled for an AC coupled version of the board
  // this code freezes the DC offset correction engine, the ADC should be at temperature and stable before this code is executed,
  // to re-enable the DC offset correction use a read / modify /write on address 0x6068 / 0x7068 and clear bit 7
  if (1){
    sleep(0.5); // allow everything to stabilize
    std::cout << "Freezing DC offset correction" << std::endl;
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4005,0x01); // enable single channel writes
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4004,0x61); // Upper byte of page address
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4003,0x00); // middle byte 
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4002,0x00); // middle byte
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4001,0x00); // lower byte of 32bit page address
    std::cout << "Working ..." << std::endl;
    // Read Modify Write
    dword = static_cast<uint16_t>(spi_120.spi_read(hw,ADC0_SELECT,0x6068)); // Read reset value on FOVR Threshold
    dword = dword | 0x80;
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x6068,dword); // for CH-A, write to register address 68 data 0x82 in page 68000 Setting bit 7 Freezes DC correction
    // clearing bit 7 enables DC offset correction, 0x02 enables DC correction. Bits D6-D3 set Bandwidth value
    std::cout << "Working ..." << std::endl;
    // Read Modify Write
    dword = static_cast<uint16_t>(spi_120.spi_read(hw,ADC0_SELECT,0x7068)); // Read reset value on FOVR Threshold
    dword = dword |0x80;
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x7068,dword); // for CH-B, write to register address 68 data 0x82 in page 680000 Setting bit 7 Freezes DC correction.
    // clearing bit 7 enables DC offset correction, 0x02 enables DC correction. Bits D6-D3 set Bandwidth value
    std::cout << "Working ..." << std::endl;
    // select main digital page 6800, put ADC addressing where we found it
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4005,0x00); // enable broadcast
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4003,0x00); // select JESD Digital Page
    spi_120.spi_write(hw,ADC_SELECT_BOTH,0x4004,0x68); // select JESD Digital Page
    sleep(0.2);
  }

  return 0;

}


