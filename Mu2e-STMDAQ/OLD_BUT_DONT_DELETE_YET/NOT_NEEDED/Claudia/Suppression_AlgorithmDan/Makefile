ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

include $(STMDAQ_ROOT)/Makefile.inc

WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated

CCOPTS = -O3 -DUSE_SQL
CCOPTS = -O3 
CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_SQL -DUSE_GRAPHICS # turn on debugging information and graphics
#CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_SQL # turn on debugging information and no graphics 

MAIN_CC = nonpeaksuppression_newDan.cc
MAIN_EXE = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(MAIN_CC))
INCLUDE_CC = -I$(BOOST_INC) -I$(PQXX_INC) -I$(STM_INC) -I$(MIDAS_INC) -I$(ROOT_INC)
DEPENDS := $(patsubst %.cc,%.d,$(MAIN_CC))

.PHONY: all clean

all: $(MAIN_EXE)

clean:
	touch $(MAIN_CC) 

-include $(DEPENDS)

$(BUILD_DIR)/obj/%.o: %.cc Makefile
	echo "<**Compiling**>" $<
	g++ $(CCOPTS) $(WARNING) $(INCLUDE_CC) -MMD -MP -c $< -o  $@

$(BUILD_DIR)/bin/%.exe: $(BUILD_DIR)/obj/%.o
	echo "<**Linking**>"  $(notdir $@)
	g++ $(CCOPTS) $^ -o $@ $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) $(MIDAS_LIBS) $(ROOT_LIBS) 




