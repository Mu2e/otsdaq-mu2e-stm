#Import common Makefile definitions
include $(STMDAQ_ROOT)/Makefile.inc

# turn off some warnings and compile with fPIC for shared object
#CC_OPTS = -Wall
CC_OPTS = -Wall -Wno-unused-variable -O3 # turn on code optimisation
# -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC


INCLUDE_CC = -I$(ZMQ_INC) -I$(ROOT_INC)

all: $(BUILD_DIR)/bin/server.exe $(BUILD_DIR)/bin/client.exe $(BUILD_DIR)/bin/clientplusSuppression.exe

clean: 
	rm -rf $(BUILD_DIR)/obj/server.o $(BUILD_DIR)/obj/client.o $(BUILD_DIR)/obj/clientplusSuppression.o

$(BUILD_DIR)/obj/server.o :  server.cc 
	echo "<**Compiling server.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) server.cc -o $(BUILD_DIR)/obj/server.o

$(BUILD_DIR)/obj/client.o : client.cc
	echo "<**Compiling client.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) client.cc -o $(BUILD_DIR)/obj/client.o

$(BUILD_DIR)/obj/clientplusSuppression.o : clientplusSuppression.cc
	echo "<**Compiling clientplusSuppression.cc**>"
	g++ -c $(CC_OPTS) $(INCLUDE_CC) clientplusSuppression.cc -o $(BUILD_DIR)/obj/clientplusSuppression.o

$(BUILD_DIR)/bin/server.exe : $(BUILD_DIR)/obj/server.o
	echo "<**Linking server**>"
	echo "g++ $(BUILD_DIR)/obj/server.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/server.exe"
	g++ $(BUILD_DIR)/obj/server.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/server.exe

$(BUILD_DIR)/bin/client.exe : $(BUILD_DIR)/obj/client.o
	echo "<**Linking client**>"
	echo "g++ $(BUILD_DIR)/obj/client.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/client.exe"
	g++ $(BUILD_DIR)/obj/client.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/client.exe

$(BUILD_DIR)/bin/clientplusSuppression.exe : $(BUILD_DIR)/obj/clientplusSuppression.o
	echo "<**Linking clientplusSuppression**>"
	echo "g++ $(BUILD_DIR)/obj/clientplusSuppression.o $(ZMQ_LIBS) -o $(BUILD_DIR)/bin/clientplusSuppression.exe"
	g++ $(BUILD_DIR)/obj/clientplusSuppression.o -L $(ZMQ_LIBS) $(ROOT_LIBS) -o $(BUILD_DIR)/bin/clientplusSuppression.exe
