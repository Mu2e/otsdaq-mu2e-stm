#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
#CC_OPTS = -Wall
CC_OPTS = -Wall -Wno-unused-variable -O3 # turn on code optimisation
# -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC


INCLUDE_CC = -I$(ZMQ_INC) -I$(ROOT_INC)

all: $(BUILD_DIR)/bin/gensendData.exe $(BUILD_DIR)/bin/receiveData.exe $(BUILD_DIR)/bin/receivesuppData.exe

clean: 
	rm -rf $(BUILD_DIR)/obj/gensendData.o $(BUILD_DIR)/obj/receiveData.o $(BUILD_DIR)/obj/receivesuppData.o

$(BUILD_DIR)/obj/gensendData.o :  gensendData.cc 
	echo "<**Compiling gensendData.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) gensendData.cc -o $(BUILD_DIR)/obj/gensendData.o

$(BUILD_DIR)/obj/receiveData.o : receiveData.cc
	echo "<**Compiling receiveData.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) receiveData.cc -o $(BUILD_DIR)/obj/receiveData.o

$(BUILD_DIR)/obj/receivesuppData.o : receivesuppData.cc
	echo "<**Compiling receivesuppData.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) receivesuppData.cc -o $(BUILD_DIR)/obj/receivesuppData.o

$(BUILD_DIR)/bin/gensendData.exe : $(BUILD_DIR)/obj/gensendData.o
	echo "<**Linking gensendData**>"
	echo "g++ $(BUILD_DIR)/obj/gensendData.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/gensendData.exe"
	g++ $(BUILD_DIR)/obj/gensendData.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/gensendData.exe

$(BUILD_DIR)/bin/receiveData.exe : $(BUILD_DIR)/obj/receiveData.o
	echo "<**Linking receiveData**>"
	echo "g++ $(BUILD_DIR)/obj/receiveData.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/receiveData.exe"
	g++ $(BUILD_DIR)/obj/receiveData.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/receiveData.exe

$(BUILD_DIR)/bin/receivesuppData.exe : $(BUILD_DIR)/obj/receivesuppData.o
	echo "<**Linking receivesuppData**>"
	echo "g++ $(BUILD_DIR)/obj/receivesuppData.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/receivesuppData.exe"
	g++ $(BUILD_DIR)/obj/receivesuppData.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/receivesuppData.exe
