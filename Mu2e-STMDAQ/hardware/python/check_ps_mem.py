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

psmem0=hw.getNode("Buffers.dtc_ctrl_stat_regs.ps_mem_0_wready").read()
hw.dispatch()
if (psmem0 == 0): success = False

psmem1=hw.getNode("Buffers.dtc_ctrl_stat_regs.ps_mem_1_wready").read()
hw.dispatch()
if (psmem1 == 0): success = False

print("PS Memory Ready =", bool(psmem0), "and", bool(psmem1))
#print("PS Memory Ready =", success)
time.sleep(0.5)
