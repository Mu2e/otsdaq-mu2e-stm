#!/bin/bash 


M=400
L=200
PEAKSSIM=0

EXE=/home/stm_mu2e/STMDAQ-TestBeam/build/bin/MWDROOT.exe

if [ ! -e ${OUTPUT_LOGFOLDER} ] ; then
 echo "Folder does not exist : creating ${OUTPUT_LOGFOLDER}"
 mkdir -p ${OUTPUT_LOGFOLDER}
fi
echo "OUTPUT LOG FOLDER = "${OUTPUT_LOGFOLDER}


#for FILENUM in `echo "0 1 2 3 4 5 6 7 8"`
#do 
#    INFILE=/data1/STM_VST_DATA/HPGeAndSigGen_11-09-23/stm-daq_2023-11-09_18-18-46_HPGe_${FILENUM}.bin
#    LOGFILE=/data1/STM_VST_DATA/MWD_K_Data/M400L200_notrigcut/logfile_stm-daq_2023-11-09_18-18-46_HPGe_${FILENUM}.log
#    echo ${LOGFILE}
    
#    echo "$EXE $INFILE $M $L $PEAKSSIM > $LOGFILE 2>&1 &"
#    $EXE $INFILE $M $L $PEAKSSIM > $LOGFILE 2>&1 &
#    sleep 1
#done



#for FILENUM in `echo "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119"`

#for FILENUM in `echo "9 10 98 99"`
#do 
#    INFILE=/data1/STM_VST_DATA/HPGeAndSigGen_11-09-23/stm-daq_2023-11-09_19-09-36_HPGe_${FILENUM}.bin
#    LOGFILE=/data1/STM_VST_DATA/MWD_K_Data/M400L200_notrigcut/logfile_stm-daq_2023-11-09_19-09-36_HPGe_${FILENUM}.log
#    echo ${LOGFILE}
    
#    echo "$EXE $INFILE $M $L $PEAKSSIM > $LOGFILE 2>&1 &"
#    $EXE $INFILE $M $L $PEAKSSIM > $LOGFILE 2>&1 &
#    sleep 1
#done



for FILENUM in `echo "52"`
do 
    INFILE=/data1/STM_VST_DATA/HPGeAndSigGen_11-09-23/stm-daq_2023-11-09_14-00-17_HPGe_${FILENUM}.bin
    LOGFILE=/data1/STM_VST_DATA/MWD_Cosmics_Data/M400L200_notrigcut/logfile_stm-daq_2023-11-09_14-00-17_HPGe_${FILENUM}.log
    echo ${LOGFILE}
    
    echo "$EXE $INFILE $M $L $PEAKSSIM > $LOGFILE 2>&1 &"
    $EXE $INFILE $M $L $PEAKSSIM > $LOGFILE 2>&1 &
    sleep 1
done




