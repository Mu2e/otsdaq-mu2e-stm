#!/usr/bin/env bash
# ======================================
# STMDAQ environment setup script
# ======================================

# ---- Color constants ----
RED='\e[31m'
GREEN='\e[32m'
CYAN='\e[96m'
ORANGE='\e[38;5;208m'
NC='\e[0m'

# ---- Logging helpers ----
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

# ---- Resolve this script's real directory ----
# Handles symbolic links and ensures STMDAQ_ROOT is absolute
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
  DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
STMDAQ_ROOT="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"

# ---- Export environment variables ----
export STMDAQ_ROOT
export STMDAQ="$(dirname "$STMDAQ_ROOT")"

success "STMDAQ_ROOT set to: $STMDAQ_ROOT"

# ---- Paths ----
export LD_LIBRARY_PATH="$STMDAQ_ROOT/build/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export XML_PATH="$STMDAQ_ROOT/config"
export STM_XML="$STMDAQ_ROOT/config/xml/master.xml"
export LOG_DIR="$STMDAQ_ROOT/log"

# ---- External dependencies ----
export CACTUS_ROOT=/opt/cactus
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$CACTUS_ROOT/lib"


# ---- Python packages for DQM ----
export PYTHONPATH="/usr/lib64/python3.9/site-packages${PYTHONPATH:+:$PYTHONPATH}"
export PYTHONPATH="/home/mu2estm/.local/lib/python3.9/site-packages:$PYTHONPATH"

# ---- Load Spack Environment  ----
#if [ -f /mu2e/spack_areas/mu2e-tdaq-v8_00_00_cand/setup-env.sh ]; then
  # source /mu2e/spack_areas/mu2e-tdaq-v8_00_00_cand/setup-env.sh
  # info "Loaded Mu2e spack environment"
  # spack load boost && info "Boost loaded via Spack"
  # spack load fftw/5v37hxy && info "FFTW loaded via Spack"
#  spack load python && info "Python loaded via Spack"
#  spack load py-pybind11 && info "Pybind loaded via Spack"
#else
#  error "Mu2e spack environment not found"
#fi
unset BOOST_ROOT BOOST_INCLUDEDIR BOOST_LIBRARYDIR CMAKE_PREFIX_PATH
