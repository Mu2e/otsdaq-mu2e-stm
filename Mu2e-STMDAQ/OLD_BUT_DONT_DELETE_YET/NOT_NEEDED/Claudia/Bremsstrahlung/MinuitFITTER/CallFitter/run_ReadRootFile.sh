#!/bin/bash 

SEED=0
FIT_OPT=2000
FITRANGE_LOW=0.32
FITRANGE_MAX=0.37

MAX_RUNNING_JOBS=20
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=22


ROOTNUM=0

EXE=/work/mu2e/cgarcia/STMDAQ-TestBeam/build/bin/ReadRootFile_Fit.exe

for BREMS_RATE in `echo "1 50 200"`
do
    for SIGMA in `echo "0.001 0.002 0.003"`
    do
	for TIME_SIM in `echo "2000"`
	do
	    OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/SystematicsFittingRange/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
	    if [ ! -e ${OUTPUT_FOLDER} ] ; then
		echo "Folder does not exist : creating ${OUTPUT_FOLDER}"
		mkdir -p ${OUTPUT_FOLDER}
	    fi
	    echo "OUTPUT FOLDER = "${OUTPUT_FOLDER}

	    INPUTROOTFILE=$OUTPUT_FOLDER/BinnedLoglike_NOIntegral_${BREMS_RATE}.00kHz_TimeSim_${TIME_SIM}s_seed_0_${SIGMA}0MeV_1Runs_Job_0.root
	    
	    if [ $JOB_NUMBER -lt $MAX_JOBS_TO_RUN ] ; then
		
		NUM_RUNNING=`ps -au | grep -i $EXE | wc -l`
		NUM_RUNNING=$[NUM_RUNNING-1]
		     
		if [ $JOB_NUMBER -lt 10 ] ; then
		    NUM=`echo "0"$JOB_NUMBER`
		else
		    NUM=$JOB_NUMBER
		fi
		     
		echo "# Jobs running : "$NUM_RUNNING

		if [ $NUM_RUNNING -le $MAX_RUNNING_JOBS ] ; then
			 
		    echo "$EXE $INPUTROOTFILE $FIT_OPT $FITRANGE_LOW $FITRANGE_MAX $SEED $OUTPUT_FOLDER 2>&1 &"
		    
		    $EXE $INPUTROOTFILE $FIT_OPT $FITRANGE_LOW $FITRANGE_MAX $SEED $OUTPUT_FOLDER 2>&1 &
			 
		    JOB_NUMBER=$[JOB_NUMBER+1]
		fi
	    fi
	    sleep 4
	done
	sleep 4
    done
    sleep 4
done
