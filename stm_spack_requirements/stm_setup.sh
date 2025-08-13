# Host mappings format:
# Each line defines a mapping with three fields separated by spaces or tabs:
#   <HOSTNAME>       <THIS_HOST>             <NFS_HOST>
#
# - <HOSTNAME> is the domain name of the machine
# - <THIS_HOST> is the host name to be used for THIS_HOST environment variable
# - <NFS_HOST> is the host name to be used for NFS_HOST environment variable
#
# Fields can be separated by any number of spaces or tabs — alignment is for readability only.
#
# Example:
# mu2edaq11.fnal.gov   mu2edaq11-data.fnal.gov   mu2edaq13-ctrl.fnal.gov
# means that if the machine's hostname is mu2edaq11.fnal.gov,
# THEN THIS_HOST=mu2edaq11-data.fnal.gov and NFS_HOST=mu2edaq13-ctrl.fnal.gov

read -r -d '' HOST_MAPPINGS <<'EOF'
mu2edaq11.fnal.gov   mu2edaq11-data.fnal.gov   mu2edaq13-ctrl.fnal.gov
mu2edaq13.fnal.gov   mu2edaq13-data.fnal.gov   mu2edaq13-ctrl.fnal.gov
mu2e-stm-01.fnal.gov mu2e-stm-01.fnal.gov      mu2e-dl-01.fnal.gov
mu2e-stm-02.fnal.gov mu2e-stm-02.fnal.gov      mu2e-dl-01.fnal.gov
mu2e-dl-01.fnal.gov  mu2e-dl-01.fnal.gov       mu2e-dl-01.fnal.gov
EOF

if command -v hostname &>/dev/null; then
    HOST_TO_LOOKUP=$(hostname -f 2>/dev/null || hostname)
else
    HOST_TO_LOOKUP="$HOSTNAME"
fi

HOST_TO_LOOKUP=${HOST_TO_LOOKUP:-$(hostname)}

match=$(grep -E "^${HOST_TO_LOOKUP}[[:space:]]" <<< "$HOST_MAPPINGS" | head -n1)

if [[ -z "$match" ]]; then
    echo "ERROR: Unknown host '$HOST_TO_LOOKUP'. Cannot set THIS_HOST or NFS_HOST."
    return 1
fi

read -r _ THIS_HOST NFS_HOST <<< "$match"

export THIS_HOST NFS_HOST

echo "Set up for user: $USER"
echo "On host: $THIS_HOST"
echo "NFS host: $NFS_HOST"

export STM_HPGE_FW_PORT=51872 # HPGe (CH0) UDP IP port
export STM_LABR_FW_PORT=51874 # LaBr (CH1) UDP IP port
export STM_HPGE_SW_IP="127.0.0.2" # HPGe (CH0) UDP IP address (send)
export STM_LABR_SW_IP="127.0.0.3" # LaBr (CH1) UDP IP address (send)
export STM_HPGE_SW_PORT=10010 # HPGe (CH0) UDP IP port (send)
export STM_LABR_SW_PORT=10012 # LaBr (CH1) UDP IP port (send)

echo
echo "Set HPGE port as $STM_HPGE_SW_IP:$STM_HPGE_SW_PORT"
echo "Set LABR port as $STM_LABR_SW_IP:$STM_LABR_SW_PORT"
echo
