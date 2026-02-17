#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import spi_functions as s
#import FMC14x_monitor as m
import FMC120_300MSPS as f
import FMC120_freqcnt as fc
import os
#import FMC14x_freqcnt as t
#import repeat as r
#import capture as c
#import utils as u

def start_10G_readout(hw_dir):
    
    uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
    manager = uhal.ConnectionManager("file://"+hw_dir+"/connections.xml")
    hw = manager.getDevice("ZCU102-FMC120")
    
    os.system("ping -c 1 -w2 192.168.34.10")
    time.sleep(0.5)
    os.system("ping -c 1 -w2 192.168.36.10")
    time.sleep(0.5)
    
    print ("Info")
    #hw.getNode("General_reset").write(0x01)
    #hw.dispatch()
    #time.sleep(0.5)
    #raw_input("Press Enter to continue...")
    # Configure I2C switch to HPC connector on the ZCU102
    #Set one byte per cycle
    
    hw.getNode("Buffers.Readout_regs.10g_readout_chan1_chan2").write(0x0)
    hw.dispatch()
    
    #hw.getNode("Buffers.10g_readout_2.disable_reset_memory_readout").write(0x1)
    #hw.dispatch()
    
    #hw.getNode("Buffers.10g_readout_2.disable_reset_memory_readout").write(0x0)
    #hw.dispatch()
    
    hw.getNode("Buffers.10g_readout_2.reset_readout").write(0x1)
    hw.dispatch()
    
    # 156.25 MHz
    #hw.getNode("Buffers.Readout_regs.teng_interpacket_pause").write(0x40)
    hw.getNode("Buffers.Readout_regs.teng_interpacket_pause").write(0x10)
    hw.dispatch()
    
    hw.getNode("Buffers.Readout_regs.reset_trig_count").write(0x1)
    hw.dispatch()
    
    hw.getNode("Buffers.Debug_controls_pulse_4.DDR_wr_addr_reset").write(0x1)
    hw.dispatch()
    
    hw.getNode("Buffers.Readout_regs.ext_trig_timeout").write(0x15F90) #(0x249F0) # timeout is 75 MHz clock, 200 us = 0x3A98 
    hw.dispatch()
    
    
    
    #hw.getNode("Buffers.Readout_regs.prescale_hpge_raw_on").write(0x3)
    #hw.dispatch()
    
    hw.getNode("Buffers.Readout_regs.10g_readout_chan1_chan2").write(0x3)
    hw.dispatch()
    
    hw.getNode("Buffers.dtc_sim_params.dtc_sim_reset").write(0x1)
    hw.dispatch()
    hw.getNode("Buffers.dtc_sim_params.dtc_sim_reset").write(0x0)
    hw.dispatch()
    
    hw.getNode("Buffers.Readout_regs.reset_40MHz_75MHz_timers").write(0x1)
    hw.dispatch()
    
    hw.getNode("Buffers.dtc_sim_params.deltahb").write(0x30D4) #(0x30D4) #(0x240) # #(0x160) #(0x200)
    hw.dispatch()
    hw.getNode("Buffers.dtc_sim_params.numberhb").write(0xFFF)#(0x6FFFFFFF)  #(0x1000) # #(0x120) #(0xf0) #(0xc0) #(0x60) #(0x30) 
    hw.dispatch()
    time.sleep(0.02)
    hw.getNode("Buffers.dtc_sim_params.dtc_sim_start").write(0x1)
    hw.dispatch()
    time.sleep(10)
    #hw.getNode("Buffers.Readout_regs.Test_beam_10g_readout").write(0x0)
    #hw.dispatch()


def reset_readout(hw_dir):
    
    uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
    manager = uhal.ConnectionManager("file://"+hw_dir+"/connections.xml")
    hw = manager.getDevice("ZCU102-FMC120")
    
    hw.getNode("Buffers.10g_readout_2.reset_readout").write(0x1)
    hw.dispatch()

    hw.getNode("Buffers.Readout_regs.reset_40MHz_75MHz_timers").write(0x1)
    hw.dispatch()   
