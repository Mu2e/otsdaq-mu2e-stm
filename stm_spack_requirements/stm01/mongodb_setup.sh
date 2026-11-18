#!/bin/sh
# Credentials are not stored in this repository. Set ARTDAQ_DATABASE_PWD in the
# environment (e.g. from a mode-600 file outside the repo) before sourcing this script.
if [ -z "${ARTDAQ_DATABASE_PWD:-}" ]; then
    echo -e "mongodb_setup.sh:${LINENO} |  \t WARNING: ARTDAQ_DATABASE_PWD is not set; MongoDB authentication will fail."
fi
export ARTDAQ_DATABASE_URI=mongodb://admin:${ARTDAQ_DATABASE_PWD}@localhost:27017/teststand_db?authSource=admin
                                                                                                                             
export ARTDAQ_DATABASE_SSH_PORT="27017"
# export ARTDAQ_DATABASE_SSH_JUMP="-J mu2edaq-gateway.fnal.gov"
export ARTDAQ_DATABASE_SSH_JUMP=""
export ARTDAQ_DATABASE_SSH_HOST="" #will not define otsUserSshTunnels() in setup script, and not make tunnels with ots script
# export ARTDAQ_DATABASE_SSH_HOST="mu2edaq13.fnal.gov"

#echo -e "mongodb_setup.sh:${LINENO} |  \t "
#echo -e "mongodb_setup.sh:${LINENO} |  \t Tunnel in background example to FCC mongodb:"
#echo "ssh -o ExitOnForwardFailure=yes -f -KX -N -L 27017:localhost:27017 -J mu2edaq-gateway.fnal.gov mu2edaq13.fnal.gov"
#ssh -f -KX -N -L 27017:localhost:27017 -J mu2edaq-gateway.fnal.gov mu2edaq13.fnal.gov >/dev/null 
#echo -e "mongodb_setup.sh:${LINENO} |  \t " 

SHORT_HOSTNAME=$(hostname -s) 
echo -e "mongodb_setup.sh:${LINENO} |  \t SHORT_HOSTNAME=${SHORT_HOSTNAME}"

#============================
#always use -data on certain hosts
if [[ "$SHORT_HOSTNAME" == "mu2edaq14" || "$SHORT_HOSTNAME" == "mu2edaq13" || 
        "$SHORT_HOSTNAME" == "mu2edaq11" || 
        "$SHORT_HOSTNAME" == "mu2edaq22"  || "$SHORT_HOSTNAME" == "mu2edaq04" ]]; then
    export THIS_HOST="$SHORT_HOSTNAME-data.fnal.gov"
    echo -e "mongodb_setup.sh:${LINENO} |  \t THIS_HOST=${THIS_HOST}"
fi

#always use -ctrl on certain hosts
if [[ "$SHORT_HOSTNAME" == "mu2edaq07" ]]; then
    export THIS_HOST="$SHORT_HOSTNAME-ctrl.fnal.gov"
    echo -e "mongodb_setup.sh:${LINENO} |  \t THIS_HOST=${THIS_HOST}"
fi
