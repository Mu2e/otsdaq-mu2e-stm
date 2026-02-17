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


for i in range (4):
  c.capture_fifo_rst(hw,i)
#temperature = 0
#m.fmc14x_monitor_displayamctemp(hw,temperature)

#hw.getNode("Buffers.Debug_controls_pulse.cont_data_on_debug").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.Debug_controls_pulse.cont_data_off_debug").write(0x1)
#hw.dispatch()

hw.getNode("Buffers.stop_overwrite").write(0x1)
hw.dispatch()
#hw.getNode("Buffers.allow_cont_data_in_spill_patt").write(0x0)
#hw.dispatch()
#hw.getNode("Buffers.FPGA2Host_buf_stat.stop_overwrite").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.FPGA2Host_buf_stat.trigs_enable").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.Debug_controls_pulse.spill_patt_emu_start").write(0x1)
#hw.dispatch()
#time.sleep (0.01)
#temp = hw.getNode("Buffers.FPGA2Host_buf_stat.Empty").read()
#hw.dispatch()
#print (temp)

#hw.getNode("Buffers.FPGA2Host_buf_stat.stop_overwrite").write(0x1)
#hw.dispatch()
#while (1):
#    mem = []
#    mem = hw.getNode("Buffers.FPGA2Host_buf_reg").readBlock(hw.getNode("Buffers.FPGA2Host_buf_reg").getSize())
#    hw.dispatch()

  
#    for x in mem:
#        if x == 0xAAAAAAAA:
#            print (hex(x)),
#            temp = list(mem).index(x)
#            print (hex(mem[temp+1])),
#            print (hex(mem[temp+2])),
#            print (hex(mem[temp+3])),
#            print (hex(mem[temp+4])),
#            print (hex(mem[temp+5])),
#            print (hex(mem[temp+6])),
#            print (hex(mem[temp+7]))

#hw.getNode("Buffers.Debug_controls_pulse.DDR_read_addr_reset").write(0x1)
#hw.dispatch()

#hw.getNode("Router_ctrl.Output_4_select").write(0x0)
#hw.dispatch()



while (1):
    temperature = 0
    m.fmc14x_monitor_displayamctemp(hw,temperature)
    time.sleep (1)

#for j in range (4):
#    hw.getNode("Router_ctrl.Output_0_select").write(int(j+1))
#    hw.dispatch()

#    #c.capture_enable_upload(hw,j)


#    size_adc_data = hw.getNode("Buffers.FPGA2Host_buf_stat.Count").read()
#    hw.dispatch()
#    print (size_adc_data)

#    fifo_stat = hw.getNode("Buffers.FPGA2Host_buf_stat.Empty").read()
#    hw.dispatch()
    
    
#    v = []
#    if fifo_stat == 0:
        
#        for i in range(burstlength_capture/4):
#            r =  hw.getNode("Buffers.FPGA2Host_buf_reg").read()
#            hw.dispatch()
#            v.append( hex(r))
    
#    c.capture_disable_upload(hw,j)
    
    
#    for i in range(16):
#        print (v[i])
    
    




  
















