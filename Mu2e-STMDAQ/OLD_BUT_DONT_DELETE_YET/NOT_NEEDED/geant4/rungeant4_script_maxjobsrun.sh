#!/bin/bash

input="geant_config.txt"
i=0
MAX_RUNNING_JOBS=3
MAX_JOBS_TO_RUN=4
NUM_RUNNING=0
EXE=geant4_main.exe

MOMENTUM=1.809

#ANGLE=0
for ANGLE in `echo "0 5 10 15 20 25 30 35 40 45 50"`
do
    #XOFFSET cm
    for XOFFSET in `echo "0 -0.5 -1 -1.2 -1.5 -2 -2.2 -2.5 0.5 1 1.2 1.5 2 2.2 2.5 3 3.5 4"`
    do
	
	NUM_RUNNING=`ps -au | grep -i $EXE | wc -l`
	NUM_RUNNING=$[NUM_RUNNING-1]
	
	echo "# Jobs running : "$NUM_RUNNING
	
	echo "ANGLE = "${ANGLE}"deg, XOFFSET = "${XOFFSET}
	
	cat > $input <<EOF

#Primary
pdgid: 22
momMeV: $MOMENTUM
thetamin: 0.99
//phi always between 0 and 2pi
initpos_x_cm: $XOFFSET
initpos_y_cm: 0
initpos_z_cm: -11

bool_dodisk_pos: 1
init_rmax_cm: 0.5642

bool_docyl_pos: 0

bool_dofixed_pos: 0

#Geometry
bool_doHPGe: 1
bool_doPoly: 0
bool_doKClPot: 0
bool_doVDplane: 0
bool_doAlWindow: 1 

HPGe_rot_angley: $ANGLE
AlWindow_rmax_cm: 5
AlWindow_halflength_cm: 0.075
AlWindow_xcenter_cm: 0
AlWindow_ycenter_cm: 0
AlWindow_zcenter_cm: -0.475

VD_rmax_cm: 5
VD_halflength_cm: 0.05
VD_xcenter_cm: 0
VD_ycenter_cm: 0
VD_zcenter_cm: -0.7

#RootFile
rootname: 10to6_${MOMENTUM}MeV_0.99costheta_${ANGLE}yrot_${XOFFSET}.root

EOF


	TEXTFILE=rungeant4_${ANGLE}deg_xoffset${XOFFSET}.log
	
	while [ $NUM_RUNNING -gt $MAX_RUNNING_JOBS ] ; do
	    sleep 10
	    NUM_RUNNING=`ps -au | grep -i $EXE | wc -l`
	done
	
	if [ $NUM_RUNNING -le $MAX_RUNNING_JOBS ] ; then
	    "./"$EXE > $TEXTFILE 2>&1 &
	    echo $EXE " >  " $TEXTFILE " 2>&1 &" 
	    sleep 5
	    NUM_RUNNING=`ps -au | grep -i $EXE | wc -l`
	fi	
    done
done
