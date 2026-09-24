#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import os
#import spi_functions as s
#import FMC14x_monitor as m
#import FMC14x as f
#import FMC14x_freqcnt as t
#import repeat as r
import capture as c
#import utils as u

uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
manager = uhal.ConnectionManager("file://connections.xml")
#print sys.argv
#hw = manager.getDevice(sys.argv[1])
hw = manager.getDevice("KCU105")

os.system("ping -c 1 -w2 192.168.99.207")
os.system("ping -c 1 -w2 192.168.34.8")

hw.getNode("Buffers.stop_overwrite").write(0x1)
hw.dispatch()

#for i in range (4):
#   c.capture_fifo_rst(hw,i)

for j in range (4):
    c.capture_enable_upload(hw,j)

#for i in range (4):
  #c.capture_external_trigger_enable(hw,i)  
#  c.capture_arm(hw,i)

hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x0)
hw.dispatch()

hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x1)
hw.dispatch()
hw.getNode("Buffers.ADC_emulation.enable").write(0x1)
hw.dispatch()
#hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x0)
#hw.dispatch()

i=0
while(i<1e6):
    try:
        hw.getNode("Buffers.10g_readout.8kbyte_read").write(0x1)
        hw.dispatch()
        i += 1
    except:
        print("IPBus Error. Continue...")
  #time.sleep(0.1)
  #print "Packet requested"

#hw.getNode("Buffers.ADC_emulation.enable").write(0x0)
#hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x0)
hw.dispatch()
