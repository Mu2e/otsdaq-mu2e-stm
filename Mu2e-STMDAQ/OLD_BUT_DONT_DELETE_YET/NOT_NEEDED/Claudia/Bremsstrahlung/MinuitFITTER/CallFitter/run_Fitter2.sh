#!/bin/bash 
VAR=20 #change this to the humber of REPEATED times we run one macro (NUMOFJOBS)

MEAN_E=0.347
SEED=0
FIT_OPT=3000
NEXP=50 #number of fits we do in one executable run
txtBackgroundParameters=Brems_ShapeNominal.txt

MAX_RUNNING_JOBS=20
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=20

ROOTNUM=0
NUMOFJOBS=20 #number of jobs we send (lower than 20) (from rootnum to numofjobs))

EXE=/work/mu2e/cgarcia/STMDAQ-TestBeam/build/bin/FitterROOT.exe

for BREMS_RATE in `echo "1"`
do
    for SIGMA in `echo "0.001"`
    do
	for TIME_SIM in `echo "10"`
	do
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/SystematicsFittingRange/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/Systematics1000JOBsSignalReco/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time_2000BINS
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/Systematics1000JOBsSignalReco/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time_${TIME_SIM}s_20000BINS
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/SystematicsCalibrationShape/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/LaBr3/MinuitFit_BinnedLogLike_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/BinnedChi2/Systematics1000JOBsSignalReco/MinuitFit_BinnedChi2_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time_2000BINS
	    #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/Unbinned/Systematics1000JOBsSignalReco/MinuitFit_Unbinned_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time
	     #OUTPUT_FOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/Systematics1000JOBsSignalReco/MinuitFit_Unbinned_NoInt_${BREMS_RATE}kHz_${SIGMA}MeV_time_10s
	    OUTPUT_FOLDER=.
	    
	    if [ ! -e ${OUTPUT_FOLDER} ] ; then
		echo "Folder does not exist : creating ${OUTPUT_FOLDER}"
		mkdir -p ${OUTPUT_FOLDER}
	    fi
	    echo "OUTPUT FOLDER = "${OUTPUT_FOLDER}
	    	    
	    if [ $JOB_NUMBER -lt $MAX_JOBS_TO_RUN ] ; then
		
		NUM_RUNNING=`ps -au | grep -i $EXE | wc -l`
		NUM_RUNNING=$[NUM_RUNNING-1]

		echo "NUM_RUNNING = "${NUM_RUNNING}

		if [ $JOB_NUMBER -lt 10 ] ; then
		    NUM=`echo "0"$JOB_NUMBER`
		else
		    NUM=$JOB_NUMBER
		fi
	    
		while [ $ROOTNUM -lt $NUMOFJOBS ]; do

		    if [ $NUM_RUNNING -le $MAX_RUNNING_JOBS ] ; then

			echo "# Jobs running : "$NUM_RUNNING
			echo "Jobs # : "$JOB_NUMBER
			
			echo "$EXE $BREMS_RATE $TIME_SIM $MEAN_E $SIGMA $SEED $FIT_OPT $NEXP $ROOTNUM $OUTPUT_FOLDER $txtBackgroundParameters 2>&1 &"
		    
			$EXE $BREMS_RATE $TIME_SIM $MEAN_E $SIGMA $SEED $FIT_OPT $NEXP $ROOTNUM $OUTPUT_FOLDER $txtBackgroundParameters 2>&1 &
		    
			JOB_NUMBER=$[JOB_NUMBER+1]
		    
		    fi
		    ROOTNUM=$[ROOTNUM+1]
		    sleep 4
		done
		if [ $NUM_RUNNING -le $VAR ] ; then
		    ROOTNUM=0
		fi
	    fi
	    sleep 4
	done
	sleep 4
    done
    sleep 4
done
