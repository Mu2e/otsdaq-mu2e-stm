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

export MIDAS_INC=$(MU2EDAQ_ROOT)/midas/include
export ZMQ_INC=$(MU2EDAQ_ROOT)/zmq/include
export ROOT_INC=$(shell $(ROOTSYS)/bin/root-config --incdir) 
#export PQXX_INC=$(MU2EDAQ_ROOT)/pqxx/include
export STM_INC=$(STMDAQ)
export MWD_INC=$(STMDAQ)/MWD

export ROOT_LIBS=$(shell $(ROOTSYS)/bin/root-config --glibs) 
#LDD_OPTS += -g $(shell $(ROOTSYS)/bin/root-config --ldflags)
export MIDAS_LIBS=-L$(MIDASSYS)/lib -lmfe -lmidas -lm -lz -lutil -lnsl -lpthread -lrt
export BOOST_LIBS=-L$(CACTUS_ROOT)/lib -L$(BOOST_ROOT)/lib -lpq -lpthread -lboost_filesystem -lboost_regex -lboost_system -lboost_thread -lboost_program_options -lboost_serialization -lboost_chrono -lboost_program_options -lboost_random -lrt
export IPBUS_LIBS=$(BOOST_LIBS) -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal
export ZMQ_LIBS=$(MU2EDAQ_ROOT)/zmq/lib -lzmq
#export PQXX_LIBS=-L$(MU2EDAQ_ROOT)/pqxx/lib -lpqxx -lpq
export STM_LIBS=-L$(STMDAQ_ROOT)/build/lib -lUtils -lFMC120 -lFMC144 -lHardware -lProcessData -lDQM -lFE #-lHPGeSim 


