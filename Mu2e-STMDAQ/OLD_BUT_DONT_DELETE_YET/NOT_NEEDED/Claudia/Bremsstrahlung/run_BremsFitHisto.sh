#!/bin/bash 

TIMESIM=100
MEANE=0.347
SIGMACUT=3
RATE=200

for RESOL in `echo "0.0010 0.0012 0.0014 0.0016 0.0018 0.0020 0.0022 0.0024 0.0026 0.0028 0.0030"`
#for RESOL in `echo "0.0010"` 
do
    echo "root '/home/stm_mu2e/claudiaa/STMDAQ-TestBeam/Claudia/Bremsstrahlung/BremsstrahlungFit.C($RATE,$TIMESIM,$MEANE,$RESOL,$SIGMACUT)' 2>&1 &"

    root \/home\/stm\_mu2e\/claudiaa\/STMDAQ\-TestBeam\/Claudia\/Bremsstrahlung\/BremsstrahlungFit.C\($RATE\,$TIMESIM\,$MEANE,$RESOL,$SIGMACUT\) 2>&1 &
    sleep 1
done




