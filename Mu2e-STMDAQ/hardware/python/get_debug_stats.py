import sys
import time
import uhal
import spi_functions as s
import os

CLOCKTREE_CLKSRC_INTERNAL = 0,                          #< FMC120_clocktree_init() configure the clock tree for internal clock operations */
CLOCKTREE_CLKSRC_EXTERNAL = 1,                          #< FMC120_clocktree_init() configure the clock tree for external clock operations
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2,                      #< FMC120_clocktree_init() configure the clock tree for external reference operations */

uhal.setLogLevelTo(uhal.LogLevel.NOTICE)
here = os.path.dirname(os.path.abspath(__file__))
xml = os.path.join(here, "connections.xml")
manager = uhal.ConnectionManager("file://" + xml)
hw = manager.getDevice(sys.argv[1])


psmem=hw.getNode("Buffers.dtc_ctrl_stat_regs.ps_mem_1_wready").read()
hw.dispatch()
if (psmem == 0): success = False
print("PS Memory Ready =",bool(psmem))

timeout=hw.getNode("Buffers.Readout_regs.stat_timeout_occured").read()
hw.dispatch()
print("Timeout = ", bool(timeout))
time.sleep(0.01)

heartbeat=hw.getNode("Buffers.Readout_regs.stat_first_hb_received").read()
hw.dispatch()
print("First heartbeat = ", bool(heartbeat))
time.sleep(0.01)

temp = hw.getNode("Buffers.dtc_ctrl_stat_regs.hb_received_count").read()
hw.dispatch()
print("Heart beats received: ",hex(temp))
time.sleep(0.01)

temp = hw.getNode("Buffers.dtc_ctrl_stat_regs.evm_received_count").read()
hw.dispatch()
print("Event markers received: ",hex(temp))
time.sleep(0.01)

temp = hw.getNode("Buffers.dtc_ctrl_stat_regs.evm_processed_count_0").read()
hw.dispatch()
print("Event markers processed HPGe: ", hex(temp))
time.sleep(0.01)

temp = hw.getNode("Buffers.dtc_ctrl_stat_regs.evm_processed_count_1").read()
hw.dispatch()
print("Event markers processed LaBr: ", hex(temp))
time.sleep(0.2)
