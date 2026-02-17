#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal

LMK_SELECT=0x1
DAC_SELECT=0x2
ADC0_SELECT=0x4
ADC1_SELECT=0x8
ADC_SELECT_BOTH=0xC

def i2c_write(wibNode, slaveaddr, data):
  i2c_busy = 1
  rc = 0
  wibNode.getNode('i2c_master.i2c.address').write(slaveaddr)
  wibNode.dispatch()
  wibNode.getNode('i2c_master.i2c.wdat').write(data)
  wibNode.dispatch()
  wibNode.getNode('i2c_master.i2c.cmd').write(0x1)
  wibNode.dispatch()
  while (i2c_busy & 0x1):
    i2c_busy = wibNode.getNode("i2c_master.i2c.cmd").read()
    wibNode.dispatch()
    #print (hex(i2c_busy))
    time.sleep(0.1)

def i2c_read(wibNode, slaveaddr):
    i2c_busy = 1
    rc = 0
    wibNode.getNode('i2c_master.i2c.address').write(slaveaddr)
    wibNode.dispatch()
    wibNode.getNode('i2c_master.i2c.cmd').write(0x02)
    wibNode.dispatch()
    while (i2c_busy & 0x1):
       i2c_busy = wibNode.getNode("i2c_master.i2c.cmd").read()
       wibNode.dispatch()
       #print (hex(i2c_busy))
       time.sleep(0.1)
    rc = wibNode.getNode("i2c_master.i2c.read").read()
    wibNode.dispatch()
    return rc

def spi_read(wibNode, spi_select, spi_addr):
  
  spi_value = 0   

  wibNode.getNode("i2c_master.i2c.byte").write(0x00)
  wibNode.dispatch()
  
  if (spi_select==LMK_SELECT):
     spi_value += 0x1 << 23 # read enable
     spi_value += (spi_addr & 0x1FFF) << 8  # Address is in bits 20 downto 8
  elif (spi_select==DAC_SELECT):
     spi_value += 0x1 << 23 # read enable
     spi_value += (spi_addr & 0x7F) << 16   # Address is in bits 22 downto 16
  elif (spi_select==ADC0_SELECT) or (spi_select==ADC1_SELECT) :
     spi_value += 0x1 << 23 # read enable
     spi_value += (spi_addr & 0x7FFF) << 8   # Address is in bits 22 downto 8
  else:
     print("Unsupported SPI write access")
     return -1
  
  i2c_write(wibNode,0x1C06,spi_value >> 0) # Write 1st byte
  i2c_write(wibNode,0x1C07,spi_value >> 8) # Write 1st byte
  i2c_write(wibNode,0x1C08,spi_value >> 16) # Write 1st byte
  i2c_write(wibNode,0x1C00,spi_select) # Initiate SPI cycle
  dword0=i2c_read(wibNode,0x1C0E)
  dword1=i2c_read(wibNode,0x1C0F)
  data=(dword1 << 8) | dword0 
 
  return hex(data)
  

def spi_write(wibNode, spi_select, spi_addr, spi_value):
  
  rc=0
  dword = 0  

  wibNode.getNode("i2c_master.i2c.byte").write(0x00)
  wibNode.dispatch()
  
  if (spi_select==LMK_SELECT):
     dword += (spi_addr & 0x1FFF) << 8 # Adress is in bits 20 downto 8
     dword += spi_value & 0xFF   # 8 bit data
  elif (spi_select==DAC_SELECT):
     dword += (spi_addr & 0x7F) << 16 # Adress is in bits 22 downto 16
     dword += spi_value & 0xFFFF   # 16 bit data
  elif (spi_select==ADC0_SELECT) or (spi_select==ADC1_SELECT) or (spi_select==ADC_SELECT_BOTH):
     dword += (spi_addr & 0x7FFF) << 8 # Adress is in bits 22 downto 8
     dword += spi_value & 0xFF   # 8 bit data
  else:
     print("Unsupported SPI write access")
     return -1

  i2c_write(wibNode,0x1C06,dword >> 0) # Write 1st byte
  i2c_write(wibNode,0x1C07,dword >> 8) # Write 1st byte
  i2c_write(wibNode,0x1C08,dword >> 16) # Write 1st byte
  i2c_write(wibNode,0x1C00,spi_select) # Initiate SPI cycle

  return rc
