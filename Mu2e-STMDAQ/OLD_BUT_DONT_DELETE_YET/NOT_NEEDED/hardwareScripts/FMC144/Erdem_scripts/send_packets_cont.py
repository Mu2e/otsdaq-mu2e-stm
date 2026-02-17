#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import os
import spi_functions as s
import FMC14x_monitor as m
import FMC14x as f
import FMC14x_freqcnt as t
import repeat as r
import capture as c
import utils as u

uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
manager = uhal.ConnectionManager("file://connections.xml")
print sys.argv
#hw = manager.getDevice(sys.argv[1])
hw = manager.getDevice("KCU105")

os.system("ping -c 1 -w2 192.168.99.207")
os.system("ping -c 1 -w2 192.168.34.8")

#hw.getNode("Buffers.stop_overwrite").write(0x1)
#hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_on_debug").write(0x0)
hw.dispatch()

hw.getNode("Buffers.10g_readout.cont_readout_8K_chunks").write(0x1)
hw.dispatch()
time.sleep (1)
hw.getNode("Buffers.Debug_controls_pulse_2.cont_data_off_debug").write(0x1)
hw.dispatch()
hw.getNode("Buffers.10g_readout.cont_readout_8K_chunks").write(0x0)
hw.dispatch()


