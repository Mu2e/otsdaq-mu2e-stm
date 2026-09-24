ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

all : dirs utils_code udp_testing # dqm_code process_data  exes 

dirs :
	if [ ! -e $(BUILD_DIR)/lib ] ; then \
		echo "<**Creating directory: build/lib**>" ;\
		mkdir -p $(BUILD_DIR)/lib ;\
	fi
	if [ ! -e $(BUILD_DIR)/obj ] ; then \
		echo "<**Creating directory: build/obj**>" ;\
		mkdir -p $(BUILD_DIR)/obj ;\
	fi
	if [ ! -e $(BUILD_DIR)/bin ] ; then \
		echo "<**Creating directory: build/bin**>" ;\
		mkdir -p $(BUILD_DIR)/bin ;\
	fi
	if [ ! -e $(BUILD_DIR)/dep ] ; then \
		echo "<**Creating directory: build/dep**>" ;\
		mkdir -p $(BUILD_DIR)/dep ;\
	fi

utils_code:
	echo "<**Building utils directory**>"
	make -C utils all

udp_testing :
	echo "<**Building UDPtesting directory**>"
	make -C UDPtesting lib

process_data :
	echo "<**Building data processing directory**>"
	make -C processData all

dqm_code :
	echo "<**Building DQM directory**>"
	make -C dqm all

exes:
	echo "<**Building UDPtesting exes**>"
	make -C UDPtesting all

# Clean-up
.PHONY: clean
clean:
	rm -rf build/bin/ build/obj/ build/lib/ build/dep/
