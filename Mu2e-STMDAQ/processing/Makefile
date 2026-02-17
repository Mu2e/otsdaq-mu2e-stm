ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated  -Wno-sign-compare

# Compiler flags
CCOPTS = -O3 -march=native # Optimisation
#CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 # debugging
#CCOPTS = -Wall -Wno-unused-variable -pg # profiler
DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d

INCLUDE_CC = -I$(BOOST_INC) -I$(FFTW_INC) -I$(STM_INC)
LIB_CC = template_module.cc buffer_pool.cc udp.cc check_data.cc form_events.cc baseline.cc zero_suppress.cc noise.cc mwd.cc prescale.cc event_builder.cc write_manager.cc tcp.cc dqm_manager.cc operation_manager.cc thread_manager.cc 
LIB_OBJ = $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(LIB_CC))
LIB = $(BUILD_DIR)/lib/libProcessing.so 
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(LIB_CC))

.PHONY: all clean

all: $(LIB)

clean:
	rm $(LIB_OBJ) $(DEPENDS)

$(LIB) : $(LIB_OBJ)
	echo "<***Building Library**>" $(notdir $(LIB))
	g++ -shared -o $(LIB) $(LIB_OBJ) $(FFTW_LIBS)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CCOPTS) $(DEPOPTS) $(WARNING) $(INCLUDE_CC) -c $< -o  $@

$(DEPFILES):
include $(wildcard $(DEPFILES))

