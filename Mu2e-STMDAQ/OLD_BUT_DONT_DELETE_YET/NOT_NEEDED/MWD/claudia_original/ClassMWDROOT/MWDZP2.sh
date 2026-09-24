#!/bin/bash 

ENERGY=662
NOISE_SIGMA=0.32
M=400
L=200

declare -a SIMPEAKS=(9987, 9982, 9980, 10000, 9999, 10000, 10000, 10000, 9999, 9995, 9999, 9999, 10000)
INDEX=0

EXE=/work/cgarcia/STMDAQ-TestBeam/build/bin/MWDROOT.exe
INPUT_FOLDER=/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV/SuppresedFiles_tbefore1us_tafter2us_newcodev3
OUTPUT_FOLDER=/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV/SuppresedFiles_tbefore1us_tafter2us_newcodev3/M${M}L${L}

if [ ! -e ${OUTPUT_FOLDER} ] ; then
 echo "Folder does not exist : creating ${OUTPUT_FOLDER}"
 mkdir -p ${OUTPUT_FOLDER}
fi
echo "OUTPUT FOLDER = "${OUTPUT_FOLDER}

for RATE in `echo "01 05 10 20 30 40 50 80 100 120 150 180 200"`
do 
  echo "RATE = "${RATE}" kHz"

  TEXTFILE=${OUTPUT_FOLDER}/run_${RATE}kHz_M${M}_L${L}.txt
  echo ${TEXTFILE}
  INPUT=${INPUT_FOLDER}/SupData${ENERGY}keV_${RATE}kHz.bin
  echo ${INPUT}

  echo "$EXE $INPUT $M $L ${SIMPEAKS[$INDEX]} > $TEXTFILE 2>&1 &"
  $EXE $INPUT $M $L ${SIMPEAKS[$INDEX]} > $TEXTFILE 2>&1 &
  sleep 1
  INDEX=$[INDEX+1]
done

