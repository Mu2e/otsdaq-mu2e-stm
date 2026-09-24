#!/bin/bash

input="geant_config.txt"
i=0

ANGLE=25
#XOFFSET cm
#for XOFFSET in `echo "-2.2 -2.5 -3 -3.5 -4"`
#for XOFFSET in `echo "-2 -1.5 -1.2 -1 -0.5"`
#for XOFFSET in `echo "0.5 1 1.2 1.5 2"`
for XOFFSET in `echo "2.2 2.5 3 3.5 4"`
do
 echo "ANGLE = "${ANGLE}"deg, XOFFSET = "${XOFFSET}

 cat > $input <<EOF

#Primary
pdgid: 22
momMeV: 0.347
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
bool_doVDplane: 1

VD_rot_angley: $ANGLE
VD_rmax_cm: 4.5
VD_halflength_cm: 0.1
VD_xcenter_cm: 0
VD_ycenter_cm: 0
VD_zcenter_cm: -0.195

#RootFile
rootname: 10to6_347keV_0.99costheta_${ANGLE}yrot_${XOFFSET}.root

EOF


TEXTFILE=rungeant4_${ANGLE}deg_xoffset${XOFFSET}.log
 
./geant4_main.exe > $TEXTFILE 2>&1 &
sleep 5
done
