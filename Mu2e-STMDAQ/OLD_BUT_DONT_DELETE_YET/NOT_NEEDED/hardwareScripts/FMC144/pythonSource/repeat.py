#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

def repeat_setburstlength (hw,repeatnode_nr,burstlength):
  
  hw.getNode("Repeats.repeat_%s.burst_size" %(repeatnode_nr)).write(burstlength)
  hw.dispatch()
  
  return 1

def repeat_prepare_wfm_upload (hw,repeatnode_nr):
  hw.getNode("Repeats.repeat_%s.ctrl" %(repeatnode_nr)).write(0x08)
  hw.dispatch()
  
  return 1
  
def repeat_arm(hw,repeatnode_nr):
  hw.getNode("Repeats.repeat_%s.ctrl" %(repeatnode_nr)).write(0x01)
  hw.dispatch()
  
  return 1

def repeat_disarm(hw,repeatnode_nr):
  hw.getNode("Repeats.repeat_%s.ctrl" %(repeatnode_nr)).write(0x02)
  hw.dispatch()
  
  return 1

def repeat_rst(hw,repeatnode_nr):
  hw.getNode("Repeats.repeat_%s.ctrl" %(repeatnode_nr)).write(0x10)
  hw.dispatch()
  
  return 1
  
def repeat_check_status(hw,repeatnode_nr):
  value = hw.getNode("Repeats.repeat_%s.status" %(repeatnode_nr)).read()
  hw.dispatch()
  
  return value
