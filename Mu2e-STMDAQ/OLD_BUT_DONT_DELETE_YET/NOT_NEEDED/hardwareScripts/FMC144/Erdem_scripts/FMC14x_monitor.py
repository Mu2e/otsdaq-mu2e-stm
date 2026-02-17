#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

def fmc14x_monitor_init (hw):
  
  commandword = 1<<15
  commandword |= (1<<12)
  commandword |= (0x1E<<6)
  print (hex(commandword))
  
  #Check part revision number for the monitor 
  
  t1 = s.spi_read(hw,"AMC7823_CTRL0",commandword)
  print(t1)
  s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0D<<6)),0x8000)
  print (t1)
  t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0A<<6)),0x0000)
  print (t1)
  t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0B<<6)),0x8080)
  print (t1)

def fmc14x_monitor_setthreshold(hw,ANA0,ANA1,ANA2,ANA3):
   
   THRH0 = int(float(ANA0)*1.05 / 2.0 * 4096.0 / 2.5)
   THRH1 = int(float(ANA0)*1.05 / 1.0 * 4096.0 / 2.5)
   THRH2 = int(float(ANA0)*1.05 / 1.0 * 4096.0 / 2.5)
   THRH3 = int(float(ANA0)*1.05 / 1.0 * 4096.0 / 2.5)
   
   THRL0 = int(float(ANA0)*0.95 / 2.0 * 4096.0 / 2.5)
   THRL1 = int(float(ANA0)*0.95 / 1.0 * 4096.0 / 2.5)
   THRL2 = int(float(ANA0)*0.95 / 1.0 * 4096.0 / 2.5)
   THRL3 = int(float(ANA0)*0.95 / 1.0 * 4096.0 / 2.5)
   
   if (THRH0 > 0xFFF):
     THRH0 = 0xFFF
   if (THRH1 > 0xFFF):
     THRH1 = 0xFFF
   if (THRH2 > 0xFFF):
     THRH2 = 0xFFF
   if (THRH3 > 0xFFF):
     THRH3 = 0xFFF
   
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0E<<6)),THRH0)
   if not(t1):
     print ("error")
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0F<<6)),THRL0)
   if not(t1):
     print ("error")
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x10<<6)),THRH1)
   if not(t1):
     print ("error")
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x11<<6)),THRL1)
   if not(t1):
     print ("error")
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x12<<6)),THRH2)
   if not(t1):
     print ("error")
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x13<<6)),THRL2)
   if not(t1):
     print ("error")
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x14<<6)),THRH3)
   if not(t1):
     print ("error")
   t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x15<<6)),THRL3)
   if not(t1):
     print ("error")
     
   return 1
 
def fmc14x_monitor_vcxopowercontrol(hw,enable):
   
   if not(enable):
     t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0A<<6)),0xFFFD)
     if not(t1):
       print ("error")
   else:
     t1 = s.spi_write(hw,"AMC7823_CTRL0",((0x1<<12)|(0x0A<<6)),0xFFFF)
     if not(t1):
       print ("error")
   return 1
 
def fmc14x_monitor_checkvoltages (hw,r):
 
   ANA0 =float(3.0)
   ANA1 =float(0.9)
   ANA2 =float(1.2)
   ANA3 =float(1.8)
   ANA4 =float(5.0)
   ANA5 =float(-5.0)
   ANA6 =float(3.3)
   
   THRH0 = ANA0 * 1.05
   THRH1 = ANA1 * 1.05
   THRH2 = ANA2 * 1.05
   THRH3 = ANA3 * 1.05
   THRH4 = ANA4 * 1.05
   THRH5 = ANA5 * 0.95
   THRH6 = ANA6 * 1.05
   THRH7 = float(5.5)
   
   THRL0 = ANA0 * 0.95
   THRL1 = ANA1 * 0.95
   THRL2 = ANA2 * 0.95
   THRL3 = ANA3 * 0.95
   THRL4 = ANA4 * 0.95
   THRL5 = ANA5 * 1.05
   THRL6 = ANA6 * 0.95
   THRL7 = float(1.65)
   
   iserror=0
   
   for i in range(8):
     
     dword = s.spi_read(hw,"AMC7823_CTRL0",(i<<6))
     print (dword)
     result = 2.5 * float(int(dword,0) & 0xFFF) / 4096.0
     if i == 0:
       result = result * 2.0
       r.append(result)
       if result > THRL0 and result < THRH0:
	 print ("3.0V ANALOG : OK %.2f" %result)
       else:
         print ("3.0V ANALOG : ERROR %.2f" %result)
         iserror = 1
     elif i == 1:
       result = result * 1.0
       r.append(result)
       if result > THRL1 and result < THRH1:
	 print ("0.9V ANALOG : OK %.2f" %result)
       else:
         print ("0.9V ANALOG : ERROR %.2f" %result)
         iserror = 1
     elif i == 2:
       result = result * 1.0
       r.append(result)
       if result > THRL2 and result < THRH2:
	 print ("1.2V ANALOG : OK %.2f" %result)
       else:
         print ("1.2V ANALOG : ERROR %.2f" %result)
         iserror = 1
     elif i == 3:
       result = result * 1.0
       r.append(result)
       if result > THRL3 and result < THRH3:
	 print ("1.8V ANALOG : OK %.2f" %result)
       else:
         print ("1.8V ANALOG : ERROR %.2f" %result)
         iserror = 1
     elif i == 4:
       result = result * 3.0
       r.append(result)
       if result > THRL4 and result < THRH4:
	 print ("5V ANALOG : OK %.2f" %result)
       else:
         print ("5V ANALOG : ERROR %.2f" %result)
         iserror = 1
     elif i == 5:
       result = (4095.0 - (float(int(dword,0) & 0xFFF) + 34.0)) * -2.4533
       result = result /1000.0
       r.append(result)
       if result > THRL5 and result < THRH5:
	 print ("-5V ANALOG : OK %.2f" %result)
       else:
         print ("-5V ANALOG : ERROR %.2f" %result)
         iserror = 1
     elif i == 6:
       result = result * 2.0
       r.append(result)
       if result > THRL6 and result < THRH6:
	 print ("3.3V ANALOG : OK %.2f" %result)
       else:
         print ("3.3V ANALOG : ERROR %.2f" %result)
         iserror = 1
     elif i == 7:
       result = result * 2.0
       r.append(result)
       if result > THRL7 and result < THRH7:
	 print ("VADJ : OK %.2f" %result)
       else:
         print ("VADJ ANALOG : ERROR %.2f" %result)
         iserror = 1
   
   if iserror:
     return 0
   else:
     return 1
   
def fmc14x_monitor_displayamctemp(hw,temperature):
   
   dword = s.spi_read(hw,"AMC7823_CTRL0",((0x0<<12)|(0x08<<6)))
   print dword
   result = 0.61 * float(int(dword,0) & 0xFFF)
   result = 2.60 * result - 273.0
   print ("Temperature  : %d" %result)
   temperature = result
   
   return 1
 
def fmc14x_monitor_getdiags(hw):
   
   fmc14x_monitor_init(hw)
   fmc14x_monitor_setthreshold(hw,3.0,0.9,1.2,1.8)
   r = []
   fmc14x_monitor_checkvoltages(hw,r)
   temperature = 0
   fmc14x_monitor_displayamctemp(hw,temperature)
         
         
       