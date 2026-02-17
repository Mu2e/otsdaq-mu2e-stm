#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal

def spi_read(wibNode, spictrlname, slaveaddress):
  
  spi_busy = 1
  
  wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_address' %(spictrlname)).write(slaveaddress)
  wibNode.dispatch()
  wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_command_status'%(spictrlname)).write(0x02)
  wibNode.dispatch()
  
  while (spi_busy & 0x0003 != 0x0002):
    
    value = wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_command_status'%(spictrlname)).read()
    wibNode.dispatch()
    spi_busy = int(value)
    #print spi_busy
    time.sleep(0.02)
    
  #print ("Response")
  
  data = wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_read_data'%(spictrlname)).read()
  wibNode.dispatch()
  return hex(data)
  

def spi_write(wibNode, spictrlname, slaveaddress, data):
  
  spi_busy = 1
  
  wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_address' %(spictrlname)).write(slaveaddress)
  wibNode.dispatch()
  wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_write_data'%(spictrlname)).write(data)
  wibNode.dispatch()
  wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_command_status'%(spictrlname)).write(0x01)
  wibNode.dispatch()
  
  while (spi_busy & 0x1):
    
    value = wibNode.getNode('axi_fmc144_8lane.spi_ctrl.%s.spi_command_status'%(spictrlname)).read()
    wibNode.dispatch()
    spi_busy = int(value)
    #print spi_busy
    time.sleep(0.02)
    
  #print ("Response")
  return 1