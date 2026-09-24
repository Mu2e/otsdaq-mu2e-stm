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
# Color constants
#!/usr/bin/env bash

# ============================================================================
#  Source-only environment setup for Mu2e STMDAQ
#
#  Usage:
#    source setup_stmdaq.sh
# ============================================================================

# ----------------------------------------------------------------------------
# Logging helpers
# ----------------------------------------------------------------------------

RED='\e[31m'
GREEN='\e[32m'
CYAN='\e[96m'
ORANGE='\e[38;5;208m'
NC='\e[0m'

log() {
  local timestamp message color
  timestamp=$(date +'%Y-%m-%d %H:%M:%S')
  message="$1"
  color="${2:-$NC}"
  printf "%b[%s] %s%b\n" "$color" "$timestamp" "$message" "$NC"
}

info()    { log "[INFO] $*"    "$CYAN"; }
error()   { log "[ERROR] $*"   "$RED" >&2; }
success() { log "[SUCCESS] $*" "$GREEN"; }

# ----------------------------------------------------------------------------
# Host mappings
# ----------------------------------------------------------------------------

read -r -d '' HOST_MAPPINGS <<'EOF'
mu2edaq11.fnal.gov   mu2edaq11-data.fnal.gov   mu2edaq13-ctrl.fnal.gov
mu2edaq13.fnal.gov   mu2edaq13-data.fnal.gov   mu2edaq13-ctrl.fnal.gov
mu2e-stm-01.fnal.gov mu2e-stm-01.fnal.gov      mu2e-dl-01.fnal.gov
mu2e-stm-02.fnal.gov mu2e-stm-02.fnal.gov      mu2e-dl-01.fnal.gov
mu2e-dl-01.fnal.gov  mu2e-dl-01.fnal.gov       mu2e-dl-01.fnal.gov
EOF

# ----------------------------------------------------------------------------
# Host detection
# ----------------------------------------------------------------------------

if command -v hostname &>/dev/null; then
  HOST_TO_LOOKUP=$(hostname -f 2>/dev/null || hostname)
else
  HOST_TO_LOOKUP="${HOSTNAME:-}"
fi

HOST_TO_LOOKUP=${HOST_TO_LOOKUP:-$(hostname)}

match=$(awk -v host="$HOST_TO_LOOKUP" '$1 == host { print; exit }' <<< "$HOST_MAPPINGS")

if [[ -z "$match" ]]; then
  error "Unknown host '$HOST_TO_LOOKUP'. Cannot set THIS_HOST or NFS_HOST."
  return 1
fi

read -r _ THIS_HOST NFS_HOST <<< "$match"
export THIS_HOST NFS_HOST

info "User     : $USER"
info "Host     : $THIS_HOST"
info "NFS host : $NFS_HOST"

# ----------------------------------------------------------------------------
# STM networking
# ----------------------------------------------------------------------------

export STM_HPGE_FW_PORT=51872
export STM_LABR_FW_PORT=51874

export STM_HPGE_SW_IP="127.0.0.2"
export STM_LABR_SW_IP="127.0.0.3"

export STM_HPGE_SW_PORT=10010
export STM_LABR_SW_PORT=10012

info "HPGE send : $STM_HPGE_SW_IP:$STM_HPGE_SW_PORT"
info "LABR send : $STM_LABR_SW_IP:$STM_LABR_SW_PORT"

# ----------------------------------------------------------------------------
# Resolve script location
# ----------------------------------------------------------------------------

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SPACK_DIR="$SCRIPT_DIR"

# ----------------------------------------------------------------------------
# STMDAQ paths
# ----------------------------------------------------------------------------

export STMDAQ_ROOT="$SPACK_DIR/srcs/otsdaq-mu2e-stm/Mu2e-STMDAQ"
export STMDAQ="$(dirname "$STMDAQ_ROOT")"

success "STMDAQ_ROOT set to: $STMDAQ_ROOT"

export XML_PATH="$STMDAQ_ROOT/config"
export STM_XML="$STMDAQ_ROOT/config/xml/master.xml"
export LOG_DIR="$STMDAQ_ROOT/log"

# ----------------------------------------------------------------------------
# Library paths (two roots, composed once)
# ----------------------------------------------------------------------------

export CACTUS_ROOT=/opt/cactus

STM_LIB_DIR="$STMDAQ_ROOT/build/lib"
CACTUS_LIB_DIR="$CACTUS_ROOT/lib"

export LD_LIBRARY_PATH="$STM_LIB_DIR:$CACTUS_LIB_DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# ----------------------------------------------------------------------------
# Python environment (DQM)
# ----------------------------------------------------------------------------

PY_VER=$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")' \
         2>/dev/null || echo "3.9")

export PYTHONPATH="/usr/lib64/python${PY_VER}/site-packages${PYTHONPATH:+:$PYTHONPATH}"
export PYTHONPATH="/home/mu2estm/.local/lib/python${PY_VER}/site-packages:$PYTHONPATH"

# ----------------------------------------------------------------------------
# Done
# ----------------------------------------------------------------------------

success "STMDAQ environment loaded"
