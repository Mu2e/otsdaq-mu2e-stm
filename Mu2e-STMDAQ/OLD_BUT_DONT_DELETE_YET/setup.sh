# Location of users STM git-repository code
#
RH_VERSION=`cat /etc/system-release | cut -d '.' -f1 | awk '{print($3)}'`
if [ "$RH_VERSION" = "9" ] ; then
  GCC_ALLOWED_VERSION="11.4.1"
  PYT_ALLOWED_VERSION="3.9.18"
elif [ "$RH_VERSION" = "7" ] ; then
  GCC_ALLOWED_VERSION="8.3.1"
  PYT_ALLOWED_VERSION="3.6.12"
else
  echo "ERROR in setup.sh - your RedHat Linux version is not 7 or 9, but = $RH_VERSION"
fi

GCC_VERSION=`g++ --version | head -1 | awk '{print $3}'`
if [ "$GCC_VERSION" != "$GCC_ALLOWED_VERSION" ] ; then
     echo "ERROR in setup.sh - you g+++ version is not $GCC_ALLOWED_VERSION, it is instead = $GCC_VERSION"
fi
# Check python version is correct and throw error if not
PYTHON_VERSION=`python --version | head -1 | awk '{print $2}'`
if [ "$PYTHON_VERSION" != "$PYT_ALLOWED_VERSION" ] ; then
     echo "ERROR in setup.sh - your python version is not $PYT_ALLOWED_VERSION, it is instead = $PYTHON_VERSION"
fi

# Initialise varibales
export STM_IPBUS_IP=xxx # IPBUS IP Address
export STM_10G_IP=xxx # The 10G interface IP address
export STM_HPGE_FW_IP=xxx # HPGe (CH0) UDP IP address (receive)
export STM_LABR_FW_IP=xxx # LaBr (CH1) UDP IP address (receive)
export STM_HPGE_FW_PORT=51872 # HPGe (CH0) UDP IP port
export STM_LABR_FW_PORT=51874 # LaBr (CH1) UDP IP port
export STM_HPGE_SW_IP="127.0.0.2" # HPGe (CH0) UDP IP address (send)
export STM_LABR_SW_IP="127.0.0.3" # LaBr (CH1) UDP IP address (send)
export STM_DQM_IP="127.0.0.4" # DQM UDP IP address (send)
export STM_HPGE_SW_PORT=10010 # HPGe (CH0) UDP IP port (send)
export STM_LABR_SW_PORT=10012 # LaBr (CH1) UDP IP port (send)
export STM_LABR_SW_PORT=10004 # DQM UDP IP port (send)
export STMDAQ=xxx # Root working directory

# If the host machine is Fermilab UK Server
if [ "$HOSTNAME" = "mu2e-stm-01.fnal.gov" ]
then
    # Set the IP address for the ipbus interface
    export STM_IPBUS_IP="192.168.42.210"
    # Set the IP address for the 10Gb ethernet interface
    export STM_10G_IP="192.168.34.10"
    # Set the UDP IP address to receive HPGE data (CH0)
    export STM_HPGE_FW_IP="192.168.34.12"
    # Set the UDP IP address to receive LaBr data (CH1)
    export STM_LABR_FW_IP="192.168.36.12"
    # Setup the Xilinx license
    export XILINXD_LICENSE_FILE=1709@daqlic.hep.man.ac.uk
    # Set the user working directory
    if [ "$USER" = "stm_mu2e" ]
    then
        export STMDAQ=/home/stm_mu2e
	if [ "$PWD" != "/home/stm_mu2e/STMDAQ-TestBeam" ]
	then
	    export KUSER="$(klist -l | grep @FNAL.GOV | cut -d "@" -f 1)"
	    export STMDAQ=/home/stm_mu2e/$KUSER
	fi
    fi

# If the host machine is Fermilab UK Server
elif [ "$HOSTNAME" = "mu2edaq11.fnal.gov" ]
then
    # Set the IP address for the ipbus interface
    export STM_IPBUS_IP="192.168.42.210"
    # Set the IP address for the 10Gb ethernet interface
    export STM_10G_IP="192.168.34.10"
    # Set the UDP IP address to receive HPGE data (CH0)
    export STM_HPGE_FW_IP="192.168.34.12"
    # Set the UDP IP address to receive LaBr data (CH1)
    export STM_LABR_FW_IP="192.168.36.12"
    # Setup the Xilinx license
    export XILINXD_LICENSE_FILE=1709@daqlic.hep.man.ac.uk
    # Set the user working directory
    if [ "$USER" = "mu2estm" ]
    then
        export STMDAQ=/home/mu2estm
	if [ "$PWD" != "/home/mu2estm/STMDAQ-TestBeam" ]
	then
	    export KUSER="$(klist -l | grep @FNAL.GOV | cut -d "@" -f 1)"
	    export STMDAQ=/home/mu2estm/$KUSER
	fi
        export PACKAGES=/home/mu2estm/packages # may have to setup python/ROOT etc in /work/mu2estm/packages if /home is too slow.
    fi

else
    echo "ERROR in setup.sh - $HOSTNAME not recgonised!"
    echo "Setting UDP IP addresses to 127.0.0.3 (HPGe) & 127.0.0.4 (LaBr)"
    export STM_HPGE_FW_IP="127.0.0.3"
    export STM_LABR_FW_IP="127.0.0.4"
fi

if [ "$STMDAQ" = "xxx" ] ; then
      echo "ERROR in setup.sh - STMDAQ not set - edit setup.sh and set STMDAQ depending on username and host"
fi
if [ "$STMDAQ" != "xxx" ] ; then
export STMDAQ_ROOT=$STMDAQ/STMDAQ-TestBeam

#
# Below shouldn't need changing ------------------------------
#
# Location of the external packages: ROOT, BOOST, CACTUS
export MU2EDAQ_ROOT=/work
export PATH=$PATH:$STMDAQ_ROOT/build/bin

#
#export CACTUS_ROOT=$MU2EDAQ_ROOT/cactus
export CACTUS_ROOT=/opt/cactus
export BOOST_ROOT=$MU2EDAQ_ROOT/boost
#
export PATH=$PATH:$CACTUS_ROOT/bin
export LD_LIBRARY_PATH=$CACTUS_ROOT/lib:$BOOST_ROOT/lib:$STMDAQ_ROOT/build/lib

# STM config xml
export XML_PATH=$STMDAQ_ROOT/config
export STM_XML=$STMDAQ_ROOT/config/stmdaq.xml

# Hardware connections
#export STM_FPGA="KCU105"
export STM_FPGA="ZCU102-FMC120"
#export STM_ADC="FMC144"
export STM_ADC="FMC120" 

# Python config
export PATH=$PATH:$MU2EDAQ_ROOT/python_packages/bin
export PYTHONPATH=$STMDAQ_ROOT/utils/python:$MU2EDAQ_ROOT/python_packages/lib64/python3.6/site-packages/:$MU2EDAQ_ROOT/python_packages/lib/python3.6/site-packages/:$MU2EDAQ_ROOT/cactus/lib/python3.6/site-packages/

# $LD_LIBRARY_PATH:deps/ipbus-software/extern/boost/RPMBUILD/SOURCES/lib/
# export MIDAS_SERVER_HOST=pc203
fi
alias rpsql="psql -U stm_reader -d stm -h localhost -p 5432"

# Get system kernel buffer sizes for UDP 
export RMEM_MAX=`more /proc/sys/net/core/rmem_max` # RECEIVE
export WMEM_MAX=`more /proc/sys/net/core/wmem_max` # SEND

# Start STM DQM
#screen -dmS dqm python $STMDAQ_ROOT/dqm/alex/app.py
