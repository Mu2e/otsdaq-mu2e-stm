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

#uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
#manager = uhal.ConnectionManager("file://connections.xml")
#print sys.argv
#hw = manager.getDevice(sys.argv[1])

DATAGEN_SINE_WAVE = 0
DATAGEN_SAW_WAVE  = 1

adc_mode = 1
#N_SAMPS = 8192
#N_SIZE = 8192
numberburst = 0
burstlength = 1536 #512 * 2 * 2
burstlength_capture = 1256 #628

#print hw.id()
	
print "Info"

y=-1022
i=0
line_print=0
value_d = -2
#hw.getNode("Buffers.FPGA2Host_buf_stat.stop_overwrite").write(0x1)
#hw.dispatch()
with open ("output.txt","r") as txt_file:
    for line in txt_file:
        if line == '0xcccccccc\n':
            
            if i - y != 1024:
                print i
                print (i-y)
                line_print = 1
                print(line)
            y=i
            value_d = -2
        #if (line_print==1):    
        #    #print i
        #    print (line)
        if (i > (y+6) and i < y+1020 and i > 2 ):
            line_print = 0;
            #print(line)
            value = int(line.split("0x")[-1].strip("\n"),32)&0x0000000f
            #print(int(value,32)&0x0000000f)
            #print (value)
            if (not(((value-value_d) == 2) or ((value-value_d) == -14))):
                print (value-value_d),
                print ("error"), 
                print ("line"), 
                print(i)
            value_d = value
        i +=1

#while (n > 0):
#    mem = []
#    mem = hw.getNode("Buffers.FPGA2Host_buf_reg").readBlock(hw.getNode("Buffers.FPGA2Host_buf_reg").getSize())
#    hw.dispatch()
#    n -= 1
#    with open("output.txt","a") as txt_file:
#       for x in mem:
        #if x == 0xAAAAAAAA:
#            txt_file.write(hex(x) + "\n")
            #print (hex(x)),
#            temp = list(mem).index(x)
            
            #print (hex(mem[temp+1])),
            #print (hex(mem[temp+2])),
            #print (hex(mem[temp+3])),
            #print (hex(mem[temp+4])),
            #print (hex(mem[temp+5])),
            #print (hex(mem[temp+6])),
            #print (hex(mem[temp+7]))
       #temperature = 0
       #m.fmc14x_monitor_displayamctemp(hw,temperature)
       #time.sleep (1)          
#hw.getNode("Buffers.Debug_controls_pulse.DDR_read_addr_reset").write(0x1)
#hw.dispatch()

#hw.getNode("Router_ctrl.Output_4_select").write(0x0)
#hw.dispatch()



#while (1):
    #temperature = 0
    #m.fmc14x_monitor_displayamctemp(hw,temperature)
    #time.sleep (1)

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
    
    




  
















