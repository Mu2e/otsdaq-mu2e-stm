#!/bin/bash 

ENERGY=662;
NOISE_SIGMA=0.32

EXE=/work/cgarcia/STMDAQ-TestBeam/build/bin/gradient_peaksuppression_v3.exe

INPUT_FOLDER=/data1/work/cgarcia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV

OUTPUT_FOLDER=/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV/SuppresedFiles_tbefore1us_tafter2us_newcodev3
if [ ! -e ${OUTPUT_FOLDER} ] ; then
 echo "Folder does not exist : creating ${OUTPUT_FOLDER}"
 mkdir -p ${OUTPUT_FOLDER}
fi
echo "OUTPUT FOLDER = "${OUTPUT_FOLDER}

for RATE in `echo "20 30 40 50 80 100 120 150 180 200"`
do 
  echo "RATE = "${RATE}" kHz"
  TEXTFILE=${OUTPUT_FOLDER}/Supdata_${RATE}kHz.txt
  echo ${TEXTFILE}
  INPUT=${INPUT_FOLDER}/genData${ENERGY}keV_${RATE}kHz.bin
  echo ${INPUT}

  echo "$EXE $INPUT > $TEXTFILE 2>&1 &"
  $EXE $INPUT > $TEXTFILE 2>&1 &
  sleep 1
done

