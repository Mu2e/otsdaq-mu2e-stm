
export DCS_ARCHIVE_DATABASE="dcs_archive_dev"
export DCS_ARCHIVE_DATABASE_HOST="mu2edaq14-ctrl.fnal.gov"
export DCS_ARCHIVE_DATABASE_PORT="5434"
export DCS_ARCHIVE_DATABASE_USER="dcs_writer"
export DCS_ARCHIVE_DATABASE_PWD="wrt4_mu2e_daq"

export OTSDAQ_COMPILE_RUNINFO="1"
export OTSDAQ_RUNINFO_DATABASE="run_info_dev"
export OTSDAQ_RUNINFO_DATABASE_HOST="mu2edaq14-ctrl"
#export OTSDAQ_RUNINFO_DATABASE_HOST="mu2e-dcs-01"
export OTSDAQ_RUNINFO_DATABASE_PORT="5434"
export OTSDAQ_RUNINFO_DATABASE_USER="run_user"
export OTSDAQ_RUNINFO_DATABASE_SCHEMA="test"
export OTSDAQ_RUNINFO_DATABASE_RUNTYPE="99" #generic test

# if [ "x$OTSDAQ_RUNINFO_DATABASE_PWD" == "x" ]; then #when RUNINFO password is not setup, prompt user
#        stty -echo
#        printf "(prototype_run_info input)\n"
#        read runinfopass
#        stty echo
#        export OTSDAQ_RUNINFO_DATABASE_PWD=$runinfopass
# fi
export OTSDAQ_RUNINFO_DATABASE_PWD="daq14_run_user"

########
SHORT_HOSTNAME=$(hostname -s) 
echo -e "db_setup_ots.sh:${LINENO} |  \t SHORT_HOSTNAME=${SHORT_HOSTNAME}"

export OTSDAQ_RUNINFO_DATABASE_JUMP="-J mu2eshift@mu2edaq-gateway.fnal.gov"
export OTSDAQ_RUNINFO_DATABASE_JUMP2="mu2edaq13"

export DCS_ARCHIVE_DATABASE_JUMP="-J mu2eshift@mu2edaq-gateway.fnal.gov"
export DCS_ARCHIVE_DATABASE_JUMP2="mu2edaq13"

export EPICS_CA_NAME_SERVERS_JUMP="-J mu2eshift@mu2edaq-gateway.fnal.gov"
export EPICS_CA_NAME_SERVERS_JUMP2="mu2edaq13"

#============================
#setup 1 tunnel per cluster for Run Conditions db
if [[ "$SHORT_HOSTNAME" == "mu2e-calo-01" ]]; then
    # Example of cluster tunnel: ssh -J mu2edaq-gateway -L mu2edaq18-data:3045:localhost:3045 mu2edaq13

    if [[ "$OTSDAQ_RUNINFO_DATABASE_HOST" == "$HOSTNAME" || \
            "$OTSDAQ_RUNINFO_DATABASE_HOST" == "$(hostname -f)" || \
            "$OTSDAQ_RUNINFO_DATABASE_HOST" == "$(hostname -s)" ]]; then
        
        echo -e "db_setup_ots.sh:${LINENO} |  \t SSH Tunnel destination on this host, so no need to make tunnel."
        echo -e "db_setup_ots.sh:${LINENO} |  \t " 

    elif lsof -i :$OTSDAQ_RUNINFO_DATABASE_PORT > /dev/null; then

        echo -e "db_setup_ots.sh:${LINENO} |  \t SSH Tunnel Port already in use or tunnel already exists"
        echo -e "db_setup_ots.sh:${LINENO} |  \t " 

    else  
        echo -e "db_setup_ots.sh:${LINENO} |  \t "
        echo -e "db_setup_ots.sh:${LINENO} |  \t SSH Tunnel in background to IERC Run Conditions db:"
        echo "ssh -f -K -N $OTSDAQ_RUNINFO_DATABASE_JUMP -L $OTSDAQ_RUNINFO_DATABASE_PORT:$OTSDAQ_RUNINFO_DATABASE_HOST:$OTSDAQ_RUNINFO_DATABASE_PORT $OTSDAQ_RUNINFO_DATABASE_JUMP2"
        
        timeout 10s ssh -f -K -N $OTSDAQ_RUNINFO_DATABASE_JUMP -L $OTSDAQ_RUNINFO_DATABASE_PORT:$OTSDAQ_RUNINFO_DATABASE_HOST:$OTSDAQ_RUNINFO_DATABASE_PORT -o ExitOnForwardFailure=yes -o HostKeyAlgorithms=- $OTSDAQ_RUNINFO_DATABASE_JUMP2            

        echo -e "db_setup_ots.sh:${LINENO} |  \t " 
    fi
fi

#============================
#setup 2 tunnel per cluster for epics archiver db
if [[ "$SHORT_HOSTNAME" == "mu2e-calo-01" ]]; then
    # Example of cluster tunnel: ssh -J mu2edaq-gateway -L mu2edaq18-data:3045:localhost:3045 mu2edaq13

    if [[   "$DCS_ARCHIVE_DATABASE_HOST" == "$HOSTNAME" || \
            "$DCS_ARCHIVE_DATABASE_HOST" == "$(hostname -f)" || \
            "$DCS_ARCHIVE_DATABASE_HOST" == "$(hostname -s)" ]]; then

        echo -e "db_setup_ots.sh:${LINENO} |  \t SSH Tunnel destination on this host, so no need to make tunnel."
        echo -e "db_setup_ots.sh:${LINENO} |  \t "

    elif lsof -i :$DCS_ARCHIVE_DATABASE_PORT > /dev/null; then

        echo -e "db_setup_ots.sh:${LINENO} |  \t SSH Tunnel Port already in use or tunnel already exists"
        echo -e "db_setup_ots.sh:${LINENO} |  \t "

    else
        echo -e "db_setup_ots.sh:${LINENO} |  \t "
        echo -e "db_setup_ots.sh:${LINENO} |  \t SSH Tunnel in background to IERC Run Conditions db:"
        echo "ssh -f -K -N $DCS_ARCHIVE_DATABASE_JUMP -L $DCS_ARCHIVE_DATABASE_PORT:$DCS_ARCHIVE_DATABASE_HOST:$DCS_ARCHIVE_DATABASE_PORT $DCS_ARCHIVE_DATABASE_JUMP2"

        timeout 10s ssh -f -K -N $DCS_ARCHIVE_DATABASE_JUMP -L $DCS_ARCHIVE_DATABASE_PORT:$DCS_ARCHIVE_DATABASE_HOST:$DCS_ARCHIVE_DATABASE_PORT -o ExitOnForwardFailure=yes -o HostKeyAlgorithms=- $DCS_ARCHIVE_DATABASE_JUMP2

        echo -e "db_setup_ots.sh:${LINENO} |  \t "
    fi
fi



#override to local operation
OTSDAQ_RUNINFO_DATABASE_HOST="localhost"
DCS_ARCHIVE_DATABASE_HOST="localhost"
#EPICS_CA_NAME_SERVERS=localhost:5064

