#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated -Wno-sign-compare

# Turn on optimsation and also the flag that we are using the database
#CCOPTS = -O3
CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information
#CCOPTS = -Wall -Wno-unused-variable -pg # turn on profiler information
DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d

INCLUDE_CC = -I$(BOOST_INC) -I$(STM_INC) -I$(PQXX_INC)

MAIN_CC = CallZS.cc
LIB_CC = ZS.cc
SOURCE_CC = $(LIB_CC) $(MAIN_CC)
LIB_OBJ = $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(LIB_CC))
LIB = $(BUILD_DIR)/lib/libZSnew.so 
EXE = $(patsubst %.cc, %.exe, $(MAIN_CC))
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(SOURCE_CC))

.PHONY: all clean

all: $(EXE)

clean:
	rm $(LIB_OBJ) $(DEPFILES)

$(EXE) : $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC)) $(LIB)
	echo "<**Linking**>"
	g++ $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC)) $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) -o $(EXE)

$(LIB) : $(LIB_OBJ)
	echo "<***Building Library**>" $(notdir $(LIB))
	g++ -shared -o $(LIB) $(LIB_OBJ)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CCOPTS) $(DEPOPTS) $(WARNING) $(INCLUDE_CC) -c $< -o  $@

$(DEPFILES):
include $(wildcard $(DEPFILES))



