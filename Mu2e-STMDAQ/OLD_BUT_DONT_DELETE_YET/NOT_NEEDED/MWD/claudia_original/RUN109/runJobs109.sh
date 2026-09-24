#!/bin/bash

MAX_RUNNING_JOBS=19
EXE=MWDROOT.exe
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=20
EXE_FULL=/work/cgarcia/STMDAQ-TestBeam/build/bin/MWDROOT.exe

M=300
L=200

#raw 109
BIN_FILE=/data1/cgarcia/DATA/MWD_Analysis/RUN109/run00109.new.bin
LOG_FILE=/data1/cgarcia/DATA/MWD_Analysis/RUN109/M${M}L${L}/logrun109
#suppressed 109
#BIN_FILE=/data1/cgarcia/DATA/MWD_Analysis/RUN109/SuppresedFiles_PRE5us_POST10us_HZDR/Suppressed_run109.bin
#LOG_FILE=/data1/cgarcia/DATA/MWD_Analysis/RUN109/SuppresedFiles_PRE5us_POST10us_HZDR/M1000L500/logrun109

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
     EXEC_COMMAND="$EXE_FULL ${BIN_FILE}_${NUM} $M $L 0 > ${LOG_FILE}_${NUM} 2>&1 &"
     echo $EXEC_COMMAND
     eval $EXEC_COMMAND
     JOB_NUMBER=$[JOB_NUMBER+1]
  fi
  sleep 4
done
