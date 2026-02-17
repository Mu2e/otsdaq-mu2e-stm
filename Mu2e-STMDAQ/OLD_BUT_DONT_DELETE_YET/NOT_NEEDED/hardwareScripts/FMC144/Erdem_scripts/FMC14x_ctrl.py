#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

SW_TRIGGER_ENABLE = 4
HW_TRIGGER_ENABLE = 0

def FMC14x_ctrl_sw_trigger_enable (hw,enable):
  
  if enable == SW_TRIGGER_ENABLE:
    val |= (1<<SW_TRIGGER_ENABLE)
    
  if enable == HW_TRIGGER_ENABLE:
    val |= (1<<HW_TRIGGER_ENABLE)
    
  hw.getNode("axi_fmc144_8lane.FMC144_ctrl.adc_ctrl").write(val)
  hw.dispatch()

def FMC14x_ctrl_sw_trigger(hw):
  
  val = hw.getNode("axi_fmc144_8lane.FMC144_ctrl.adc_ctrl").read()
  hw.dispatch()
  
  val |= (1<<5)
  
  hw.getNode("axi_fmc144_8lane.FMC144_ctrl.adc_ctrl").write(val)
  hw.dispatch()

def FMC14x_ctrl_arm(hw):
  
  hw.getNode("axi_fmc144_8lane.FMC144_ctrl.fmc_info").write(0x01)
  hw.dispatch()

def FMC14x_ctrl_disarm(hw):
  
  hw.getNode("axi_fmc144_8lane.FMC144_ctrl.fmc_info").write(0x02)
  hw.dispatch()