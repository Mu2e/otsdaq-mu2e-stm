#!/usr/bin/python
# -- coding: utf-8 --
import xml.etree.ElementTree as ET
import sys
import time
import uhal
import spi_functions as s
#import FMC14x_monitor as m
#import FMC120_300MSPS as f
#import FMC120_freqcnt as fc
import os
#import FMC14x_freqcnt as t
#import repeat as r
#import capture as c
#import utils as u
CLOCKTREE_CLKSRC_INTERNAL = 0,							#< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,							#< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,			            #< FMC120_clocktree_init() configure the clock tree for external reference operations */
uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
here = os.path.dirname(os.path.abspath(__file__))
xml = os.path.join(here, "connections.xml")
manager = uhal.ConnectionManager("file://" + xml)
print (sys.argv)
hw = manager.getDevice(sys.argv[1])
DATAGEN_SINE_WAVE = 0
DATAGEN_SAW_WAVE  = 1
adc_mode = 1
#N_SAMPS = 8192
#N_SIZE = 8192
numberburst = 0
burstlength = 1536 #512 * 2 * 2
burstlength_capture = 1256 #628
print (hw.id())
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
#hw.getNode("Buffers.Readout_regs.10g_readout_chan1_chan2").write(0x0)
#hw.dispatch()
#hw.getNode("Buffers.10g_readout_2.disable_reset_memory_readout").write(0x1)
#hw.dispatch()
#hw.getNode("Buffers.10g_readout_2.disable_reset_memory_readout").write(0x0)
#hw.dispatch()
hw.getNode("Buffers.10g_readout_2.reset_readout").write(0x1)
hw.dispatch()
# --- Load DTC sim register values from firmware.xml ---
stmdaq_root = os.environ.get("STMDAQ_ROOT")
if not stmdaq_root:
    raise RuntimeError("STMDAQ_ROOT is not set in the environment")
fw_xml = os.path.join(stmdaq_root, "config", "xml", "firmware.xml")
root = ET.parse(fw_xml).getroot()

use = root.find(".//use_dtc_sim")
if use is None:
    raise KeyError("Could not find <use_dtc_sim> in {}".format(fw_xml))

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
# --- End firmware.xml load ---

hw.getNode("Buffers.Readout_regs.teng_interpacket_pause").write(dtc_regs["teng_interpacket_pause"])
hw.dispatch()
hw.getNode("Buffers.Readout_regs.reset_trig_count").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Debug_controls_pulse_4.DDR_wr_addr_reset").write(0x1)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.ext_trig_timeout").write(dtc_regs["ext_trig_timeout"]) #(0x249F0)
hw.dispatch()
#hw.getNode("Buffers.Readout_regs.prescale_hpge_raw_on").write(0x3)
#hw.dispatch()
hw.getNode("Buffers.Readout_regs.10g_readout_chan1").write(dtc_regs["10g_readout_chan1"])
hw.dispatch()
hw.getNode("Buffers.Readout_regs.10g_readout_chan2").write(dtc_regs["10g_readout_chan2"])
hw.dispatch()
hw.getNode("Buffers.Controls_simADC_simDTC.actual_dtc_enable").write(0x0)
hw.dispatch()
hw.getNode("Buffers.dtc_sim_params.dtc_sim_reset").write(0x1)
hw.dispatch()
hw.getNode("Buffers.dtc_sim_params.dtc_sim_reset").write(0x0)
hw.dispatch()
hw.getNode("Buffers.Readout_regs.reset_40MHz_75MHz_timers").write(0x1)
hw.dispatch()
hw.getNode("Buffers.dtc_sim_params.deltahb").write(dtc_regs["deltahb"]) #(0x2064) #(0x30D4) #(0x240) # #(0x160) #(0x200) (0x920) #
hw.dispatch()
hw.getNode("Buffers.dtc_sim_params.numberhb").write(dtc_regs["numberhb"]) #(0x2428e) #(0xffffe80)  #(0x1000) # #(0x120) #(0xf0) #(0xc0) #(0x60) #(0x30)
hw.dispatch()
hw.getNode("Buffers.Controls_simADC_simDTC.actual_adc_enable").write(dtc_regs["actual_adc_enable"])
hw.dispatch()
time.sleep(0.02)
hw.getNode("Buffers.dtc_sim_params.dtc_sim_start").write(0x1)
hw.dispatch()
time.sleep(10)
#hw.getNode("Buffers.Readout_regs.Test_beam_10g_readout").write(0x0)
#hw.dispatch()

finished = True
