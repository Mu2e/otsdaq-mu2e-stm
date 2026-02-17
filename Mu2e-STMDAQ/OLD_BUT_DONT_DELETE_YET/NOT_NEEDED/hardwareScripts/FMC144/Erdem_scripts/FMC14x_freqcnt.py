#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

ADC_MODE_2CH_8LANE = 0
ADC_MODE_4CH_8LANE = 1
ADC_MODE_2CH_4LANE = 2
ADC_MODE_4CH_4LANE = 3

def FMC14x_freqcnt_getfrequency(hw,clksel,adc_mode):
  # Tell the firmware to start a measure on a given clock index
  hw.getNode("axi_fmc144_8lane.Freq_ctr.clk_sel").write(clksel)
  hw.dispatch()
  time.sleep(0.500)
  
  # Read back the just measured value
  dword = hw.getNode("axi_fmc144_8lane.Freq_ctr.clk_cnt").read()
  hw.dispatch()
  print (dword)
  
  testClkPeriod = 1.0/125.0
  tmp = 8192 * testClkPeriod
  tmp = tmp / (dword + 1)
  tmp = 1.0/tmp
  
  mult_factor = 4
  if adc_mode == ADC_MODE_2CH_4LANE or adc_mode == ADC_MODE_4CH_4LANE:
    mult_factor = 2
    
  if clksel==0:
    print ("Eth clock: %6.2f MHz" %tmp)
  elif clksel == 1:
    print ("ADCx PHY Clock: %6.2f MHz (Fs = %7.2f)" %(tmp,mult_factor*tmp))
  elif clksel == 2:
    print ("SYSREF Clock : %6.2f MHz (Fs = %7.2f)" %(tmp,tmp))
  elif clksel == 3:
    print ("DACx PHY Clock: %6.2f MHz (Fs = %7.2f)" %(tmp, mult_factor*tmp))
  elif clksel == 4:
    print ("External Trigger : %6.2f MHz" %tmp)
    