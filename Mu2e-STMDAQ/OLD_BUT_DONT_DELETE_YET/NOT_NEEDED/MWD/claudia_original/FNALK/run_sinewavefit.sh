#!/bin/bash 

RATE=3000
RATEMHz=3

#RATE=50

#50kHz
#for VPP in `echo "0.005 0.01 0.05 0.10 0.20 0.25 0.30 0.35 0.40 0.45 0.50 0.55 0.60 0.65 0.75 0.80 0.85 0.90 0.95 1.00 1.02 1.04 1.06 1.08 1.10 1.20"`
#3MHz
for VPP in `echo "0.005 0.01 0.05 0.10 0.30 0.40 0.50 0.60 0.70 0.80 0.90 1.00 1.05 1.08 1.10 1.12 1.14"`
do 
    #INFILE=/data1/STM_VST_DATA/FMC120tests_27-11-23/adc_only_files/${VPP}Vptp_${RATE}kHz.bin
    INFILE=/data1/STM_VST_DATA/FMC120tests_27-11-23/adc_only_files/${VPP}Vptp_${RATEMHz}MHz.bin

    echo "root '/home/stm_mu2e/claudiaa/STMDAQ-TestBeam/MWD/FNALK/AnalyseSineWave.C(\"$INFILE\",$VPP,$RATE)' 2>&1 &"

    root \/home\/stm\_mu2e\/claudiaa\/STMDAQ\-TestBeam\/MWD\/FNALK\/AnalyseSineWave.C\(\"$INFILE\"\,$VPP\,$RATE\) 2>&1 &
    sleep 1
done




