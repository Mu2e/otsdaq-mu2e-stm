#!/bin/bash 

BREMS_RATE=50
#TIME_SIM=100000
MEAN_E=0.347
SIGMA=0.003
SEED=0
FIT_OPT=2000
NEXP=1
txtBackgroundParameters=Brems_ShapeNominal.txt

MAX_RUNNING_JOBS=1
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=1 #number of jobs we send


#EXE=/work/cgarcia/STMDAQ-TestBeam/build/bin/FitterROOT.exe
EXE=/work/mu2e/cgarcia/STMDAQ-TestBeam/build/bin/FitterROOT.exe 
#OUTPUT_FOLDER=/data1/cgarcia/SignaltoBackground/MinuitFit_1kHz_100000s_347keV_1keVres/1000JOBS
#OUTPUT_FOLDER=/data1/cgarcia/SignaltoBackground/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
#OUTPUT_FOLDER=/data1/cgarcia/SignaltoBackground_LaBr/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
OUTPUT_FOLDER=/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/CallFitter

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

	for TIME_SIM in `echo "2000"`
	do
	    echo "$EXE $BREMS_RATE $TIME_SIM $MEAN_E $SIGMA $SEED $FIT_OPT $NEXP $JOB_NUMBER $OUTPUT_FOLDER $txtBackgroundParameters 2>&1 &"
	
	    $EXE $BREMS_RATE $TIME_SIM $MEAN_E $SIGMA $SEED $FIT_OPT $NEXP $JOB_NUMBER $OUTPUT_FOLDER $txtBackgroundParameters #2>&1 &

	done

	JOB_NUMBER=$[JOB_NUMBER+1]
    fi
    sleep 4
done
