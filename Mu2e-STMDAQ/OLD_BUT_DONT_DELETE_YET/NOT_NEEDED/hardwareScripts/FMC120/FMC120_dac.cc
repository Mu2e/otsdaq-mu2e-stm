////////////////////////////////////////////////////////////////////////
/// This module initialises the FMC120 digital to analogue (DAC) converter 
////////////////////////////////////////////////////////////////////////

// FMC120_dac
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_dac.hh"

// FMC120 spi and i^2c read/write 
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_spi.hh"

// Constructor
FMC120_dac::FMC120_dac() {}

// Reset and wake DAC                                                     
int FMC120_dac::reset_and_wake_dac(IPBusManager* hw){

  // Set one byte per cycle
  hw->write("i2c_master.i2c.byte",0x00);
  
  // Read, Modify, Write to avoid clobbering any other register settings
  uint32_t dword = spi_120.i2c_read(hw,0x1C02);
  
  // Assert reset (active-LOW) and deassert unitapi_sleep_ms
  dword &= 0xCF;
  spi_120.i2c_write(hw, 0x1C02, dword);
  
  // Clear reset bit
  dword |= 0x20;
  spi_120.i2c_write(hw, 0x1C02, dword);
  
  return 0;
  
}

// Short patten test                                                      
int FMC120_dac::short_pattern_test(IPBusManager* hw){

  // 15-8 : alarm_from_shortest  bit8 = lane0 alarm, bit1 = lane1_alarm 
  /// 7-0  : Loss of signal dectect outputs from SerDes lanes
  uint32_t dword = spi_120.spi_read(hw, DAC_SELECT, 0x6D);
  std::cout << "Starting short pattern test .." << std::endl;
  
  // FPGA pattern generation
  //     0x00 - WFM
  //     0x10 - Short Pattern Test
  //     0x20 - 4 Point Wave
  //     0x30 - WFM FIFO Swapped

  hw->write("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl",0x11);
  sleep(0.2);
  
  // Enable pattern checking in DAC bit12 = shorttest_ena
  // 0x1082 Twos Comp, 0x1080 Offset Binary
  spi_120.spi_write(hw, DAC_SELECT, 0x02, 0x1082);
  
  // Enable alarms for short pattern test
  spi_120.spi_write(hw, DAC_SELECT, 0x06, 0x00FF);
  sleep(1);
  dword = static_cast<uint16_t>(spi_120.spi_read(hw, DAC_SELECT, 0x6D));
  dword &= 0xFF00;
  if (dword) {
    std::cout << "Pattern failed" << std::endl;
    std::cout << dword << std::endl;
  } 
  else {
    std::cout << "Pattern Passed" << std::endl;
  }
    
  // Force error
  hw->write("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl",0x01);
  sleep(1);
  dword = static_cast<uint16_t>(spi_120.spi_read(hw, DAC_SELECT, 0x6D));
  dword &= 0xFF00;
  if (dword != 0xFF00) {
    std::cout << "Error Injection Failed!" << std::endl;
    std::cout << dword << std::endl;
  } 
  else {
    std::cout << "Error Injection Passed." << std::endl;
  }
  
  // Disable short pattern test.
  // 0x0082 Twos, 0x0080 Offset Binary
  spi_120.spi_write(hw, DAC_SELECT, 0x02, 0x0082);

  return 0;

}

// Initialise DAC                                                         
int FMC120_dac::dac_init(IPBusManager* hw, uint32_t odelay){
  
  uint32_t dword = 0;

  // turn off dac output, may be off already
  hw->write("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl",0x00);
  
  // perform hardware reset on DAC39J84 IC
  reset_and_wake_dac(hw);
  
  // Reset DAC Block 
  // this puts the JESD in init state until everything is programed
  spi_120.spi_write(hw, DAC_SELECT, 0x4A, 0xFF1E);
  spi_120.spi_write(hw, DAC_SELECT, 0x46, 0x0044);
  spi_120.spi_write(hw, DAC_SELECT, 0x47, 0x190A);
  
  // Turn off sysref
  spi_120.spi_write(hw, LMK_SELECT, 0x10F, 0x01);
  sleep(0.1);
  
  // Disable TX
  hw->write("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl",0x00);
  
  // Assert Reset for SPI, config for 4 wire mode
  spi_120.spi_write(hw, DAC_SELECT, 0x02, 0x83);
  sleep(0.01);
  
  // Clear reset Bit, data set to twos complement 4-wire SPI
  // 0x2082 Twos 0x2080 Offset Binary
  spi_120.spi_write(hw, DAC_SELECT, 0x02, 0x2082);
  
  // DAC SPI check
  dword = spi_120.spi_read(hw, DAC_SELECT, 0x7F);
  dword = static_cast<uint16_t>(dword) & 0xFF;
  if (dword != FMC120_DAC_PART_ID) {
    std::cout << "Wrong DAC part ID" << std::endl;
    return -1;
  }

  dword &= 0xFF;
  uint32_t ddword = dword & 0x03;
  dword &= 0x18;
  dword = dword/8;
  printf("DAC vendor %5d Version %5d", dword, ddword);

  // Output Polarity. Ch0 and Ch2 need to have their ouput
  // polarity inverted to compensate for trace layout on the board.
  
  // complement DAC outputs Ch0 and Ch2
  spi_120.spi_write(hw,DAC_SELECT,0x01,0x0050);
  // DAC coarse current adjust , set to 20mA
  spi_120.spi_write(hw,DAC_SELECT,0x03,0xA300);

  // ALARM CONTROL- Assuming all are on by default
  spi_120.spi_write(hw,DAC_SELECT,0x04,0x0000);
  spi_120.spi_write(hw,DAC_SELECT,0x05,0xFF03);
  spi_120.spi_write(hw,DAC_SELECT,0x06,0xFFFF);

  // Puts DACs and PLL into NOT unitapi_sleep_ms
  spi_120.spi_write(hw,DAC_SELECT,0x1A,0x0000);
  
  // synchronization from where ? mixer and nco
  // PROBLEM WHEN SYNC WHEN USING SYSREF 4440 ?
  spi_120.spi_write(hw,DAC_SELECT,0x1F,0x4440);

  // synchronization from where ? mixer and nco
  // PROBLEM WHEN SYNCWHEN USING SYSREF 4440
  spi_120.spi_write(hw,DAC_SELECT,0x1E,0x4444);
  // PROBLEM WHEN SYNC WHEN USING SYSREF 4440
  spi_120.spi_write(hw,DAC_SELECT,0x20,0x4044);

  // sysref + clock dividers 
  // (use all sync pulses = 0x10, don't use 
  // synch pulse = 0x00, use only the next 0x20)
  spi_120.spi_write(hw,DAC_SELECT,0x24,0x0020);
  // "CP=350uA, VCO_bias=9.8mA, 4GHz VCO, 25d VCO fine adj"  // AF high
  spi_120.spi_write(hw,DAC_SELECT,0x33,0xAF40);

  // Clock settings
  // 0x0028 = /5 mpy // Low vco
  // SCR=0, High Density,  N=16  N'=16
  spi_120.spi_write(hw,DAC_SELECT,0x3C,0x0228);

  // Enable Sampler compensation
  // bits 43 per TI ap note
  spi_120.spi_write(hw,DAC_SELECT,0x3D,0x0088);

  // This may be an issue, undescribed bits LOS bits 15:13
  // 0x0108  15:13 000 LOS?? 12:11 00 reserved, 10:8 term 010-AC,
  // 7:=0 reserved   6:5=  Full rate   4:2 = 010    
  // Busswidth 20bit,  1: ! sleep, 0: = 0 reserved
  spi_120.spi_write(hw,DAC_SELECT,0x3E,0x0108);

  // Write default state serdes pair inversion register.
  spi_120.spi_write(hw,DAC_SELECT,0x3F,0x0000);

  // debug variable
  int INTERPOLATE = 0;
  // 0  no Interpolation, use LMK_DAC_CLK directly
  // 1  no interpolation, use DAC PLL
  // 2  Interpolate by 2, use DAC PLL 

  // 0  no Interpolation, use LMK_DAC_CLK directly
  if (INTERPOLATE == 0){
    // Interpolate by 1, Enable Alarm out, Pos alarm polarity
    spi_120.spi_write(hw,DAC_SELECT,0x00,0x0018);
    //JESD Clock divider = Lanes * Interpolation / 4 ADCS = 0x4000  !! error
    // jesd pll reference = DACCLK/2
    spi_120.spi_write(hw,DAC_SELECT,0x25,0x2000);
    // 31 statys same if still use PLL
    // PLL in reset, not used 
    spi_120.spi_write(hw,DAC_SELECT,0x31,0x1000);
    // 0x0020 = P = 4
    // 0x0120 P = 4 = 1GHz, M = 2 = 500MHz  4Ghz 
    // P-2 = 1GHz, 1GHz / 2 = 500MHz PFD
    spi_120.spi_write(hw,DAC_SELECT,0x32,0x0000);
    
    // 0x0xxx selects DAC PLL as serdes ref 8xxx selects DACCLK input pins
    // 0x8xxx selects the DAC CLK as the reference
    // 0xx8xx divided 1GHz DAC clk input by to for a 
    // 500MHz Serdes reference clock 
    // controls serdes_refclk_div, 
    // serdes_clk_sel divide by 10 100 * 10 = 10GBPS
    // 00xx, vco feedback devider = 2, xx0x VCO prescaler = 0
    spi_120.spi_write(hw,DAC_SELECT,0x3B,0x0800);

  } // End if INTERPOLATE == 0

  // 1  no interpolation, use DAC PLL
  // use DAC PLL 1GHz in 1GHz out
  if (INTERPOLATE == 1){
    // Interpolate by 1, Enable Alarm out, Pos alarm polarity
    spi_120.spi_write(hw,DAC_SELECT,0x00,0x0018);
    // JESD Clock divider = Lanes * Interpolation / 4 ADCS = 0x4000  !! error
    // jesd pll reference = DACCLK/2   
    spi_120.spi_write(hw,DAC_SELECT,0x25,0x2000);

    // 31 statys same if still use PLL 
    // 6xxx widest lock detect, 
    // x4xx DACCLK PLL on N =2 2 for interpolation 6408 PLL N = 
    spi_120.spi_write(hw,DAC_SELECT,0x31,0x6408);
    // 0x0020 = P = 4
    // 0x0120 P = 4 = 1GHz, M = 2 = 500MHz  4Ghz 
    // P-2 = 1GHz, 1GHz / 2 = 500MHz PFD
    spi_120.spi_write(hw,DAC_SELECT,0x32,0x0120);

    // 0x0xxx selects DAC PLL as serdes ref 8xxx selects DACCLK input pins
    // 0x8xxx selects the DAC CLK as the reference
    // 0xx8xx divided 1GHz DAC clk input by to for a 
    // 500MHz Serdes reference clock 
    // controls serdes_refclk_div, 
    // serdes_clk_sel divide by 10 100 * 10 = 10GBPS
    // 00xx, vco feedback devider = 2, xx0x VCO prescaler = 0
    spi_120.spi_write(hw,DAC_SELECT,0x3B,0x8800);

  } // End if INTERPOLATE == 1

  // 2  Interpolate by 2, use DAC PLL 
  // use DAC PLL 1GHz in 2GHz out, interpolate by 2
  if (INTERPOLATE == 2){
    // Interpolate by 2, Enable Alarm out, Pos alarm polarity
    spi_120.spi_write(hw,DAC_SELECT,0x00,0x0018);
    // JESD Clock divider = Lanes * Interpolation / 4 ADCS = 0x4000
    // (8L * interp2 = 16)/4 jesd pll reference = DACCLK//4
    spi_120.spi_write(hw,DAC_SELECT,0x25,0x4000);
    // 31 statys same if still use PLL
    // 6xxx widest lock detect 
    // x4xx DACCLK PLL on N =2 2 for interpolation 6408 PLL N =
    spi_120.spi_write(hw,DAC_SELECT,0x31,0x6408);
    // 0x0020 = P = 4
    spi_120.spi_write(hw,DAC_SELECT,0x32,0x0300);

    // 0x0xxx selects DAC PLL as serdes ref 8xxx selects DACCLK input pins
    // 0x8xxx selects the DAC CLK as the reference
    // 0xx8xx divided 1GHz DAC clk input by to for a 
    // 500MHz Serdes reference clock 
    // controls serdes_refclk_div, 
    // serdes_clk_sel divide by 10 100 * 10 = 10GBPS
    // 00xx, vco feedback devider = 2, xx0x VCO prescaler = 0
    spi_120.spi_write(hw,DAC_SELECT,0x3B,0x9800);
    
  } // End if INTERPOLATE == 2

  //  config_73 Link3=6v7 Link2=45 Link1=23 Link0=01 
  // SUPPOSELY zero this out // 0xFA50 // 0x5050
  spi_120.spi_write(hw,DAC_SELECT,0x49,0x0);

  //  We will only use Link 0 for Sync output request 
  spi_120.spi_write(hw,DAC_SELECT,0x61,0x01);

  // JESD SETTINGS
  // Buffer=32  Frames = 1, Octets per frame = 1
  spi_120.spi_write(hw,DAC_SELECT,0x4B,0x1F00);
  // K=32   L=8  #######
  spi_120.spi_write(hw,DAC_SELECT,0x4C,0x1F07);
  // M = 3 S = 1            
  // (4 converters per link 2 samples per frame? 
  spi_120.spi_write(hw,DAC_SELECT,0x4D,0x0300);
  // SCR=0, High Density,  N=16  N'=16
  spi_120.spi_write(hw,DAC_SELECT,0x4E,0x0F4F);
  // jesd synchronizatoin settingspi_120. 
  // no lane sync = bit 5  (0x1cE1 no lane sync) (0x1cc1 default)
  spi_120.spi_write(hw,DAC_SELECT,0x4F,0x1CC1);

  // sysref + jesd synchronizaton (0x0 = Ignore all SYSREF) 
  // (deufalt = use all synch pulses) !!!!!!!!!!!
  spi_120.spi_write(hw,DAC_SELECT,0x5C,0x1111);

  spi_120.spi_write(hw,DAC_SELECT,0x5F,0x0123);
  spi_120.spi_write(hw,DAC_SELECT,0x60,0x4567);



  // Active DAC JESD
  // MStart sysref generation
  // turn on sysref output driver 
  // better to leave driver active and set mode to pulse  earlier in cod 
  spi_120.spi_write(hw,LMK_SELECT,0x10F,0x11);

  // 0 - tx_enable
  // 1 - jesd_hold
  // 4-5 - data_select
  // Enable TX  
  sleep(0.5);

  hw->write("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl",0x01);
  sleep(0.05);

  // JESD is still in reset 
  spi_120.spi_write(hw,DAC_SELECT,0x4A,0xFF1E); //JESD in Init State
  sleep(0.05);

  // clear init bits per TI
  spi_120.spi_write(hw,DAC_SELECT,0x4A,0xFF00); // JESD in Init State
  sleep(0.05);
  //  Then assert Reset bit which starts the JESD initialization
  spi_120.spi_write(hw,DAC_SELECT,0x4A,0xFF01);
  sleep(0.05); //  allow to stabelize 

  // Clear alarms 
  // Write to clear PLL LOCK Errors
  spi_120.spi_write(hw,DAC_SELECT,0x6C,0x0); // sysref errors
  // WRITE 0 LOSS of signal
  spi_120.spi_write(hw,DAC_SELECT,0x6D,0x0); // lane alarm LOS alarm

  // Clear lane errors before checking
  for (int i = 0; i < 8; i++){
    spi_120.spi_write(hw,DAC_SELECT,0x64+i,0x0);
  }

  // unitapi_sleep_ms to allow an error to occur
  sleep(1);
  printf ("Status: \n");
  printf ("---------------------------------\n");
  // READ PLL STATUS
  dword = static_cast<uint16_t>(spi_120.spi_read(hw,DAC_SELECT,0x6C));
  if (INTERPOLATE == 0){
    dword &= ~0x3;
  }
  else{
    dword &= ~0x2;
  }
  if (dword !=0){
    printf ("PLL LOCK ERROR : %5x\n", dword);
    printf ("Register 0x6C = %4X\n", dword);
  }
  if (dword & 0x0001){
    printf("DAC PLL out of lock\n");
  }
  else{
    printf("DAC PLL Locked\n");
  }
  if (dword & 0x0008){
    printf("SERDES Block 0 out of LOCK\n");
  }
  else{
    printf("SERDES Block 0 Locked\n");
  }
  if (dword & 0x0004){
    printf("SERDES Block 1 out of LOCK\n");
  }
  else{
    printf("SERDES Block 1 Locked\n");
  }

  // READ PLL LOOP Filter Voltage
  dword = static_cast<uint16_t>(spi_120.spi_read(hw,DAC_SELECT,0x31));
  dword &= 0x07;
  printf("PLL Loop filter voltage = %5d\n", dword);

  // LOSS OF SIGNAL
  dword = static_cast<uint16_t>(spi_120.spi_read(hw,DAC_SELECT,0x6D));
  if (dword != 0x0){
    printf ("LANE LOSS SIGNAL ERROR %5x\n", dword);
  }
  for (int i = 0; i < 8; i++){
    dword = static_cast<uint16_t>(spi_120.spi_read(hw,DAC_SELECT,0x64+i));
    if (dword == 0x2 || dword == 0x3 || dword == 0x0){
      printf ("LANE %3d STATS: OK %5x\n", i, dword);
    }
    else{
      printf ("LANE %3d STATUS: Error %5x\n", i, dword);
    }
  }
  printf("-------------------------------------------\n");

  // error_cnt_link0. 
  // This error count for link0. What is counted as an error is determined
  // by error_ena_link0(0x52). 
  // This is a 16-bit value that is cleared when JE SD synchronization
  // is performed or err_cnt_clk_link0 is programmed to a '1'.
  for (int i = 0; i < 4; i++){
    dword = static_cast<uint16_t>(spi_120.spi_read(hw,DAC_SELECT,0x41+i));
    if (dword !=0x0){
      printf ("LINK %d %d errors found!!!\n", i, dword);
    }
  }

  // Short pattern test
  short_pattern_test(hw);

  return 0;

}

