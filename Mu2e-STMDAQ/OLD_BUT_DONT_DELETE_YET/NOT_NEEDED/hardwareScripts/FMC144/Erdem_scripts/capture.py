#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s

def capture_setburstlength (hw,capturenode_nr,numberburst,burstlength):
  
  hw.getNode("Captures.capture_%s.nb_of_bursts" %(capturenode_nr)).write(numberburst)
  hw.dispatch()
  hw.getNode("Captures.capture_%s.burst_length" %(capturenode_nr)).write(burstlength)
  hw.dispatch()
  
  return 1

def capture_enable_upload (hw,capturenode_nr):
  hw.getNode("Captures.capture_%s.ctrl" %(capturenode_nr)).write(0x01)
  hw.dispatch()
  
  return 1

def capture_enable_upload_hw_trig (hw,capturenode_nr):
  hw.getNode("Captures.capture_%s.ctrl" %(capturenode_nr)).write(0x03)
  hw.dispatch()
  
  return 1


def capture_disable_upload (hw,capturenode_nr):
  hw.getNode("Captures.capture_%s.ctrl" %(capturenode_nr)).write(0x00)
  hw.dispatch()
  
  return 1
  
def capture_arm(hw,capturenode_nr):
  hw.getNode("Captures.capture_%s.command" %(capturenode_nr)).write(0x01)
  hw.dispatch()
  
  return 1
  
def capture_fifo_pointer_val(hw,capturenode_nr):
  value = hw.getNode("Captures.capture_%s.status" %(capturenode_nr)).read()
  hw.dispatch()
  
  return value

def capture_sw_trigger(hw,capturenode_nr):
  hw.getNode("Captures.capture_%s.command" %(capturenode_nr)).write(0x08)
  hw.dispatch()
  
  return 1

def capture_external_trigger_enable(hw,capturenode_nr):
  hw.getNode("Captures.capture_%s.ctrl" %(capturenode_nr)).write(0x02)
  hw.dispatch()
  
  return 1

def capture_fifo_rst(hw,capturenode_nr):
  hw.getNode("Captures.capture_%s.command" %(capturenode_nr)).write(0x10)
  hw.dispatch()
  
  return 1
