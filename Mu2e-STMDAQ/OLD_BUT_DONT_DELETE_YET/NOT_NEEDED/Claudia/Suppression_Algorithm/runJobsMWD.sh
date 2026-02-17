#!/bin/bash

MAX_RUNNING_JOBS=19
EXE=test_new.exe
NUM_RUNNING=0
JOB_NUMBER=0
MAX_JOBS_TO_RUN=20
EXE_FULL=/work/cgarcia/STMDAQ-TestBeam/build/bin/test_new.exe
BIN_FILE=/work/cgarcia/DATA/Claudia/Suppression_Algorithm/RUN109Sup_Gradient/run00109.new_sup.bin
LOG_FILE=/work/cgarcia/DATA/Claudia/Suppression_Algorithm/M1000L500/logrun109

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
  sleep 4
done
