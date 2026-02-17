ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

include $(STMDAQ_ROOT)/Makefile.inc

WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated

#CC_OPTS = -O3 -DUSE_SQL
#CC_OPTS = -O3 
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_SQL -DUSE_GRAPHICS # turn on debugging information and graphics
#CC_OPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_SQL # turn on debugging information and no graphics 

#Use this
CC_OPTS = -Wall -Wno-unused-variable -O3 -DUSE_SQL # turn on code optimisation and database access
#CC_OPTS = -Wall -Wno-unused-variable -O3 -DUSE_SQL -DPRINT_COUT # turn on code optimisation, database access and couts

DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d


MAIN_CC = CallZS.cc
MAIN_OBJ = $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC))
MAIN_EXE = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(MAIN_CC))
INCLUDE_CC = -I$(BOOST_INC) -I$(PQXX_INC) -I$(STM_INC) -I$(MIDAS_INC) -I$(ROOT_INC)
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(MAIN_CC))


.PHONY: all clean

all: $(MAIN_EXE)

clean:
	rm $(MAIN_OBJ) $(DEPFILES)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CC_OPTS) $(WARNING) $(DEPOPTS) $(INCLUDE_CC) -c $< -o  $@

$(BUILD_DIR)/bin/%.exe: $(BUILD_DIR)/obj/%.o
	echo "<**Linking**>"  $(notdir $@)
#	echo "g++ $(CC_OPTS) $^ -o $@ $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) $(MIDAS_LIBS) $(ROOT_LIBS) "
#	g++ $(CC_OPTS) $^ -o $@ $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) $(MIDAS_LIBS) $(ROOT_LIBS) 
	echo "g++ $(CC_OPTS) $^ -o $@ $(BOOST_LIBS) "
	g++ $(CC_OPTS) $^ -o $@ $(BOOST_LIBS) -L$(STMDAQ_ROOT)/build/lib -lZS

$(DEPFILES):
include $(wildcard $(DEPFILES))





