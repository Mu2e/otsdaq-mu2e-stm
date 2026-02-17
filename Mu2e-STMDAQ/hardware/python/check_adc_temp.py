#!/usr/bin/python

# -*- coding: utf-8 -*-
import os
import sys
import time
import uhal
import spi_functions as s


uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
here = os.path.dirname(os.path.abspath(__file__))
xml = os.path.join(here, "connections.xml")
manager = uhal.ConnectionManager("file://" + xml)
print (sys.argv)
hw = manager.getDevice(sys.argv[1])

print("Get the diagnostics from the FMC120 daughter board")
hw.getNode("i2c_master.i2c.byte").write(0x01)
hw.dispatch()
s.i2c_write(hw,0x2F00,0x2200)
time.sleep(0.200)
print("1st step")
s.i2c_write(hw,0x2F00,0xA000)
time.sleep(0.200)
print("2nd step")
temp=s.i2c_read(hw,0x2F02)
print(temp)
if ((temp&0xF000)>>12 == 8):
  result = (temp&0xFFF) / 4.0
  if (result  > -40.0 and result < 85.0):
    print("Temp (AD7291) : OK %.2f" %result)
  else:
    print("Temp (AD7291) : ERROR %.2f" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

