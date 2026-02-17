#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import os
import spi_functions as s
import FMC14x_monitor as m
import FMC14x as f
import FMC14x_freqcnt as t
import repeat as r
import capture as c
import utils as u
import xml.etree.ElementTree as XML


uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
manager = uhal.ConnectionManager("file://connections.xml")
print(sys.argv)
hw = manager.getDevice(sys.argv[1])

os.system("ping -c 1 -w2 192.168.99.207")
os.system("ping -c 1 -w2 192.168.34.8")

DATAGEN_SINE_WAVE = 0
DATAGEN_SAW_WAVE  = 1

adc_mode = 1
#N_SAMPS = 8192
#N_SIZE = 8192
numberburst = 0
burstlength = 1536 #512 * 2 * 2
burstlength_capture = 1280 #1256 #628

print(hw.id())
	
print("Info")


#temperature = 0
#m.fmc14x_monitor_displayamctemp(hw,temperature)

# Get the register values from the XML
# test code to read xml file
xml = {}
STMDAQ_ROOT = os.getenv('STMDAQ_ROOT')
xml_file = STMDAQ_ROOT+"/config/stmdaq.xml"
xroot = XML.parse(xml_file).getroot()
for value in xroot.iter():
    if (value.tag != 'stm') and ('fmc144' in value.tag):
        xml[value.tag] = value.text.strip()

print("Setting Register values using xml file = %s" % (xml_file))
print(xml)
for key in xml:
    ivalue = int (xml[key], 16)
    print(key)
    print(" %s = 0x%x (%d)" % (key, ivalue, ivalue)) 

hw.getNode("Buffers.Test_beam.ext_ADC_offset").write(int (xml["fmc144_ext_ADC_offset"], 16))
hw.dispatch()
hw.getNode("Buffers.Test_beam.Start_reset_stop_clock").write(int (xml["fmc144_Start_reset_stop_clock"],16))
hw.dispatch()
hw.getNode("Buffers.Controls_misc.reset_trig_fifos").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Test_beam.Test_beam_10g_readout").write(int (xml["fmc144_Test_beam_10g_readout"],16))
hw.dispatch()
hw.getNode("Buffers.Test_beam.ext_slice_length").write(int (xml["fmc144_ext_slice_length"],16))
hw.dispatch()
hw.getNode("Buffers.Test_beam.ext_slice_num").write(int (xml["fmc144_ext_slice_num"],16))
hw.dispatch()
#time.sleep (0.5)
hw.getNode("Buffers.Test_beam.ext_int_delay").write(int (xml["fmc144_ext_int_delay"],16))
hw.dispatch()
hw.getNode("Buffers.Test_beam.int_ADC_offset").write(int (xml["fmc144_int_ADC_offset"],16))
hw.dispatch()
hw.getNode("Buffers.Test_beam.int_slice_length").write(int (xml["fmc144_int_slice_length"],16))
hw.dispatch()
hw.getNode("Buffers.Test_beam.int_slice_num").write(int (xml["fmc144_int_slice_num"],16))
hw.dispatch()
hw.getNode("Buffers.Test_beam.ext_trig_timeout").write(int (xml["fmc144_ext_trig_timeout"],16))
hw.dispatch()

hw.getNode("Buffers.Debug_controls_pulse_4.DDR_wr_addr_reset").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_4.DDR_read_addr_reset").write(0x1)
hw.dispatch()

for i in range (4):
   c.capture_fifo_rst(hw,i)
#   c.capture_setburstlength(hw,i,numberburst,burstlength_capture)

for i in range (4):
  c.capture_external_trigger_enable(hw,i)  
  c.capture_arm(hw,i)

for j in range (4):
    c.capture_enable_upload_hw_trig(hw,j)


#hw.getNode("Buffers.stop_overwrite").write(0x1)
#hw.dispatch()

#for i in range (4):
#  c.capture_fifo_rst(hw,i)

hw.getNode("Buffers.Controls_misc.adc_emulation_enable").write(0x1)
hw.dispatch()

time.sleep (0.2)
hw.getNode("Buffers.FPGA2Host_buf_stat.trigs_enable").write(0x1)
hw.dispatch()
time.sleep (0.1)
hw.getNode("Buffers.10g_readout.cont_readout_8K_chunks").write(0x1)
hw.dispatch()
n=1
while (n > 0):
   temp = hw.getNode("Buffers.Test_beam.Mem_rollover_stat").read()
   hw.dispatch()
   print(hex(temp))
   temp2 = temp >> 31
   if temp2==1:
     n= -1
   time.sleep (0.8)
   hw.getNode("Buffers.Debug_RAM_Status.Status_trig_and_wr_address_return").write(0x1)
   hw.dispatch()
   temp = hw.getNode("Buffers.Debug_RAM_Status.Status_trig_and_wr_address_return").read()
   hw.dispatch()
   print("Write Address:"), 
   print(hex(temp))

print("While exit")
hw.getNode("Buffers.Controls_misc.adc_emulation_enable").write(0x0)
hw.dispatch()
hw.getNode("Buffers.FPGA2Host_buf_stat.trigs_enable").write(0x0)
hw.dispatch()
#hw.getNode("Buffers.FPGA2Host_buf_stat.trigs_enable").write(0x0)
#hw.dispatch()

#time.sleep (2)
print ("So far")
#hw.getNode("Buffers.10g_readout.cont_readout_8K_chunks").write(0x0)
#hw.dispatch()
hw.getNode("Buffers.Test_beam.Test_beam_10g_readout").write(0x0)
hw.dispatch()
#temp = hw.getNode("Buffers.FPGA2Host_buf_stat.Empty").read()
#hw.dispatch()


#hw.getNode("Buffers.FPGA2Host_buf_stat.stop_overwrite").write(0x1)
#hw.dispatch()




print ("Finished")
hw.getNode("Buffers.Debug_RAM_Status.Status_trig_and_wr_address_return").write(0x1)
hw.dispatch()


    
    




  
















