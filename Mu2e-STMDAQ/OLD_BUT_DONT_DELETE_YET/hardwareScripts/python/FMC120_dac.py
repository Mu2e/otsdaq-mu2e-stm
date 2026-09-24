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

FMC120_DAC_PART_ID = 0x0A

def reset_and_wake_dac(wibNode):
    # Set one byte per cycle
    wibNode.getNode("i2c_master.i2c.byte").write(0x00)
    wibNode.dispatch()
    #Read, Modify, Write to avoid clobbering any other register settings
    dword = s.i2c_read(wibNode,0x1C02)
    #Assert reset (active-LOW) and deassert unitapi_sleep_ms
    dword &= 0xCF
    s.i2c_write(wibNode,0x1C02,dword)
    #Clear reset bit
    dword |= 0x20
    s.i2c_write(wibNode,0x1C02,dword)
    return 0

def FMC120_short_pattern_test(wibNode):
    # 15-8 : alarm_from_shortest  bit8 = lane0 alarm, bit1 = lane1_alarm 
    # 7-0  : Loss of signal dectect outputs from SerDes lanes
    dword = s.spi_read(wibNode,DAC_SELECT,0x6D)
    print("Starting short pattern test ..")
    
    # FPGA pattern generation
    #     0x00 - WFM
    #     0x10 - Short Pattern Test
    #     0x20 - 4 Point Wave
    #     0x30 - WFM FIFO Swapped
    wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl").write(0x11)
    wibNode.dispatch()
    time.sleep(0.02)
   
    # Enable pattern checking in DAC bit12 = shorttest_ena
    s.spi_write(wibNode,DAC_SELECT,0x02,0x1082) # 0x1082 Twos Comp 0x1080 Offset Binary
    
    # Enable alarms for short pattern test
    s.spi_write(wibNode,DAC_SELECT,0x06,0x00FF) #
    time.sleep(1)
    dword = int(s.spi_read(wibNode,DAC_SELECT,0x6D),16)
    dword &=0xFF00
    if (dword):
       print ("Pattern failed")
       print(dword)
    else:
       print("Pattern Passed")
    
    # Force error
    wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl").write(0x01)
    wibNode.dispatch()
    time.sleep(1)
    dword = int(s.spi_read(wibNode,DAC_SELECT,0x6D),16)
    dword &=0xFF00
    if (dword != 0xFF00):
       print ("Error Injection Failed!")
       print (dword)
    else:
       print("Error Injection Passed.")

    # Disable short pattern test.
    s.spi_write(wibNode,DAC_SELECT,0x02,0x0082) # 0x0082 Twos 0x0080 Offset Binary

    return 0

def FMC120_dac_init(wibNode,odelay):
   
   dword = 0
   
   #turn off dac output, may be off already
   wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl").write(0x00)
   wibNode.dispatch()
   
   #perform hardware reset on DAC39J84 IC
   reset_and_wake_dac(wibNode)

   #Reset DAC Block, this puts the JESD in the init state untill everything is programed
   s.spi_write(wibNode,DAC_SELECT,0x4A,0xFF1E) 
   s.spi_write(wibNode,DAC_SELECT,0x46,0x0044)
   s.spi_write(wibNode,DAC_SELECT,0x47,0x190A)
   
   # Turn off sysref
   s.spi_write(wibNode,LMK_SELECT,0x10F,0x01)
   time.sleep(0.1)
   
   
   # Disable TX
   wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl").write(0x00)
   wibNode.dispatch()
   
   # Assert Reset for SPI, config for 4 wire mode
   s.spi_write(wibNode,DAC_SELECT,0x02,0x83) #0x0083 Twos	0x0081 Offset Binary
   time.sleep(0.01) #remove or set lower
   
   # Clear reset Bit, data set to twos complement 4-wire SPI
   s.spi_write(wibNode,DAC_SELECT,0x02,0x2082) #0x2082 Twos 0x2080 Offset Binary

   #DAC SPI check
   dword = s.spi_read(wibNode,DAC_SELECT,0x7F)
   dword = int(dword,16) & 0xFF
   if (dword != FMC120_DAC_PART_ID):
      print("Wrong DAC part ID")
      return -1 
   
   dword &= 0xFF 
   ddword = dword & 0x03
   dword &= 0x18 
   dword = dword/8
   print("DAC vendor %5x Version %5x" %( dword, ddword))
   
   #Output Polarity, Ch0 and Ch2 need to have their output polarity inverted to compensate for trace layout on the board.
   s.spi_write(wibNode,DAC_SELECT,0x01,0x0050) #complement DAC outputs Ch0 and Ch2 
   s.spi_write(wibNode,DAC_SELECT,0x03,0xA300) #DAC coarse current adjust , set to 20mA,
   
   # ALARM CONTROL- Assuming all are on by default
   s.spi_write(wibNode,DAC_SELECT,0x04,0x0000)
   s.spi_write(wibNode,DAC_SELECT,0x05,0xFF03)
   s.spi_write(wibNode,DAC_SELECT,0x06,0xFFFF)

   s.spi_write(wibNode,DAC_SELECT,0x1A,0x0000) #Puts DACs and PLL into NOT unitapi_sleep_ms
   
   #synchronization from where ? mixer and nco
   s.spi_write(wibNode,DAC_SELECT,0x1F,0x4440)  #PROBLEM WHEN SYNC WHEN USING SYSREF 4440 ?
   #synchronization from where ? mixer and nco
   s.spi_write(wibNode,DAC_SELECT,0x1E,0x4444) #PROBLEM WHEN SYNCWHEN USING SYSREF 4440
   s.spi_write(wibNode,DAC_SELECT,0x20,0x4044) #PROBLEM WHEN SYNC WHEN USING SYSREF 4440

   #sysref + clock dividers (use all sync pulses = 0x10, don't use synch pulse = 0x00, use only the next 0x20)
   s.spi_write(wibNode,DAC_SELECT,0x24,0x0020)
   s.spi_write(wibNode,DAC_SELECT,0x33,0xAF40) #"CP=350uA, VCO_bias=9.8mA, 4GHz VCO, 25d VCO fine adj"	// AF high 
   
   #Clock settings                              0x0028 = /5 mpy	// Low vco
   s.spi_write(wibNode,DAC_SELECT,0x3C,0x0228)  #SCR=0, High Density,  N=16  N'=16
   
   s.spi_write(wibNode,DAC_SELECT,0x3D,0x0088) # Enable Sampler compensation
                                               # bits 43 per TI ap note

   #This may be an issue, undescribed bits LOS bits 15:13
   s.spi_write(wibNode,DAC_SELECT,0x3E,0x0108) # 0x0108	 15:13 000 LOS?? 12:11 00 reserved, 10:8 term 010-AC,  
                                               # 7:=0 reserved   6:5=  Full rate   4:2 = 010    Busswidth 20bit,  1: ! sleep, 0: = 0 reserved
   
   # Write default state serdes pair inversion register.
   s.spi_write(wibNode,DAC_SELECT,0x3F,0x0000)

   INTERPOLATE = 0  # debug variable	
   # 0	no Interpolation, use LMK_DAC_CLK directly
   # 1	no interpolation, use DAC PLL
   # 2	Interpolate by 2, use DAC PLL

   if (INTERPOLATE == 0):
      s.spi_write(wibNode,DAC_SELECT,0x00,0x0018) #Interpolate by 1, Enable Alarm out, Pos alarm polarity
      # JESD Clock divider = Lanes * Interpolation / 4 ADCS = 0x4000  !! error
      s.spi_write(wibNode,DAC_SELECT,0x25,0x2000) # jesd pll reference = DACCLK/2
      # 31 statys same if still use PLL
      s.spi_write(wibNode,DAC_SELECT,0x31,0x1000) #  PLL in reset, not used
      #  0x0020 = P = 4
      s.spi_write(wibNode,DAC_SELECT,0x32,0x0000) #  0x0120 P = 4 = 1GHz, M = 2 = 500MHz  4Ghz / P-2 = 1GHz, 1GHz / 2 = 500MHz PFD
      # 0x0xxx selects DAC PLL as serdes ref 8xxx selects DACCLK input pins
      # 0x8xxx selects the DAC CLK as the referenece
      # 0xx8xx divided 1GHz DAC clk input by to for a 500MHz Serdes reference clock
      s.spi_write(wibNode,DAC_SELECT,0x3B,0x0800) # controls  serdes_refclk_div, serdes_clk_sel divide by 10 100 * 10 = 10GBPS 
                                                  # 00xx	vco feedback devider = 2	xx0x VCO prescaler = 0 
    
   if (INTERPOLATE == 1): #use DAC PLL 1GHz in 1GHz out 
      s.spi_write(wibNode,DAC_SELECT,0x00,0x0018) #Interpolate by 1, Enable Alarm out, Pos alarm polarity
      # JESD Clock divider = Lanes * Interpolation / 4 ADCS = 0x4000  !! error
      s.spi_write(wibNode,DAC_SELECT,0x25,0x2000) # jesd pll reference = DACCLK/2
      # 31 statys same if still use PLL
      s.spi_write(wibNode,DAC_SELECT,0x31,0x6408) #  6xxx widest lock detect, x4xx DACCLK PLL on N =2 2 for interpolation 6408 PLL N = 
      #  0x0020 = P = 4
      s.spi_write(wibNode,DAC_SELECT,0x32,0x0120) #  0x0120 P = 4 = 1GHz, M = 2 = 500MHz  4Ghz / P-2 = 1GHz, 1GHz / 2 = 500MHz PFD
      # 0x0xxx selects DAC PLL as serdes ref 8xxx selects DACCLK input pins
      # 0x8xxx selects the DAC CLK as the referenece
      # 0xx8xx divided 1GHz DAC clk input by to for a 500MHz Serdes reference clock
      s.spi_write(wibNode,DAC_SELECT,0x3B,0x8800) # controls  serdes_refclk_div, serdes_clk_sel divide by 10 100 * 10 = 10GBPS 
                                                  # 00xx	vco feedback devider = 2	xx0x VCO prescaler = 0 

   if (INTERPOLATE == 2): #use DAC PLL 1GHz in 2GHz out, interpolate by 2
      s.spi_write(wibNode,DAC_SELECT,0x00,0x0018) #Interpolate by 2, Enable Alarm out, Pos alarm polarity
      # JESD Clock divider = Lanes * Interpolation / 4 ADCS = 0x4000  
      s.spi_write(wibNode,DAC_SELECT,0x25,0x4000) # (8L * interp2 = 16)/4 jesd pll reference = DACCLK//4
      # 31 statys same if still use PLL
      s.spi_write(wibNode,DAC_SELECT,0x31,0x6408) #  6xxx widest lock detect, x4xx DACCLK PLL on N =2 2 for interpolation 6408 PLL N = 
      #  0x0020 = P = 4
      s.spi_write(wibNode,DAC_SELECT,0x32,0x0300) #  0x0300 P = 2 = 2GHz, M = 4 = 500MHz  4Ghz / P-2 = 2GHz, 2GHz / 4 = 500MHz PFD
      # 0x0xxx selects DAC PLL as serdes ref 8xxx selects DACCLK input pins
      # 0x8xxx selects the DAC CLK as the referenece
      # 0xx8xx divided 1GHz DAC clk input by to for a 500MHz Serdes reference clock
      s.spi_write(wibNode,DAC_SELECT,0x3B,0x9800) # controls  serdes_refclk_div, serdes_clk_sel divide by 10 100 * 10 = 10GBPS 
                                                  # 00xx	vco feedback devider = 2	xx0x VCO prescaler = 0

   # config_73 Link3=67 Link2=45 Link1=23 Link0=01 // SUPPOSELY zero this out // 0xFA50 // 0x5050
   s.spi_write(wibNode,DAC_SELECT,0x49,0x0)
   # We will only use Link 0 for Sync output request
   s.spi_write(wibNode,DAC_SELECT,0x61,0x01)
   # JESD SETTINGS
   s.spi_write(wibNode,DAC_SELECT,0x4B,0x1F00) # Buffer=32  Frames = 1, Octets per frame = 1
   s.spi_write(wibNode,DAC_SELECT,0x4C,0x1F07) # K=32   L=8  ######### Verify john?
   s.spi_write(wibNode,DAC_SELECT,0x4D,0x0300) # M = 3 S = 1		(4 converters per link 2 samples per frame? ############# verify
   s.spi_write(wibNode,DAC_SELECT,0x4E,0x0F4F) # SCR=0, High Density,  N=16  N'=16
   # jesd synchronizatoin settings. no lane sync = bit 5  (0x1cE1 no lane sync) (0x1cc1 default)
   s.spi_write(wibNode,DAC_SELECT,0x4F,0x1CC1) #

   # sysref + jesd synchronizaton (0x0 = Ignore all SYSREF) (deufalt = use all synch pulses) !!!!!!!!!!!
   s.spi_write(wibNode,DAC_SELECT,0x5C,0x1111)

   s.spi_write(wibNode,DAC_SELECT,0x5F,0x0123)
   s.spi_write(wibNode,DAC_SELECT,0x60,0x4567)

   # Active DAC JESD
   # Start sysref generation
   # turn on sysref output driver %%%%%%%%%%%%%% this is probably not the way to go about this,
   # better to leave driver active and set mode to pulse  earlier in cod
   s.spi_write(wibNode,LMK_SELECT,0x10F,0x11) 
   
   #  0    - tx_enable
	# 1    - jesd_hold
	# 4- 5 - data_select 
	# Enable TX
   time.sleep(0.05)
   # 
   wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.dac_ctrl").write(0x01)
   wibNode.dispatch()
   time.sleep(0.05)
   # JESD is still in reset
   s.spi_write(wibNode,DAC_SELECT,0x4A,0xFF1E) # JESD in Init State
   time.sleep(0.05)
   # clear init bits per TI
   s.spi_write(wibNode,DAC_SELECT,0x4A,0xFF00) # JESD in Init State
   time.sleep(0.05)
   # Then assert Reset bit which starts the JESD initialization
   s.spi_write(wibNode,DAC_SELECT,0x4A,0xFF01) # 
   time.sleep(0.05) #  allow to stabelize
   # Clear alarms
   # Write to clear PLL LOCK Errors
   s.spi_write(wibNode,DAC_SELECT,0x6C,0x0) # sysref errors
   # WRITE 0 LOSS of signal
   s.spi_write(wibNode,DAC_SELECT,0x6D,0x0) #lane alarm LOS alarm

   # Clear lane errors before checking
   for i in range(8):
      s.spi_write(wibNode,DAC_SELECT,0x64+i,0x0)
   # unitapi_sleep_ms to allow an error to occur
   time.sleep(1)
   print ("Status: ")
   print ("---------------------------------")
   # READ PLL STATUS
   dword = int(s.spi_read(wibNode,DAC_SELECT,0x6C),16)
   if (INTERPOLATE == 0):
      dword &= ~0x3
   else:
      dword &= ~0x2
   if (dword !=0):
      print ("PLL LOCK ERROR : %5x" %dword)
      print ("Register 0x6C = %4X" %dword)
      
   
   if (dword & 0x0001):
     print("DAC PLL out of lock")
   else:
     print("DAC PLL Locked")
   
   if (dword & 0x0008):
     print("SERDES Block 0 out of LOCK")
   else:
     print("SERDES Block 0 Locked")
   
   if (dword & 0x0004):
     print("SERDES Block 1 out of LOCK")
   else:
     print("SERDES Block 1 Locked")

   #READ PLL LOOP Filter Voltage
   dword = int(s.spi_read(wibNode,DAC_SELECT,0x31),16)
   dword &= 0x07
   print("PLL Loop filter voltage = %5d" %dword)

   #LOSS OF SIGNAL
   dword = int(s.spi_read(wibNode,DAC_SELECT,0x6D),16)
   if (dword != 0x0):
      print ("LANE LOSS SIGNAL ERROR %5x" %dword)

   for i in range(8):
      dword = int(s.spi_read(wibNode,DAC_SELECT,0x64+i),16)
      if (dword == 0x2) or (dword == 0x3) or (dword == 0x0):
         print ("LANE %3d STATUS: OK %5x"%( i, dword))
      else:
         print ("LANE %3d STATUS: Error %5x" %(i, dword))
   print("-------------------------------------------")
   
   # error_cnt_link0. This error count for link0. What is counted as an error is determined
   # by error_ena_link0(0x52). This is a 16-bit value that is cleared when JESD synchronization
   # is performed or err_cnt_clk_link0 is programmed to a '1'.
   for i in range(4):
      dword = int(s.spi_read(wibNode,DAC_SELECT,0x41+i),16)
      if (dword !=0x0):
         print ("LINK %d %d errors found!!!"%(i,dword))
   #Short pattern test
   FMC120_short_pattern_test(wibNode)

   return 0
