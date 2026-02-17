ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

 # turn off some warnings and compile with fPIC for shared object
WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated -Wno-sign-compare

#OPTIMISE = -O3 -DUSE_SQL
CCOPTS = -O3
#CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information	
#CCOPTS = -Wall -Wno-unused-variable -pg # turn on profiler information  
DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d

INCLUDE_CC = -I$(STM_INC)	

LIB_CC = MWD.cc
MAIN_CC = main.cc
SOURCE_CC = $(LIB_CC) $(MAIN_CC)
LIB_OBJ = $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(LIB_CC))
LIB = $(BUILD_DIR)/lib/libMWD.so
MAIN_EXE = run_MWD.exe
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(SOURCE_CC))

.PHONY: all clean

all: $(MAIN_EXE) 

lib: $(LIB)

clean: 
	rm $(MAIN_EXE) $(LIB_OBJ) $(DEPFILES)

$(MAIN_EXE) : $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC)) $(LIB)
	echo "<**Linking**>"
	g++ $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC)) $(STM_LIBS) $(IPBUS_LIBS) -o $(MAIN_EXE)

$(LIB) : $(LIB_OBJ)
	echo "<***Building Library**>" $(notdir $(LIB))
	g++ -shared -o $(LIB) $(LIB_OBJ)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CCOPTS) $(DEPOPTS) $(WARNING) $(INCLUDE_CC) -c $< -o  $@

$(DEPFILES):
include $(wildcard $(DEPFILES))

