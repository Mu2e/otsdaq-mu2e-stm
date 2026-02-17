#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

 # turn off some warnings and compile with fPIC for shared object
CC_OPTS = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated

#OPTIMISE = -O3 -DUSE_SQL
OPTIMISE = -O3

MAIN_CC = testFE_circBuff.cc
MAIN_EXE = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(MAIN_CC))
INCLUDE_CC = -I$(CACTUS_INC) -I$(BOOST_INC) -I$(PQXX_INC) -I$(STM_INC)
DEPENDS := $(patsubst %.cc,%.d,$(SOURCE_CC))

.PHONY: all clean

all: $(MAIN_EXE)

clean: 
	rm $(MAIN_EXE) $(BUILD_DIR)/obj/testFE.o

-include $(DEPENDS)

$(BUILD_DIR)/obj/%.o: %.cc Makefile
	echo "<**Compiling**>" $<
	g++ $(CC_OPTS) $(OPTIMISE) $(INCLUDE_CC) -MMD -MP -c $< -o  $@

$(BUILD_DIR)/bin/%.exe: $(BUILD_DIR)/obj/%.o
	echo "<**Linking**>"  $(notdir $@)
	g++ $(CCOPTS) $^ -o $@ $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS)

