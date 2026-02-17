#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

 # turn off some warnings and compile with fPIC for shared object
WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated -Wno-sign-compare

INCLUDE_CC = -I$(CACTUS_INC) -I$(BOOST_INC) -I$(PQXX_INC) -I$(STM_INC)

#OPTIMISE = -O3 -DUSE_SQL
#OPTIMISE = -O3
CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information	
#CCOPTS = -Wall -Wno-unused-variable -pg # turn on profiler information  
DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d

# If using the database
ifdef USE_SQL
	INCLUDE_CC = -I$(CACTUS_INC) -I$(BOOST_INC) -I$(PQXX_INC) -I$(STM_INC)	
else
	INCLUDE_CC = -I$(CACTUS_INC) -I$(BOOST_INC) -I$(STM_INC)	
endif

MAIN_CC_1 = cons.cc
MAIN_CC_2 = prod.cc
SOURCE_CC = $(MAIN_CC_1) $(MAIN_CC_2)
MAIN_EXE_1 = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(MAIN_CC_1))
MAIN_EXE_2 = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(MAIN_CC_2))
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(SOURCE_CC))

.PHONY: all clean

all: $(MAIN_EXE_1) $(MAIN_EXE_2)

clean: 
	rm $(MAIN_EXE_1) $(MAIN_EXE_2) $(DEPFILES)

$(MAIN_EXE_1) : $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC_1))
	echo "<**Linking**>"
	g++ $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC_1)) $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) -lrt -o $(MAIN_EXE_1)

$(MAIN_EXE_2) : $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC_2))
	echo "<**Linking**>"
	g++ $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC_2)) $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) -lrt -o $(MAIN_EXE_2)

$(LIB) : $(LIB_OBJ)
	echo "<***Building Library**>" $(notdir $(LIB))
	g++ -shared -o $(LIB) $(LIB_OBJ)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CCOPTS) $(DEPOPTS) $(WARNING) $(INCLUDE_CC) -c $< -o  $@

$(DEPFILES):
include $(wildcard $(DEPFILES))

