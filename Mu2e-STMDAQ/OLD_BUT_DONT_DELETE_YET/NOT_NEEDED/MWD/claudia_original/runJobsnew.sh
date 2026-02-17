#!/bin/bash

MAX_RUNNING_JOBS=19
EXE=energy_histo.exe
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=17
EXE_FULL=/work/cgarcia/STMDAQ-TestBeam/build/bin/energy_histo.exe
BIN_FILE=/work/data/run00110.bin
LOG_FILE=/work/cgarcia/run00110.log

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
     EXEC_COMMAND="$EXE_FULL ${BIN_FILE}_${NUM} > ${LOG_FILE}_${NUM} 2>&1 &"
     echo $EXEC_COMMAND
     eval $EXEC_COMMAND
     JOB_NUMBER=$[JOB_NUMBER+1]
  fi
  sleep 2
done
