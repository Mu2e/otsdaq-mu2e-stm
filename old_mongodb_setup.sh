#!/bin/sh
export ARTDAQ_DATABASE_URI=mongodb://admin:m2edaqdb@localhost:27017/teststand_db?authSource=admin
#example tunnel: ssh -f -KX -N -L 27017:localhost:27017 -J mu2edaq-gateway.fnal.gov mu2edaq13.fnal.gov
#example: 27017 -J mu2edaq-gateway.fnal.gov mu2edaq13.fnal.gov
export ARTDAQ_DATABASE_SSH_HOST="mu2edaq13.fnal.gov"
export ARTDAQ_DATABASE_SSH_JUMP="-J mu2edaq-gateway.fnal.gov"
export ARTDAQ_DATABASE_SSH_PORT="27017"
# export ARTDAQ_DATABASE_SSH_TUNNEL="27017 -J mu2edaq-gateway.fnal.gov mu2edaq13.fnal.gov"



SHORT_HOSTNAME=$(hostname -s) 
echo -e "mongodb_setup.sh:${LINENO} |  \t SHORT_HOSTNAME=${SHORT_HOSTNAME}"

if [[ "$SHORT_HOSTNAME" == "mu2e-calo-01" || "$SHORT_HOSTNAME" == "mu2e-calo-02" || "$SHORT_HOSTNAME" == "mu2e-calo-03" || "$SHORT_HOSTNAME" == "mu2e-calo-04" || "$SHORT_HOSTNAME" == "mu2e-calo-05" || "$SHORT_HOSTNAME" == "mu2e-dl-01"  || "$SHORT_HOSTNAME" == "mu2e-cfo-01" || "$SHORT_HOSTNAME" == "mu2e-trk-05" ]]; then
    export THIS_HOST="$SHORT_HOSTNAME-data.fnal.gov"
    echo -e "mongodb_setup.sh:${LINENO} |  \t THIS_HOST=${THIS_HOST}"
fi 

# export ARTDAQ_DATABASE_SSH_CLUSTER_HOST="mu2e-calo-01-data"
# #============================
# #setup 1 tunnel per cluster, allow others to connect to this port
# if [[ "$SHORT_HOSTNAME" == "mu2e-calo-01" ]]; then
#     # Example of cluster tunnel: ssh -J mu2edaq-gateway -L mu2e-calo-01-data:3045:localhost:3045 mu2edaq13

#     if [[ "$ARTDAQ_DATABASE_SSH_HOST" == "$HOSTNAME" || \
#             "$ARTDAQ_DATABASE_SSH_HOST" == "$(hostname -f)" || \
#             "$ARTDAQ_DATABASE_SSH_HOST" == "$(hostname -s)" ]]; then
        
#         echo -e "mongodb_setup.sh:${LINENO} |  \t Cluster SSH Tunnel destination on this host, so no need to make tunnel."
#         echo -e "mongodb_setup.sh:${LINENO} |  \t " 

#     elif lsof -i :$ARTDAQ_DATABASE_SSH_PORT > /dev/null; then

#         echo -e "mongodb_setup.sh:${LINENO} |  \t Cluster SSH Tunnel Port already in use or tunnel already exists"
#         echo -e "mongodb_setup.sh:${LINENO} |  \t " 

#     else  
#         echo -e "mongodb_setup.sh:${LINENO} |  \t "
#         echo -e "mongodb_setup.sh:${LINENO} |  \t Cluster SSH Tunnel in background to IERC mongodb:"
#         echo "ssh -f -K -N $ARTDAQ_DATABASE_SSH_JUMP -L $ARTDAQ_DATABASE_SSH_CLUSTER_HOST:$ARTDAQ_DATABASE_SSH_PORT:localhost:$ARTDAQ_DATABASE_SSH_PORT $ARTDAQ_DATABASE_SSH_HOST"
        
#         timeout 10s ssh -f -K -N $ARTDAQ_DATABASE_SSH_JUMP -L $ARTDAQ_DATABASE_SSH_CLUSTER_HOST:$ARTDAQ_DATABASE_SSH_PORT:localhost:$ARTDAQ_DATABASE_SSH_PORT -o ExitOnForwardFailure=yes -o HostKeyAlgorithms=- $ARTDAQ_DATABASE_SSH_HOST            

#         echo -e "mongodb_setup.sh:${LINENO} |  \t " 
#     fi
# fi

#now override for local ots instance tunnel
#ARTDAQ_DATABASE_SSH_JUMP=""
#ARTDAQ_DATABASE_SSH_HOST=""
#ARTDAQ_DATABASE_URI="mongodb://admin:m2edaqdb@$ARTDAQ_DATABASE_SSH_CLUSTER_HOST:27017/teststand_db?authSource=admin"
