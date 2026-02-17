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
numberburst = 1
burstlength = 1536 #512 * 2 * 2

print hw.id()
	
print "Info"

#m.fmc14x_monitor_getdiags(hw)
buf = u.GenerateWaveform16(burstlength, 128, pow(2.0,16.0),DATAGEN_SINE_WAVE)
for i in range(128):
    print (hex(buf[i]))
 






       
#if f.FMC14x_dac_init(hw,0,2) == 0:
#    print ("Could not initialise FMC14x.DAC0")


  














#dword = s.spi_read(hw,"ADC16DX370_CTRL0",0x04)
#print (dword)
#s=hw.getNode("CID_registers.constellation_id").read()
#hw.dispatch()
#time.sleep(0.01)
#print hex(s)
#d=hw.getNode("CID_registers.sw_build").read()
#hw.dispatch()
#print hex(d)
#hw.getNode("axi_fmc144_8lane.FMC144_ctrl.adc_ctrl").write(1)
#hw.dispatch()
#time.sleep(0.5)
#e=hw.getNode("axi_fmc144_8lane.FMC144_ctrl.adc_ctrl").read()
#hw.dispatch()
#print hex(e)
#hw.getNode("Buffers.Host2FPGA_buf_reg").write(99)
#hw.dispatch
#hw.getNode("Buffers.Host2FPGA_buf_reg").write(136)
#hw.dispatch
#e=hw.getNode("Buffers.Host2FPGA_buf_stat.Full").read()
#f=hw.getNode("Buffers.Host2FPGA_buf_stat.Count").read()
#hw.dispatch()
#print e	
#print f
#hw.getNode("Buffers.FPGA2Host_buf_reg").read()
#hw.dispatch()
#time.sleep(1)
	



#t = 0
#id = 0


#print "Filling buffer", hex(t), hex(len(data[0]))

#d = []
#for i in range(N_SAMPS):
#		
#		for j in range(0,2): 
#			d.append(data[i][j])
#
#
#
#hw.getNode("wibtor.buf.data").writeBlock(d) # Channels
#hw.dispatch()

#print "Enable source and sink"
#hw.getNode("wibsink.csr.ctrl.en").write(1)
#hw.dispatch()
#hw.getNode("wibtor.csr.ctrl.fire").write(1)
#hw.dispatch()




#v = []
#s = 1
#i = 0
#print "Starting playback"
#for i in range(8192):
#while True:
#    s = hw.getNode("wibsink.csr.stat.empty").read()
#    hw.dispatch()
#    if s == 1:
#        time.sleep(0.2)
#    else:
#        r =  hw.getNode("wibsink.buf.data").read()
#        hw.dispatch()
#        v.append( hex(r))
#        i+= 1
#    if i==8194:
#        break

#print "Writing to file"
#line = ""
#x=0
#with open('demofile3.txt', 'w') as f:
#    for item in v:
#        x+=1
#        
#        if not x%2:
#            line += "{:01x}".format(int(item,0))
#            line += "\n"
#        else:
#            line += "0x{:08x} ".format(int(item,0)) #"%x " % int(item,0)
#    f.write("%s" % line)

