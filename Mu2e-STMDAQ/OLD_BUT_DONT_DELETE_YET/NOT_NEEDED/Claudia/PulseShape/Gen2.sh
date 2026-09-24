#!/bin/bash 

ENERGY=662
NOISE_SIGMA=1
NUM_XRAYS=10000
SEED=0

EXE=/work/mu2e/cgarcia/STMDAQ-TestBeam/build/bin/HPGe_ehModelSim_setseed.exe

OUTPUT_LOGFOLDER=/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV
if [ ! -e ${OUTPUT_LOGFOLDER} ] ; then
 echo "Folder does not exist : creating ${OUTPUT_LOGFOLDER}"
 mkdir -p ${OUTPUT_LOGFOLDER}
fi
echo "OUTPUT LOG FOLDER = "${OUTPUT_LOGFOLDER}

OUTPUT_BINFOLDER=/data1/cgarcia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV
if [ ! -e ${OUTPUT_BINFOLDER} ] ; then
 echo "Folder does not exist : creating ${OUTPUT_BINFOLDER}"
 mkdir -p ${OUTPUT_BINFOLDER}
fi
echo "OUTPUT BIN FOLDER = "${OUTPUT_BINFOLDER}

for RATE in `echo "20 30"`
do 
  echo "RATE = "${RATE}" kHz"
  TIME_MUSEC=`echo "$NUM_XRAYS*1000 / $RATE" | bc`
  echo "TIME IN MU SEC = "${TIME_MUSEC} 
  LOGFILE=${OUTPUT_LOGFOLDER}/logfile${RATE}kHz_${NUM_XRAYS}.log
  echo ${LOGFILE}
  OUTPUTFILE=${OUTPUT_BINFOLDER}/genData${ENERGY}keV_${RATE}kHz.bin
  echo ${OUTPUTFILE}

  echo "$EXE $RATE $ENERGY $NOISE_SIGMA $TIME_MUSEC $SEED $OUTPUTFILE > $LOGFILE 2>&1 &"
  $EXE $RATE $ENERGY $NOISE_SIGMA $TIME_MUSEC $SEED $OUTPUTFILE > $LOGFILE 2>&1 &
  sleep 1
done

