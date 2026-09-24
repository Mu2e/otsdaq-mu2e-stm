#!/bin/bash 

BREMS_RATE=2000
TIME_SIM=1000
MEAN_E=0.347
SIGMA=0.001
SEED=0
FIT_OPT=2000
NEXP=1

MAX_RUNNING_JOBS=20
NUM_RUNNING=0
JOB_NUMBER=5
MAX_JOBS_TO_RUN=10 #number of jobs we send


EXE=/work/cgarcia/STMDAQ-TestBeam/build/bin/FitterROOT.exe
#IMPORTANT CHANGE THIS
#OUTPUT_FOLDER=/data1/cgarcia/SignaltoBackground_LaBr/MinuitFit_${BREMS_RATE}kHz_${TIME_SIM}s_347keV_20keVres/10jobs
OUTPUT_FOLDER=/data1/cgarcia/SignaltoBackground/MinuitFit_${BREMS_RATE}kHz_${TIME_SIM}s_347keV_${SIGMA}MeVres/10jobs

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

	echo "$EXE $BREMS_RATE $TIME_SIM $MEAN_E $SIGMA $SEED $FIT_OPT $NEXP $JOB_NUMBER $OUTPUT_FOLDER 2>&1 &"
	
	$EXE $BREMS_RATE $TIME_SIM $MEAN_E $SIGMA $SEED $FIT_OPT $NEXP $JOB_NUMBER $OUTPUT_FOLDER 2>&1 &

	JOB_NUMBER=$[JOB_NUMBER+1]
    fi
    sleep 4
done
