# Color constants
RED='\e[31m'
GREEN='\e[32m'
CYAN='\e[96m'
ORANGE='\e[38;5;208m'
NC='\e[0m'

log() {
  local timestamp message color
  timestamp=$(date +'%Y-%m-%d %H:%M:%S')
  message="$1"
  color="$2"
  printf "%b[%s] %s%b\n" "$color" "$timestamp" "$message" "$NC"
}

info() { log "[INFO] $*" "$CYAN"; }
error() { log "[ERROR] $*" "$RED" >&2; }
success() { log "[SUCCESS] $*" "$GREEN"; }

confirm() {
  local msg="$1"
  local timestamp
  timestamp=$(date +"%Y-%m-%d %H:%M:%S")
  local formatted="[$timestamp] [CONFIRM] $msg (y/n): "
  printf "${ORANGE}%s${NC}" "$formatted"
  read -r -p "" response
  [[ "$response" =~ ^[Yy]$ ]]
}

CURRENT_DIR="$(pwd)"
DEFAULT_DIR="$HOME/Mu2e-STMDAQ"

if [[ -d "$CURRENT_DIR" && -d "$DEFAULT_DIR" ]]; then
  info "Found both current directory ($CURRENT_DIR) and default ($DEFAULT_DIR)."
  if confirm "Use current directory as STMDAQ_ROOT?"; then
    STMDAQ_ROOT="$CURRENT_DIR"
  else
    STMDAQ_ROOT="$DEFAULT_DIR"
  fi

elif [[ -d "$CURRENT_DIR" ]]; then
  info "Only current directory found: $CURRENT_DIR"
  if confirm "Use current directory as STMDAQ_ROOT?"; then
    STMDAQ_ROOT="$CURRENT_DIR"
  else
    error "No default directory available. Exiting setup."
    return 1
  fi

elif [[ -d "$DEFAULT_DIR" ]]; then
  info "Only default directory found: $DEFAULT_DIR"
  if confirm "Use default directory as STMDAQ_ROOT?"; then
    STMDAQ_ROOT="$DEFAULT_DIR"
  else
    error "No other valid STMDAQ_ROOT found. Exiting setup."
    return 1
  fi

else
  error "Neither current directory nor default directory exist."
  return 1
fi

export STMDAQ_ROOT
export STMDAQ="$(dirname "$STMDAQ_ROOT")"

success "STMDAQ_ROOT set to: $STMDAQ_ROOT"

export LD_LIBRARY_PATH="$STMDAQ_ROOT/build/lib"
export XML_PATH="$STMDAQ_ROOT/config"
export STM_XML="$STMDAQ_ROOT/config/stmdaq.xml"
export LOG_DIR="$STMDAQ_ROOT/log"

export CACTUS_ROOT=/opt/cactus
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CACTUS_ROOT/lib

# For DQM: numpy, psutils and dash
#source /mu2e/spack_areas/mu2e-tdaq-v8_00_00_cand/setup-env.sh
#spack load py-numpy
export PYTHONPATH=/usr/lib64/python3.9/site-packages:$PYTHONPATH
export PYTHONPATH=/home/mu2estm/.local/lib/python3.9/site-packages:$PYTHONPATH
