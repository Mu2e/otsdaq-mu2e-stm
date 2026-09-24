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


def check_dtc_link(hw_dir):
   uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
   manager = uhal.ConnectionManager("file://"+hw_dir+"/connections.xml")
   hw = manager.getDevice("ZCU102-FMC120")

   temp = 0

   try:
      temp=hw.getNode("Buffers.dtc_ctrl_stat_regs.dtc_link_up").read()
      hw.dispatch()
   except:
      print("DTC link check failed")
      return -1
      
   return temp


def dtc_comma_check(hw_dir):

   if (check_dtc_link(hw_dir) != 1):
      print("No DTC link detected")
      return -1

   uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
   manager = uhal.ConnectionManager("file://"+hw_dir+"/connections.xml")
   hw = manager.getDevice("ZCU102-FMC120")

   temp = 0

   try:
      hw.getNode("Buffers.dtc_ctrl_stat_regs.comma_detect_start").write(0x1)
      hw.dispatch()
   except:
      print("DTC comma character check failed")
      return -1
   
   print("Counting dtc comma characters for 60 seconds...")
   count = 61
   while(temp != 1):
      try:
         temp=hw.getNode("Buffers.dtc_ctrl_stat_regs.comma_detect_stats").read()
         hw.dispatch()
         time.sleep(1)
         count -= 1
         if (count < 60):
            if (count % 10 == 0):
               print("Counting dtc comma characters for %d seconds..." % count) 
         if (count == 0 and temp != 1):
            print("DTC comma character check failed")
            return -1
      except:
         print("DTC comma character check failed")
         return -1

   temp=(temp>>1) & 0x7FFF
   return temp

def dtc_check_counters(hw_dir):

   if (check_dtc_link(hw_dir) != 1):
      #print("No DTC link detected")
      return [-1,-1,-1,-1,-1]

   uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
   manager = uhal.ConnectionManager("file://"+hw_dir+"/connections.xml")
   hw = manager.getDevice("ZCU102-FMC120")

   # Check how many times the HB packet doesn come before the event marker
   # 24 bits
   evm_hb_seq_err_count = 0
   try:
      evm_hb_seq_err_count = hw.getNode("Buffers.dtc_ctrl_stat_regs.evm_hb_seq_err_count").read()
      hw.dispatch()
   except:
      print("Event marker / heartbeat sequence check failed")
      evm_hb_seq_err_count = -1

   # Check how many times the HB packet comes < 850 ns before the event marker
   # 24 bits
   hb_evm_int_err_count = 0
   try:
      hb_evm_int_err_count = hw.getNode("Buffers.dtc_ctrl_stat_regs.hb_evm_int_err_count").read()
      hw.dispatch()
   except:
      print("Event marker / heartbeat interval  check failed")
      hb_evm_int_err_count = -1

   # Count heartbeat packet CRC errors
   # 24 bits
   hb_crc_err_count = 0
   try:
      hb_crc_err_count = hw.getNode("Buffers.dtc_ctrl_stat_regs.hb_crc_err_count").read()
      hw.dispatch()
   except:
      print("Heartbeat CRC error counter check failed")
      hb_crc_err_count = -1

   # Count number of skipped EWTs
   # 24 bits
   event_tag_skipped_count = 0  
   try:
      event_tag_skipped_count = hw.getNode("Buffers.dtc_ctrl_stat_regs.event_tag_skipped_count").read()
      hw.dispatch()
   except:
      print("Skipped EWT check failed")
      event_tag_skipped_count = -1

   # Count number of times the clock marker fell in the wrong bin
   # 24 bits
   dtc_clk_marker_sync_off = 0  
   try:
      dtc_clk_marker_sync_off = hw.getNode("Buffers.dtc_ctrl_stat_regs.dtc_clk_marker_sync_off").read()
      hw.dispatch()
   except:
      print("Clock marker bin synchronisation check failed")
      dtc_clk_marker_sync_off = -1

   return [int(evm_hb_seq_err_count),
           int(hb_evm_int_err_count),
           int(hb_crc_err_count),
           int(event_tag_skipped_count),
           int(dtc_clk_marker_sync_off)]
