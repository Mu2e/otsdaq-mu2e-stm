////////////////////////////////////////////
/// This module sets the FMC120 clocktree 
////////////////////////////////////////////

// FMC120_clocktree
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_clocktree.hh"

// FMC120 spi and i^2c read/write                                           
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_spi.hh"

//Standard constructor - shouldn't be used
FMC120_clocktree::FMC120_clocktree() {}

// Reset the clock chip
int FMC120_clocktree::reset_clock_chip(IPBusManager* hw){

  uint32_t dword = 0;
  
  hw->write("i2c_master.i2c.byte",0x00);
    
  dword=spi_120.i2c_read(hw,0x1C03);
  dword |= 0x01; // Set reset bit                                            
  spi_120.i2c_write(hw,0x1c03,dword);
  dword &= 0xFE; // Clear reset bit                                          
  spi_120.i2c_write(hw,0x1c03,dword);
  
  return 0;

}

// osc100 enable                                                          
int FMC120_clocktree::osc100_enable(IPBusManager* hw, bool enable){
  
  hw->write("i2c_master.i2c.byte",0x00);
  
  // Read, Modify, Write to avoid clobbering any other register settings    
  uint32_t dword = spi_120.i2c_read(hw,0x1C01);
  if (enable){
    dword |= 0x01; // Set enable bit                                        
    spi_120.i2c_write(hw,0x1c01,dword);
  }
  else{
    dword &= 0xFE; // Clear enable bit                                      
    spi_120.i2c_write(hw,0x1c01,dword);
  }    

  return 0;
  
}

// osc500 enable                                                          
int FMC120_clocktree::osc500_enable(IPBusManager* hw, bool enable){
  
  hw->write("i2c_master.i2c.byte",0x00);
  
  // Read, Modify, Write to avoid clobbering any other register settings    
  uint32_t dword = spi_120.i2c_read(hw,0x1C01);
  if (enable){
    dword |= 0x02; // Set enable bit                                        
    spi_120.i2c_write(hw,0x1c01,dword);
  }
  else{
    dword &= 0xFD; // Clear enable bit                                      
    spi_120.i2c_write(hw,0x1c01,dword);
  }
   
  return 0;
  
}

// Initialise the clocktree                                               
int FMC120_clocktree::clocktree_init(IPBusManager* hw, uint clockmode){
  
  std::cout << "Resetting the clock chip" << std::endl; 
  reset_clock_chip(hw);
  sleep(5);
   
  spi_120.spi_write(hw,LMK_SELECT,0x000,0x80); //Force a reset
  spi_120.spi_write(hw,LMK_SELECT,0x000,0x00); //Clear reset 
  spi_120.spi_write(hw,LMK_SELECT,0x000,0x10); //Force SPI to be 4-wire
  spi_120.spi_write(hw,LMK_SELECT,0x148,0x33); //CLKIN_SEL0_MUX Configured as LMK MISO Push Pull Output
  spi_120.spi_write(hw,LMK_SELECT,0x002,0x00); //POWERDOWN Disabled (Normal operation); 
  std::cout << "CLK 0/1" << std::endl;
  // CLK0/1 Settings DAC 1GHz
  //DIV_CLKOUT1_0 DIV_BY_3 = 1GHz, IDL/ODL == 1  ==> In/Out Drive level = higher
  spi_120.spi_write(hw,LMK_SELECT,0x100,0x6A); // 300 MHz
  //  spi_120.spi_write(hw,LMK_SELECT,0x100,0x63); // 333.33 MHZ
  spi_120.spi_write(hw,LMK_SELECT,0x101,0x22); //DIG_DLY_DCLK0 Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x103,0x05); //ANA_DLY_DCLK2 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX);   
  spi_120.spi_write(hw,LMK_SELECT,0x104,0x62); //DIG_DLY_SCLK1 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out   
  spi_120.spi_write(hw,LMK_SELECT,0x105,0x00); //ANA_DLY_SCLK1 SCLK analog delay disabled 
  spi_120.spi_write(hw,LMK_SELECT,0x106,0xB0); //PD_CLK1_0 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
  spi_120.spi_write(hw,LMK_SELECT,0x107,0x55); //Dclk =LVPECL, Sclk = LVPECL, !DCLK_INV, !SCLK_INV    
  std::cout << "CLK 2/3" << std::endl;
  // CLK2/3 Settings  Output to FPGA
  //DIV_CLKOUT  DIV_CLKOUT1_0 DIV_BY_3 = 1GHz, IDL/ODL == 1  ==> In/Out Drive level = higher
  spi_120.spi_write(hw,LMK_SELECT,0x108,0x66); // 300 Mhz
  //  spi_120.spi_write(hw,LMK_SELECT,0x108,0x74); // 333.33 MHz
  spi_120.spi_write(hw,LMK_SELECT,0x109,0x22); //DIG_DLY_DCLK0 Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x10B,0x05); //ANA_DLY_DCLK2 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX);   
  spi_120.spi_write(hw,LMK_SELECT,0x10C,0x62); //DIG_DLY_SCLK3  DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
  spi_120.spi_write(hw,LMK_SELECT,0x10D,0x0A); //ANA_DLY_SCLK3  SCLK analog delay disabled
  spi_120.spi_write(hw,LMK_SELECT,0x10E,0xB0); //PD_CLK2/3 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK  
  spi_120.spi_write(hw,LMK_SELECT,0x10F,0x11); //FMT_CLK3_3 Dclock = LVPECL, Sclock = LVPECL !DCLK_INV, !SCLK_INV   
  std::cout << "CLK 4/5" << std::endl;
  // CLK4/5 Settings  ADCB 1GHz
  //DIV_CLKOUT5_4  DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level = higher  
  spi_120.spi_write(hw,LMK_SELECT,0x110,0x6A); // 300 Mhz
  //  spi_120.spi_write(hw,LMK_SELECT,0x110,0x63); // 333.33 MHz
  spi_120.spi_write(hw,LMK_SELECT,0x111,0x22); //DIG_DLY_DCLK0 Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x113,0x05); //ANA_DLY_DCLK4 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX);         
  spi_120.spi_write(hw,LMK_SELECT,0x114,0x62); //DIG_DLY_SCLK5 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
  spi_120.spi_write(hw,LMK_SELECT,0x115,0x00); //ANA_DLY_SCLK5 SCLK analog delay disabled
  spi_120.spi_write(hw,LMK_SELECT,0x116,0xB0); //PD_CLK5_4 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
  spi_120.spi_write(hw,LMK_SELECT,0x117,0x57); //DIV_CLKOUT5_4 Dclk =LVPECL, Sclk =LVPECL, !DCLK_INV, !SCLK_INV 
  std::cout << "CLK 6/7" << std::endl;
  // // CLK6/7 Settings  ADCA 1GHz
  //DIV_CLKOUT7_6     DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level 
  spi_120.spi_write(hw,LMK_SELECT,0x118,0x6A); // 300 Mhz 
  //  spi_120.spi_write(hw,LMK_SELECT,0x118,0x63); // 333.33 MHz
  spi_120.spi_write(hw,LMK_SELECT,0x119,0x22); //DIG_DLY_DCLK0 Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x11B,0x05); //ANA_DLY_DCLK6 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX);         
  spi_120.spi_write(hw,LMK_SELECT,0x11C,0x62); //DIG_DLY_SCLK7 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
  spi_120.spi_write(hw,LMK_SELECT,0x11D,0x00); //ANA_DLY_SCLK7 SCLK analog delay disabled
  spi_120.spi_write(hw,LMK_SELECT,0x11E,0xB0); //PD_CLK7_6 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
  spi_120.spi_write(hw,LMK_SELECT,0x11F,0x57); //DIV_CLKOUT7_6 Dclk =LVPECL, Sclk =LVPECL, !DCLK_INV, !SCLK_INV  
  std::cout << "CLK 8/9" << std::endl;
  // CLK8/9 Settings  DCLK 8 (LMK_DCLK8_M2C_TO_FPGA_P); this clock drives GBTCLK0M2C_PN FMC pins D4,D5 Clock to FPGa (250MHz for new designs);
  //DIV_CLKOUT9_8   // DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level normal 
  spi_120.spi_write(hw,LMK_SELECT,0x120,0x74); // 300 Mhz 
  //spi_120.spi_write(hw,LMK_SELECT,0x120,0x66); // 333.33 MHz
  spi_120.spi_write(hw,LMK_SELECT,0x121,0x22); //DIG_DLY_DCLK0 Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x123,0x05); //ANA_DLY_DCLK8 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX);         
  spi_120.spi_write(hw,LMK_SELECT,0x124,0x62); //DIG_DLY_SCLK9 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
  spi_120.spi_write(hw,LMK_SELECT,0x125,0x00); //ANA_DLY_SCLK9 SCLK analog delay disabled
  spi_120.spi_write(hw,LMK_SELECT,0x126,0xB7); //PD_CLK9_8 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
  spi_120.spi_write(hw,LMK_SELECT,0x127,0x11); //Sclk OFF   Dclock = LVDS 
  std::cout << "CLK 10/11" << std::endl;
  // CLK10/11 Settings  DCLK 10 (LMK_DCLK10_M2C_TO_FPGA_P); this clock drives GBTCLK0M2C_PN FMC pins B20,B21
  //DIV_CLKOUT9_8   // DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level normal  
  spi_120.spi_write(hw,LMK_SELECT,0x128,0x74); // 300 Mhz
  //  spi_120.spi_write(hw,LMK_SELECT,0x128,0x66); // 333.33 MHz
  spi_120.spi_write(hw,LMK_SELECT,0x129,0x22); //DIG_DLY_DCLK0 Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x12B,0x05); //ANA_DLY_DCLK10 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX);         
  spi_120.spi_write(hw,LMK_SELECT,0x12C,0x62); //DIG_DLY_SCLK11 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
  spi_120.spi_write(hw,LMK_SELECT,0x12D,0x00); //ANA_DLY_SCLK11 SCLK analog delay disabled
  spi_120.spi_write(hw,LMK_SELECT,0x12E,0xB7); //PD_CLK11_10 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
  spi_120.spi_write(hw,LMK_SELECT,0x12F,0x11); //Sclk OFF   Dclock = LVDS         
  std::cout << "CLK 12/13" << std::endl;
  // External Clock Output, This will be programed 'on' but we will Clear it off in final configuration
  // CLK12/13 Settings  DCLK 12 EXTERNAL CLOCK OUTPUT 
  spi_120.spi_write(hw,LMK_SELECT,0x130,0x63); //DIV_CLKOUT13_12   // DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level normal  
  spi_120.spi_write(hw,LMK_SELECT,0x131,0x22); //DIG_DLY_DCLK0 Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x133,0x05); //ANA_DLY_DCLK12 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX);         
  spi_120.spi_write(hw,LMK_SELECT,0x134,0x62); //DIG_DLY_SCLK13    SCLK analog = 2 VCO Clock Cycles (666pS << from FMC144 );     
  spi_120.spi_write(hw,LMK_SELECT,0x135,0x00); //ANA_DLY_SCLK13 SCLK analog delay disabled
  spi_120.spi_write(hw,LMK_SELECT,0x136,0xB7); //PD_CLK13_12 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, SCLK Disabled at VCM     
  spi_120.spi_write(hw,LMK_SELECT,0x137,0x00); //Sclk Off, Dclock On = LVPECL<< set to 0x00 to turn off Externl Clock option 
      
  //Select VCO1 PLL1 source
  spi_120.spi_write(hw,LMK_SELECT,0x138,0x20); //VCO_OSC_OUT VCO01 (High Speed);, PLL1_FB=OSCin,OSCOUT=input (Drivers powerd down);     
  spi_120.spi_write(hw,LMK_SELECT,0x139,0x03); //SYSREF_MUX SYSREF output source = Normal Sync start with 0 reprogram when turning on sysref   
      
  //SYSREF Divider (SYSREF = LINE_RATE / (10 * F * K ););
  spi_120.spi_write(hw,LMK_SELECT,0x13A,0x01); //SYSREF_DIV(MS); SYSREF Divider, 300 MHz
  spi_120.spi_write(hw,LMK_SELECT,0x13B,0x40); //SYSREF_DIV(LS); SYSREF Divider, 300 MHz
  // spi_120.spi_write(hw,LMK_SELECT,0x13A,0x00); //SYSREF_DIV(MS); SYSREF Divider, 333.33 MHz
  //  spi_120.spi_write(hw,LMK_SELECT,0x13B,0x60); //SYSREF_DIV(LS); SYSREF Divider, 333.33 MHz              
      
  //SYSREF Digital Delay
  spi_120.spi_write(hw,LMK_SELECT,0x13C,0x00); //SYSREF_DDLY(MS); SYSREF Digital Delay  - Not Used  
  spi_120.spi_write(hw,LMK_SELECT,0x13D,0x08); //SYSREF_DDLY(LS); SYSREF Digital Delay  - Not Used 
      
  spi_120.spi_write(hw,LMK_SELECT,0x13E,0x00); //SYSREF_PULSE_CNT 8 Pulses - Not Used
  spi_120.spi_write(hw,LMK_SELECT,0x13F,0x00); //FB_CTRL PLL2_FB=prescaler, PLL1_FB=OSCIN   This is default for internal Oscillator, this changes on EXT osc   
  spi_120.spi_write(hw,LMK_SELECT,0x140,0x01); //OSCIN_SYSREF_PD Active= PLL1, LDO, VCO, OSCIN, SYSREF         
  spi_120.spi_write(hw,LMK_SELECT,0x141,0x00); //DIG_DLY_REG Disable all digital delays     
  spi_120.spi_write(hw,LMK_SELECT,0x142,0x00); //DIG_DLY_STEP_CNT No Adjustment of Digital Delay 
  spi_120.spi_write(hw,LMK_SELECT,0x143,0x70); //SYNC_SYSREF SYNC functionality enabled, prevent SYNC pin and DLD flags from generating SYNC event
  //DCLK12, DCLK10, DCLK8 do not re-sync during a sync event    
  spi_120.spi_write(hw,LMK_SELECT,0x144,0xFF); //DISABLE_DCLK_SYNC Prevent SYSREF clocks from synchronizing << set to 0x00 to turn off Externl Clock option      
  spi_120.spi_write(hw,LMK_SELECT,0x145,0x7F); //FIXED Always 0x7F      
  spi_120.spi_write(hw,LMK_SELECT,0x146,0x00); //CLKIN_SRC No AutoSwitching of clock inputs, all 3 CLKINx pins are set t0 Bipolar,  
      
  //******** discussion required ***
  //CLKin0 = Hardware trig muxuse for external trigger function
  //CLKin1 = External reference Signal (or externals Clock Signal);
  //CLKin2 = Onboard 100MHz reference, (uses OSCOUT pins); 
  //NOTE Rich thinks that since CLKin0 is the output of the hardware trigger mux, that the lower nibble of 0x147 should be different
  //i.e 0x8 instead of 0xA, not sending clkin0 to PLL1     
      
  // External ref
  if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF){
    std::cout << "Using External Clock" << std::endl;
    osc100_enable(hw,0);
    std::cout << "100 MHz Oscillator OFF" << std::endl; 
    spi_120.spi_write(hw,LMK_SELECT,0x147,0x1A); // CLKin_SEL_MODE = CLKin1 Manual, CLKin1_OUT_MUX = Fin, CLKin0_OUT_MUX = PLL1 
  }
      
  // External clock
  if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL){
    std::cout << "Using External Clock" << std::endl;
    osc100_enable(hw,0);
    std::cout << "100 MHz Oscillator OFF" << std::endl; 
    osc500_enable(hw,0);
    std::cout << "500 Mhz VCSO OFF" << std::endl;
    //External clock
    spi_120.spi_write(hw,LMK_SELECT,0x147,0x12); // CLKin_SEL_MODE = CLKin1 Manual, CLKin1_OUT_MUX = Fin, CLKin0_OUT_MUX = PLL1  
  }

  // Internal reference
  if (clockmode == CLOCKTREE_CLKSRC_INTERNAL){
    spi_120.spi_write(hw,LMK_SELECT,0x147,0x2A); // CLKIN_MUX CLKIN = clkin1 (External Reference connector);, !INVERT, CLKIN1=PLL1, CLKIN0=SYSREF MUX 
    std::cout << "Using Internal Reference Oscillator" << std::endl;
    osc100_enable(hw,1);
    std::cout << "100 MHz Oscillator ON" << std::endl; 
    osc500_enable(hw,1);
    std::cout << "500 Mhz VCSO ON" << std::endl;
  }
      
  spi_120.spi_write(hw,LMK_SELECT,0x148,0x33); //CLKIN_SEL0_MUX Configured as LMK MISO Push Pull Output
  spi_120.spi_write(hw,LMK_SELECT,0x149,0x00); //CLKIN_SEL1_MUX SPI SDIO_readback = PUSH-PULL !! CLKIN_SEL1=input  
      
  spi_120.spi_write(hw,LMK_SELECT,0x14A,0x00); //RESET_MUX RESET Pin=Input   
  spi_120.spi_write(hw,LMK_SELECT,0x14B,0x05); //Holdover mode off Manual DAC Enabled   
  spi_120.spi_write(hw,LMK_SELECT,0x14C,0xFF); //MANUAL_DAC Force DAC to midscale 0x01FF        
  spi_120.spi_write(hw,LMK_SELECT,0x14D,0x00); //DAC_TRIP_LOW Min Voltage to force HOLDOVER    
  spi_120.spi_write(hw,LMK_SELECT,0x14E,0x00); // DAC_TRIP_HIGH Mult=4 Max Voltage to force HOLDOVER
      
  spi_120.spi_write(hw,LMK_SELECT,0x14F,0x7F); //DAC_UPDATE_CNTR      
  spi_120.spi_write(hw,LMK_SELECT,0x150,0x00); //HOLDOVER_SET HOLDOVER disable      
  spi_120.spi_write(hw,LMK_SELECT,0x151,0x02); //HOLD_EXIT_COUNT(MS);
  spi_120.spi_write(hw,LMK_SELECT,0x152,0x00); //HOLD_EXIT_COUNT(LS);
      
  //PLL1 CLKIN0 Divider, FMC120 trigger mux feeds this input
  spi_120.spi_write(hw,LMK_SELECT,0x153,0x00); //CLKIN0_DIV (MS);    
  spi_120.spi_write(hw,LMK_SELECT,0x154,0x78); //CLKIN0_DIV (LS);
      
  //External Reference Input
  // PLL1 CLKIN1 Divider External Reference input, default 100MHz and a 100.000KHz Phase Detector Frequency
  spi_120.spi_write(hw,LMK_SELECT,0x155,0x00); //CLKIN1_DIV (MS);      100MHz / 1000 = 100KHz
  spi_120.spi_write(hw,LMK_SELECT,0x156,0x0A); //CLKIN1_DIV (LS) 
      
  //*********** PLL 1 REF Set
  //note. The FMC120 PLL1 Loop filter is designed for an 11Hz bandwidth at a PDF of 1MHz
  if (0){ //100 KHzDefault PLL Frequency
    std::cout << "100 KHz PLL Freq" << std::endl;
    //onboard 100MHz Oscillator at LMK OSCOUT pins
    //PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
    spi_120.spi_write(hw,LMK_SELECT,0x157,0x03); //CLKIN2_DIV (MS); 100MHz / 1000 = 100KHz 1000d = 0x03E8  
    spi_120.spi_write(hw,LMK_SELECT,0x158,0xE8); //CLKIN2_DIV (LS); 
    //Onboard 500MHz VCSO at LMK OSCIN pins
    //PLL1 N divider, Divide 500MHz VCSO down to PDF
    spi_120.spi_write(hw,LMK_SELECT,0x159,0x13); //PLL1_NDIV (MS)  PLL1 Ndivider = 5000 for 100HHz PDF  
    spi_120.spi_write(hw,LMK_SELECT,0x15A,0x88); //PLL1_NDIV (LS) 
  }
      
  if (0){ //500 KHz
    std::cout << "500 KHz PLL Freq" << std::endl;
    //onboard 100MHz Oscillator at LMK OSCOUT pins
    //PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
    spi_120.spi_write(hw,LMK_SELECT,0x157,0x00); //CLKIN2_DIV (MS) R divide = 200 = 500KHz  
    spi_120.spi_write(hw,LMK_SELECT,0x158,0xC8); //CLKIN2_DIV (LS) 
    //Onboard 500MHz VCSO at LMK OSCIN pins
    //PLL1 N divider, Divide 500MHz VCSO down to PDF
    spi_120.spi_write(hw,LMK_SELECT,0x159,0x03); //PLL1_NDIV (MS)  N divide = 1000 = 500MHz PDF  
    spi_120.spi_write(hw,LMK_SELECT,0x15A,0xE8); //PLL1_NDIV (LS) 
  }
      
  if (0){ //1 MHz
    std::cout << "1 MHz PLL Freq" << std::endl;
    //onboard 100MHz Oscillator at LMK OSCOUT pins
    //PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
    spi_120.spi_write(hw,LMK_SELECT,0x157,0x00); //CLKIN2_DIV (MS) R divide = 0x64 = 100 = 1MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x158,0x64); //CLKIN2_DIV (LS) 
    //Onboard 500MHz VCSO at LMK OSCIN pins
    //PLL1 N divider, Divide 500MHz VCSO down to PDF
    spi_120.spi_write(hw,LMK_SELECT,0x159,0x01); //PLL1_NDIV (MS)  N divide =0x1f4 = 500 = 1 MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x15A,0xF4); //PLL1_NDIV (LS)
  }

  if (1){ //10 MHz
    std::cout << "10 MHz PLL Freq" << std::endl;
    //onboard 100MHz Oscillator at LMK OSCOUT pins
    //PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
    spi_120.spi_write(hw,LMK_SELECT,0x157,0x00); //CLKIN2_DIV (MS) R divide = A = 10 = 10MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x158,0x0A); //CLKIN2_DIV (LS) 
    //Onboard 500MHz VCSO at LMK OSCIN pins
    //PLL1 N divider, Divide 500MHz VCSO down to PDF
    spi_120.spi_write(hw,LMK_SELECT,0x159,0x00); //PLL1_NDIV (MS)   N divide =0x32 = 50 = 10 MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x15A,0x32); //PLL1_NDIV (LS)  
  }

  if (0){ //25 MHz
    std::cout << "25 MHz PLL Freq" << std::endl;
    //onboard 100MHz Oscillator at LMK OSCOUT pins
    //PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
    spi_120.spi_write(hw,LMK_SELECT,0x157,0x00); //CLKIN2_DIV (MS) R divide = 4 = 4 = 25MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x158,0x04); //CLKIN2_DIV (LS) 
    //Onboard 500MHz VCSO at LMK OSCIN pins
    //PLL1 N divider, Divide 500MHz VCSO down to PDF
    spi_120.spi_write(hw,LMK_SELECT,0x159,0x00); //PLL1_NDIV (MS)   N divide =0x14 = 20 = 25 MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x15A,0x14); //PLL1_NDIV (LS) 
  }

  if (0){ //33,333.333 KHz
    std::cout << "33.333 MHz PLL Freq" << std::endl;
    //onboard 100MHz Oscillator at LMK OSCOUT pins
    //PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
    spi_120.spi_write(hw,LMK_SELECT,0x157,0x00); //CLKIN2_DIV (MS) R divide = 3 = 3 = 33.333333 MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x158,0x03); //CLKIN2_DIV (LS) 
    //Onboard 500MHz VCSO at LMK OSCIN pins
    //PLL1 N divider, Divide 500MHz VCSO down to PDF
    spi_120.spi_write(hw,LMK_SELECT,0x159,0x00); //PLL1_NDIV (MS)   N divide =0xF = 15 = 33.333333 MHz  
    spi_120.spi_write(hw,LMK_SELECT,0x15A,0x0F); //PLL1_NDIV (LS) 
  }

  //PLL1 Configuration
  std::cout << "PLL1 configuration" << std::endl;
  spi_120.spi_write(hw,LMK_SELECT,0x15B,0xDF); //PLL1_SETUP,  Dlig lock det window 43ns, PLL active, Pos Slope, max pump current 
  spi_120.spi_write(hw,LMK_SELECT,0x15C,0x20); //PLL1_LOCK_CNT (MS)    Lock detector window,  must be valid for 1024 cycles
  spi_120.spi_write(hw,LMK_SELECT,0x15D,0x00); //PLL1_LOCK_CNT (LS)   
  spi_120.spi_write(hw,LMK_SELECT,0x15E,0x00); //PLL1_DLY   // not applicable keep at 0 
  spi_120.spi_write(hw,LMK_SELECT,0x15F,0x0B); //STATUS_LD1_MUX == LD1 Push-Pull Output
      
  //PLL2 configured to lock VCO1 at 3000MHz to 500MHz VCSO with a PFD of 125MHz, (4N * 6P = 24) * 125MHz = 3000MHz
  //a prescale value of 6 allows the PLL2 N and R to match 
  std::cout << "PLL2 configuration" << std::endl;
  spi_120.spi_write(hw,LMK_SELECT,0x160,0x00); //PLL2_RDIV (MS) PLL2 Reference Divider = 4 refference frequency = 125MHz
  spi_120.spi_write(hw,LMK_SELECT,0x161,0x04); //PLL2_RDIV (LS)
  spi_120.spi_write(hw,LMK_SELECT,0x162,0xD0); //PLL2_PRESCALE PRE=6,  >255 to 500MHz  range  amp off, doubler off  
  spi_120.spi_write(hw,LMK_SELECT,0x163,0x00); //PLL2_NCAL (HI) Only used during CAL 
  spi_120.spi_write(hw,LMK_SELECT,0x164,0x00); //PLL2_NCAL (MID)
  spi_120.spi_write(hw,LMK_SELECT,0x165,0x04); //PLL2_NCAL (LOW)
      
  //the following 5 writes are out of sequence per the TI programming sequence recomendations in the data sheet
  spi_120.spi_write(hw,LMK_SELECT,0x145,0x7F); //always 127 / 0x7F
  spi_120.spi_write(hw,LMK_SELECT,0x171,0xAA); //
  spi_120.spi_write(hw,LMK_SELECT,0x171,0x02); //   
  spi_120.spi_write(hw,LMK_SELECT,0x17C,0x15); //OPT_REG1 
  spi_120.spi_write(hw,LMK_SELECT,0x17D,0x33); //OPT_REG2
      
  spi_120.spi_write(hw,LMK_SELECT,0x166,0x00); //PLL2_NDIV (HI) Allow CAL 
  spi_120.spi_write(hw,LMK_SELECT,0x167,0x00); //PLL2_NDIV (MID) PLL2 N-Divider     
  spi_120.spi_write(hw,LMK_SELECT,0x168,0x04); //PLL2_NDIV (LOW) Cal after writing this register     >>P = 3, N = 8  (24 * 125Mhz_ref = 3G) 
  spi_120.spi_write(hw,LMK_SELECT,0x169,0x49); //PLL2_SETUP Window 3.7nS,  I(cp)=1.6mA, Pos Slope, CP ! Tristate, Bit 0 always 1 
  //1.6mA gives better close in phase  noise than 3.2mA
      
  spi_120.spi_write(hw,LMK_SELECT,0x16A,0x00); //PLL2_LOCK_CNT (MS)
  spi_120.spi_write(hw,LMK_SELECT,0x16B,0x20); //PLL2_LOCK_CNT (LS)  PD must be in lock for 16 cycles 
  spi_120.spi_write(hw,LMK_SELECT,0x16C,0x00); //PLL2_LOOP_FILTER_R Disable Internal Resistors<< Uses externla Loop Filter 
  // R3 = 200 Ohms  R4 = 200 Ohms  
  spi_120.spi_write(hw,LMK_SELECT,0x16D,0x00); //PLL2_LOOP_FILTER_C Disable Internal Caps     << uses externla loop filter
  //C3 = 10pF  C4 = 10pF
  spi_120.spi_write(hw,LMK_SELECT,0x16E,0x12); //STATUS_LD2_MUX LD2=Locked   Push Pull Output
  spi_120.spi_write(hw,LMK_SELECT,0x173,0x00); //PLL2_MISC PLL2 Active, normal opperation 
      
  //register 0x174 not used on LMK04828
      
  if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL){
    // Enable the following code to allow for external clocking
    // External clock    Modified for FMC14x
    // Assumes FPGA has been programmed for CTRL1=0  CTRL2=1   Select External Clock
    spi_120.spi_write(hw,LMK_SELECT,0x138,0x40);  //PLL2_MISC PLL2 Active 
    spi_120.spi_write(hw,LMK_SELECT,0x100,0x01);  //Dclk0  divider = 1
    spi_120.spi_write(hw,LMK_SELECT,0x108,0x22);  //Dclk1  divider = 1
    spi_120.spi_write(hw,LMK_SELECT,0x110,0x01);  //Dclk2  divider = 1 
    spi_120.spi_write(hw,LMK_SELECT,0x118,0x01);  //Dclk3  divider = 1 
    spi_120.spi_write(hw,LMK_SELECT,0x120,0x22);  //Dclk4  divider = 2
    spi_120.spi_write(hw,LMK_SELECT,0x128,0x22);  //Dclk5  divider = 2
    spi_120.spi_write(hw,LMK_SELECT,0x130,0x01);  //Dclk6  divider = 1
	
    spi_120.spi_write(hw,LMK_SELECT,0x103,0x02);  //Dclk0 = Bypass 
    spi_120.spi_write(hw,LMK_SELECT,0x10B,0x02);  //Dclk2 = Bypass 
    spi_120.spi_write(hw,LMK_SELECT,0x113,0x02);  //Dclk4 = Bypass 
    spi_120.spi_write(hw,LMK_SELECT,0x11B,0x02);  //Dclk6 = Bypass 
    spi_120.spi_write(hw,LMK_SELECT,0x133,0x02);  //Dclk10 = Bypass 
	
    spi_120.spi_write(hw,LMK_SELECT,0x13A,0x00);  //Sysref divider high
    spi_120.spi_write(hw,LMK_SELECT,0x13B,0x20);  //sysref divider low = 32
	
    //CLkin0 is not used for clock generation in this mode. Disconnect to prevent undesired effects when syncing the dividerspi_120.
    spi_120.spi_write(hw,LMK_SELECT,0x147,0x03);  //Clock Input Select CLKIN0 Manual, CLKin0_Buffer-> SYSREF Mux CLKin1 = Fin
  }

  sleep(0.1); //allow PLL to lock
      
  //Clear PLL1 Errors regardless of if we use them
  spi_120.spi_write(hw,LMK_SELECT,0x182,0x01);
  spi_120.spi_write(hw,LMK_SELECT,0x182,0x00);
      
  //Clear PLL2 Errors regardless of if we use them
  spi_120.spi_write(hw,LMK_SELECT,0x183,0x01);
  spi_120.spi_write(hw,LMK_SELECT,0x183,0x00);
      
  //IF we are using Either PLL then wait500ms  to see if we ever go out of lock
  if (clockmode == CLOCKTREE_CLKSRC_INTERNAL or 
      clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF){
    sleep(0.5);
    //verify PLL1 status
    uint32_t dword = spi_120.spi_read(hw,LMK_SELECT,0x182); 
    uint32_t dword2 = static_cast<uint16_t>(dword) & 0x07;
    if (static_cast<uint16_t>(dword) & 0x02 != 0x02){
      std::cout << "PLL1 not locked" << std::endl;
    }
    else{
      std::cout << "PLL1 locked" << std::endl;
    }
    //verify PLL2 status
    dword = spi_120.spi_read(hw,LMK_SELECT,0x183); 
    dword2 = static_cast<uint16_t>(dword)& 0x07;
    if (static_cast<uint16_t>(dword) & 0x02 != 0x02){
      std::cout << "PLL2 not locked" << std::endl;
    }
    else{
      std::cout << "PLL2 locked" << std::endl;
    }
  }
      
  //try to sync all the output dividers
  // SYNC_MODE enable to SYNC event
  // SYSREF_CLR = 1
  // SYNC_1SHOT_EN = 1
  // SYNC_POL = 0 (Normal)
  // SYNC_EN = 1
  // SYNC_MODE = 1 (sync_event_generatedfrom SYNC pin);
  spi_120.spi_write(hw,LMK_SELECT,0x143,0xD1);
  // change SYSREF_MUX to normal SYNC (0)
  spi_120.spi_write(hw,LMK_SELECT,0x139,0x00);
  // Enable dividers reset
  spi_120.spi_write(hw,LMK_SELECT,0x144,0x00);
  // toggle the polarity (keep SYSREF_CLR active)
  spi_120.spi_write(hw,LMK_SELECT,0x143,0xF1);
  sleep(0.01);
      
  spi_120.spi_write(hw,LMK_SELECT,0x143,0xD1);
  // disable dividers
  spi_120.spi_write(hw,LMK_SELECT,0x144,0xFF);
  // change SYSREF_MUX back to continuous
  spi_120.spi_write(hw,LMK_SELECT,0x139,0x03);
  // restore SYNC_MODE & remove SYSREF_CLR
  spi_120.spi_write(hw,LMK_SELECT,0x143,0x50);
      
  return 0;
  
}
    
