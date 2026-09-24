#!/bin/bash 

ENERGY=662 #keV
NOISE_SIGMA=1 #mV
NUM_XRAYS=500

MAX_RUNNING_JOBS=20
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=20
RATE=10 #kHz
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
	LOGFILE=${OUTPUT_LOGFOLDER}/logfile${RATE}kHz_${NUM_XRAYS}_job${NUM}.log
	echo ${LOGFILE}
	OUTPUTFILE=${OUTPUT_BINFOLDER}/genData${ENERGY}keV_${RATE}kHz_job${NUM}.bin
	echo ${OUTPUTFILE}

	echo "$EXE $RATE $ENERGY $NOISE_SIGMA $TIME_MUSEC $SEED $OUTPUTFILE > $LOGFILE 2>&1 &"
	#$EXE $RATE $ENERGY $NOISE_SIGMA $TIME_MUSEC $SEED $OUTPUTFILE > $LOGFILE 2>&1 &
	
	JOB_NUMBER=$[JOB_NUMBER+1]
    fi
    sleep 1
done

