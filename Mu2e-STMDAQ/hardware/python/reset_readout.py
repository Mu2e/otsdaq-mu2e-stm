#!/usr/bin/python
# -- coding: utf-8 --
import xml.etree.ElementTree as ET
import sys
import time
import uhal
import spi_functions as s
import os

uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
here = os.path.dirname(os.path.abspath(__file__))
xml = os.path.join(here, "connections.xml")
manager = uhal.ConnectionManager("file://" + xml)
hw = manager.getDevice(sys.argv[1])

hw.getNode("Buffers.10g_readout_2.reset_readout").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.reset_trig_count").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_4.DDR_wr_addr_reset").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.reset_40MHz_75MHz_timers").write(0x1)
hw.dispatch()

hw.getNode("Buffers.Readout_regs.10g_readout_chan1").write(0x0)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.10g_readout_chan2").write(0x0)
hw.dispatch()
time.sleep(0.01)
hw.getNode("Buffers.Readout_regs.10g_readout_chan1").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.10g_readout_chan2").write(0x1)
hw.dispatch()

finished = True
