#!/bin/bash 

#XRAYS
#ENERGY=347
#FLASH
ENERGY=0 #keV
NOISE_SIGMA=0 #mV
#XRAYS
#TIME_MUSEC=5000000
#FLASH
#TIME_MUSEC=500000
TIME_MUSEC=50000

MAX_RUNNING_JOBS=3 #If =1 run just 2 jobs at the same time
NUM_RUNNING=0
JOB_NUMBER=0 #number in the output bin file
MAX_JOBS_TO_RUN=1000 #100
RATE=30 #kHz
SEED=0

EXE=/work/mu2e/cgarcia/STMDAQ-TestBeam/build/bin/HPGe_ehModelSim_setseed.exe

#OUTPUT_LOGFOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/FlashXrayOverlappedSim/${RATE}kHz_${ENERGY}keV_Xrays
OUTPUT_LOGFOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/FlashXrayOverlappedSim/${RATE}kHz_Flash

if [ ! -e ${OUTPUT_LOGFOLDER} ] ; then
 echo "Folder does not exist : creating ${OUTPUT_LOGFOLDER}"
 mkdir -p ${OUTPUT_LOGFOLDER}
fi
echo "OUTPUT LOG FOLDER = "${OUTPUT_LOGFOLDER}


#OUTPUT_BINFOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/FlashXrayOverlappedSim/${RATE}kHz_${ENERGY}keV_Xrays
OUTPUT_BINFOLDER=/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/FlashXrayOverlappedSim/${RATE}kHz_Flash

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

	LOGFILE=${OUTPUT_LOGFOLDER}/logfile${RATE}kHz_job${NUM}.log
	echo ${LOGFILE}
	OUTPUTFILE=${OUTPUT_BINFOLDER}/genData${ENERGY}keV_${RATE}kHz_job${NUM}.bin
	echo ${OUTPUTFILE}

	echo "$EXE $RATE $ENERGY $NOISE_SIGMA $TIME_MUSEC $SEED $OUTPUTFILE > $LOGFILE 2>&1 &"
	$EXE $RATE $ENERGY $NOISE_SIGMA $TIME_MUSEC $SEED $OUTPUTFILE > $LOGFILE 2>&1 &
	
	JOB_NUMBER=$[JOB_NUMBER+1]
    fi
    rm -rf "$LOGFILE"
    sleep 1
done

