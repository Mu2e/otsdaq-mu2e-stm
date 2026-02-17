#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
CC_OPTS = -Wall
# -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC


INCLUDE_CC = -I$(ZMQ_INC) -I$(ROOT_INC)

all: $(BUILD_DIR)/bin/zeromq_test_server1old.exe $(BUILD_DIR)/bin/zeromq_test_client1old.exe $(BUILD_DIR)/bin/zeromq_test_client2old.exe

clean: 
	rm -rf $(BUILD_DIR)/obj/zeromq_test_server1old.o $(BUILD_DIR)/obj/zeromq_test_client1old.o $(BUILD_DIR)/obj/zeromq_test_client2old.o

$(BUILD_DIR)/obj/zeromq_test_server1old.o :  zeromq_test_server1old.cc 
	echo "<**Compiling zeromq_test_server1old.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) zeromq_test_server1old.cc -o $(BUILD_DIR)/obj/zeromq_test_server1old.o

$(BUILD_DIR)/obj/zeromq_test_client1old.o : zeromq_test_client1old.cc
	echo "<**Compiling zeromq_test_client1old.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) zeromq_test_client1old.cc -o $(BUILD_DIR)/obj/zeromq_test_client1old.o

$(BUILD_DIR)/obj/zeromq_test_client2old.o :  zeromq_test_client2old.cc 
	echo "<**Compiling zeromq_test_client2old.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) zeromq_test_client2old.cc -o $(BUILD_DIR)/obj/zeromq_test_client2old.o

$(BUILD_DIR)/bin/zeromq_test_server1old.exe : $(BUILD_DIR)/obj/zeromq_test_server1old.o
	echo "<**Linking zeromq_test_server1old**>"
	echo "g++ $(BUILD_DIR)/obj/zeromq_test_server1old.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/zeromq_test_server1old.exe"
	g++ $(BUILD_DIR)/obj/zeromq_test_server1old.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/zeromq_test_server1old.exe

$(BUILD_DIR)/bin/zeromq_test_client1old.exe : $(BUILD_DIR)/obj/zeromq_test_client1old.o
	echo "<**Linking zeromq_test_client1old**>"
	echo "g++ $(BUILD_DIR)/obj/zeromq_test_client1old.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/zeromq_test_client1old.exe"
	g++ $(BUILD_DIR)/obj/zeromq_test_client1old.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/zeromq_test_client1old.exe

$(BUILD_DIR)/bin/zeromq_test_client2old.exe : $(BUILD_DIR)/obj/zeromq_test_client2old.o
	echo "<**Linking zeromq_test_client2old**>"
	echo "g++ $(BUILD_DIR)/obj/zeromq_test_client2old.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/zeromq_test_client2old.exe"
	g++ $(BUILD_DIR)/obj/zeromq_test_client2old.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/zeromq_test_client2old.exe
