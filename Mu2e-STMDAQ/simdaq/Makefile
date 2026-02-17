ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated -Wno-sign-compare

#CCOPTS = -O3 # optimiase
CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information
#CCOPTS = -Wall -Wno-unused-variable -pg # turn on profiler information
DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d


INCLUDE_CC = -I$(BOOST_INC) -I$(STM_INC)
MAIN_CC = run_eventSim.cc
REC_CC = recieveTCP.cc
LIB_CC = send_TCP.cc gen_eventData.cc
SOURCE_CC = $(LIB_CC) $(REC_CC) $(MAIN_CC)  
LIB_OBJ = $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(LIB_CC))
LIB = $(BUILD_DIR)/lib/libEventSim.so 
EXE = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(MAIN_CC))
EXE2 = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(REC_CC))
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(SOURCE_CC))

.PHONY: all clean

all: $(EXE) $(EXE2)

clean:
	rm $(LIB_OBJ) $(DEPFILES)

$(EXE) : $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC)) $(LIB)
		echo "<**Linking**>"
		g++ $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC)) $(STM_LIBS) -o $(EXE)


$(EXE2) : $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(REC_CC)) 
		echo "<**Building TCP reciever**>"
		g++ $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(REC_CC)) -o $(EXE2)


$(LIB) : $(LIB_OBJ)
	echo "<***Building Library**>" $(notdir $(LIB))
	g++ -shared -o $(LIB) $(LIB_OBJ)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CCOPTS) $(DEPOPTS) $(WARNING) $(INCLUDE_CC) -c $< -o  $@

$(DEPFILES):
include $(wildcard $(DEPFILES))



