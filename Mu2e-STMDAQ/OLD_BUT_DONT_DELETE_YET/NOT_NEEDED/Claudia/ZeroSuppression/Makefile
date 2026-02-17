#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
CC_OPTS = -Wall -Wno-unused-variable -O3 # turn on code optimisation
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information
#CC_OPTS = -Wall -Wno-unused-variable -pg # turn on profiler information
# -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC


INCLUDE_CC = -I$(ROOT_INC)

all: $(BUILD_DIR)/bin/zero_suppression_cutout_test_bench.exe

clean: 
	rm -rf $(BUILD_DIR)/obj/zero_suppression_cutout_test_bench.o

$(BUILD_DIR)/obj/zero_suppression_cutout_test_bench.o : zero_suppression_cutout_test_bench.cc
	echo "<**Compiling zero_suppression_cutout_test_bench.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) zero_suppression_cutout_test_bench.cc -o $(BUILD_DIR)/obj/zero_suppression_cutout_test_bench.o

$(BUILD_DIR)/bin/zero_suppression_cutout_test_bench.exe : $(BUILD_DIR)/obj/zero_suppression_cutout_test_bench.o
	echo "<**Linking zero_suppression_cutout_test_bench**>"
	g++ $(BUILD_DIR)/obj/zero_suppression_cutout_test_bench.o $(ROOT_LIBS) -o $(BUILD_DIR)/bin/zero_suppression_cutout_test_bench.exe
