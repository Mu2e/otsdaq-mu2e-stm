#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
#CC_OPTS = -Wall # Normal compilation
CC_OPTS = -Wall -Wno-unused-variable -O3 # turn on code optimisation
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information
#CC_OPTS = -Wall -Wno-unused-variable -pg # turn on profiler information
# -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC


INCLUDE_CC = -I$(ROOT_INC) -I$(BOOST_INC)

all: $(BUILD_DIR)/bin/GenData.exe

clean: 
	rm -rf $(BUILD_DIR)/obj/GenData.o

$(BUILD_DIR)/obj/GenData.o : GenData.cc
	echo "<**Compiling GenData.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) GenData.cc -o $(BUILD_DIR)/obj/GenData.o

$(BUILD_DIR)/bin/GenData.exe : $(BUILD_DIR)/obj/GenData.o
	echo "<**Linking GenData**>"
	g++ $(BUILD_DIR)/obj/GenData.o $(ROOT_LIBS) $(BOOST_LIBS) -o $(BUILD_DIR)/bin/GenData.exe
