#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated -Wno-sign-compare

# Turn on optimsation and also the flag that we are using the database
#CC_OPTS = -O3 -DUSE_SQL

#Use this
CC_OPTS = -Wall -Wno-unused-variable -O3 -DUSE_SQL # turn on code optimisation and database access
#CC_OPTS = -Wall -Wno-unused-variable -O3 -DUSE_SQL -DPRINT_COUT # turn on code optimisation, database access and couts 

#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_SQL # turn on debugging information
#CC_OPTS = -Wall -Wno-unused-variable -pg # turn on profiler information
DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d

INCLUDE_CC = -I$(BOOST_INC) -I$(STM_INC)
LIB_CC = ZS.cc
LIB_OBJ = $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(LIB_CC))
LIB = $(BUILD_DIR)/lib/libZS.so 
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(LIB_CC))


.PHONY: all clean

all: $(LIB)

clean:
	rm $(LIB_OBJ) $(DEPFILES)


$(LIB) : $(LIB_OBJ)
	echo "<***Building Library**>" $(notdir $(LIB))
	g++ -shared -o $(LIB) $(LIB_OBJ)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CC_OPTS) $(DEPOPTS) $(WARNING) $(INCLUDE_CC) -c $< -o  $@

$(DEPFILES):
include $(wildcard $(DEPFILES))



