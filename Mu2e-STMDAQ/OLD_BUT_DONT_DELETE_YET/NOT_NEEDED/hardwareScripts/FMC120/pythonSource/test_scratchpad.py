#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s
#import FMC14x_monitor as m
import FMC120 as f
import FMC120_freqcnt as fc
#import FMC14x_freqcnt as t
#import repeat as r
#import capture as c
#import utils as u

CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */



uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
manager = uhal.ConnectionManager("file://connections.xml")
print sys.argv
wibNode = manager.getDevice(sys.argv[1])

dword=wibNode.getNode("axi_fmc120_8lane.FMC120_ctrl.status").read()
wibNode.dispatch()
print(hex(dword))
dword &= 0xc00
print(hex(dword))
if (dword != 0xc00):
   print("QPLLs not locked")
else:
   print("QPLLs are locked")


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
    #break
else:
    print ("ADC JESD204B Initial Lane Alignment Failed")

i=0
print("Measuring on-board frequencies ---")
for i in range (5):
    fc.FMC120_freqcnt_getfrequency(wibNode,i)
