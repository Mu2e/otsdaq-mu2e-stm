ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

all : dirs utils_code fmc120 fmc144 hw_code udp_testing process_data dqm_code exes # online_code hpge_sim frontend_code

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

fmc120 :
	echo "<**Building hardware:FMC120 directory**>"
	make -C hardwareScripts/FMC120 all

fmc144 :
	echo "<**Building hardware:FMC144 directory**>"
	make -C hardwareScripts/FMC144 all

hw_code :
	echo "<**Building hardware scripts directory**>"
	make -C hardwareScripts all

udp_testing :
	echo "<**Building UDPtesting directory**>"
	make -C UDPtesting lib

process_data :
	echo "<**Building data processing directory**>"
	make -C processData all

dqm_code :
	echo "<**Building DQM directory**>"
	make -C dqm all

frontend_code :
	echo "<**Building frontends directory**>"
	make -C frontends all

exes:
	echo "<**Building UDPtesting exes**>"
	make -C UDPtesting all


#online_code :
#	echo "<**Building online directory**>"
#	make -C online all


# Clean-up
.PHONY: clean
clean:
	rm -rf build/bin/ build/obj/ build/lib/ build/dep/
