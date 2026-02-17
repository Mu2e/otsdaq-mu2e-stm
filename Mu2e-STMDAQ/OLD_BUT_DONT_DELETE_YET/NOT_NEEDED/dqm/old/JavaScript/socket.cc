#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "zlib.h"
#include "midas.h"
#include <iostream>
#include <fstream>
#include "string.h"
#include <string>
#include <vector>
#include <stdlib.h>
#include <errno.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include "midas_to_csv.hh"
#include <zmq.hpp>
#include <math.h>
#include <unistd.h>


void disconnect(){
  cm_disconnect_experiment();
}

int main(int argc, char *argv[]){
  // Define environment variables
  INT status, i;

  // Disconnect from any previous experiments
  cm_disconnect_experiment();

  // Determine the host and experiment names. This function retrieves the environment variables MIDAS_SERVER_HOST and MIDAS_EXPT_NAME, of which only the latter is defined.
  // host_name will be empty for experiments running on the local computer, otherwise they will be deifned using the host's IP.
  char host_name[HOST_NAME_LENGTH], exp_name[NAME_LENGTH];
  status = cm_get_environment(host_name, HOST_NAME_LENGTH, exp_name, NAME_LENGTH);

  // Check if name check successful
  if(status == CM_SUCCESS){
    std::cout << "Host name: " << host_name 
	      << "\nExperiment name: " << exp_name << std::endl;
  }
  else{
    std::cout << "Could not retrieve the environment variables. Exiting with error code " 
	      << status << std::endl;
    disconnect();
    return 1;
  }


  // TODO: Do something with this.
  /*
  // parse command line parameters
  for (i=1 ; i<argc ; i++){
    if (argv[i][0] == '-')
      {
	if (i+1 >= argc || argv[i+1][0] == '-'){goto usage;}
	if (argv[i][1] == 'e'){strcpy(exp_name, argv[++i]);}
	else if (argv[i][1] == 'h'){strcpy(host_name, argv[++i]);}
	else{
	usage:
	  printf("usage: test [-h Hostname] [-e Experiment]\n\n");
	  return 1;
	}
      }
  }
  */

  // Assign the analyzer name
  std::string analyzer_name = "plotting_connection";

  // Connect to the experiment via the mserver
  std::string host_name_str(host_name), exp_name_str(exp_name);
  status = cm_connect_experiment(host_name, exp_name, analyzer_name.c_str(), 0);

  // Check if connection successful
  if (status != CM_SUCCESS){
    std::cout << "Failed to connect to experiment " << exp_name_str
	      << " on " << host_name_str
	      << " . Exiting with error code " << status << std::endl;
    disconnect();
    return 1;
  }

  // State where the experiment is defined
  if (host_name_str != ""){
    std::cout << "Connected to experiment " << exp_name_str
	      << " running on " << host_name_str << "." << std::endl;
  }
  else{
    std::cout << "Connected to experiment " << exp_name_str
	      << " running on localhost.\n" << std::endl;
  }

  // Supposedly the MIDAS has taken control of ctrl+c interrupt. This can be circumvented by working with ss_ctrlc_handler(<function ptr>).
  // TODO: Implement the ctrlc handler.


  // Open the event buffer SYSTEM (32M)
  // The event buffer size set by ODB/Experiment/Buffer sizes/System
  HNDLE online_buffer;
  status = bm_open_buffer(EVENT_BUFFER_NAME, DEFAULT_BUFFER_SIZE, &online_buffer);

  // Check if the buffer was opened successfully
  if(!(status == BM_SUCCESS || status == BM_CREATED)){
    std::cout << "Failed to open the new buffer " << EVENT_BUFFER_NAME
	      << ". Exiting with error code " << status << std::endl;
    disconnect();
    return 1;
  }
  // State the buffer parameters
  std::cout << "Connected to buffer " << EVENT_BUFFER_NAME
	    << " of size " << DEFAULT_BUFFER_SIZE << " bytes ("
	    << DEFAULT_BUFFER_SIZE/(1024*1024) << " MB)." << std::endl;

  // Request events to be sent over
  // TODO - change the sampling type to GET_ALL and make it work that way.
  INT event_id;
  status = bm_request_event(online_buffer, EVENTID_ALL, TRIGGER_ALL, GET_NONBLOCKING, &event_id, NULL);
  // Check if the event request was successful
  if (status != BM_SUCCESS){
    std::cout << "Failed to setup event requests. Exiting with error code " << status << std::endl;
    disconnect();
    return 1;
  }

  // Register the starting MIDAS transitions. sequence_number 500 is the default value and these transitions will have priority equal to those of other clients (frontends).
  status = cm_register_transition(TR_START, NULL, 500);
  // Check if the transition was successful.
  if(status != CM_SUCCESS){
    std::cout << "Error in requesting starting transitions in MIDAS. Exiting with error code "
	      << status << "." << std::endl;
    disconnect();
    return 1;
  }
  // Register the stop MIDAS transitions.
  status = cm_register_transition(TR_STOP, NULL, 500);
  // Check if the registration was successful.
  if(status != CM_SUCCESS){
    std::cout << "Error in requesting ending transitions in MIDAS. Exiting with error code "
	      << status << "." << std::endl;
    disconnect();
    return 1;
  }

  // Get experiment ODB
  HNDLE ODB_handle;
  status = cm_get_experiment_database(&ODB_handle, 0);
  // Check if the ODB was retrieved successfully.
  if(status != CM_SUCCESS){
    std::cout << "Failed to retrieve the ODB handle. Exiting with error code " << status << std::endl;
    disconnect();
    return 1;
  }

  // Retrieve the current run number
  INT current_run_number, INT_SIZE(sizeof(INT));
  status = db_get_value(ODB_handle, 0, "/Runinfo/Run number", &current_run_number, &INT_SIZE, TID_INT, FALSE);
  // Check if the run number was retrieved successfully.
  if(status != DB_SUCCESS){
    std::cout << "Failed to retrieve the current run number. Exiting with error code "
	      << status << std::endl;
    disconnect();
    return 1;
  }else{
    std::cout << "The current run number is " << current_run_number << "." << std::endl;
  }

  // Check the buffer for size
  DWORD buffer_size;
  INT DWORD_SIZE(sizeof(DWORD));
  std::string buffer_ODB_path = "/Experiment/Buffer sizes/" + static_cast<std::string>(EVENT_BUFFER_NAME);
  status = db_get_value(ODB_handle, 0, buffer_ODB_path.c_str(), &buffer_size, &DWORD_SIZE, TID_DWORD, FALSE);
  // State whether the buffer size was retrieved successfully.
  if (status != DB_SUCCESS){
    std::cout << "Could not retrieve the current buffer size. Exiting with error code "
	      << status << std::endl;
    disconnect();
    return 1;
  }else{
    std::cout << "The size of the buffer is " << buffer_size << " bytes ("
	      << buffer_size/(1024*1024) << " MB)." << std::endl;
  }

  // Create the buffer
  std::vector<char> buffer;
  // Modify the buffer size to that retrieved from the ODB. This can be resized as an ODB parameter.
  if(buffer.size() < buffer_size){buffer.resize(buffer_size);}

  // Timeout the system if not all of the data should be pushed.
  // unsigned online_timeout = 10;
  // status = cm_yield(online_timeout); // This line sleeps the system for a fixed amount of time. This will be needed for non-continuous data collection.

  // Getting SS_TIMEOUT here.
  // if (status != CM_SUCCESS){std::cout << "The connection timeout has failed. Exiting withh error status " << status << "." << std::endl; return 1;}
  // else{std::cout << "Timed out for " << online_timeout << "ms." << std::endl;}

  // Check for a transition using cm_query_transition.
  int trans_id, trans_run_num, trans_time;
  status = cm_query_transition(&trans_id, &trans_run_num, &trans_time);
  // Check if a transition has been found.
  if (status == CM_SUCCESS){std::cout << "Found a transition." << std::endl;}

  // Create a storage for the extracted data
  BANK_DATA data, *p_data = &data;
  uint32_t count = 0;
  std::string bank_name = "HPGe";
  size_t len_msg = 0;

  // Setup the ZMQ socket to push data
  zmq::context_t context (1);
  zmq::socket_t socket (context, ZMQ_REP);
  // Bind the socket to a node on the localhost.
  std::string localhost_node = "5556";
  socket.bind("tcp://*:"+localhost_node);
  std::cout << "Bound the socket to node 5556." << std::endl;

  while(true){
    // Read an event from the socket
    INT buffer_size_int(buffer_size);
    // Last parameter of bm_receive_event should be TRUE/FALSE.
    status = bm_receive_event(online_buffer, buffer.data(), &buffer_size_int, FALSE); 
    // Running tests
    if(status != BM_SUCCESS){
      std::cout << "Could not receive an event from this socket. Exiting with error code " << status << std::endl; disconnect();
      break;
    }
    if(buffer_size_int < static_cast<INT>(sizeof(EVENT_HEADER))){
      std::cout << "An error has occurred with reading the data. Have expected a minimum size of "
		<< sizeof(EVENT_HEADER) << " bits. Have read " << buffer_size_int
		<< " bits. Exiting." << std::endl;
      disconnect();
      return 1;
    }

    // Extract data from bank
    EVENT_HEADER *p_event = (EVENT_HEADER*)buffer.data();
    BANK_HEADER *p_bank = (BANK_HEADER*)(p_event + 1);
    DWORD bank_type = 0, bank_length = 0;
    union{uint16_t* bank_data; void* bank_void_data;};

    // Extract the bank data
    status = bk_find(p_bank, bank_name.c_str(), &bank_length, &bank_type, &bank_void_data);
    if (status != CM_SUCCESS){std::cout << "Bank " << bank_name << " not found in header." << std::endl; break;}
    std::vector<uint16_t> data_elements(bank_data, bank_data + (rpc_tid_size(bank_type)/sizeof(char))*bank_length);

    // Send the bank data over the TCP socket 
    socket.recv(request, zmq::recv_flags::none);
    if (pulse_num != 0){
      len_msg = std::ceil(std::log10(pulse_num))+1;
    }else{
      len_msg = 1;
    }
    char _pulse_num[len_msg];
    zmq::message_t message(len_msg);
    memcpy((char*) message.data(), data_elements[2], len_msg);
    socket.send(message, 0);

    // Reset the buffers
    buffer.clear();
    data_elements.clear();

    // Reset the counters
    count++;
    if(count > 100000){break;}
  }
  p_data->export_extracted_data("socket_test");

  // Disconnect from the experiment
  disconnect();
  return 0;
}
