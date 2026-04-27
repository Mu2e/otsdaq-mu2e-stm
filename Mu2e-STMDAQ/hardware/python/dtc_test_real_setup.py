#!/usr/bin/python
# -*- coding: utf-8 -*-
import xml.etree.ElementTree as ET
import sys
import time
import uhal
import spi_functions as s
import os

CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */

uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
here = os.path.dirname(os.path.abspath(__file__))
xml = os.path.join(here, "connections.xml")
manager = uhal.ConnectionManager("file://" + xml)
hw = manager.getDevice(sys.argv[1])

def check_dtc_link():
   temp = 0

   try:
      temp=hw.getNode("Buffers.dtc_ctrl_stat_regs.dtc_link_up").read()
      hw.dispatch()
   except:
      print("DTC link check failed")
      return -1

   return temp

def dtc_comma_check():

   if (check_dtc_link() != 1):
      print("No DTC link detected")
      return -1

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

def dtc_check_counters():

   if (check_dtc_link() != 1):
      print("No DTC link detected")
      return [-1,-1,-1,-1,-1]

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
      dtc_clk_marker_sync_off = hw.getNode("Buffers.dtc_ctrl_stat_regs.clk_marker_sync_off_count").read()
      hw.dispatch()
   except:
      print("Clock marker bin synchronisation check failed")
      dtc_clk_marker_sync_off = -1

   return [int(evm_hb_seq_err_count),
           int(hb_evm_int_err_count),
           int(hb_crc_err_count),
           int(event_tag_skipped_count),
           int(dtc_clk_marker_sync_off)]

stmdaq_root = os.environ.get("STMDAQ_ROOT")
if not stmdaq_root:
    raise RuntimeError("STMDAQ_ROOT is not set in the environment")
fw_xml = os.path.join(stmdaq_root, "config", "xml", "firmware.xml")
root = ET.parse(fw_xml).getroot()

use = root.find(".//use_dtc_sim")
def _fw_use_dtc_sim_value(xml_name: str) -> int:
    el = use.find(xml_name)
    if el is None or el.text is None or not el.text.strip():
        raise KeyError("Missing <{}> under <use_dtc_sim> in {}".format(xml_name, fw_xml))
    return int(el.text.strip(), 0)


DTC_KEY_MAP = {
    "teng_interpacket_pause": "teng_interpacket_pause",
    "ext_trig_timeout": "ext_trig_timeout",
    "10g_readout_chan1": "readout_chan1",
    "10g_readout_chan2": "readout_chan2",
    "deltahb": "deltahb",
    "numberhb": "numberhb",
    "actual_adc_enable": "actual_adc_enable",
}

dtc_regs = {}
for hw_key, xml_key in DTC_KEY_MAP.items():
    dtc_regs[hw_key] = _fw_use_dtc_sim_value(xml_key)

time.sleep(0.5)
psmem=hw.getNode("Buffers.dtc_ctrl_stat_regs.ps_mem_1_wready").read()
hw.dispatch()

if (psmem == 0): success = False
#print("PS Memory Ready =",bool(psmem))

timeout=hw.getNode("Buffers.Readout_regs.stat_timeout_occured").read()
hw.dispatch()
#print("Timeout = ", bool(timeout))

heartbeat=hw.getNode("Buffers.Readout_regs.stat_first_hb_received").read()
hw.dispatch()
#print("First heartbeat = ", bool(heartbeat))
	
# Reset board
hw.getNode("Buffers.Readout_regs.teng_interpacket_pause").write(dtc_regs["teng_interpacket_pause"])
hw.dispatch()
hw.getNode("Buffers.10g_readout_2.reset_readout").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.reset_trig_count").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_4.DDR_wr_addr_reset").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.ext_trig_timeout").write(dtc_regs["ext_trig_timeout"])
hw.dispatch()
hw.getNode("Buffers.Readout_regs.reset_40MHz_75MHz_timers").write(0x1)
hw.dispatch()

# Set real ADC and DTC registers
hw.getNode("Buffers.Controls_simADC_simDTC.actual_adc_enable").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Controls_simADC_simDTC.actual_dtc_enable").write(0x1)
hw.dispatch()

# Change channels here 0x1 is read on (chan1 = HPGe, chan2 = LaBr)
hw.getNode("Buffers.Readout_regs.10g_readout_chan1").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.10g_readout_chan2").write(0x1)
hw.dispatch()

link = check_dtc_link()
print("DTC link =", bool(link))
#comma_check = dtc_comma_check()
#print(comma_check)
#counter_check = dtc_check_counters()
#print(counter_check)

finished=True
