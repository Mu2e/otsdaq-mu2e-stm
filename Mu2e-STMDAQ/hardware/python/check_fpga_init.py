#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s
import os

here = os.path.dirname(os.path.abspath(__file__))
xml = os.path.join(here, "connections.xml")
manager = uhal.ConnectionManager("file://" + xml)
uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
hw = manager.getDevice(sys.argv[1])

success = True

date=hw.getNode("CID_registers.hw_build_date").read()
hw.dispatch()
year  = (date >> 16) & 0xFFFF
month = (date >> 8)  & 0xFF
day   = date & 0xFF
print(f"Hardware build date = {hex(date)} = {year}-{month}-{day}")
time.sleep(0.5)
fw_type=hw.getNode("CID_registers.firmware_type").read()
hw.dispatch()
if (fw_type == 0):
    success = False
    fwtype = "NONE"
if (fw_type == 0xA): fwtype = "Feature"
if (fw_type == 0xB): fwtype = "Bug Fix"
if (fw_type == 0xC): fwtype = "Stable"
print(f"Firmware type = {hex(fw_type)} = {fwtype}")
# time.sleep(0.5)
# adc_type=hw.getNode("CID_registers.ADC_type").read()
# hw.dispatch()
# if (adc_type == 0):
#     success = False
#     adctype = "NONE"
# if (adc_type == 0xA): adctype = "FMC120"
# if (adc_type == 0xB): adctype = "FMC144"
# print(f"ADC type = {hex(adc_type)} = {adctype}")
# time.sleep(0.5)
# adc0=hw.getNode("CID_registers.ADC_channel_0_valid").read()
# hw.dispatch()
# if (adc0 == 0): success = False
# print("ADC Channel 0 Initialised =",bool(adc0))
# time.sleep(0.5)
# adc1=hw.getNode("CID_registers.ADC_channel_1_valid").read()
# hw.dispatch()
# if (adc1 == 0): success = False
# print("ADC Channel 1 Initialised =",bool(adc0))
# time.sleep(0.5)
# psmem=hw.getNode("Buffers.dtc_ctrl_stat_regs.ps_mem_1_wready").read()
# hw.dispatch()
# if (psmem == 0): success = False
# print("PS Memory Ready =",bool(psmem))
# time.sleep(0.5)
if (success): print("System Ready...")
