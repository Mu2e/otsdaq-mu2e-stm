#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s
import FMC14x_monitor as m
import FMC14x as f
import FMC14x_freqcnt as t
import repeat as r
import capture as c
import utils as u

uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
manager = uhal.ConnectionManager("file://connections.xml")
print sys.argv
hw = manager.getDevice(sys.argv[1])

DATAGEN_SINE_WAVE = 0
DATAGEN_SAW_WAVE  = 1

adc_mode = 1
#N_SAMPS = 8192
#N_SIZE = 8192
numberburst = 0
burstlength = 1536 #512 * 2 * 2
burstlength_capture = 1256 #628

print hw.id()
	
print "Info"

#m.fmc14x_monitor_getdiags(hw)

#hw.getNode("Buffers.10g_readout.10g_enable").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.10g_readout.8kbyte_read").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.10g_readout.cont_readout_8K_chunks").write(0x1)
#hw.dispatch()
#time.sleep(0.01)
hw.getNode("Buffers.10g_readout.8kbyte_read").write(0x1)
hw.dispatch()
#hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x0)
#hw.dispatch()
raw_input("Press Enter to continue...")
#hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.10g_readout.cont_readout_8K_chunks").write(0x1)
#hw.dispatch()
#time.sleep(0.01)
hw.getNode("Buffers.10g_readout.8kbyte_read").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x0)
hw.dispatch()
#buf = u.GenerateWaveform16(burstlength, 128, pow(2.0,15.0),DATAGEN_SINE_WAVE)
#print (buf)
#s.spi_write(hw,"LMK04828_CTRL",0x000,0x90)
#time.sleep(0.200)
#s.spi_write(hw,"LMK04828_CTRL",0x000,0x10)
#time.sleep(0.200)
#s.spi_write(hw,"LMK04828_CTRL",0x148,0x00)
#time.sleep(0.200)
#s.spi_write(hw,"LMK04828_CTRL",0x15F,0x3B) #STATUS_LD1_MUX LD1 = SPI_MISO
#time.sleep(0.200)
#s.spi_write(hw,"LMK04828_CTRL",0x002,0x00) #POWERDOWN enabled
#time.sleep(0.200)
#dword = s.spi_read(hw,"LMK04828_CTRL",0x003) # default 6
#print (dword)
#dword = s.spi_read(hw,"LMK04828_CTRL",0x004) # default 208
#print (dword)
#dword = s.spi_read(hw,"LMK04828_CTRL",0x005) #default 91
#print (dword)
#dword = s.spi_read(hw,"LMK04828_CTRL",0x006)
#print (dword)
#dword = s.spi_read(hw,"LMK04828_CTRL",0x00C)
#print (dword)
#dword = s.spi_read(hw,"LMK04828_CTRL",0x00D)
#print (dword)

#f.FMC14x_init(hw,0,adc_mode)
#f.FMC14x_init(hw,3,adc_mode)
#s.spi_write(hw,"AD9912_CTRL",0x000,0xBD)
#time.sleep(0.400)
    ##Clear Soft reset
#s.spi_write(hw,"AD9912_CTRL",0x000,0x99)
#time.sleep(0.500)
    #Read part ID LSB
#commandword = 0x002
    #dword = s.spi_read(hw,"AD9912_CTRL",commandword)
#dword = s.spi_read(hw,"AD9912_CTRL",0x002)
#print (dword)
#partId = dword
    #Read part ID MSB
#dword = s.spi_read(hw,"AD9912_CTRL",0x003)
#print (dword)
#partId = int(dword,0) + (int(dword,0)<<8)
    #print (partId)
    #Part ID should be 0x1902 
#if (partId == 0x1902 or partId == 0x1982):
#   print ("DDS part ID: %5.4X (OK)" %partId)
#else:
#   print ("DDS part ID: %5.4X (ERROR)" %partId)








