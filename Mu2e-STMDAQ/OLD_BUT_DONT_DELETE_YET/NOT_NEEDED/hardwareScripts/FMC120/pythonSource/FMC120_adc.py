#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

LMK_SELECT=0x1
DAC_SELECT=0x2
ADC0_SELECT=0x4
ADC1_SELECT=0x8
ADC_SELECT_BOTH=0xC

CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */



def FMC120_adc_init(wibNode):
   
   dword = 0
   # ADC initialization
   print("Pulsing ADC RESET pins")
   # First Pulse Hardware Reset Line on Both ADC's, this is the equivalent of flushing all the regs in software	
   # Read in the Register		read, modify, write to avoid clobbering any other register settings   
   dword = s.i2c_read(wibNode,0x1C03)
   
   # force a clear on the ADC Reset Pins, ensures a low on abnormal terminations or restarts
   dword &= 0xF9
   s.i2c_write(wibNode,0x1C03,dword)
   
   # Set reset bits active
   dword |= 0x06
   s.i2c_write(wibNode,0x1C03,dword)
  
   #Clear reset bit
   dword &= 0xF9
   s.i2c_write(wibNode,0x1C03,dword)

   time.sleep(0.02)
   #power up adc input amplifiers
   dword = s.i2c_read(wibNode,0x1C01)
   dword &= 0xF3 #clear ADCx_Amp_Off bits
   s.i2c_write(wibNode,0x1C01,dword)
   time.sleep(0.02)

   # Initialize ADCs
   print("Configuring ADCs")
   
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x0000,0x81) #LMFS = 4211
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x11,0x80) #select master page of analog bank

   # set analog input to DC coupling Z5K **********************************
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x004F,0x00) #DC coupling enable Bit 0 = off,  1 = Enabled shifts VCM at adc down ~ 200mV ***
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x0026,0x40) #IGNORE inputs on power down pin
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x0059,0x20) #Set the always write 1 Bit

   # select main digital page 6800
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x4003,0x00) #select JESD Digital Page
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x4004,0x68) #select JESD Digital Page
 
   #Set Nyquist Zone
   dword = 0;	# set dword to nyquist zone,
  
   if (dword==0): #First Nyquist zone
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x6842,dword) #Set nyquist Zone 0 0-500Mhz
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x684E,0x80) #Enable nyquist correction 
      print("ADC set to First Nyquist Zone DC-500MHz ")
   
   if (dword==1): #Second Nyquist zone
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x6842,dword) #Set nyquist Zone  zone 2 500 to 1000MHz
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x684E,0x80) #Enable nyquist correction 
      print("ADC set to Second Nyquist Zone 500 to 1000MHz")

   if (dword==2): #Third Nyquist zone
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x6842,dword) #Set nyquist Zone 3 = zone 3 1000 to 1500MHz
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x684E,0x80) #Enable nyquist correction
      print("ADC set to Third Nyquist Zone 1000 to 1500MHz")

   # **** Feature Updates for 54Jxx family
   if (1):
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4005,0x01) #enable single channel writes 
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4004,0x68) #Upper byte of page address
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4003,0x00) #middle byte
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4002,0x00) #middle byte
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4001,0x00) #lower byte of 32bit page address
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x604E,0x20) #for CH-A, write to register address 0x4E for a feature Update
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x704E,0x20) #for CH-B, write to register address 0x4E for a feature Update
      #select main digital page 6800, put the ADC select back were we found it
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4005,0x00) #enable broadcast
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4003,0x00) #select JESD Digital Page
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4004,0x68) #select JESD Digital Page

   #*** DIGITAL CORE RESET *** 
   # the digital reset must be pulsed for register writes to take effect
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x60F7,0x00) #Digital Reset
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x6000,0x01) #assert Digital Reset
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x6000,0x00) #clear Digital Reset
   time.sleep (0.02)
   
   # select 6A00 JESD Anlaog Page
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x4003,0x00) #select JESD Digital Page
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x4004,0x6A) #select JESD Digital Page
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x6016,0x02) #40X pll

   # select 6900 Digital JESD Page
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x4003,0x00) #select page lowbyte
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x4004,0x69) #select page highbyte
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x6001,0x02) #set LMF = 4244 
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x6007,0x08) #set internal defaults JESDV and subclass V1 
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x6000,0x80) #set control K  
   s.spi_write(wibNode,ADC_SELECT_BOTH,0x6006,0x07) #set K to 8
   time.sleep(0.05)
   
   s.spi_write(wibNode,LMK_SELECT,0x11F,0x05) # Disable Sysref to ADC 0
   s.spi_write(wibNode,LMK_SELECT,0x117,0x05) # Disable Sysref to ADC 1

   # *** DC-COUPLED PERFORMANCE IMPROVEMENT ***
   # this code should be enabled when the FMC120 is DC Coupled, It greatly improves performance when measuring signals with a DC offset
   # it also improves / removes ADC generated low frequency distortion below shevral hundred kiloherta
   # this code should be disabled for an AC coupled version of the board
   # this code freezes the DC offset correction engine, the ADC should be at temperature and stable before this code is executed,
   # to re-enable the DC offset correction use a read / modify /write on address 0x6068 / 0x7068 and clear bit 7
   if (1):
      time.sleep(0.5) # allow everything to stabilize
      print( "Freezing DC offset correction")
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4005,0x01) # enable single channel writes
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4004,0x61) # Upper byte of page address
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4003,0x00) # middle byte 
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4002,0x00) # middle byte
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4001,0x00) # lower byte of 32bit page address
      print("Working ...")
      # Read Modify Write
      dword=int(s.spi_read(wibNode,ADC0_SELECT,0x6068),16) # Read reset value on FOVR Threshold
      dword = dword | 0x80
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x6068,dword) # for CH-A, write to register address 68 data 0x82 in page 68000 Setting bit 7 Freezes DC correction
                                                        # clearing bit 7 enables DC offset correction, 0x02 enables DC correction. Bits D6-D3 set Bandwidth value
      print("Working ...")
      # Read Modify Write
      dword=int(s.spi_read(wibNode,ADC0_SELECT,0x7068),16) # Read reset value on FOVR Threshold
      dword = dword |0x80
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x7068,dword) # for CH-B, write to register address 68 data 0x82 in page 680000 Setting bit 7 Freezes DC correction.
                                                        # clearing bit 7 enables DC offset correction, 0x02 enables DC correction. Bits D6-D3 set Bandwidth value
      print("Working ...")
      # select main digital page 6800, put ADC addressing where we found it
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4005,0x00) # enable broadcast
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4003,0x00) # select JESD Digital Page
      s.spi_write(wibNode,ADC_SELECT_BOTH,0x4004,0x68) # select JESD Digital Page
      time.sleep(0.2)
   
   return 0

