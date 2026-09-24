#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as 
import FMC120_clocktree_300MSPS as f
import FMC120_adc as fa
import FMC120_dac as fad

LMK_SELECT=0x1
DAC_SELECT=0x2
ADC0_SELECT=0x4
ADC1_SELECT=0x8
ADC_SELECT_BOTH=0xC

CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */

def fmc120_config_pwr(wibNode):
   
   #Set I2C Port Expander default values
   s.i2c_write(wibNode,0x3A03,0xFF) # All pins are inputs
   
   s.i2c_write(wibNode,0x3A02,0x00) # No polarity inversion
   
   #Check for Module Present
   dword=s.i2c_read(wibNode,0x3A00)
   if ((dword & 0x8) == 0): # Pin 3 is FMC+ PRSNT_N signal, check for low value
      s.i2c_write(wibNode,0x3A01,0x80) #Set bit 7 to high when output
      s.i2c_write(wibNode,0x3A03,0x7F) #Output for only bit 7
      dword2=s.i2c_read(wibNode,0x3A00) # Check PG status 
      if ((dword2 & 0x1) == 0x1):
         print("OK") # Do nothing
      else:
         print("Power is NOT good on FMC+")
   else:
      print("No FMC found")

   time.sleep(1)
   
   s.i2c_write(wibNode,0x3A03,0xFF) # All pins are inputs
   time.sleep(1)
   s.i2c_write(wibNode,0x3A02,0x00) # No Polarity Inversion

   #Check for Module Present
   dword=s.i2c_read(wibNode,0x3A00)
   if ((dword & 0x8) ==0):
      print("Enabling VADJ with 1.8V on FMC+")
      s.i2c_write(wibNode,0x3A01,0x80) #Set bit 7 to high when output
      s.i2c_write(wibNode,0x3A03,0x7F) #Output for only bit 7
      dword2=s.i2c_read(wibNode,0x3A00) # Check PG status 
      if ((dword2 & 0x1) == 0x1):
        print("Power is good on FMC+")
      else:
        print("Power is NOT good on FMC+")
   else:
      print("No FMC found")

   time.sleep(1)
   return 0
   
def FMC120_init(wibNode,clockmode):
   
   dword = 0
   ila_count = 0
   i = 0
   
   f.FMC120_clocktree_init(wibNode,CLOCKTREE_CLKSRC_INTERNAL)
   
   while (i <=10):
      # Configure Transceiver 
      # Assert transceiver reset
      wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.transceiver").write(0x01)
      wibNode.dispatch()
      
      time.sleep(0.4)
      
      # Release transceiver reset
      wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.transceiver").write(0x00)
      wibNode.dispatch()
      print("Wait for QPLLs to lock")
      time.sleep(1)
      
      dword=wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.status").read()
      wibNode.dispatch()
      print(hex(dword))
      dword &= 0xc00
      if (dword != 0xc00):
         print("QPLLs not locked")
      else:
         print("QPLLs are locked")

      #Configure ADC0 and ADC1
      print("Configuring ADCs ...")
   
      fa.FMC120_adc_init(wibNode)
      
      # Enable manual bit alignment
      wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.transceiver").write(0x10)
      wibNode.dispatch()
     
      #Configure DAC0
      print("Configuring DAC ...")
      fad.FMC120_dac_init(wibNode,0)
      
      #Check for JESD ADC to be stable
      dword=wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.status").read()
      wibNode.dispatch()
      if (dword >> 8 & 0x1):
         print ("ADC0 aligned")
      else:
         print ("ADC0 failed bit alignment")
      if (dword,16 >> 9 & 0x1):
         print ("ADC1 aligned")
      else:
         print ("ADC1 failed bit alignment")
      
      # Check for JESD multiframe alignment
      dword=wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.adc_valid").read()
      wibNode.dispatch()
      if (dword == 0x0F):
         print("ADC JESD204B Initial Lane Alignment Complete")
         break
      else:
         print ("ADC JESD204B Initial Lane Alignment Failed")
      
      i = i+1

   if (i==10):
      print("FMC120 Init failed")
      return -1
   else:
      return 0


