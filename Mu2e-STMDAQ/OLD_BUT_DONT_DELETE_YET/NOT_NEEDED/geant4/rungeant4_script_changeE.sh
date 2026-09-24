#!/bin/bash

input="geant_config.txt"
i=0
MAX_RUNNING_JOBS=3
MAX_JOBS_TO_RUN=4
NUM_RUNNING=0
EXE=geant4_main.exe

AlHalfThickness_cm=0.075

for MOMENTUM in `echo "0.05 0.06 0.07 0.08 0.09 0.1 0.2 0.3 0.347 0.380 0.4 0.45 0.5 0.51 0.52 0.53 0.54 0.55 0.58 0.6 0.65 0.7 0.8 0.9 1 1.5 2 2.5 3 4 5"`
do
	
    NUM_RUNNING=`ps -au | grep -i $EXE | wc -l`
    NUM_RUNNING=$[NUM_RUNNING-1]
	
    echo "# Jobs running : "$NUM_RUNNING
    
    echo "MOMENTUM = "${MOMENTUM}
	
    cat > $input <<EOF

#Primary
pdgid: 11
momMeV: $MOMENTUM
thetamin: 0.99
//phi always between 0 and 2pi
initpos_x_cm: 0
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
bool_doAlWindow: 0

HPGe_rot_angley: 0

AlWindow_rmax_cm: 5
AlWindow_halflength_cm: $AlHalfThickness_cm
AlWindow_xcenter_cm: 0
AlWindow_ycenter_cm: 0
AlWindow_zcenter_cm: -0.475


#RootFile
rootname: 10to6_${MOMENTUM}MeV_0.99costheta_v8_noAl_ele.root

EOF


    TEXTFILE=rungeant4_${MOMENTUM}MeV_0.99costheta_v8_noAl_ele.log   
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
