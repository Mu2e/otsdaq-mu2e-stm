#!/bin/bash                                                                                                                                      

#/work/cgarcia/STMDAQ-TestBeam/build/bin/energy_histo.exe /work/data/run00075.bin > /work/cgarcia/run_75_time_fixmean.log 2>&1 &
#/work/cgarcia/STMDAQ-TestBeam/build/bin/energy_histo_skipROOT.exe /work/data/run00075.bin > /work/cgarcia/run_75_time_fixmean_skipROOT.log 2>&1 &
/work/cgarcia/STMDAQ-TestBeam/build/bin/energy_histo_skipROOT.exe /work/cgarcia/STMDAQ-TestBeam/energy_histo/run00075_suppressedsignal_bin.bin > /work/cgarcia/run_75suppressed_time_fixmean_skipROOT.log 2>&1 & 
sleep 2



