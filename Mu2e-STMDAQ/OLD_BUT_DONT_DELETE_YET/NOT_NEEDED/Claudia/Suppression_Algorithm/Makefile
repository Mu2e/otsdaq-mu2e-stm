#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
#CC_OPTS = -Wall # Normal compilation
CC_OPTS = -Wall -Wno-unused-variable -O3 # turn on code optimisation
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information
#CC_OPTS = -Wall -Wno-unused-variable -pg # turn on profiler information
# -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC


INCLUDE_CC = -I$(ROOT_INC) -I$(BOOST_INC)

all: $(BUILD_DIR)/bin/nonpeaksuppression_new.exe $(BUILD_DIR)/bin/gradient_peaksuppression.exe

clean: 
	rm -rf $(BUILD_DIR)/obj/nonpeaksuppression_new.o $(BUILD_DIR)/obj/gradient_peaksuppression.o

$(BUILD_DIR)/obj/nonpeaksuppression_new.o : nonpeaksuppression_new.cc
	echo "<**Compiling nonpeaksuppression_new.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) nonpeaksuppression_new.cc -o $(BUILD_DIR)/obj/nonpeaksuppression_new.o

$(BUILD_DIR)/bin/nonpeaksuppression_new.exe : $(BUILD_DIR)/obj/nonpeaksuppression_new.o
	echo "<**Linking nonpeaksuppression_new**>"
	g++ $(BUILD_DIR)/obj/nonpeaksuppression_new.o $(ROOT_LIBS) $(BOOST_LIBS) -o $(BUILD_DIR)/bin/nonpeaksuppression_new.exe

$(BUILD_DIR)/obj/gradient_peaksuppression.o : gradient_peaksuppression.cc
	echo "<**Compiling gradient_peaksuppression.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) gradient_peaksuppression.cc -o $(BUILD_DIR)/obj/gradient_peaksuppression.o

$(BUILD_DIR)/bin/gradient_peaksuppression.exe : $(BUILD_DIR)/obj/gradient_peaksuppression.o
	echo "<**Linking gradient_peaksuppression**>"
	 g++ $(BUILD_DIR)/obj/gradient_peaksuppression.o $(ROOT_LIBS) $(BOOST_LIBS) -o $(BUILD_DIR)/bin/gradient_peaksuppression.exe
