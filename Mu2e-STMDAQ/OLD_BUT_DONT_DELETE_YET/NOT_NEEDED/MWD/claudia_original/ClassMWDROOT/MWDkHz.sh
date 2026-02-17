#!/bin/bash 

ENERGY=1809
NOISE_SIGMA=0.32
NUM_XRAYS=500
SIMPEAKS=9980
M=400
L=200

MAX_RUNNING_JOBS=20
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=20
RATE=10 #01,05,10kHz


EXE=/work/cgarcia/STMDAQ-TestBeam/build/bin/MWDROOT.exe
INPUT_FOLDER=/data1/cgarcia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV
OUTPUT_FOLDER=/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/${ENERGY}keV_${NOISE_SIGMA}mV/M${M}L${L}

if [ ! -e ${OUTPUT_FOLDER} ] ; then
 echo "Folder does not exist : creating ${OUTPUT_FOLDER}"
 mkdir -p ${OUTPUT_FOLDER}
fi
echo "OUTPUT FOLDER = "${OUTPUT_FOLDER}




while [ $JOB_NUMBER -lt $MAX_JOBS_TO_RUN ] ; do

    
    NUM_RUNNING=`ps -au | grep -i $EXE | wc -l`
    NUM_RUNNING=$[NUM_RUNNING-1]

    if [ $JOB_NUMBER -lt 10 ] ; then
	NUM=`echo "0"$JOB_NUMBER`
    else
	NUM=$JOB_NUMBER
    fi
    
    echo "# Jobs running : "$NUM_RUNNING 


    if [ $NUM_RUNNING -le $MAX_RUNNING_JOBS ] ; then
	echo "RATE = "${RATE}" kHz"
	TIME_MUSEC=`echo "$NUM_XRAYS*1000 / $RATE" | bc`
	echo "TIME IN MU SEC = "${TIME_MUSEC} 
	TEXTFILE=${OUTPUT_FOLDER}/run_${RATE}kHz_M${M}_L${L}_job${NUM}.txt
	echo ${TEXTFILE}
	INPUT=${INPUT_FOLDER}/genData${ENERGY}keV_${RATE}kHz_job${NUM}.bin
	echo ${INPUT}

	echo "$EXE $INPUT $M $L ${SIMPEAKS[$INDEX]} > $TEXTFILE 2>&1 &"
	$EXE $INPUT $M $L ${SIMPEAKS} > $TEXTFILE 2>&1 &
	
	JOB_NUMBER=$[JOB_NUMBER+1]
    fi
    sleep 4
done

