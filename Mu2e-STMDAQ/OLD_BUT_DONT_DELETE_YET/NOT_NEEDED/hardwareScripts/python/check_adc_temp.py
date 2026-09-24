#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

def get_adc_temp(hw_dir):
  uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
  manager = uhal.ConnectionManager("file://"+hw_dir+"/connections.xml")
  hw = manager.getDevice("ZCU102-FMC120")

  hw.getNode("i2c_master.i2c.byte").write(0x01)
  hw.dispatch()
  s.i2c_write(hw,0x2F00,0x2200)
  time.sleep(0.200)
  s.i2c_write(hw,0x2F00,0xA000)
  time.sleep(0.200)
  temp=s.i2c_read(hw,0x2F02)
  if ((temp&0xF000)>>12 == 8):
    result = (temp&0xFFF) / 4.0
    if (result  > -40.0 and result < 85.0):
      return float(result)
    else:
      return ("ERROR %.2f" %result)
  else:
    print("Error reading from monitoring device. Wrong channel number returned")
    return -1

