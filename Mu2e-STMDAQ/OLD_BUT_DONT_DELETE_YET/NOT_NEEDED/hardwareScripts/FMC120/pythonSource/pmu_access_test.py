#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s
#import FMC14x_monitor as m
import FMC120_300MSPS as f
import FMC120_freqcnt as fc
#import FMC14x_freqcnt as t
#import repeat as r
#import capture as c
#import utils as u

CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */



uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
manager = uhal.ConnectionManager("file://connections.xml")
print sys.argv
hw = manager.getDevice(sys.argv[1])

DATAGEN_SINE_WAVE = 0
DATAGEN_SAW_WAVE  = 1

adc_mode = 1
#N_SAMPS = 8192
#N_SIZE = 8192
numberburst = 0
burstlength = 1536 #512 * 2 * 2
burstlength_capture = 1256 #628

print hw.id()
	
print "Info"
#hw.getNode("General_reset").write(0x01)
#hw.dispatch()
#time.sleep(0.5)
#raw_input("Press Enter to continue...")
# Configure I2C switch to HPC connector on the ZCU102
#Set one byte per cycle

#hw.getNode("Buffers.pmu_access.address").write(0xFFD80410)
#hw.dispatch()
#hw.getNode("Buffers.pmu_access.command").write(0x0)
#hw.dispatch()

hw.getNode("Buffers.pmu_access.address").write(0xFFD8041c)
hw.dispatch()
hw.getNode("Buffers.pmu_access.wr_data").write(0x0)
hw.dispatch()
hw.getNode("Buffers.pmu_access.command").write(0x1)
hw.dispatch()

hw.getNode("Buffers.pmu_access.address").write(0xFFD80418)
hw.dispatch()
hw.getNode("Buffers.pmu_access.wr_data").write(0xD0000000)
hw.dispatch()
hw.getNode("Buffers.pmu_access.command").write(0x1)
hw.dispatch()



hw.getNode("Buffers.pmu_access.address").write(0xFFD80414)
hw.dispatch()
hw.getNode("Buffers.pmu_access.command").write(0x0)
hw.dispatch()

hw.getNode("Buffers.pmu_access.address").write(0xFFD80420)
hw.dispatch()
hw.getNode("Buffers.pmu_access.wr_data").write(0x40000000)
hw.dispatch()
hw.getNode("Buffers.pmu_access.command").write(0x1)
hw.dispatch()

hw.getNode("Buffers.pmu_access.address").write(0xFFD80410)
hw.dispatch()
hw.getNode("Buffers.pmu_access.command").write(0x0)
hw.dispatch()

hw.getNode("Buffers.pmu_access.address").write(0xFFD80410)
hw.dispatch()
hw.getNode("Buffers.pmu_access.wr_data").write(0x40000000)
hw.dispatch()
hw.getNode("Buffers.pmu_access.command").write(0x1)
hw.dispatch()

hw.getNode("Buffers.pmu_access.address").write(0xFFD80410)
hw.dispatch()
hw.getNode("Buffers.pmu_access.command").write(0x0)
hw.dispatch()

