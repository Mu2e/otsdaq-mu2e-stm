#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s


def FMC120_freqcnt_getfrequency(wibNode,clksel):
   
   # tell the firmware to start a measure on a given clock index
   wibNode.getNode("axi_fmc120_8lane.Freq_ctr.clk_sel").write(clksel)
   wibNode.dispatch()

   time.sleep(0.5)

   # read back the just measured value
   dword = wibNode.getNode("axi_fmc120_8lane.Freq_ctr.clk_cnt").read()
   wibNode.dispatch()

   cmdfreq = 125.0

   testClkperiod = 1.0 /125
   tmp = 8192 * testClkperiod
   tmp = (tmp /(dword+1))
   tmp = 1.00/tmp

   mult_factor = 4
   
   if (clksel == 0):
       print("Stellar IP Clock: %6.2f MHz" %tmp) 
   elif (clksel == 1):
       print("ADCx PHY Clock: %6.2f MHz" %tmp)
   elif (clksel == 2):
       print("LMK Clock : %6.2f MHz" %tmp)
   elif (clksel == 3):
       print("DACx PHY Clock: %6.2f MHz" %tmp)
   elif (clksel == 4):
       print("External Trigger: %6.2f MHz" %tmp)
   return 0
