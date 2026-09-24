#Set local paths and env
export MAKEFLAGS=-s
export BUILD_DIR=$(STMDAQ_ROOT)/build

#Check here that it's a supported machine with gcc 8.3, centos7 etc
#ifeq ($(OSTYPE),linux64)
#COMPILE_ONLY=-c
#OS_FLAG=-DOS_LINUX
#else
#$(error Only supported build system is Scientific Linux 6 64-bit)
#endif

#export CPPSTD= -std=c++0x

export BOOST_INC=$(MU2EDAQ_ROOT)/boost/include
# ML:kludge for now AL9.
export BOOST_INC=/opt/boost/include
export CACTUS_INC=$(MU2EDAQ_ROOT)/cactus/include
# ML:kludge for now AL9.
export CACTUS_INC=/opt/cactus/include
#

export STM_INC=$(STMDAQ)

export BOOST_LIBS=-L$(CACTUS_ROOT)/lib -L$(BOOST_ROOT)/lib -lpq -lpthread -lboost_filesystem -lboost_regex -lboost_system -lboost_thread -lboost_program_options -lboost_serialization -lboost_chrono -lboost_program_options -lboost_random -lrt
export IPBUS_LIBS=$(BOOST_LIBS) -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal
export STM_LIBS=-L$(STMDAQ_ROOT)/build/lib -lUtils -lProcessData -lDQM -lFE


