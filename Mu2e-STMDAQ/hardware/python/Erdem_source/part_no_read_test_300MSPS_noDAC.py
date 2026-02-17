#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s
#import FMC14x_monitor as m
import FMC120_300MSPS as f
import FMC120_freqcnt as fc
#import FMC14x_freqcnt as t
#import repeat as r
#import capture as c
#import utils as u

CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */

LMK_SELECT=0x1
DAC_SELECT=0x2
ADC0_SELECT=0x4
ADC1_SELECT=0x8
ADC_SELECT_BOTH=0xC

clockmode=CLOCKTREE_CLKSRC_INTERNAL
tp_on = 0
uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
manager = uhal.ConnectionManager("file://connections.xml")
print (sys.argv)
hw = manager.getDevice(sys.argv[1])
if (len(sys.argv)>2):
   tp_on = (sys.argv[2])
   print(tp_on)
DATAGEN_SINE_WAVE = 0
DATAGEN_SAW_WAVE  = 1

adc_mode = 1
#N_SAMPS = 8192
#N_SIZE = 8192
numberburst = 0
burstlength = 1536 #512 * 2 * 2
burstlength_capture = 1256 #628

print (hw.id())

	
print ("Info")
#hw.getNode("General_reset").write(0x01)
#hw.dispatch()
#time.sleep(0.5)
#raw_input("Press Enter to continue...")
# Configure I2C switch to HPC connector on the ZCU102
#Set one byte per cycle
hw.getNode("i2c_master.i2c.byte").write(0x00)
hw.dispatch()
s.i2c_write(hw,0x7500,0x01) # For HPC0?
#s.i2c_write(hw,0x7500,0x02) # For HPC1

time.sleep(0.200)
print("1st step")
temp = hw.getNode("CID_registers.hw_build_date").read()
hw.dispatch()
print(hex(temp))
temp = hw.getNode("CID_registers.firmware_type").read()
hw.dispatch()
print(hex(temp))
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

s.i2c_write(hw,0x2F00,0x2080)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 0):
  result = (temp&0xFFF) * 2.5 / 4096
  if (result  > (0.9*0.9) and result < (0.9*1.1)):
    print("Vin0 (0.9Va) : OK %.2fV" %result)
  else:
    print("Vin0 (0.9Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

s.i2c_write(hw,0x2F00,0x2040)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 1):
  result = (temp&0xFFF) * 2.5 / 4096
  if (result  > (1.15*0.9) and result < (1.15*1.1)):
    print("Vin1 (1.15Va) : OK %.2fV" %result)
  else:
    print("Vin1 (1.15Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

s.i2c_write(hw,0x2F00,0x2020)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 2):
  result = (temp&0xFFF) * 2.5 / 4096
  if (result  > (1.8*0.9) and result < (1.8*1.1)):
    print("Vin2 (1.8Va) : OK %.2fV" %result)
  else:
    print("Vin2 (1.8Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

s.i2c_write(hw,0x2F00,0x2010)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 3):
  result = (temp&0xFFF) * 2.5 / 4096
  if (result  > (1.9*0.9) and result < (1.9*1.1)):
    print("Vin3 (1.9Va) : OK %.2fV" %result)
  else:
    print("Vin3 (1.9Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")


s.i2c_write(hw,0x2F00,0x2008)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 4):
  result = (temp&0xFFF) * 5.0 / 4096
  if (result  > (2.6*0.9) and result < (2.6*1.1)):
    print("Vin4 (2.6Va) : OK %.2fV" %result)
  else:
    print("Vin4 (2.6Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

s.i2c_write(hw,0x2F00,0x2004)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 5):
  result = (temp&0xFFF) * 5.0 / 4096
  if (result  > (3.0*0.9) and result < (3.0*1.1)):
    print("Vin5 (3.0Va) : OK %.2fV" %result)
  else:
    print("Vin5 (3.0Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

s.i2c_write(hw,0x2F00,0x2002)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 6):
  result = (temp&0xFFF) * 5.0 / 4096
  if (result  > (3.3*0.9) and result < (3.3*1.1)):
    print("Vin6 (3.3Va) : OK %.2fV" %result)
  else:
    print("Vin6 (3.3Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

s.i2c_write(hw,0x2F00,0x2001)
time.sleep(0.200)
temp=s.i2c_read(hw,0x2F01)
print(temp)
if ((temp&0xF000)>>12 == 7):
  result = -3.3 + ((temp&0xFFF) * 5.0 / 4096)
  if (result  < (-2.6*0.9) and result > (-2.6*1.1)):
    print("Vin7 (-2.6Va) : OK %.2fV" %result)
  else:
    print("Vin7 (-2.6Va) : ERROR %.2fV" %result)
else:
  print("Error reading from monitoring device. Wrong channel number returned")

if tp_on == "tp":
  print("Test pattern output from the ADC")
  f.FMC120_init_noDAC(hw,CLOCKTREE_CLKSRC_INTERNAL,1)	
else:
  f.FMC120_init_noDAC(hw,CLOCKTREE_CLKSRC_INTERNAL,0)

i=0
print("Measuring on-board frequencies ---")
for i in range (5):
    fc.FMC120_freqcnt_getfrequency(hw,i)

if (clockmode==CLOCKTREE_CLKSRC_INTERNAL) or (clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF):
  time.sleep(0.5)
	#verify PLL1 status
  dword=s.spi_read(hw,LMK_SELECT,0x182) 
  dword2=(int(dword,16)& 0x07)
  if (int(dword,16)&0x02 != 0x02):
    print("PLL1 not locked")
  else:
    print("PLL1 locked")
	#verify PLL2 status
  dword=s.spi_read(hw,LMK_SELECT,0x183) 
  dword2=(int(dword,16)& 0x07)
  if (int(dword,16)&0x02 != 0x02):
    print("PLL2 not locked")
  else:
    print("PLL2 locked")

