#Set local paths and env
export MAKEFLAGS=-s
export BUILD_DIR=$(STMDAQ_ROOT)/build

# ---- Boost (from Spack) ----
export BOOST_INC := $(BOOST_ROOT)/include

# Library dir (Spack usually uses lib; if your install has lib64, flip this)
export BOOST_LIB_DIR := $(BOOST_ROOT)/lib

# ---- Python / pybind11 ----
export PYTHON ?= python3

# Python include dir (contains Python.h)
# pybind11 include dir (contains pybind11/embed.h)
export PY_INC := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_paths()['include'])") -I$(shell $(PYTHON) -c "import pybind11; print(pybind11.get_include())")

#export PYTHON ?= python3
#export PYB11_INC := -I$(shell $(PYTHON) -c "import pybind11; print(pybind11.g#et_include())")
#export PY_INC    := $(shell $(PYTHON)-config --includes) $(PYB11_INC)

# Link + rpath (add the components you actually use)
# Note: program_options & chrono depend on boost_system; thread needs pthread.
export BOOST_LIBS := \
  -L$(BOOST_LIB_DIR) \
  -Wl,-rpath,$(BOOST_LIB_DIR) \
  -lboost_program_options \
  -lboost_chrono \
  -lboost_random \
  -lboost_system \
  -pthread

# ---- fftw (from Spack, environment variable not auto set) ----
export FFTW_ROOT=/home/mu2e/spack_areas/artdaq-v4_03_00/spack/opt/spack/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_pat/linux-almalinux9-x86_64_v3/gcc-13.1.0/fftw-3.3.10-5v37hxys2g5txuz6urshrazypjgqlm3o

export FFTW_INC=$(FFTW_ROOT)/include
export FFTW_LIBS=-L$(FFTW_ROOT)/lib -lfftw3 -lm

export STM_INC=$(STMDAQ)

export BOOST_LIBS=-L$(CACTUS_ROOT)/lib -L$(BOOST_ROOT)/lib -lpq -lboost_chrono -lboost_program_options -lboost_random -lrt
#export BOOST_LIBS=-L$(CACTUS_ROOT)/lib -L$(BOOST_ROOT)/lib -lpq ...ation -lboost_chrono -lboost_program_options -lboost_random -lrt
export IPBUS_LIBS=$(BOOST_LIBS) -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal
export STM_LIBS=-L$(STMDAQ_ROOT)/build/lib -lUtils -lConfig -lHardware -lBuffers -lpugixml -lDebug -lProcessing -lDQM -lSim
#export PY_LDFLAGS := $(shell $(PYTHON)-config --embed --ldflags 2>/dev/null || $(PYTHON)-config --ldflags)
export PY_LDFLAGS := $(shell python3-config --embed --ldflags 2>/dev/null || python3-config --ldflags)


