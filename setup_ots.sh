#!/bin/sh
echo # This script is intended to be sourced.

# Create two temporary files for compare and contrast of bash environment (used for qiuck environment recall, e.g. for artdaq)

if [ ${SETUP_OTS_SOURCED:-0} -eq 0 ]; then
	env_before_setupOTS=$(mktemp)
	env_after_setupOTS=$(mktemp)
	declare -x > $env_before_setupOTS
fi

sh -c "[ `ps $$ | grep bash | wc -l` -gt 0 ] || { echo 'Please switch to the bash shell before running ots.'; exit; }" || exit

export HOSTNAME="$(hostname -f)"

export SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
otsdir=$SCRIPT_DIR

unset OTSDAQ_DIR
unset OTSDAQ_LIB
unset OTSDAQ_UTILITIES_LIB
unset ARTDAQ_DAQINTERFACE_DIR

export DISABLE_DOXYGEN=1

echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t SCRIPT_DIR=$SCRIPT_DIR"

export SPACK_DISABLE_LOCAL_CONFIG=true
source $SCRIPT_DIR/spack/share/spack/setup-env.sh

spack load --first gcc@13.1.0 > /dev/null 2>&1
# spack compiler find #this causes problem when clang is installed or setup happens multiple times in same terminal

export OTS_SPACK_ENV=`ls -l -rta ${SCRIPT_DIR}/spack/var/spack/environments/ | grep tdaq- | rev | cut -d ' ' -f1 | rev | head -1` #if a mu2e environment, then will return tdaq target version
if [ "x$OTS_SPACK_ENV" == "x" ]; then # likely an ots quick install dev environment
	export OTS_ENV="ots-develop" #`ls -l -rta | grep tdaq-v | rev | cut -d '/' -f1 | rev | head -1`
	export OTS_SPACK_ENV=`ls -l -rta ${SCRIPT_DIR}/spack/var/spack/environments/ | grep ots- | rev | cut -d ' ' -f1 | rev | head -1` #if an ots environment, then will return ots target version
else #likely a mu2e dev environment
	export OTS_ENV="tdaq-develop" #"ots-develop" #`ls -l -rta | grep tdaq-v | rev | cut -d '/' -f1 | rev | head -1`
fi

if [ "x$SPACK_ENV" != "x" ]; then
	OTS_SPACK_ENV=$SPACK_ENV
fi
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t OTS_ENV=$OTS_ENV"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t OTS_SPACK_ENV=$OTS_SPACK_ENV"

spack env activate $OTS_ENV # >/dev/null 2>&1
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t SPACK_ROOT=$SPACK_ROOT"

#handle using ots-develop local install area
if [ -d $SCRIPT_DIR/local/install ]; then
    export PATH=$SCRIPT_DIR/local/install/bin:$PATH
    export LD_LIBRARY_PATH=$SCRIPT_DIR/local/install/lib:$SCRIPT_DIR/local/install/lib64:$LD_LIBRARY_PATH
    export CET_PLUGIN_PATH=$SCRIPT_DIR/local/install/lib:$CET_PLUGIN_PATH
    export ROOT_INCLUDE_PATH=$SCRIPT_DIR/local/install/include:$ROOT_INCLUDE_PATH

	export OTSDAQ_DIR=${OTSDAQ_DIR:-$SCRIPT_DIR/local/install} #only set if not set by spack, e.g. needed by UpdateOTS.sh
	export OTSDAQ_LIB=${OTSDAQ_LIB:-$SCRIPT_DIR/local/install/lib} #only set if not set by spack, e.g. needed by otsConfiguration_Wizard_CMake.xml, otsConfiguration_MacroMaker_CMake.xml
	export OTSDAQ_UTILITIES_LIB=${OTSDAQ_UTILITIES_LIB:-$SCRIPT_DIR/local/install/lib} #only set if not set by spack, needed by otsConfiguration_Wizard_CMake.xml, otsConfiguration_MacroMaker_CMake.xml
	export  OTSDAQ_UTILITIES_DIR=${OTSDAQ_DIR:-$SCRIPT_DIR/local/install}

	# in ots-develop mode, set WebPath because OTSDAQ_UTILITIES_DIR is not setup
	if [ -d $SCRIPT_DIR/srcs/otsdaq-utilities/WebGUI ]; then
		export OTSDAQ_WEB_PATH=$SCRIPT_DIR/srcs/otsdaq-utilities/WebGUI
	else
		export OTSDAQ_WEB_PATH=$(readlink -f "$OTSDAQ_UTILITIES_LIB/../WebGUI")
	fi
	export OTS_FILE_PARSE_PATTERN="/srcs/" #will be used to parse filename (i.e. for TRACE)

	export FHICL_FILE_PATH=$SCRIPT_DIR/local/install/fcl:$FHICL_FILE_PATH
	export MU2E_SEARCH_PATH=$SCRIPT_DIR/local/install/share:$MU2E_SEARCH_PATH

	export  ARTDAQ_DAQINTERFACE_DIR=${ARTDAQ_DAQINTERFACE_DIR:-$SCRIPT_DIR/local/install} #only set if not set by spack, e.g. needed by UpdateOTS.sh

fi
export  ARTDAQ_DAQINTERFACE_VERSION="SPACK"




export OTS_SOURCE=$(cd -P ${SCRIPT_DIR}/srcs && pwd) #follow potential link
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t OTS_SOURCE=$OTS_SOURCE"

cd $otsdir

if ! [ -d srcs ] && ! [ -z "$PS1" ]; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t --> You must run this script from an OTSDAQ installation directory (where the srcs/ folder is)!"
	return 1
fi

if [ -e "srcs/otsdaq-utilities/tools/UpdateOTS.sh" ] ; then
	alias UpdateOTS.sh=${PWD}/srcs/otsdaq-utilities/tools/UpdateOTS.sh
fi


#------------------------------------------------------------------------------
# assume that ots is always launched from $MRB_TOP and that
#------------------------------------------------------------------------------


subsystem=$1
if [ ".$subsystem" == ".for_running" ]; then
	subsystem=`cat $SCRIPT_DIR/.ots_artdaq_setup_type.txt`;
elif [ ".$1" == "." ] && [ -e ${SCRIPT_DIR}/.ots_setup_type.txt ]; then
	subsystem=`cat $SCRIPT_DIR/.ots_setup_type.txt`;
fi
export OTS_SETUP_TYPE=$subsystem

export LOGNAME=$USER              # ksu might have messed up LOGNAME
#------------------------------------------------------------------------------
# 'subsystem' has to be either defined explicitly, or defined previously
# it has the meaning of a 'subsystem'
# if not, bail out
#------------------------------------------------------------------------------
if [ ".$subsystem" == "." ]; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t --> You are user $USER on $HOSTNAME in directory `pwd`"
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t ================================================="
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t usage:  source setup_ots.sh <subsystem>"
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t ... where  subsystem = (sync,stm,stmdbtest,calo,trigger,02,dcs,HWDev,tracker,shift,dqmcalo)"
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t ================================================="
	return 1;
fi


repository="notGoodRepository"
unset ARTDAQ_PARTITION

export OTS_USER_STUB=${USER}_${subsystem}_`echo $PWD | awk -F / '{print $NF}' | sed 's/ots//'`
use_mongodb=0 #default to filesystem db
use_scratch=1 #default to trying to use scratch
use_tunnel=1  #default to using tunnel if environment variable set
keep_logs=0   #default to erasing artdaq logs during development


export  OTS_OWNER=Mu2e
export CONSOLE_SUPERVISOR_IP=127.0.0.1
export USER_WEB_PATH=${otsdir}/srcs/otsdaq-mu2e/UserWebGUI

if [[ $subsystem == "stm_readout" || $subsystem == "stm" ]]; then
    export USER_WEB_PATH=${otsdir}/srcs/otsdaq-mu2e-stm/UserWebGUI
fi

export OTS_LOG_LOGIN_LOGOUT=0 #do not make logbook entries for user login/logout
export OTS_LOG_INTERMEDIATE_STATES=0 #do not make logbook entries for intermediate states
export OTS_LOG_TRANSITION_STARTS=0 #do not log transitions launching (just completing)

export CHECK_GIT_REPO_STATUS=1 #warn at ots run-time when there are uncommitted changes in srcs/

#########################################################################
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t subsystem=$subsystem"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
#########################################################################
if [ $subsystem == "sync" ]; then
	export          OTS_MAIN_PORT=3015
	export OTS_WIZ_MODE_MAIN_PORT=3015
	export       ARTDAQ_PARTITION=2
	repository="otsdaq_mu2e"
elif [ $subsystem == "stm" ]; then
	export  OTS_OWNER="Mu2e-STM"
	export      OTS_DISABLE_TRACE_DEFAULT=1
	export          OTS_MAIN_PORT=30351
	export OTS_WIZ_MODE_MAIN_PORT=30351
	export       ARTDAQ_PARTITION=5
	export LD_LIBRARY_PATH=$SCRIPT_DIR/local/install/lib:/home/mu2estm/STMDAQ-TestBeam/build/lib:$LD_LIBRARY_PATH
	# export         OTS_DEBUG_MODE=2 #to use shared ~mu2ecalo/srcs
	repository="otsdaq_mu2e_stm"
	use_mongodb=1

	source ${SCRIPT_DIR}/stm_setup.sh
elif [ $subsystem == "stm_readout" ]; then
	export  OTS_OWNER="Mu2e-STM"
	export      OTS_DISABLE_TRACE_DEFAULT=1
	export          OTS_MAIN_PORT=30371
	export OTS_WIZ_MODE_MAIN_PORT=30371
	export       ARTDAQ_PARTITION=5
	export LD_LIBRARY_PATH=$SCRIPT_DIR/local/install/lib:/home/mu2estm/STMDAQ-TestBeam/build/lib:$LD_LIBRARY_PATH
	# export         OTS_DEBUG_MODE=2 #to use shared ~mu2ecalo/srcs
	repository="otsdaq_mu2e_stm"
	use_mongodb=1
	echo $subsystem > ${SCRIPT_DIR}/.ots_artdaq_setup_type.txt #for artdaq .for_running setup

	source ${SCRIPT_DIR}/stm_setup.sh

elif [ $subsystem == "stmdbtest" ]; then
	export          OTS_MAIN_PORT=3040
	export OTS_WIZ_MODE_MAIN_PORT=3040
	export       ARTDAQ_PARTITION=11
	repository="otsdaq_mu2e_stm"
elif [ $subsystem == "calo" ]; then

	export  OTS_OWNER="Mu2e-Calo"
	export          OTS_MAIN_PORT=3025
	export OTS_WIZ_MODE_MAIN_PORT=3025
	export       ARTDAQ_PARTITION=4
	repository="otsdaq_mu2e_calorimeter"
	use_mongodb=1
	export      OTS_DISABLE_TRACE_DEFAULT=1

    #env vars (THIS_HOST, CONFIG_PATH)
    source ${SCRIPT_DIR}/calo_setup.sh
    #export MU2E_CALORIMETER_CONFIG_PATH="otsdaq-mu2e-calorimeter/boardConfig/"

elif [ $subsystem == "tracker" ]; then
	export  OTS_OWNER="Mu2e-Tracker"
	export          OTS_MAIN_PORT=3065
	export OTS_WIZ_MODE_MAIN_PORT=3065
	export       ARTDAQ_PARTITION=8
	export        FEWRITE_RUNFILE=1
	# export         OTS_DEBUG_MODE=2 #to use shared ~mu2ecalo/srcs
	repository="otsdaq_mu2e_tracker"
	use_mongodb=1
elif [ $subsystem == "dqmtrk" ]; then
	export          OTS_MAIN_PORT=3070
	export OTS_WIZ_MODE_MAIN_PORT=3070
	export       ARTDAQ_PARTITION=14
	##export      FEWRITE_RUNFILE=1
	repository="otsdaq_mu2e_tracker"
elif [ $subsystem == "mergetrk" ]; then
	export          OTS_MAIN_PORT=3090
	export OTS_WIZ_MODE_MAIN_PORT=3090
	export       ARTDAQ_PARTITION=15
	export        FEWRITE_RUNFILE=1
	repository="otsdaq_mu2e_tracker"
elif [ $subsystem == "shift" ]; then
	export          OTS_MAIN_PORT=3075
	export OTS_WIZ_MODE_MAIN_PORT=3075
	export       ARTDAQ_PARTITION=12
	repository="otsdaq_mu2e"
	use_mongodb=1
	export      OTS_DISABLE_TRACE_DEFAULT=1
	source ${otsdir}/ecl_setup_ots.sh
	export OTS_LOG_ROLLOVER=100000000 #rollover primary xdaq/mf/trace log files at 100MB boundaries
elif [ $subsystem == "shift1" ]; then
	export          OTS_MAIN_PORT=3095 #4015
	export OTS_WIZ_MODE_MAIN_PORT=3095 #4015
	export       ARTDAQ_PARTITION=12
	repository="otsdaq_mu2e"
	use_mongodb=1
	export      OTS_DISABLE_TRACE_DEFAULT=1
	export  OTS_OWNER="Mu2e-Hardware"
elif [ $subsystem == "cfo" ]; then
	export          OTS_MAIN_PORT=3095 #4015
	export OTS_WIZ_MODE_MAIN_PORT=3095 #4015
	export       ARTDAQ_PARTITION=12
	repository="otsdaq_mu2e"
	use_mongodb=1
	export      OTS_DISABLE_TRACE_DEFAULT=1
	export  OTS_OWNER="Mu2e-Hardware"
elif [ $subsystem == "shift2" ]; then
	export          OTS_MAIN_PORT=3080
	export OTS_WIZ_MODE_MAIN_PORT=3080
	export       ARTDAQ_PARTITION=12
	repository="otsdaq_mu2e"
elif [ $subsystem == "crv" ]; then
	export  OTS_OWNER="Mu2e-CRV"
	export          OTS_MAIN_PORT=3085
	export OTS_WIZ_MODE_MAIN_PORT=3085
	export       ARTDAQ_PARTITION=9
	repository="otsdaq_mu2e_crv"
	use_mongodb=1
	export      OTS_DISABLE_TRACE_DEFAULT=1
elif [ $subsystem == "trigger" ]; then
    export OTS_DISABLE_TRACE_DEFAULT=1
    export          OTS_MAIN_PORT=3045
    export OTS_WIZ_MODE_MAIN_PORT=3045
    export       ARTDAQ_PARTITION=6
    repository="otsdaq_mu2e_trigger"
    use_mongodb=1
    use_scratch=1 #on mu2edaq13, do not need scratch
    use_tunnel=1 #on mu2edaq13, do not need tunnela
    keep_logs=1 #keep pmt and DTC logs from artdaq
    echo $subsystem > ${SCRIPT_DIR}/.ots_artdaq_setup_type.txt #for artdaq .for_running setup
    export  OTS_OWNER="Mu2e-Trigger"
    source ${SCRIPT_DIR}/stm_setup.sh
elif [ $subsystem == "aggregator" ]; then
	export          OTS_MAIN_PORT=3040
	export OTS_WIZ_MODE_MAIN_PORT=3040
	export       ARTDAQ_PARTITION=13
	repository="otsdaq_mu2e_dqm"
elif [ $subsystem == "tem" ]; then
	export  OTS_OWNER="Mu2e-ExtMon"
	export          OTS_MAIN_PORT=4045
	export OTS_WIZ_MODE_MAIN_PORT=4045
	repository="otsdaq_mu2e_extmon"
elif [ $subsystem == "dqmcalo" ]; then
	export          OTS_MAIN_PORT=3095
	export OTS_WIZ_MODE_MAIN_PORT=3095
	export ARTDAQ_PARTITION=3
	repository="otsdaq_mu2e_trigger"
elif [ $subsystem == "02" ]; then
	export         OTS_MAIN_PORT=2015
	export OTS_WIZ_MODE_MAIN_PORT=2015
	repository="otsdaq_mu2e"
elif [ $subsystem == "dcs" ]; then
    export          OTS_MAIN_PORT=5019
    export OTS_WIZ_MODE_MAIN_PORT=5019
    export       ARTDAQ_PARTITION=12
    repository="otsdaq-mu2e"
    use_mongodb=1
    export      OTS_DISABLE_TRACE_DEFAULT=1
    export  OTS_OWNER="Mu2e-DCS"
elif [ $subsystem == "dqmstm" ]; then
    export OTS_DISABLE_TRACE_DEFAULT=1
    export          OTS_MAIN_PORT=5039
    export OTS_WIZ_MODE_MAIN_PORT=5039
    export       ARTDAQ_PARTITION=6
    repository="otsdaq_mu2e"
    use_mongodb=1
    source ${SCRIPT_DIR}/stm_setup.sh
elif [ $subsystem == "dqm" ]; then
    export OTS_DISABLE_TRACE_DEFAULT=1
    export          OTS_MAIN_PORT=5029
    export OTS_WIZ_MODE_MAIN_PORT=5029
    export       ARTDAQ_PARTITION=17
    repository="otsdaq_mu2e"
    use_mongodb=1

elif [ $subsystem == "HWDev" ]; then
	export          OTS_MAIN_PORT=3055
	export OTS_WIZ_MODE_MAIN_PORT=3055
	export       ARTDAQ_PARTITION=7
	# export         OTS_DEBUG_MODE=2 #to use shared ~mu2ecalo/srcs
	repository="otsdaq_mu2e"
    # use_mongodb=1
elif [ $subsystem == "HWDev2" ]; then
	export          OTS_MAIN_PORT=3055
	export OTS_WIZ_MODE_MAIN_PORT=3055
	export       ARTDAQ_PARTITION=7
	use_mongodb=1
	repository="otsdaq_mu2e"
	export USER_WEB_PATH=${otsdir}/srcs/${repository}/UserWebGUI
else
	echo -e "Invalid parameter!"
	return 1;
fi

echo $subsystem > ${SCRIPT_DIR}/.ots_setup_type.txt

echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t ======================================================"

# echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t otsPathAppend=${otsPathAppend}"
# echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t userdataappend=${userdataappend}"

export LANGUAGE="en_US.UTF-8"
export LC_CTYPE=en_US.UTF-8
export LC_ALL=en_US.UTF-8

#to enable gdb
ulimit -c unlimited
#to view core dumps for gdb (in reverse time order):
# coredumpctl
#to open a particular core dump use PID
# coredumpctl --output=core.0 dump <PID>
# gdb xdaq.exe core.0 # to view dump



echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t To use trace, do \"tshow | grep . | tdelta -d 1 -ct 1\" with appropriate grep re to"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t filter traces. Piping into the tdelta command to add deltas and convert"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t the timestamp."

# Setup environment when building with MRB (As there's no setupARTDAQOTS file)

export OTSDAQ_DEMO_LIB=${MRB_BUILDDIR}/${repository}/lib
#export OTSDAQ_LIB=${MRB_BUILDDIR}/otsdaq/lib
#export OTSDAQ_UTILITIES_LIB=${MRB_BUILDDIR}/otsdaq_utilities/lib
#Done with Setup environment when building with MRB (As there's no setupARTDAQOTS file)

# MRB should set this itself
#export CETPKG_INSTALL=/home/mu2edaq/sync_demo/ots/products

#make the number of build threads dependent on the number of cores on the machine:
export CETPKG_J=$((`cat /proc/cpuinfo|grep processor|tail -1|awk '{print $3}'` + 1))


#------------------------------------------------------------------------------
# all temporary files reside in /scratch/mu2e/otsdaq_$OTS_USER_STUB
# $OTS_USER_STUB includes username, subdetector, and the local directory
# scratch configuration, scratch is 'per-working-area'
#------------------------------------------------------------------------------
if [ $use_scratch == 1 ]; then
	export OTS_SCRATCH=/scratch/mu2e/$OTS_USER_STUB
else
	export OTS_SCRATCH=${SCRIPT_DIR}/Data_$subsystem
fi
export ARTDAQ_OUTPUT_DIR=${OTS_SCRATCH}/OutputData

echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Checking SCRATCH directories at ... {${OTS_SCRATCH}}"
if [ ! -d $OTS_SCRATCH/Logs                  ] ; then mkdir -p $OTS_SCRATCH/Logs                  ; fi
if [ ! -d $OTS_SCRATCH/ARTDAQConfigurations  ] ; then mkdir -p $OTS_SCRATCH/ARTDAQConfigurations  ; fi
if [ ! -d $OTS_SCRATCH/TriggerConfigurations ] ; then mkdir -p $OTS_SCRATCH/TriggerConfigurations ; fi
if [ ! -d $OTS_SCRATCH/OutputData            ] ; then mkdir -p $OTS_SCRATCH/OutputData            ; fi
if [ ! -d $OTS_SCRATCH/OutputData/OtsHistos  ] ; then mkdir -p $OTS_SCRATCH/OutputData/OtsHistos  ; fi


export OTSDAQ_DATA=$OTS_SCRATCH/OutputData

#------------------------------------------------------------------------------
# local configuration in srcs/otsdaq_mu2e_config :
# local configuration is already located in the working area, so, for now, it is 'per-subsystem'
# USED_DATA, in fact, points to the user config data!
#------------------------------------------------------------------------------
configDir=$otsdir/srcs/otsdaq_mu2e_config

export USER_DATA=$otsdir/Data_$subsystem
export MU2E_OWNER=$subsystem
export ARTDAQ_DATABASE_URI=filesystemdb://$otsdir/databases_$subsystem/filesystemdb/test_db


export CERT_DATA_PATH=/home/mu2edaq/artdaq-utilities-node-server/certs/authorized_users
 offlineFhiclDir=${OFFLINE_DIR}/config
 offlineFhiclDir=${offlineFhiclDir}:${OFFLINE_DIR}/config/Offline
triggerEpilogDir=${USER_DATA}/TriggerConfigurations

#dataFilesDir=/mu2e/DataFiles
dataFilesDir=${OTSDAQ_DATA}
export  FHICL_FILE_PATH=$FHICL_FILE_PATH:$USER_DATA:$offlineFhiclDir:$triggerEpilogDir:$dataFilesDir:/mu2e/DataFiles
export MU2E_SEARCH_PATH=$MU2E_SEARCH_PATH:$FHICL_FILE_PATH:${OFFLINE_DIR}/config:/mu2e/DataFiles:/cvmfs/mu2e.opensciencegrid.org/DataFiles:${OFFLINE_DIR}/source

if [ $use_scratch == 1 ]; then
#make sure links are pointing to the right place
linkPath=$(readlink $USER_DATA/Logs)
if [ "$linkPath" != "${OTS_SCRATCH}/Logs" ] ; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Fixing found link path as ... found {${linkPath}} expecting {${OTS_SCRATCH}/Logs}"
	rm $USER_DATA/Logs
	ln -s $OTS_SCRATCH/Logs $USER_DATA/Logs
fi
linkPath=$(readlink $USER_DATA/ARTDAQConfigurations)
if [ "$linkPath" != "${OTS_SCRATCH}/ARTDAQConfigurations" ] ; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Fixing found link path as ... found {${linkPath}} expecting {${OTS_SCRATCH}/ARTDAQConfigurations}"
	rm $USER_DATA/ARTDAQConfigurations
	ln -s $OTS_SCRATCH/ARTDAQConfigurations $USER_DATA/ARTDAQConfigurations
fi
linkPath=$(readlink $USER_DATA/TriggerConfigurations)
if [ "$linkPath" != "${OTS_SCRATCH}/TriggerConfigurations" ] ; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Fixing found link path as ... found {${linkPath}} expecting {${OTS_SCRATCH}/TriggerConfigurations}"
	rm $USER_DATA/TriggerConfigurations
	ln -s $OTS_SCRATCH/TriggerConfigurations $USER_DATA/TriggerConfigurations
fi
linkPath=$(readlink $USER_DATA/OutputData)
if [ "$linkPath" != "${OTS_SCRATCH}/OutputData" ] ; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Fixing found link path as ... found {${linkPath}} expecting {${OTS_SCRATCH}/OutputData}"
	rm $USER_DATA/OutputData
	ln -s $OTS_SCRATCH/OutputData $USER_DATA/OutputData
fi
fi #end use_scratch

#if mongodb, do extra setup
if [ $use_mongodb == 1 ]; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     Setting up mongodb..."
	unset ARTDAQ_DATABASE_CA_CERT #in case new mongodb was setup
	unset ARTDAQ_DATABASE_CLIENT_CERT #in case new mongodb was setup
	source ${otsdir}/mongodb_setup.sh
	source ${otsdir}/db_setup_ots.sh
fi

unset otsUserSshTunnels
if [[ $use_tunnel == 1 && "x$ARTDAQ_DATABASE_SSH_HOST" != "x" ]] ; then
	export ARTDAQ_DATABASE_SSH_TUNNEL_COUNT=0
	export OTSDAQ_REFRESH_SSH=1 #auto refresh ssh/kinit to help keep tunnels open
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     Defining otsUserSshTunnels()"
	function otsUserSshTunnels()
	{
		#!/bin/bash

		ARTDAQ_DATABASE_SSH_TUNNEL_COUNT=$(( $ARTDAQ_DATABASE_SSH_TUNNEL_COUNT + 1 ))
		# echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  $ARTDAQ_DATABASE_SSH_TUNNEL_COUNT \t "
		if [ $ARTDAQ_DATABASE_SSH_TUNNEL_COUNT -gt 102 ]; then
			ARTDAQ_DATABASE_SSH_TUNNEL_COUNT=2 #wrap around
			kinit -R &>/dev/null #refresh kerberos
		fi


		# Check if the SSH tunnel is already running on port ARTDAQ_DATABASE_SSH_PORT
		LOCAL_IP=$(hostname -I | awk '{print $1}')
		TARGET_IP=$(getent hosts $ARTDAQ_DATABASE_SSH_HOST | awk '{print $1}')

		if [[ "$ARTDAQ_DATABASE_SSH_HOST" == "$HOSTNAME" || \
				"$ARTDAQ_DATABASE_SSH_HOST" == "$(hostname -f)" || \
				"$ARTDAQ_DATABASE_SSH_HOST" == "$(hostname -s)" ]]; then

			if [ $ARTDAQ_DATABASE_SSH_TUNNEL_COUNT == 1 ]; then #only print the first time
				echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t SSH Tunnel destination on this host, so no need to make tunnel."
				echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t "
			fi

		elif lsof -i :$ARTDAQ_DATABASE_SSH_PORT > /dev/null; then

			if [ $ARTDAQ_DATABASE_SSH_TUNNEL_COUNT == 1 ]; then #only print the first time
				echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t SSH Tunnel Port already in use or tunnel already exists"
				echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t "
			fi

		else

			if klist -s; then
				echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t Kerberos ticket is valid"
			else
				echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t Kerberos ticket is expired or missing"
			fi

			echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t "
			echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t SSH Tunnel in background to IERC mongodb:"
			echo "ssh -f -K -N $ARTDAQ_DATABASE_SSH_JUMP -L 27017:localhost:$ARTDAQ_DATABASE_SSH_PORT $ARTDAQ_DATABASE_SSH_HOST"

			timeout 10s ssh -f -K -N $ARTDAQ_DATABASE_SSH_JUMP -L 27017:localhost:$ARTDAQ_DATABASE_SSH_PORT -o ExitOnForwardFailure=yes -o HostKeyAlgorithms=- $ARTDAQ_DATABASE_SSH_HOST || sleep 20

			echo -e "$(date +%d%h%y.%T) setup_ots.sh-otsUserSshTunnels():${LINENO} |  \t "

		fi

	} # end otsUserSshTunnels()
	export -f otsUserSshTunnels
fi



echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Now your user data path is USER_DATA \t\t = ${USER_DATA}"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Now your database path is ARTDAQ_DATABASE_URI \t = ${ARTDAQ_DATABASE_URI}"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Now your output data path is OTSDAQ_DATA \t = ${OTSDAQ_DATA}"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Now your user web path is OTSDAQ_WEB_PATH \t = ${OTSDAQ_WEB_PATH}"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Now your user web path is USER_WEB_PATH \t = ${USER_WEB_PATH}"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
if [ -z $ARTDAQ_PARTITION ]; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} | \t WARNING: You have not set an ARTDAQ Partition!"
else
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} | \t Your ARTDAQ Partition is ARTDAQ_PARTITION \t = ${ARTDAQ_PARTITION}"
fi

if [ -z $OTS_MAIN_PORT ]; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} | \t WARNING: You have not set an ots MAIN PORT!"
else
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} | \t Your primary ots port is OTS_MAIN_PORT \t = ${OTS_MAIN_PORT}"
fi

echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "

alias rawEventDump="art -c ${otsdir}/srcs/otsdaq/artdaq-ots/ArtModules/fcl/rawEventDump.fcl"
alias kx='ots -k'

echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Now use 'ots --wiz' to configure otsdaq"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t  	Then use 'ots' to start otsdaq"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t  	Or use 'ots --help' for more options"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     use 'kx' to kill otsdaq processes"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "

# setup compile aliases (for more spack debug info, use spack -d install -j$CETPKG_J 2>&1)
#============================
lockfile="/tmp/mu2e_compile_${OTS_SOURCE//\//_}.lock"

get_lock() {
    # Attempt to create the lock file atomically using ln
    if ! ln -s "$$" "$lockfile" 2>/dev/null; then
	# Check if the existing lock file contains a valid PID
	if [ -L "$lockfile" ]; then
            pid=$(readlink "$lockfile")
            if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
		echo "Script is already running with PID $pid."
		return 1
            fi
	fi
	# If stale, remove it and try again
	rm -f "$lockfile"
	if ! ln -s "$$" "$lockfile" 2>/dev/null; then
            echo "Failed to acquire lock."
            return 1
	fi
    fi
    # Ensure lock file is removed on exit
    return 0
}

rel_lock() { rm -f $lockfile; }

Base=$SCRIPT_DIR
escaped_srcs=$(printf '%s\n' "$OTS_SOURCE/" | sed 's/[\/&]/\\&/g')
#  alias  mb='date; start_time=$(date +%s); spack find | grep gcc; spack mpd build -G Ninja -j$CETPKG_J 2>&1 | sed s/$escaped_srcs//g | sed s/__spack_path_placeholder__//g | sed s/\\\[padded-to-255-chars\\\]//g | sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g; pushd $Base/build; ninja install | sed s/$escaped_srcs//g; popd; end_time=$(date +%s); date; delta_time=$((end_time - start_time)); fractional_minutes=$(echo "scale=1; $delta_time / 60" | bc); echo "Full time: $delta_time seconds or $fractional_minutes minutes"'
unalias mb 2>/dev/null
mb() {
	date;
	get_lock && {
	trap 'rm -f $lockfile; echo removing lockfile' RETURN
	trap 'rm -f $lockfile; echo SIGINT; return 0' SIGINT # may need to be 'exit 0'???
	start_time=$(date +%s);
	spack find | grep gcc;

	# Temporary file to store processed build output
	temp_file=$(mktemp)

	# Run spack mpd build with tee to process output via sed, display, and save to a temp file
	stdbuf -oL spack mpd build -G Ninja -j$CETPKG_J 2>&1 | \
		tee >(sed s/$escaped_srcs//g | \
			   sed s/__spack_path_placeholder__//g | \
			   sed s/\\\[padded-to-255-chars\\\]//g | \
			   sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g | \
			   tee "$temp_file")

	# Read temp file into variable for further checks
	build_output=$(cat "$temp_file")
	rm "$temp_file"  # Clean up the temporary file

	# Check if the build failed based on specific error message
	if echo "$build_output" | grep -q "ninja: build stopped: subcommand failed."; then
		echo "Build failed! Skipping ninja install."
	else
		# Only run install if build succeeded
		pushd $Base/build;
		ninja install | sed s/$escaped_srcs//g | \
			   grep -v "Up-to-date:" | \
			   grep -v "\/\.\/README\.md" | \
			   grep -v "\/\.\/LICENSE" | \
			   grep -v "Set\ non-toolchain\ portion\ of" | \
			   sed s/__spack_path_placeholder__//g | \
			   sed s/\\\[padded-to-255-chars\\\]//g | \
			   sed s/\ to\ \".*\"//g | \
			   sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g;
		popd;
	fi

	end_time=$(date +%s);
	date;
	delta_time=$((end_time - start_time));
	fractional_minutes=$(echo "scale=1; $delta_time / 60" | bc);
	echo "Full time: $delta_time seconds or $fractional_minutes minutes";
	rel_lock;
	trap - RETURN
	trap - SIGINT
	} || { echo status=$?;
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} | \t ERROR! Another user appears to be compiling. Only one user is allowed to compile at a time in each source area.."; }
}

#  alias  ml='date; start_time=$(date +%s); spack find | grep gcc; spack mpd build -G Ninja -j$CETPKG_J 2>&1 | sed s/$escaped_srcs//g | sed s/__spack_path_placeholder__//g | sed s/\\\[padded-to-255-chars\\\]//g | sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g | tee m.txt; pushd $Base/build; ninja install | sed s/$escaped_srcs//g; popd; end_time=$(date +%s); date; delta_time=$((end_time - start_time)); fractional_minutes=$(echo "scale=1; $delta_time / 60" | bc); echo "Full time: $delta_time seconds or $fractional_minutes minutes"; less m.txt'
unalias ml 2>/dev/null
ml() {
	date;
	get_lock && {
	trap 'rm -f $lockfile; echo removing lockfile' RETURN
	trap 'rm -f $lockfile; echo SIGINT; return 0' SIGINT # may need to be 'exit 0'???
	start_time=$(date +%s);
	spack find | grep gcc;

	# Temporary file to store processed build output
	temp_file=".ml_log.txt"

	# Run spack mpd build with tee to process output via sed, display, and save to a temp file
	stdbuf -oL spack mpd build -G Ninja -j$CETPKG_J 2>&1 | \
		tee >(sed s/$escaped_srcs//g | \
			   sed s/__spack_path_placeholder__//g | \
			   sed s/\\\[padded-to-255-chars\\\]//g | \
			   sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g | \
			   tee "$temp_file")

	# Read temp file into variable for further checks
	build_output=$(cat "$temp_file")
	# rm "$temp_file"  # Clean up the temporary file

	# Check if the build failed based on specific error message
	if echo "$build_output" | grep -q "ninja: build stopped: subcommand failed."; then
		echo "Build failed! Skipping ninja install."
	else
		# Only run install if build succeeded
		pushd $Base/build;
		ninja install | sed s/$escaped_srcs//g | \
			   grep -v "Up-to-date:" | \
			   grep -v "\/\.\/README\.md" | \
			   grep -v "\/\.\/LICENSE" | \
			   grep -v "Set\ non-toolchain\ portion\ of" | \
			   sed s/__spack_path_placeholder__//g | \
			   sed s/\\\[padded-to-255-chars\\\]//g | \
			   sed s/\ to\ \".*\"//g | \
			   sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g;
		popd;
	fi

	end_time=$(date +%s);
	date;
	delta_time=$((end_time - start_time));
	fractional_minutes=$(echo "scale=1; $delta_time / 60" | bc);
	echo "Full time: $delta_time seconds or $fractional_minutes minutes"
	less $temp_file
	rel_lock;
	trap - RETURN
	trap - SIGINT
	} || { echo status=$?;
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} | \t ERROR! Another user appears to be compiling. Only one user is allowed to compile at a time in each source area.."; }
}
# alias  mz='date; start_time=$(date +%s); spack find | grep gcc; spack mpd build -G Ninja --clean -j$CETPKG_J 2>&1 | sed s/$escaped_srcs//g | sed s/__spack_path_placeholder__//g; pushd $Base/build; ninja install | sed s/$escaped_srcs//g; popd; end_time=$(date +%s); date; delta_time=$((end_time - start_time)); fractional_minutes=$(echo "scale=1; $delta_time / 60" | bc); echo "Full time: $delta_time seconds or $fractional_minutes minutes"'
unalias mz 2>/dev/null
mz() {
    date;
    get_lock && {
	trap 'rm -f $lockfile; echo removing lockfile' RETURN
	trap 'rm -f $lockfile; echo SIGINT; return 0' SIGINT # may need to be 'exit 0'???
    start_time=$(date +%s);
    rm -rf $SCRIPT_DIR/local/install
    spack find | grep gcc;

	# Temporary file to store processed build output
	temp_file=$(mktemp)

	# Run spack mpd build with tee to process output via sed, display, and save to a temp file
	stdbuf -oL spack mpd build -G Ninja --clean -j$CETPKG_J 2>&1 | \
		tee >(sed s/$escaped_srcs//g | \
			   sed s/__spack_path_placeholder__//g | \
			   sed s/\\\[padded-to-255-chars\\\]//g | \
			   sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g | \
			   tee "$temp_file")

	# Read temp file into variable for further checks
	build_output=$(cat "$temp_file")
	rm "$temp_file"  # Clean up the temporary file

	# Check if the build failed based on specific error message
	if echo "$build_output" | grep -q "ninja: build stopped: subcommand failed."; then
		echo "Build failed! Skipping ninja install."
	else
		# Only run install if build succeeded
		pushd $Base/build;
		ninja install | sed s/$escaped_srcs//g | \
			   grep -v "Up-to-date:" | \
			   grep -v "\/\.\/README\.md" | \
			   grep -v "\/\.\/LICENSE" | \
			   grep -v "Set\ non-toolchain\ portion\ of" | \
			   sed s/__spack_path_placeholder__//g | \
			   sed s/\\\[padded-to-255-chars\\\]//g | \
			   sed s/\ to\ \".*\"//g | \
			   sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g;
		popd;
	fi

	end_time=$(date +%s);
	date;
	delta_time=$((end_time - start_time));
	fractional_minutes=$(echo "scale=1; $delta_time / 60" | bc);
	echo "Full time: $delta_time seconds or $fractional_minutes minutes"
	rel_lock;
	trap - RETURN
	trap - SIGINT
	} || { echo status=$?;
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} | \t ERROR! Another user appears to be compiling. Only one user is allowed to compile at a time in each source area.."; }
}
alias mz_package='spack concretize --force && spack install && mz'

unalias mz_uc &> /dev/null
mz_uc() {
	echo "Must be in a new terminal!"
	spack env deactivate

	if spack mpd select tdaq-develop 2>/dev/null; then
		echo "tdaq-develop activated."
	else
		echo "Failed to activate tdaq-develop!"
		return 1
	fi

	spack mpd refresh --force && \
	spack env activate tdaq-develop && \
	mz
}
export -f mz_uc
# alias mz_uc='(echo "must be in new terminal!" || spack env deactivate) || (spack mpd select tdaq-develop && spack mpd refresh && spack env activate tdaq-develop && mz)'


shopt -u progcomp #let environment variables be tabbed

alias ots_which='cat ${OTS_SPACK_ENV}/spack.lock | sed s/\{/\\n/g | grep arch | grep' #to see which version to add, then spack add <repo>@<version>; spack develop <repo>@<version>
#srcs/spack.yaml will have added repos
# cd srcs/otsdaq;git describe --tags #e.g. to get active version of otsdaq if in srcs (ots_which will give nothing)

#ots_copy for otsdaq-utilities "installs" during development without compiling
ots_copy() {
	echo "ots_copy $1"
	cp srcs/otsdaq-utilities/$1 $(spack location -i otsdaq-utilities)/$1
	ll -rta $(spack location -i otsdaq-utilities)/$1 | sed s/__spack_path_placeholder__//g | sed s/\\\[padded-to-255-chars\\\]//g | sed s/\\\/tdaq-v......../\\\/tdaq-v_\ \ \ /g
}
# Export the function
export -f ots_copy




#=============================
#Trace setup and helpful commented lines:

	export TRACE_LIMIT_MS="0,50,50" #unlimited trace messages
	#tinfo #show trace info
	export TRACE_FILE=/tmp/trace_buffer_${OTS_USER_STUB} #by default it is /proc/trace/buffer
	export TRACE_PRINT_FD=2 #reroute everything to std:err (overriding > dev/null hiding)
	export TRACE_TIME_FMT="%d%b%y %H:%M:%S.%%03d" #set time %T output format
	export TRACE_PRINT="|%L:%N: %#f#/tdaq-v#:%u | %m" #to mimic ots line number output
# --- at this point, done setting up environment! so compare and contrast --------


if [[ -v env_after_setupOTS ]]; then #only if variable exists
	declare -x > $env_after_setupOTS
fi

ENV_SUBSYSTEM=$subsystem
if [ ${SETUP_OTS_SOURCED:-0} -eq 0 ]; then
	grep -v -x -F -f $env_before_setupOTS $env_after_setupOTS > ${otsdir}/.rte_${ENV_SUBSYSTEM}_setup_ots.sh
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t           Saved environment cache to ${otsdir}/.rte_${ENV_SUBSYSTEM}_setup_ots.sh"
	# Clean up the temporary files when done
	rm -f "$env_before_setupOTS" "$env_after_setupOTS"
fi
export SETUP_OTS_SOURCED=1 #to avoid doing it again


#=============================
# Extra user management and preferences

echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t  "
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t      setup_ots.sh creates some compiling aliases for you:"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     ---------------"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t            mb                             ### for incremental build"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t            mz                             ### for clean build"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t            ml                             ### for incremental build into less for searchable errors, in order from the top"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t            mz_package                     ### for clean build after modifying spack packages"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t            mz_uc                          ### for clean build after updating adding/removing repos in srcs/ area"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     ---------------"
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t  "


	if [[ "x$OTS_DEBUG_MODE" != "x2" ]]; then #allow blocking of compiling of debugging when in shared srcs/ (handled by otsdaq_utilities/CMakeLists.txt)
		export OTS_DEBUG_MODE=0
	fi
	alias less='less -S' #no word wrap

	if [ $keep_logs == 0 ]; then
		rm -rf $OTS_SCRATCH/Logs/EvtDataGenDTC* #artdaq logs keep growing too much during development
		rm -rf $OTS_SCRATCH/Logs/pmt #artdaq logs keep growing too much during development
	fi

#=============================
#Trace setup and helpful commented lines:

spack load --first trace >/dev/null 2>&1 || source trace_functions.sh
export TRACE_MSGMAX=0 #Activating TRACE

if [ "$OTS_DISABLE_TRACE_DEFAULT" == "1" ]; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Accepting existing TRACE settings on ${HOSTNAME}."
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t"
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Setup on ${HOSTNAME} complete."
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t"
elif type trace_cntl >/dev/null 2>&1; then

	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Setting up TRACE defaults on ${HOSTNAME}..."
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t"
	#echo Turning on all memory tracing via: tonMg 0-63

	#export TRACE_NAME=OTSDAQ_TRACE # this overrides default naming and is not appropriate for this environment
	# export TRACE_LIMIT_MS="0,50,50" #unlimited trace messages
	# export TRACE_LIMIT_MS="1,1,<ms off time>" #to block trace messages (count, on time, off time)
	# #tinfo #show trace info and to enact env variables
	# export TRACE_FILE=/tmp/trace_buffer_${OTS_USER_STUB} #by default it is /proc/trace/buffer
	# export TRACE_PRINT_FD=2 #reroute everything to std:err (overriding > dev/null hiding)
	# export TRACE_TIME_FMT="%d%b%y %H:%M:%S.%%03d" #set time %T output format
	# export TRACE_PRINT="|%L:%N: %#f#/tdaq-v#:%u | %m" #to mimic ots line number output
	trace_cntl cntl 1 # to force TLOG_ENTEX() and all TLOGS to always insert __func__ for the Fast/mem TRACE messages

	# to save tlvls and restore (to/from file)
	#tlvls -H > tlvl.rrivera  # to save tlvls to file
	#cat tlvl.rrivera | tlvlsRestore # to restore tlvls from file

    # -*-*- LEVEL STUFF
    # Message from Ryan:
    # the trace settings should not be reset by default for most setup types (1st parameter), there is an environment variable that blocks resetting the trace lvls.
    # [ See OTS_DISABLE_TRACE_DEFAULT below ]

    tallcmd() {
        : "t-all-cmd or tall-cmd"
        test $# -eq 0 -o "$1" = -h && {
            echo "examples: tallcmd toffMg"
            echo "          tallcmd tinfo -q"
            return
        }
	test $1 = toffMg -a $# -eq 1 && set -- toffMg 6-63
	tfiles=`/bin/ls /tmp/trace_* 2>/dev/null` || { echo "Currently no /tmp/trace_* files"; }
        for ff in $tfiles;do echo "Executing $* for TRACE_FILE=$ff"; TRACE_FILE=$ff; $* || { echo "Error status";break; };done
    }

    # Default setup moved from setup_trace to here so it can be disabled
    #trace_cntl lvlmskg 0xfff 0x1ff 0  # <memory> <slow> <trigger>, tonSg 0-8 (debug is 8 := TLVL_{FATAL,ALERT,CRIT,ERROR,WARNING,NOTICE,INFO,LOG,DEBUG})
    trace_cntl mode 3   # ton*

    #8 is Debug level, 9 is 'Trace' level (less severe than Debug)

	#8 is Debug level, 9 is 'Trace' level (less severe than Debug)

    # toffMg 0-63
    tonMg 0-8   # enable trace to memory
    tonSg 0-7   # enable trace to slow path (i.e. UDP)
    toffSg 8-63 # apparently not turned off by default?
    toffMg 9-63

    # Disable some very verbose trace outputs, BUT DON'T EVEN PRETEND TO TURN OFF (slowpath) WARNINGs,ERRORs, etc
    TRACE_NAMLVLSET="\
CONF:LdStrD_C    0x1ff 0x3f 0
FileDB:RDWRT_C   0x1ff 0x3f 0
CONF:CrtCfD_C    0x1ff 0x3f 0
COFS:DpFle_C     0x1ff 0x3f 0
PRVDR:FileDBIX_C 0x1ff 0x3f 0
JSNU:DocUtils_C  0x1ff 0x3f 0
JSNU:Document_C  0x1ff 0x3f 0
CONF:OpLdStr_C   0x1ff 0x3f 0
PRVDR:FileDB_C   0x1ff 0x3f 0
CONF:OpBase_C    0x1ff 0x3f 0
JSONDocument.cpp 0x1ff 0x3f 0
json_writer.cpp  0x1ff 0x3f 0
json_reader.cpp  0x1ff 0x3f 0
provider_filedb_readwrite.cpp 0x1ff 0x3f 0
provider_filedb_index.cpp 0x1ff 0x3f 0
provider_filedb.cpp 0x1ff 0x3f 0
dispatch_filedb.cpp 0x1ff 0x3f 0
detail_manageconfigs.cpp 0x1ff 0x3f 0
detail_managedocument.cpp 0x1ff 0x3f 0
filesystem_functions.cpp 0x1ff 0x3f 0
options_operation_base.cpp 0x1ff 0x3f 0
JSONDocument_utils.cpp  0x1ff 0x3f 0
options_operation_managedocument.cpp  0x1ff 0x3f 0
" trace_cntl namlvlset


	# tonS -N "*DTC*" 8-9 #to enable by name ("" required)
	# tonS -N "*dtc*" 8-9 #to enable by name ("" required)
	# tonS -N "*CFO*" 8-9 #to enable by name ("" required)
	# tonS -N "*cfo*" 8-9 #to enable by name ("" required)


	# # toffS -N "DTC.cpp" 4-63 #to enable by name ("" required)
	# tonS -N OTSDAQ_TRACE 0-63 #to enable by name
	# toffS -N OTSDAQ_TRACE 21 #to enable by name
	# toffS -N OTSDAQ_TRACE 19 #to enable by name
	# toffS -N OTSDAQ_TRACE 9 #to enable by name
	# toffS -N "DTC.cpp" 15 #to enable by name ("" required)

	# tonS -N "FE*" 0-63 #to enable by name
	# tonS -N "Mu2e*" 0-63 #to enable by name
	# tonS -N MacroMaker 0-63 #to enable by name
	# tonS -N "mu2e*" 0-63 #to enable by name
	# # toffS -N DTC* 4-63 #to enable by name
	# # toffS -N OTSDAQ_TRACE* 4-63 #to enable by name
	# # toffS -N Mu2eEventReceiverBase* 4-63 #to enable by name

	# toffSg 10-63;

	# # tonS -N "DTC_Packets" 14 #to see Data Payload ROC header progression
	# tonM -N "DTC_Packets" 14 #to see Data Payload ROC header progression


	# # tonS -N "DTC_SubEvent" 14 #to see Data Payload ROC header progression
	# tonM -N "DTC_SubEvent" 14 #to see Data Payload ROC header progression

	# # tonS -N CFO* 0-63 #to enable by name
	export OTS_DISABLE_TRACE_DEFAULT=1 #disable default TRACE settings from ots script

fi
#end Trace setup
#=============================



	if [[ "x$GET_OTS_DKMS_STATUS" == "x" ]]; then
		echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t    export GET_OTS_DKMS_STATUS=1 #to retrieve kernel driver version through dkms status"
	else
		echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t    dkms status"
		dkms status
			# mu2e_pcie_utils/2.08.05, 3.10.0-1160.102.1.el7.x86_64, x86_64: built
			# mu2e_pcie_utils/2.09.00, 3.10.0-1160.102.1.el7.x86_64, x86_64: installed
	fi

	UGROUP_ID=`id | cut -d '(' -f1 | cut -d '=' -f2`; #for example 55443 for mu2ehwdev
	kcacheType=`echo $KRB5CCNAME | cut -d ':' -f1`
	kcachePath=`echo $KRB5CCNAME | cut -d ':' -f2`

	if [ "$kcacheType" == "FILE" ]; then
		kcacheUser=`klist 2>/dev/null|grep "Default principal"|cut -d: -f2|sed 's/ //g' | cut -d '@' -f1`
		if [ "$kcacheUser" == "" ]; then
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t    No kerberos user found, please kinit."
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t"
			return 0;
		fi
		echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t    Saving local kcache for ${kcacheUser}..."

		mkdir $SCRIPT_DIR/tmp &>/dev/null

		#do not overwite existing keberos cache
		if [ ! -e "$SCRIPT_DIR/tmp/krb5cc_${UGROUP_ID}_${kcacheUser}" ]; then
			cp $kcachePath $SCRIPT_DIR/tmp/krb5cc_${UGROUP_ID}_${kcacheUser}
		fi
		export KRB5CCNAME=$SCRIPT_DIR/tmp/krb5cc_${UGROUP_ID}_${kcacheUser}
	fi

	kinitCache=`klist 2>&1  | grep cache`

	# echo "kinitCache $kinitCache"
	# if the klist cache is based only on group account of mu2e* --> UGROUP_ID is the suffix, and people inadvertently share kerberos certificates
	kinitCacheCut=`echo $kinitCache | cut -d ':' -f3`
	if [[ "x$kinitCacheCut" == "x${UGROUP_ID}" ]]; then
		kdestroy >/dev/null 2>&1
		randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
		export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
		echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
	fi
	#try 4th position (after AL9?)
	kinitCacheCut=`echo $kinitCache | cut -d ':' -f4`
	if [[ "x$kinitCacheCut" == "x${UGROUP_ID}" ]]; then
		kdestroy >/dev/null 2>&1
		randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
		export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
		echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
	fi


	if [[ "x$kinitCache" == "xTicket cache: FILE:/tmp/krb5cc_${UGROUP_ID}" ]]; then
		kdestroy >/dev/null 2>&1
		randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
		export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
		echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
	fi
	if [[ "x$kinitCache" == "xklist: No credentials cache found (filename: /tmp/krb5cc_${UGROUP_ID})" ]]; then
		kdestroy >/dev/null 2>&1
		randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
		export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
		echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
	fi


	if [[ "x$GET_OTS_GIT_USER" == "x" ]]; then
		echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t    export GET_OTS_GIT_USER=1 #to setup git user info"
		echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
	else
		EMAIL_ADDR=`klist 2>/dev/null|grep "Default principal"|cut -d: -f2|sed 's/ //g'`
		# git config --global user.email $EMAIL_ADDR #DO NOT DO THIS -- this changes for all users of shared username
		echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t      Found kerberos email as ${EMAIL_ADDR}, retrieving author name for GIT info."
		if [[ "x$EMAIL_ADDR" == "x" ]]; then
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t  Attempt to setup git author failed - no kerberos principal found; to setup git author please kinit and then re-setup."
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
		else

			AUTHOR_NAME=`timeout 5s ${PWD}/otsdaq-mu2e-config/tele_lookup.sh ${EMAIL_ADDR}`

			#if unknown, maybe because remote ssh connection failed, try other gateway
			if [[ "x$AUTHOR_NAME" == "x" || "x$AUTHOR_NAME" == "xUNKNOWN" ]]; then
				AUTHOR_NAME=`timeout 5s ${PWD}/srcs/otsdaq-mu2e-config/tele_lookup.sh ${EMAIL_ADDR}`
			fi
			#if unknown, maybe because remote ssh connection failed, try setting up telephone locally
			if [[ "x$AUTHOR_NAME" == "x" || "x$AUTHOR_NAME" == "xUNKNOWN" ]]; then
				AUTHOR_NAME=`timeout 15s ssh -Y mu2eshift@mu2egateway01.fnal.gov bash tele_lookup.sh ${EMAIL_ADDR} || echo "UNKNOWN"`
			fi
			#if unknown, maybe because remote ssh connection failed, try setting up telephone locally not in srcs/
			if [[ "x$AUTHOR_NAME" == "x" || "x$AUTHOR_NAME" == "xUNKNOWN" ]]; then
				AUTHOR_NAME=`timeout 15s ssh -Y mu2eshift@mu2egateway02.fnal.gov bash tele_lookup.sh ${EMAIL_ADDR} || echo "UNKNOWN"`
				echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t      Found name as ${AUTHOR_NAME}"
			else
				echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t      Found name as ${AUTHOR_NAME}"
			fi
			#if all else fails, just take kerberos principal email as author name
			if [[ "x$AUTHOR_NAME" == "x" || "x$AUTHOR_NAME" == "xUNKNOWN" ]]; then
				AUTHOR_NAME=${EMAIL_ADDR}
			fi

			# save 'cache' git info to .gitconfig
			#git config --global user.email "${EMAIL_ADDR}" #NOTE -- git config changes for all users of shared username (so other users should use the setup_ots.sh approach)
			#git config --global user.name "${AUTHOR_NAME}" #NOTE -- git config changes for all users of shared username (so other users should use the setup_ots.sh approach)
			export GIT_AUTHOR_EMAIL=${EMAIL_ADDR-} # NULL is OK
			export GIT_AUTHOR_NAME=${AUTHOR_NAME}
			export GIT_COMMITTER_NAME="$GIT_AUTHOR_NAME on $USER@$HOSTNAME"
			export GIT_COMMITTER_EMAIL=$GIT_AUTHOR_EMAIL
			export GIT_SSH_COMMAND="ssh -i ~/.ssh/id_${EMAIL_ADDR:+${EMAIL_ADDR%@*}_}rsa"

			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     GIT_AUTHOR_EMAIL=${GIT_AUTHOR_EMAIL}"
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     GIT_AUTHOR_NAME=${GIT_AUTHOR_NAME}"
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     GIT_COMMITTER_NAME=${GIT_COMMITTER_NAME}"
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t     GIT_COMMITTER_EMAIL=${GIT_COMMITTER_EMAIL}"
			echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t "
		fi
	fi

# export DTCLIB_DEBUG_WRITE_FILE_PATH=${PWD}/slow # to enable DTC/CFO register write monitoring

echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Setup on ${HOSTNAME} complete."
echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t"

if [ $CHECK_GIT_REPO_STATUS == 1 ]; then
	echo -e "$(date +%d%h%y.%T) setup_ots.sh:${LINENO} |  \t Will report a warning if any uncommitted code found..."
	UpdateOTS.sh --warn 2>&1 >/dev/null
	echo #to get back to terminal
fi
return 0


#to fix github origin to allow read access on fetch/pull without rsa:
# git remote -v #to see origin URLs
# git remote set-url origin https://github.com/Mu2e/$repo.git
# git remote set-url --push origin git@github.com:Mu2e/$repo.git
# git remote set-url origin https://github.com/art-daq/otsdaq-demo
# git remote set-url --push origin git@github.com:art-daq/otsdaq-demo.git

#clang-format example (recursive):
# clang-format -i `find . -type f -name *.cc -o -name *.c -o -name *.C -o -name *.cpp -o -name *.cxx -o -name *.h -o -name *.hh -o -name *.hxx -o -name *.icc`
#clang-foramt see errors/warnings without implementing in place:
# clang-format -n -Werror `find . -type f -name *.cc -o -name *.c -o -name *.C -o -name *.cpp -o -name *.cxx -o -name *.h -o -name *.hh -o -name *.hxx -o -name *.icc`

#clang-format example format definition operations:
# diff srcs/otsdaq/.clang-format srcs/otsdaq-components/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-utilities/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-mu2e/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-calorimeter/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-crv/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-extmon/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-stm/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-tracker/
# diff srcs/otsdaq/.clang-format srcs/otsdaq-epics/

# cp srcs/otsdaq/.clang-format srcs/otsdaq-components/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-utilities/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-mu2e/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-calorimeter/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-crv/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-extmon/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-stm/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-mu2e-tracker/
# cp srcs/otsdaq/.clang-format srcs/otsdaq-epics/

#clang-format example in-source toggle:
# // clang-format off
#     void    unformatted_code  ;
# // clang-format on
#     void    use // for manual line breaks,  //
# 	 New line, //

#to check white-space violations:
# ${OTS_SOURCE}/otsdaq-utilities/tools/check_whitespace.sh #note that it only checks the last commit!
