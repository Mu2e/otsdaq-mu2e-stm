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

m.fmc14x_monitor_getdiags(hw)
buf = u.GenerateWaveform16(burstlength, 128, pow(2.0,15.0),DATAGEN_SINE_WAVE)
#print (buf)

f.FMC14x_init(hw,0,adc_mode)
print ("Measuring on-board frequencies ---")
for i in range(5):
  t.FMC14x_freqcnt_getfrequency(hw,i,adc_mode)




hw.getNode("Router_ctrl.Output_1_select").write(0x0)
hw.dispatch()
hw.getNode("Router_ctrl.Output_2_select").write(0x0)
hw.dispatch()
hw.getNode("Router_ctrl.Output_3_select").write(0x0)
hw.dispatch()
hw.getNode("Router_ctrl.Output_4_select").write(0x0)
hw.dispatch()


r.repeat_setburstlength(hw,0,burstlength)
r.repeat_prepare_wfm_upload(hw,0)
r.repeat_setburstlength(hw,1,burstlength)
r.repeat_prepare_wfm_upload(hw,1)
r.repeat_setburstlength(hw,2,burstlength)
r.repeat_prepare_wfm_upload(hw,2)
r.repeat_setburstlength(hw,3,burstlength)
r.repeat_prepare_wfm_upload(hw,3)

x= 0
for item in range (burstlength/2):
  tmp1 = u.int2bin(buf[x],16)
  #print (buf[x])
  #print (tmp1)
  tmp2 = u.int2bin(buf[x+1],16)
  #print (buf[x+1])
  #print (tmp2)
  word = tmp2 + tmp1
  #print (word)
  word = int(word,2)
  
  x +=2
  hw.getNode("Buffers.Host2FPGA_buf_reg").write(word)
  hw.dispatch()

for j in range(4):  
  value = r.repeat_check_status (hw,j)
  print ("value_%s = %X " %(j,value))

  r.repeat_arm(hw,j)


  
for i in range (4):
  c.capture_fifo_rst(hw,i)
  c.capture_setburstlength(hw,i,numberburst,burstlength_capture)
#  c.capture_enable_upload(hw,i)
  

time.sleep (0.010)
for i in range (4):
  c.capture_external_trigger_enable(hw,i)  
  c.capture_arm(hw,i)
  #c.capture_sw_trigger(hw,i)
  

for j in range (4):
    c.capture_enable_upload_hw_trig(hw,j)

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
    
    print ("ADC_%s dump end" %(j))

while (1):
   temperature = 0
   m.fmc14x_monitor_displayamctemp(hw,temperature)
   time.sleep (1)
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
print "Starting playback"
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

