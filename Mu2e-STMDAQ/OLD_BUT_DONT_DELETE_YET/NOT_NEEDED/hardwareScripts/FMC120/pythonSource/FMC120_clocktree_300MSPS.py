#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

LMK_SELECT=0x1
DAC_SELECT=0x2
ADC0_SELECT=0x4
ADC1_SELECT=0x8
ADC_SELECT_BOTH=0xC

CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */


def reset_clock_chip(wibNode):
    
   dword = 0
   
   wibNode.getNode("i2c_master.i2c.byte").write(0x00)
   wibNode.dispatch()
   
   dword=s.i2c_read(wibNode,0x1C03)
   dword |= 0x01 # Set reset bit
   s.i2c_write(wibNode,0x1c03,dword)
   dword &= 0xFE # Clear reset bit
   s.i2c_write(wibNode,0x1c03,dword)

   return 0

def osc100_enable(wibNode,enable):
   
   wibNode.getNode("i2c_master.i2c.byte").write(0x00)
   wibNode.dispatch()
   
   # Read, Modify, Write to avoid clobbering any other register settings
   dword=s.i2c_read(wibNode,0x1C01)
   if (enable):
      dword |= 0x01 # Set enable bit
      s.i2c_write(wibNode,0x1c01,dword)
   else:
      dword &= 0xFE # Clear enable bit
      s.i2c_write(wibNode,0x1c01,dword)
   
   return 0

def osc500_enable(wibNode,enable):
   
   wibNode.getNode("i2c_master.i2c.byte").write(0x00)
   wibNode.dispatch()
   
   # Read, Modify, Write to avoid clobbering any other register settings
   dword=s.i2c_read(wibNode,0x1C01)
   if (enable):
      dword |= 0x02 # Set enable bit
      s.i2c_write(wibNode,0x1c01,dword)
   else:
      dword &= 0xFD # Clear enable bit
      s.i2c_write(wibNode,0x1c01,dword)
   
   return 0

def FMC120_clocktree_init(wibNode,clockmode):
   
   print("Resetting the clock chip") 
   reset_clock_chip(wibNode)
   time.sleep(5)
   
   s.spi_write(wibNode,LMK_SELECT,0x000,0x80) #Force a reset
   s.spi_write(wibNode,LMK_SELECT,0x000,0x00) #Clear reset 
   s.spi_write(wibNode,LMK_SELECT,0x000,0x10) #Force SPI to be 4-wire
   s.spi_write(wibNode,LMK_SELECT,0x148,0x33) #CLKIN_SEL0_MUX Configured as LMK MISO Push Pull Output
   s.spi_write(wibNode,LMK_SELECT,0x002,0x00) #POWERDOWN Disabled (Normal operation) 
   print("CLK 0/1")
   # CLK0/1 Settings DAC 1GHz
   s.spi_write(wibNode,LMK_SELECT,0x100,0x6A) #DIV_CLKOUT1_0 DIV_BY_3 = 1GHz, IDL/ODL == 1  ==> In/Out Drive level = higher
   s.spi_write(wibNode,LMK_SELECT,0x101,0x22) #DIG_DLY_DCLK0 Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x103,0x05) #ANA_DLY_DCLK2 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX)   
   s.spi_write(wibNode,LMK_SELECT,0x104,0x62) #DIG_DLY_SCLK1 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out   
   s.spi_write(wibNode,LMK_SELECT,0x105,0x00) #ANA_DLY_SCLK1 SCLK analog delay disabled 
   s.spi_write(wibNode,LMK_SELECT,0x106,0xB0) #PD_CLK1_0 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
   s.spi_write(wibNode,LMK_SELECT,0x107,0x55) #Dclk =LVPECL, Sclk = LVPECL, !DCLK_INV, !SCLK_INV    
   print("CLK 2/3")
   # CLK2/3 Settings  Output to FPGA
   s.spi_write(wibNode,LMK_SELECT,0x108,0x74) #DIV_CLKOUT  DIV_CLKOUT1_0 DIV_BY_3 = 1GHz, IDL/ODL == 1  ==> In/Out Drive level = higher
   s.spi_write(wibNode,LMK_SELECT,0x109,0x22) #DIG_DLY_DCLK0 Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x10B,0x05) #ANA_DLY_DCLK2 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX)   
   s.spi_write(wibNode,LMK_SELECT,0x10C,0x62) #DIG_DLY_SCLK3  DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
   s.spi_write(wibNode,LMK_SELECT,0x10D,0x0A) #ANA_DLY_SCLK3  SCLK analog delay disabled
   s.spi_write(wibNode,LMK_SELECT,0x10E,0xB0) #PD_CLK2/3 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK  
   s.spi_write(wibNode,LMK_SELECT,0x10F,0x11) #FMT_CLK3_3 Dclock = LVPECL, Sclock = LVPECL !DCLK_INV, !SCLK_INV   
   print("CLK 4/5")
   # CLK4/5 Settings  ADCB 1GHz
   s.spi_write(wibNode,LMK_SELECT,0x110,0x6A) #DIV_CLKOUT5_4  DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level = higher  
   s.spi_write(wibNode,LMK_SELECT,0x111,0x22) #DIG_DLY_DCLK0 Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x113,0x05) #ANA_DLY_DCLK4 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX)         
   s.spi_write(wibNode,LMK_SELECT,0x114,0x62) #DIG_DLY_SCLK5 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
   s.spi_write(wibNode,LMK_SELECT,0x115,0x00) #ANA_DLY_SCLK5 SCLK analog delay disabled
   s.spi_write(wibNode,LMK_SELECT,0x116,0xB0) #PD_CLK5_4 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
   s.spi_write(wibNode,LMK_SELECT,0x117,0x57) #DIV_CLKOUT5_4 	Dclk =LVPECL, Sclk =LVPECL, !DCLK_INV, !SCLK_INV 
   print("CLK 6/7")
   # // CLK6/7 Settings  ADCA 1GHz
   s.spi_write(wibNode,LMK_SELECT,0x118,0x6A) #DIV_CLKOUT7_6     DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level 
   s.spi_write(wibNode,LMK_SELECT,0x119,0x22) #DIG_DLY_DCLK0 Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x11B,0x05) #ANA_DLY_DCLK6 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX)         
   s.spi_write(wibNode,LMK_SELECT,0x11C,0x62) #DIG_DLY_SCLK7 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
   s.spi_write(wibNode,LMK_SELECT,0x11D,0x00) #ANA_DLY_SCLK7 SCLK analog delay disabled
   s.spi_write(wibNode,LMK_SELECT,0x11E,0xB0) #PD_CLK7_6 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
   s.spi_write(wibNode,LMK_SELECT,0x11F,0x57) #DIV_CLKOUT7_6 	Dclk =LVPECL, Sclk =LVPECL, !DCLK_INV, !SCLK_INV  
   print("CLK 8/9")
   # CLK8/9 Settings  DCLK 8 (LMK_DCLK8_M2C_TO_FPGA_P) this clock drives GBTCLK0M2C_PN FMC pins D4,D5 Clock to FPGa (250MHz for new designs)
   s.spi_write(wibNode,LMK_SELECT,0x120,0x74) #DIV_CLKOUT9_8   	// DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level normal  
   s.spi_write(wibNode,LMK_SELECT,0x121,0x22) #DIG_DLY_DCLK0 Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x123,0x05) #ANA_DLY_DCLK8 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX)         
   s.spi_write(wibNode,LMK_SELECT,0x124,0x62) #DIG_DLY_SCLK9 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
   s.spi_write(wibNode,LMK_SELECT,0x125,0x00) #ANA_DLY_SCLK9 SCLK analog delay disabled
   s.spi_write(wibNode,LMK_SELECT,0x126,0xB7) #PD_CLK9_8 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
   s.spi_write(wibNode,LMK_SELECT,0x127,0x11) #Sclk OFF   Dclock = LVDS 
   print("CLK 10/11")
   # CLK10/11 Settings  DCLK 10 (LMK_DCLK10_M2C_TO_FPGA_P) this clock drives GBTCLK0M2C_PN FMC pins B20,B21
   s.spi_write(wibNode,LMK_SELECT,0x128,0x74) #DIV_CLKOUT9_8   	// DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level normal  
   s.spi_write(wibNode,LMK_SELECT,0x129,0x22) #DIG_DLY_DCLK0 Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x12B,0x05) #ANA_DLY_DCLK10 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX)         
   s.spi_write(wibNode,LMK_SELECT,0x12C,0x62) #DIG_DLY_SCLK11 DCLK halfstep=-0.5, SCLK sourced from DCLK,  2 vco clock cycle delay on sysref out       
   s.spi_write(wibNode,LMK_SELECT,0x12D,0x00) #ANA_DLY_SCLK11 SCLK analog delay disabled
   s.spi_write(wibNode,LMK_SELECT,0x12E,0xB7) #PD_CLK11_10 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, EN_SCLK 
   s.spi_write(wibNode,LMK_SELECT,0x12F,0x11) #Sclk OFF   Dclock = LVDS         
   print("CLK 12/13")
   # External Clock Output, This will be programed 'on' but we will Clear it off in final configuration
   # CLK12/13 Settings  DCLK 12 EXTERNAL CLOCK OUTPUT 
   s.spi_write(wibNode,LMK_SELECT,0x130,0x63) #DIV_CLKOUT13_12   	// DIV_BY_3 = 1GHz, IDL/ODL= 1  ==>In/Out Drive level normal  
   s.spi_write(wibNode,LMK_SELECT,0x131,0x22) #DIG_DLY_DCLK0 Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x133,0x05) #ANA_DLY_DCLK12 No Analog Delay, Half step duty cycle correction, 50% Duty Cycle!  (DAC Spec id 40 -/ 60% MAX)         
   s.spi_write(wibNode,LMK_SELECT,0x134,0x62) #DIG_DLY_SCLK13    SCLK analog = 2 VCO Clock Cycles (666pS << from FMC144 )     
   s.spi_write(wibNode,LMK_SELECT,0x135,0x00) #ANA_DLY_SCLK13 SCLK analog delay disabled
   s.spi_write(wibNode,LMK_SELECT,0x136,0xB7) #PD_CLK13_12 !DIG_DLY, GLITCHLESS Half Step ON, !ANAGLITCH, !ANA_DLY, EN_DCLK, ACTIVE, SCLK Disabled at VCM     
   s.spi_write(wibNode,LMK_SELECT,0x137,0x00) #Sclk Off, Dclock On = LVPECL				<< set to 0x00 to turn off Externl Clock option 
   
   #Select VCO1 PLL1 source
   s.spi_write(wibNode,LMK_SELECT,0x138,0x20) #VCO_OSC_OUT VCO01 (High Speed), PLL1_FB=OSCin,OSCOUT=input (Drivers powerd down)     
   s.spi_write(wibNode,LMK_SELECT,0x139,0x03) #SYSREF_MUX SYSREF output source = Normal Sync 	start with 0 reprogram when turning on sysref   

   #SYSREF Divider (SYSREF = LINE_RATE / (10 * F * K ))
   s.spi_write(wibNode,LMK_SELECT,0x13A,0x01) #SYSREF_DIV(MS) SYSREF Divider   
   s.spi_write(wibNode,LMK_SELECT,0x13B,0x40) #SYSREF_DIV(LS) SYSREF Divider              

   #SYSREF Digital Delay
   s.spi_write(wibNode,LMK_SELECT,0x13C,0x00) #SYSREF_DDLY(MS) SYSREF Digital Delay  - Not Used  
   s.spi_write(wibNode,LMK_SELECT,0x13D,0x08) #SYSREF_DDLY(LS) SYSREF Digital Delay  - Not Used 
   
   s.spi_write(wibNode,LMK_SELECT,0x13E,0x00) #SYSREF_PULSE_CNT 8 Pulses - Not Used
   s.spi_write(wibNode,LMK_SELECT,0x13F,0x00) #FB_CTRL PLL2_FB=prescaler, PLL1_FB=OSCIN   This is default for internal Oscillator, this changes on EXT osc   
   s.spi_write(wibNode,LMK_SELECT,0x140,0x01) #OSCIN_SYSREF_PD Active= PLL1, LDO, VCO, OSCIN, SYSREF         
   s.spi_write(wibNode,LMK_SELECT,0x141,0x00) #DIG_DLY_REG Disable all digital delays     
   s.spi_write(wibNode,LMK_SELECT,0x142,0x00) #DIG_DLY_STEP_CNT No Adjustment of Digital Delay 
   s.spi_write(wibNode,LMK_SELECT,0x143,0x70) #SYNC_SYSREF SYNC functionality enabled, prevent SYNC pin and DLD flags from generating SYNC event
                                              #DCLK12, DCLK10, DCLK8 do not re-sync during a sync event    
   s.spi_write(wibNode,LMK_SELECT,0x144,0xFF) #DISABLE_DCLK_SYNC Prevent SYSREF clocks from synchronizing 		<< set to 0x00 to turn off Externl Clock option      
   s.spi_write(wibNode,LMK_SELECT,0x145,0x7F) #FIXED Always 0x7F      
   s.spi_write(wibNode,LMK_SELECT,0x146,0x00) #CLKIN_SRC No AutoSwitching of clock inputs, all 3 CLKINx pins are set t0 Bipolar,  

   #******** discussion required ***
   #	CLKin0 = Hardware trig mux	use for external trigger function
   #	CLKin1 = External reference Signal (or externals Clock Signal)
   #	CLKin2 = Onboard 100MHz reference, (uses OSCOUT pins) 
   #		NOTE Rich thinks that since CLKin0 is the output of the hardware trigger mux, that the lower nibble of 0x147 should be different
   #		i.e 0x8 instead of 0xA, not sending clkin0 to PLL1     
   
   # External ref
   if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF):
      print("Using External Clock")
      osc100_enable(wibNode,0)
      print("100 MHz Oscillator OFF") 
      s.spi_write(wibNode,LMK_SELECT,0x147,0x1A) # CLKin_SEL_MODE = CLKin1 Manual, CLKin1_OUT_MUX = Fin, CLKin0_OUT_MUX = PLL1 
  
   # External clock
   if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL):
      print("Using External Clock")
      osc100_enable(wibNode,0)
      print("100 MHz Oscillator OFF") 
      osc500_enable(wibNode,0)
      print("500 Mhz VCSO OFF")
      #External clock
      s.spi_write(wibNode,LMK_SELECT,0x147,0x12) # CLKin_SEL_MODE = CLKin1 Manual, CLKin1_OUT_MUX = Fin, CLKin0_OUT_MUX = PLL1  

   # Internal reference
   if (clockmode == CLOCKTREE_CLKSRC_INTERNAL):
      s.spi_write(wibNode,LMK_SELECT,0x147,0x2A) # CLKIN_MUX CLKIN = clkin1 (External Reference connector), !INVERT, CLKIN1=PLL1, CLKIN0=SYSREF MUX 
      print("Using Internal Reference Oscillator")
      osc100_enable(wibNode,1)
      print("100 MHz Oscillator ON") 
      osc500_enable(wibNode,1)
      print("500 Mhz VCSO ON")
   
   s.spi_write(wibNode,LMK_SELECT,0x148,0x33) #CLKIN_SEL0_MUX Configured as LMK MISO Push Pull Output
   s.spi_write(wibNode,LMK_SELECT,0x149,0x00) #CLKIN_SEL1_MUX SPI SDIO_readback = PUSH-PULL !! CLKIN_SEL1=input  
   
   s.spi_write(wibNode,LMK_SELECT,0x14A,0x00) #RESET_MUX RESET Pin=Input   
   s.spi_write(wibNode,LMK_SELECT,0x14B,0x05) #Holdover mode off Manual DAC Enabled   
   s.spi_write(wibNode,LMK_SELECT,0x14C,0xFF) #MANUAL_DAC Force DAC to midscale 0x01FF        
   s.spi_write(wibNode,LMK_SELECT,0x14D,0x00) #DAC_TRIP_LOW Min Voltage to force HOLDOVER    
   s.spi_write(wibNode,LMK_SELECT,0x14E,0x00) # DAC_TRIP_HIGH Mult=4 Max Voltage to force HOLDOVER
                                               
   s.spi_write(wibNode,LMK_SELECT,0x14F,0x7F) #DAC_UPDATE_CNTR      
   s.spi_write(wibNode,LMK_SELECT,0x150,0x00) #HOLDOVER_SET HOLDOVER disable      
   s.spi_write(wibNode,LMK_SELECT,0x151,0x02) #HOLD_EXIT_COUNT(MS)
   s.spi_write(wibNode,LMK_SELECT,0x152,0x00) #HOLD_EXIT_COUNT(LS)

   #PLL1 CLKIN0 Divider, FMC120 trigger mux feeds this input
   s.spi_write(wibNode,LMK_SELECT,0x153,0x00) #CLKIN0_DIV (MS)    
   s.spi_write(wibNode,LMK_SELECT,0x154,0x78) #CLKIN0_DIV (LS)
   
   #External Reference Input
   # PLL1 CLKIN1 Divider External Reference input, default 100MHz and a 100.000KHz Phase Detector Frequency
   s.spi_write(wibNode,LMK_SELECT,0x155,0x00) #CLKIN1_DIV (MS)      100MHz / 1000 = 100KHz
   s.spi_write(wibNode,LMK_SELECT,0x156,0x0A) #CLKIN1_DIV (LS) 
   
   #*********** PLL 1 REF Set
   #note. The FMC120 PLL1 Loop filter is designed for an 11Hz bandwidth at a PDF of 1MHz
   if (0): #100 KHz	Default PLL Frequency
       print("100 KHz PLL Freq")
       #onboard 100MHz Oscillator at LMK OSCOUT pins
       #PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
       s.spi_write(wibNode,LMK_SELECT,0x157,0x03) #CLKIN2_DIV (MS) 		100MHz / 1000 = 100KHz 1000d = 0x03E8  
       s.spi_write(wibNode,LMK_SELECT,0x158,0xE8) #CLKIN2_DIV (LS) 
       #Onboard 500MHz VCSO at LMK OSCIN pins
       #PLL1 N divider, Divide 500MHz VCSO down to PDF
       s.spi_write(wibNode,LMK_SELECT,0x159,0x13) #PLL1_NDIV (MS)  PLL1 Ndivider = 5000 for 100HHz PDF  
       s.spi_write(wibNode,LMK_SELECT,0x15A,0x88) #PLL1_NDIV (LS) 

   if (0): #500 KHz	
       print("500 KHz PLL Freq")
       #onboard 100MHz Oscillator at LMK OSCOUT pins
       #PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
       s.spi_write(wibNode,LMK_SELECT,0x157,0x00) #CLKIN2_DIV (MS) 		R divide = 200 = 500KHz  
       s.spi_write(wibNode,LMK_SELECT,0x158,0xC8) #CLKIN2_DIV (LS) 
       #Onboard 500MHz VCSO at LMK OSCIN pins
       #PLL1 N divider, Divide 500MHz VCSO down to PDF
       s.spi_write(wibNode,LMK_SELECT,0x159,0x03) #PLL1_NDIV (MS)  N divide = 1000 = 500MHz PDF  
       s.spi_write(wibNode,LMK_SELECT,0x15A,0xE8) #PLL1_NDIV (LS) 
   
   if (0): #1 MHz	
       print("1 MHz PLL Freq")
       #onboard 100MHz Oscillator at LMK OSCOUT pins
       #PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
       s.spi_write(wibNode,LMK_SELECT,0x157,0x00) #CLKIN2_DIV (MS) 		R divide = 0x64 = 100 = 1MHz  
       s.spi_write(wibNode,LMK_SELECT,0x158,0x64) #CLKIN2_DIV (LS) 
       #Onboard 500MHz VCSO at LMK OSCIN pins
       #PLL1 N divider, Divide 500MHz VCSO down to PDF
       s.spi_write(wibNode,LMK_SELECT,0x159,0x01) #PLL1_NDIV (MS)  N divide =0x1f4 = 500 = 1 MHz  
       s.spi_write(wibNode,LMK_SELECT,0x15A,0xF4) #PLL1_NDIV (LS)
   
   if (1): #10 MHz	
       print("10 MHz PLL Freq")
       #onboard 100MHz Oscillator at LMK OSCOUT pins
       #PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
       s.spi_write(wibNode,LMK_SELECT,0x157,0x00) #CLKIN2_DIV (MS) 		R divide = A = 10 = 10MHz  
       s.spi_write(wibNode,LMK_SELECT,0x158,0x0A) #CLKIN2_DIV (LS) 
       #Onboard 500MHz VCSO at LMK OSCIN pins
       #PLL1 N divider, Divide 500MHz VCSO down to PDF
       s.spi_write(wibNode,LMK_SELECT,0x159,0x00) #PLL1_NDIV (MS)   N divide =0x32 = 50 = 10 MHz  
       s.spi_write(wibNode,LMK_SELECT,0x15A,0x32) #PLL1_NDIV (LS)  

   if (0): #25 MHz	
       print("25 MHz PLL Freq")
       #onboard 100MHz Oscillator at LMK OSCOUT pins
       #PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
       s.spi_write(wibNode,LMK_SELECT,0x157,0x00) #CLKIN2_DIV (MS) 		R divide = 4 = 4 = 25MHz  
       s.spi_write(wibNode,LMK_SELECT,0x158,0x04) #CLKIN2_DIV (LS) 
       #Onboard 500MHz VCSO at LMK OSCIN pins
       #PLL1 N divider, Divide 500MHz VCSO down to PDF
       s.spi_write(wibNode,LMK_SELECT,0x159,0x00) #PLL1_NDIV (MS)   N divide =0x14 = 20 = 25 MHz  
       s.spi_write(wibNode,LMK_SELECT,0x15A,0x14) #PLL1_NDIV (LS) 

   if (0): #33,333.333 KHz	
       print("33.333 MHz PLL Freq")
       #onboard 100MHz Oscillator at LMK OSCOUT pins
       #PLL1 CLKIN2 Divider, divides onboard 100MHz Ref Osc output down to Phase Detector Frequency 
       s.spi_write(wibNode,LMK_SELECT,0x157,0x00) #CLKIN2_DIV (MS) 		R divide = 3 = 3 = 33.333333 MHz  
       s.spi_write(wibNode,LMK_SELECT,0x158,0x03) #CLKIN2_DIV (LS) 
       #Onboard 500MHz VCSO at LMK OSCIN pins
       #PLL1 N divider, Divide 500MHz VCSO down to PDF
       s.spi_write(wibNode,LMK_SELECT,0x159,0x00) #PLL1_NDIV (MS)   N divide =0xF = 15 = 33.333333 MHz  
       s.spi_write(wibNode,LMK_SELECT,0x15A,0x0F) #PLL1_NDIV (LS) 

   #PLL1 Configuration
   print("PLL1 configuration")
   s.spi_write(wibNode,LMK_SELECT,0x15B,0xDF) #PLL1_SETUP,  Dlig lock det window 43ns, PLL active, Pos Slope, max pump current 
   s.spi_write(wibNode,LMK_SELECT,0x15C,0x20) #PLL1_LOCK_CNT (MS)    Lock detector window,  must be valid for 1024 cycles
   s.spi_write(wibNode,LMK_SELECT,0x15D,0x00) #PLL1_LOCK_CNT (LS)   
   s.spi_write(wibNode,LMK_SELECT,0x15E,0x00) #PLL1_DLY   // not applicable keep at 0 
   s.spi_write(wibNode,LMK_SELECT,0x15F,0x0B) #STATUS_LD1_MUX == LD1 Push-Pull Output

   #PLL2 configured to lock VCO1 at 3000MHz to 500MHz VCSO with a PFD of 125MHz, (4N * 6P = 24) * 125MHz = 3000MHz
   #a prescale value of 6 allows the PLL2 N and R to match 
   print("PLL2 configuration")
   s.spi_write(wibNode,LMK_SELECT,0x160,0x00) #PLL2_RDIV (MS) PLL2 Reference Divider = 4 refference frequency = 125MHz
   s.spi_write(wibNode,LMK_SELECT,0x161,0x04) #PLL2_RDIV (LS)
   s.spi_write(wibNode,LMK_SELECT,0x162,0xD0) #PLL2_PRESCALE PRE=6,  >255 to 500MHz  range  amp off, doubler off  
   s.spi_write(wibNode,LMK_SELECT,0x163,0x00) #PLL2_NCAL (HI) Only used during CAL 
   s.spi_write(wibNode,LMK_SELECT,0x164,0x00) #PLL2_NCAL (MID)
   s.spi_write(wibNode,LMK_SELECT,0x165,0x04) #PLL2_NCAL (LOW)

   #the following 5 writes are out of sequence per the TI programming sequence recomendations in the data sheet
   s.spi_write(wibNode,LMK_SELECT,0x145,0x7F) #always 127 / 0x7F
   s.spi_write(wibNode,LMK_SELECT,0x171,0xAA) #
   s.spi_write(wibNode,LMK_SELECT,0x171,0x02) #   
   s.spi_write(wibNode,LMK_SELECT,0x17C,0x15) #OPT_REG1 
   s.spi_write(wibNode,LMK_SELECT,0x17D,0x33) #OPT_REG2
   
   s.spi_write(wibNode,LMK_SELECT,0x166,0x00) #PLL2_NDIV (HI) Allow CAL 
   s.spi_write(wibNode,LMK_SELECT,0x167,0x00) #PLL2_NDIV (MID) PLL2 N-Divider     
   s.spi_write(wibNode,LMK_SELECT,0x168,0x04) #PLL2_NDIV (LOW) Cal after writing this register     >>P = 3, N = 8  (24 * 125Mhz_ref = 3G) 
   s.spi_write(wibNode,LMK_SELECT,0x169,0x49) #PLL2_SETUP Window 3.7nS,  I(cp)=1.6mA, Pos Slope, CP ! Tristate, Bit 0 always 1 
                                              #1.6mA gives better close in phase  noise than 3.2mA

   s.spi_write(wibNode,LMK_SELECT,0x16A,0x00) #PLL2_LOCK_CNT (MS)
   s.spi_write(wibNode,LMK_SELECT,0x16B,0x20) #PLL2_LOCK_CNT (LS)  PD must be in lock for 16 cycles 
   s.spi_write(wibNode,LMK_SELECT,0x16C,0x00) #PLL2_LOOP_FILTER_R Disable Internal Resistors	<< Uses externla Loop Filter 
                                              # R3 = 200 Ohms  R4 = 200 Ohms  
   s.spi_write(wibNode,LMK_SELECT,0x16D,0x00) #PLL2_LOOP_FILTER_C Disable Internal Caps     	<< uses externla loop filter
                                              #C3 = 10pF  C4 = 10pF
   s.spi_write(wibNode,LMK_SELECT,0x16E,0x12) #STATUS_LD2_MUX LD2=Locked   Push Pull Output
   s.spi_write(wibNode,LMK_SELECT,0x173,0x00) #PLL2_MISC PLL2 Active, normal opperation 

   #register 0x174 not used on LMK04828

   if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL):
       # Enable the following code to allow for external clocking
       # External clock    Modified for FMC14x
       # Assumes FPGA has been programmed for CTRL1=0  CTRL2=1   Select External Clock
       s.spi_write(wibNode,LMK_SELECT,0x138,0x40)  #PLL2_MISC PLL2 Active 
       s.spi_write(wibNode,LMK_SELECT,0x100,0x01)  #Dclk0  divider = 1
       s.spi_write(wibNode,LMK_SELECT,0x108,0x22)  #Dclk1  divider = 1
       s.spi_write(wibNode,LMK_SELECT,0x110,0x01)  #Dclk2  divider = 1 
       s.spi_write(wibNode,LMK_SELECT,0x118,0x01)  #Dclk3  divider = 1 
       s.spi_write(wibNode,LMK_SELECT,0x120,0x22)  #Dclk4  divider = 2
       s.spi_write(wibNode,LMK_SELECT,0x128,0x22)  #Dclk5  divider = 2
       s.spi_write(wibNode,LMK_SELECT,0x130,0x01)  #Dclk6  divider = 1
       
       s.spi_write(wibNode,LMK_SELECT,0x103,0x02)  #Dclk0 = Bypass 
       s.spi_write(wibNode,LMK_SELECT,0x10B,0x02)  #Dclk2 = Bypass 
       s.spi_write(wibNode,LMK_SELECT,0x113,0x02)  #Dclk4 = Bypass 
       s.spi_write(wibNode,LMK_SELECT,0x11B,0x02)  #Dclk6 = Bypass 
       s.spi_write(wibNode,LMK_SELECT,0x133,0x02)  #Dclk10 = Bypass 
       
       s.spi_write(wibNode,LMK_SELECT,0x13A,0x00)  #Sysref divider high
       s.spi_write(wibNode,LMK_SELECT,0x13B,0x20)  #sysref divider low = 32
       
       #CLkin0 is not used for clock generation in this mode. Disconnect to prevent undesired effects when syncing the dividers.
       s.spi_write(wibNode,LMK_SELECT,0x147,0x03)  #Clock Input Select CLKIN0 Manual, CLKin0_Buffer-> SYSREF Mux CLKin1 = Fin

   time.sleep(0.1) #allow PLL to lock

   #Clear PLL1 Errors regardless of if we use them	
   s.spi_write(wibNode,LMK_SELECT,0x182,0x01)
   s.spi_write(wibNode,LMK_SELECT,0x182,0x00)
   
   #Clear PLL2 Errors regardless of if we use them	
   s.spi_write(wibNode,LMK_SELECT,0x183,0x01)
   s.spi_write(wibNode,LMK_SELECT,0x183,0x00)

   #IF we are using Either PLL then wait500ms  to see if we ever go out of lock
   if (clockmode==CLOCKTREE_CLKSRC_INTERNAL) or (clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF):
      time.sleep(0.5)
      #verify PLL1 status
      dword=s.spi_read(wibNode,LMK_SELECT,0x182) 
      dword2=(int(dword,16)& 0x07)
      if (int(dword,16)&0x02 != 0x02):
         print("PLL1 not locked")
      else:
         print("PLL1 locked")
      #verify PLL2 status
      dword=s.spi_read(wibNode,LMK_SELECT,0x183) 
      dword2=(int(dword,16)& 0x07)
      if (int(dword,16)&0x02 != 0x02):
         print("PLL2 not locked")
      else:
         print("PLL2 locked")
   
   #try to sync all the output dividers
   # SYNC_MODE enable to SYNC event
   # SYSREF_CLR = 1
   # SYNC_1SHOT_EN = 1
   # SYNC_POL = 0 (Normal)
   # SYNC_EN = 1
   # SYNC_MODE = 1 (sync_event_generatedfrom SYNC pin)
   s.spi_write(wibNode,LMK_SELECT,0x143,0xD1)
   # change SYSREF_MUX to normal SYNC (0)
   s.spi_write(wibNode,LMK_SELECT,0x139,0x00)
   # Enable dividers reset
   s.spi_write(wibNode,LMK_SELECT,0x144,0x00)
   # toggle the polarity (keep SYSREF_CLR active)
   s.spi_write(wibNode,LMK_SELECT,0x143,0xF1)
   time.sleep(0.01)

   s.spi_write(wibNode,LMK_SELECT,0x143,0xD1)
   # disable dividers
   s.spi_write(wibNode,LMK_SELECT,0x144,0xFF)
   # change SYSREF_MUX back to continuous
   s.spi_write(wibNode,LMK_SELECT,0x139,0x03)
   # restore SYNC_MODE & remove SYSREF_CLR
   s.spi_write(wibNode,LMK_SELECT,0x143,0x50)
   
   return 0
