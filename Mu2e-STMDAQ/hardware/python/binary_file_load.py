#!/usr/bin/python

# -*- coding: utf-8 -*-
import os
import sys
import time
import uhal
import struct

uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
here = os.path.dirname(os.path.abspath(__file__))
xml = os.path.join(here, "connections.xml")
manager = uhal.ConnectionManager("file://" + xml)
print (sys.argv)
hw = manager.getDevice(sys.argv[1])

print (hw.id())
	
print ("Info")


file_handler = open(sys.argv[2], "rb")
file_handler2 = open("written_values.txt", "w")

i=1

#for i in range(9000):
while i > 0 :
  
  temp = hw.getNode("Buffers.Host2FPGA_buf_stat.Full").read()
  hw.dispatch()
  if temp == 1:
      break
  
  if i==100:
     print("Still running")
  
  data_byte = file_handler.read(4)

  number = struct.unpack("<I",data_byte)
  #print(hex(number[0]))
  #print(hex(number[1]))
  file_handler2.write(hex(number[0] & 0x0000FFFF))
  file_handler2.write('\n')
  file_handler2.write(hex((number[0] & 0xFFFF0000)>>16))
  file_handler2.write('\n')
  hw.getNode("Buffers.Host2FPGA_buf_reg").write(number[0])
  hw.dispatch()
  i+=1


temp_count = hw.getNode("Buffers.Host2FPGA_buf_stat.Count").read()
hw.dispatch()
print(hex(temp_count))
print(temp)
file_handler.close( )


if len(sys.argv) > 3:
   print("Writing the second buffer")
   file_handler_2 = open(sys.argv[3], "rb")
   hw.getNode("Buffers.Host2FPGA_buf_reg_select").write(1)
   hw.dispatch()

   i=1
   while i > 0 :
  
      temp2 = hw.getNode("Buffers.Host2FPGA_buf_stat.Full").read()
      hw.dispatch()
      if temp2 == 1:
          break
      #print(i)
      if i==100:
         print("Still running")
  
      data_byte = file_handler_2.read(4)

      number = struct.unpack("<I",data_byte)
      #print(hex(number[0]))
      #print(hex(number[1]))
      hw.getNode("Buffers.Host2FPGA_buf_reg").write(number[0])
      hw.dispatch()
      i+=1

   temp_count = hw.getNode("Buffers.Host2FPGA_buf_stat.Count").read()
   hw.dispatch()
   print(hex(temp_count))
   file_handler_2.close()
else:
   temp2=1

if temp==1 and temp2==1:
   #hw.getNode("Buffers.allow_cont_data_in_spill_patt").write(1)
   hw.getNode("Buffers.Controls_misc.adc_emulation_enable").write(1)
   hw.dispatch()

   #hw.getNode("Buffers.Controls_misc.reset_trig_fifos").write(1)
   #hw.dispatch()

