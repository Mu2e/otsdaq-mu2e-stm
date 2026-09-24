#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
CC_OPTS = -Wall -Wno-unused-variable -O3 # turn on code optimisation
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information
#CC_OPTS = -Wall -Wno-unused-variable -pg # turn on profiler information
# -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC


INCLUDE_CC = -I$(ROOT_INC)

all: $(BUILD_DIR)/bin/energy_histo.exe $(BUILD_DIR)/bin/nonpeaksuppression.exe $(BUILD_DIR)/bin/energy_histo_skipROOT.exe

clean: 
	rm -rf $(BUILD_DIR)/obj/energy_histo.o $(BUILD_DIR)/obj/nonpeaksuppression.o $(BUILD_DIR)/obj/energy_histo_skipROOT.o

$(BUILD_DIR)/obj/energy_histo.o :  energy_histo.cc 
	echo "<**Compiling energy_histo.cc**>"
	g++  -c $(CC_OPTS) $(INCLUDE_CC) energy_histo.cc -o $(BUILD_DIR)/obj/energy_histo.o

$(BUILD_DIR)/bin/energy_histo.exe : $(BUILD_DIR)/obj/energy_histo.o
	echo "<**Linking energy_histo**>"
	g++ $(BUILD_DIR)/obj/energy_histo.o $(ROOT_LIBS) -o $(BUILD_DIR)/bin/energy_histo.exe

$(BUILD_DIR)/obj/nonpeaksuppression.o : nonpeaksuppression.cc
	echo "<**Compiling nonpeaksuppression.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) nonpeaksuppression.cc -o $(BUILD_DIR)/obj/nonpeaksuppression.o

$(BUILD_DIR)/bin/nonpeaksuppression.exe : $(BUILD_DIR)/obj/nonpeaksuppression.o
	echo "<**Linking nonpeaksuppression**>"
	g++ $(BUILD_DIR)/obj/nonpeaksuppression.o $(ROOT_LIBS) -o $(BUILD_DIR)/bin/nonpeaksuppression.exe

$(BUILD_DIR)/obj/energy_histo_skipROOT.o : energy_histo_skipROOT.cc
	echo "<**Compiling energy_histo_skipROOT**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) energy_histo_skipROOT.cc -o $(BUILD_DIR)/obj/energy_histo_skipROOT.o -pg

$(BUILD_DIR)/bin/energy_histo_skipROOT.exe : $(BUILD_DIR)/obj/energy_histo_skipROOT.o
	echo "<**Linking energy_histo_skipROOT**>"
	g++ $(BUILD_DIR)/obj/energy_histo_skipROOT.o $(ROOT_LIBS) -o $(BUILD_DIR)/bin/energy_histo_skipROOT.exe -pg
