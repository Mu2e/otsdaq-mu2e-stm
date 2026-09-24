ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

all : dirs utils_dir config_dir  hw_dir buffers_dir debug_dir processing_dir sim_dir frontend_dir  exes_dir# dqm_dir 

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
	if [ ! -e $(LOG_DIR) ] ; then \
		echo "<**Creating directory: log**>" ;\
		mkdir -p $(LOG_DIR) ;\
	fi

utils_dir:
	echo "<**Building utils directory**>"
	make -C utils all

config_dir :
	echo "<**Building configuration directory**>"
	make -C config all

hw_dir :
	echo "<**Building firmware/hardware directory**>"
	make -C hardware all

buffers_dir :
	echo "<**Building buffers directory**>"
	make -C buffers all

debug_dir :
	echo "<**Building debug directory**>"
	make -C debug all

processing_dir :
	echo "<**Building data processing directory**>"
	make -C processing all

dqm_dir :
	echo "<**Building DQM directory**>"
	make -C dqm all

sim_dir :
	echo "<**Building simulation directory**>"
	make -C sim all

frontend_dir:
	echo "<**Building frontend**>"
	make -C frontend all

exes_dir:
	echo "<**Building executables**>"
	make -C exes all

# Clean-up
.PHONY: clean
clean:
	rm -rf build/bin/ build/obj/ build/lib/ build/dep/
