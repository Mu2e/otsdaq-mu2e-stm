//////////////////////////////////////////////////////////////////////////////////
/// This module creates a UDP socket for 10G readout (main).  
/////////////////////////////////////////////////////////////////////////////////// 
/********************************************************************/

#include<iostream>
#include<fstream>
#include <vector>
#include <string>

#include <thread>
#include <mutex>

#include<string.h> //memset 
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close 

#include <time.h>
#include <math.h>

// Frontend threads
#include "STMDAQ-TestBeam/UDPtesting/fe_threads.hh"

// Test functions
#include "STMDAQ-TestBeam/UDPtesting/test_funcs.hh"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// HW
#include "STMDAQ-TestBeam/hardwareScripts/initHW.hh"

// Gen data
#include "STMDAQ-TestBeam/UDPtesting/genData.hh"

// DQM
#include "STMDAQ-TestBeam/dqm/dqm.hh"

// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)
static const uint chNum = 1;

// The number of threads
static uint thread_num;

// Send, Receive, Check, Form events, ZS, Prescale, Write, Distribute
static const int process_max = 9;

// Process map
static const int SEND = 0;
static const int RECEIVE = 1;
static const int CHECK = 2;
static const int EVENTS = 3;
static const int ZS = 4;
static const int PRESCALE = 5;
static const int WRITE = 6;
static const int DISTRIBUTE_RAW = 7;
static const int COMPARE = 8;

// Get number of queue buffers 
// One ring buffer for each queue
// Plus three buffers for each operation
// All multipled by the number of channels
static const uint buffer_max = chNum * 4 * process_max;

// Doing those processes
static const bool process[process_max] = {false, // Send
					  true, // Receive
					  true, // Check
					  true, // Form Events
					  false, // ZS
					  false, // Prescale
					  false, // Write
					  true, // Distribute
					  false}; // Compare - TESTING ONLY 

// Boolean to indicate receiving packets from firmware
bool fw = false;

// !!!! CHANGE THESE ONLY !!! //
// Number of heartbeats
static const uint32_t numberhb = 50000; //0xFFFF;
// Heartbeat length (multiple of 8ns) 
static const uint32_t deltahb = 0xD4;//0x30D4; 
// !!!!!!!!!!!!!!!!!!!!!!!!!! //

// The first packet number
uint32_t start_packet_num = 0;
// The first event number
uint32_t start_event_num = 0;
// Boolean to compare input/output data
bool compareOutput = true;
// Boolean to write data
bool writeData = false;

// Instance of UDP socket class
UDPsocket udp[chNum];

// Instance of UDP socket class
UDPsocket sw_udp[chNum];
  
// Instance of UDP class for DQM
UDPsocket udp_dqm;

// Generate data
genData gen(numberhb,deltahb);
//genData gen("/data1/claudia_simulation_binary_files/662keV_0.32mV/genData662keV_20kHz.bin");

// Instance of process data class
checkData chData;

// Instance of frontend threads class
FEthreads fe;

// Instance of IPBus Manager
IPBusManager* hw = new IPBusManager();

// Instance of HW class
initHW initHW_;

// Instance of test functions class
test_funcs testing;

// Convert Gbits to bytes
static const double bytes2Gbits = 8e-9;

// Total number of packets with data to generate
static const uint64_t genNum = 0xFFFF;

// The number of packets to send
uint64_t packetNum = 0;

// Define packet arrays for each channel
int16_t* send_packets[chNum];

// Initialise events
uint64_t event_array_len[chNum] = {};
int16_t *event_data[chNum];

// Test array to pull data from queue to compare input
int16_t *outData[chNum];

// Main function
int main(){
  
  // Get current date and time 
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t); 
  std::ostringstream oss;
  oss << std::put_time(&tm,"%Y-%m-%d_%H-%M-%S");
  string date_time = oss.str();

  // Get file name starter
  std::string stm_file = "stm-daq_"+date_time;
  
  // Get log file name
  std::string stm_log_file = stm_file+".log";

  // The log directory
  std::string mainOutDir = "/scratch/mu2e/mu2estm/";
  std::string log_dir = mainOutDir+"logs/";

  // Logger
  Logger::Instance(Logger::DEBUG);
  Logger::Instance()->setStylePlain();
  Logger::Instance()->LogToFile(log_dir+stm_log_file);
  Logger::Instance()->write(1,"Logger initialised");
  
  // Initialise all hardware
  //initHW_.init(hw);
  
  // If generating data in software
  if (process[SEND]){    
    std::cout << "Generating data and storing in memory..." << std::endl;
    // Initialise events
    for (uint c = 0; c < chNum; c++){
      // Generate events
      std::pair<uint64_t,int16_t*> events = gen.genEvents(c,numberhb);      
      // Store event array length
      event_array_len[c] = events.first;
      // Store event data
      event_data[c] = events.second;
      // Generate packets from events
      std::pair<uint64_t,int16_t*> packets = gen.genPackets(c,event_data[c],numberhb);
      // Store number of packets
      packetNum = packets.first;
      // Store packets to send
      send_packets[c] = packets.second;
      // Check the created packets
      chData.check_packets(c,packetNum*MAX_PACKET_LEN,send_packets[c]);   
      // If comparing output data, initialise output array
      if (compareOutput) outData[c] = new int16_t [event_array_len[c]];
    }

    // Print message about data to be sent
    cout << "Sending " << packetNum*MAX_PACKET_SIZE*1e-9 << " Gbytes as " << packetNum << " packets of size " << MAX_PACKET_SIZE*1e-3 << " kbytes" << endl;
   
    // If a reasonbaly small number of packets, calculate loop time
    if (packetNum <= 1e7){
      // Get time now
      auto start = chrono::steady_clock::now();
      // Loop through and touch all packets
      for (uint i = 0; i < packetNum; i++) send_packets[0][0] = send_packets[0][0];
      // Get time after loop
      auto end = chrono::steady_clock::now();
      // Find time taken to touch all packets
      auto diff = end - start;
      // Get time in nano seconds
      double timeNano = chrono::duration <double, nano> (diff).count();
      
      // Print time taken to loop through pakcets
      cout << "Time taken to loop through 1 * " << packetNum << " packets = " << timeNano*1e-6 << " ms" << endl;
      // Print speed to loop through pakcets
      cout << "Speed to loop through 1 * " << packetNum << " packets = " << packetNum*MAX_PACKET_SIZE*bytes2Gbits/(timeNano*1e-9) << " Gbit/s" << endl;
      
    }
  }

  // Get the available memeory of the system
  std::cout << "Calculating memory allocation..." << std::endl;
  static const uint64_t free_mem = sysconf(_SC_AVPHYS_PAGES)*sysconf(_SC_PAGE_SIZE);

  // Leave 5% of free memory
  uint64_t buffer_mem = free_mem*0.5; 
  std::cout << "Available system memory  = " << free_mem*1e-9 << " Gbytes" << std::endl;
  std::cout << "Allocating 50% of available memory (" << buffer_mem*1e-9 << " Gbytes) to DAQ operations and buffers" << std::endl;
    
  // Initiliase sockets and get memory used for UDP kernel buffers
  int send_sock[chNum] = {}; // Send sockets (mimic fw)
  int recv_sock[chNum] = {}; // Receive sockets (from fw socket)
  int dist_sock_raw[chNum] = {}; // Distribution socket (to ArtDAQ)x
  int dqm_sock = 0; // DQM socket
  
  //Socket buffer size variables
  socklen_t xx = sizeof(xx);
  uint64_t sock_mem = 0;
  
  // // Loop over number of channels and threads
  for (uint i = 0; i < chNum; i++){    
    
    // Send clients
    if (process[SEND]){
      send_sock[i] = udp[i].setupClient(i,FW);
    }
    // Receive servers
    if (process[RECEIVE]){
      recv_sock[i] = udp[i].setupServer(i,FW);
    }
    // Distribute clients
    if (process[DISTRIBUTE_RAW]){
      dist_sock_raw[i] = sw_udp[i].setupClient(i,SW);
    }
    
    // Increase used socket memory
    sock_mem += udp[i].sock_mem;
    sock_mem += sw_udp[i].sock_mem;
    
  }
  
  // DQM client clients
  dqm_sock = udp_dqm.setupClient(0,SW);
  sock_mem += udp_dqm.sock_mem;
  
  // Subtract from available memory
  std::cout << "Allocating " << sock_mem*1e-9 << " Gbytes to all UDP socket kernel buffers" << std::endl;
  buffer_mem -= sock_mem;  

  // Process counter
  int process_count = 0;   
  for (int i = 0; i < process_max; i++) if (process[i]) process_count++;
  // Calculate the actual number of process
  static const int process_num = process_count; 
  // Create array of process identifiers
  int processID[process_num] = {};
  process_count = 0;   
  for (int i = 0; i < process_max; i++){
    if (process[i]){
      processID[process_count] = i;
      process_count++;
    }
  }
  
  // Calculate the number of buffers needed
  static const int buffer_num = buffer_max/process_max*process_num;

  // Calculate the buffer size of each queue
  static const uint64_t buffer_size = buffer_mem/buffer_num;
  uint64_t buffer_len = buffer_size/sizeof(int16_t);
  std::cout << "Allocating " << buffer_size*1e-9 << " Gbytes each to " << buffer_num << " data buffers..." << std::endl;

  // Now create and allocate queues
  queue_buffer *process_queue[process_num];
  for (int i = 0; i < process_num; i++){
    process_queue[i] = new queue_buffer (buffer_len);
  }

  // Write event data
  queue_write *writeq;
  
  //if(process[WRITE]){
  writeq = new queue_write(stm_file, buffer_len);
  //}
  
  // Initialise classes
  //Instance of form events data class
  formEvents ev(buffer_len);

  //Instance of form events data class
  daqZS zs(buffer_len);

  // Instance of prescale class
  prescaleData ps;

  // Instance of DQM class
  stmDQM dqm(udp,hw,chData,ev,zs,ps,writeq);

  // Get available cores
  cpu_set_t cpuset;
  sched_getaffinity(0, sizeof(cpuset), &cpuset);
  static const uint cores = CPU_COUNT(&cpuset) - 2; // (leave two for main + DQM)
  std::cout << "System detected " << cores << " available cores" << std::endl;
  
  // Number of threads (number of channels * number of processes + dqm thread)
  int thread_count = 1; // DQM thread
  for (int i = 0; i < chNum; i++){
    for (int j = 0; j < process_num; j++){
      thread_count++;
    }
  }
  
  // Check if thread_count is <= number of available cores 
  if (thread_count > cores){
    std::cout << "ERROR: More threads to be initialised than available cores!" << std::endl;
    std::cout << "Exiting..." <<  std::endl;
    exit(0);
  }
  // Set as static const
  thread_num = thread_count;

  // Start threads
  std::cout << "Initialising " << thread_num << " threads..." << std::endl;

  // Start DQM thread first                                              
  std::thread *dqm_thread = new std::thread (&stmDQM::dqm_client, // dqm
  					     ref(dqm), // Frontend threads class
  					     dqm_sock, // UDP socket number
  					     std::ref(udp_dqm)); // DQM udp
  std::thread *write_monitor_thread[chNum];
  std::thread *write_thread[chNum];

  // Start all other threads 
  std::thread **process_thread[process_num];
  // Loop over number of processes and start threads for each channel  
  for (int i = 0; i < process_num; i++){          
    process_thread[i] = new std::thread* [chNum];
    // Don't start send thread now...
    if (processID[i] != SEND){
      // Loop over processes
      for (int j = 0; j < chNum; j++){      	  
	// If intialising receive data process
	if (processID[i] == RECEIVE){
	  // Start receive data thread
	  process_thread[i][j] = new std::thread (&FEthreads::server, // Server funtion
						  ref(fe), // Frontend threads class 
						  j, // Channel number
						  recv_sock[j], // UDP socket number
						  std::ref(udp[j]), // UDP socket class 
						  std::ref(process_queue[i])); // Push queue
	}	
	// If intialising check data process
	if (processID[i] == CHECK){
	  // Start check data thread
	  process_thread[i][j] = new std::thread (&FEthreads::check_data, // Check data function
						  ref(fe), // Frontend threads class
						  j, // Channel number
						  std::ref(udp[j]), // UDP socket class 
						  process_queue[i-1], // Check data buffer queue
						  process_queue[i]); // Form events buffer queue
	  
	}
	// If intialising form events process
	if (processID[i] == EVENTS){
	  process_thread[i][j] = new std::thread (&FEthreads::form_events, // Check data function
	     					  ref(fe), // Frontend threads class
	     					  j, // Channel number
	     					  std::ref(ev), // Form events class
	     					  std::ref(udp[j]), // UDP socket class 
	     					  process_queue[i-1], // Form events buffer queue
	     					  process_queue[i]); // Zero suppress buffer queue
	  
	}
	// If intialising zero suppression process
	  if (processID[i] == ZS){
	    process_thread[i][j] = new std::thread (&FEthreads::zs_data, // Check data function
						    ref(fe), // Frontend threads class
						    j, // Channel number
						    std::ref(zs), // Zero suppress class
						    std::ref(udp[j]), // UDP socket class 
						    process_queue[i-1], // Zero suppress buffer queue
						    process_queue[i]); // Prescale buffer queue
	    
	  }
	  // If intialising prescale process
	  if (processID[i] == PRESCALE){
	  }	  

	  // Else if writing data to file
	  if (processID[i] == WRITE){
	    // Start write data monitor thread
	    write_monitor_thread[j] 
	      = new std::thread(&queue_write::write_monitor_func,
				writeq,
				j,
				ref(udp[j]));
	    
	    // Start write data thread
	    write_thread[j] 
	      = new std::thread (&queue_write::write_data_func,
				 writeq,
				 j,
				 ref(udp[j]));
	    
	    // Start write push thread
	    process_thread[i][j]
	      = new std::thread (&FEthreads::write_data, // Push data to write function
				 ref(fe), // Frontend threads class
				 j, // Channel number
				 buffer_len, // Size of buffer for pull
				 ref(udp[j]), // UDP socket class  
				 process_queue[i-1],
				 writeq);
	  }
	  
	  // If intialising distribute to artDAQ 
	  if (processID[i] == DISTRIBUTE_RAW){
	    process_thread[i][j] = new std::thread (&FEthreads::distribute_raw, // Check data function
						    ref(fe), // Frontend threads class
						    j, // Channel number
						    dist_sock_raw[j],
						    std::ref(udp[j]), // UDP socket class 
						    std::ref(sw_udp[j]),
						    process_queue[i-1], // Distribute queue
						    buffer_len
						    );
	    
	  }
	  
	  
	  // If initialising compare process - TESTING ONLY
	  if (processID[i] == COMPARE){
	    // Start pull data thread
	    process_thread[i][j] = new std::thread (&test_funcs::pull_data, 
						    std::ref(testing),
						    j, // Channel number
						    ref(udp[j]), // UDP 
						    process_queue[i-1],
						    buffer_len,
						    outData[j]);	    
	  }	  
	  
	  
      } // End loop over channels
      
    } // End if not sent
    
  }  
  

  // If sending
  if (process[SEND]){      
    // Pause and countdown to sending
    for (int i = 0; i < 3; i++){
      cout << "Sending data in " << 3-i << endl;
      sleep(1);
    }	    
    // Start send data threads
    for (int j = 0; j < chNum; j++){      
	process_thread[SEND][j] = new std::thread (&test_funcs::client,
						   std::ref(testing),
						   j,
						   std::ref(udp[j]),
						   send_sock[j],
						   send_packets[j],
						   packetNum);
    }	
  }	

  // Join threads
  for (int i = 0; i < process_num; i++){       
    // Loop over processes
    for (int j = 0; j < chNum; j++){      
      process_thread[i][j]->join();
    }
  }  
 
  // Join dqm thread
  dqm_thread->join(); 
  
  // Join write threads
  if (process[WRITE]){
    for (int j = 0; j < chNum; j++){      
      write_monitor_thread[j]->join();
      write_thread[j]->join();
    }    
  }
  // // Define threads
  // // Standard threads
  // std::thread *prescale_thread[chNum];      
  // // std::thread *dist_thread[chNum];      
  // // // Test threads
  // // std::thread *write_monitor_thread[chNum];
  // // std::thread *write_thread[chNum];
  // // std::thread *write_push_thread[chNum];

  // // Loop over number of channels and start receive/getthreads
  // for (uint i = 0; i < chNum; i++){   
    
  //   // Start precale data thread
  //   prescale_thread[i] = new std::thread (&FEthreads::prescale_data, // Check data function
  //    					  ref(fe), // Frontend threads class
  //    					  i, // Channel number
  //    					  std::ref(udp[i]), // UDP socket class 
  //    					  std::ref(pscq), // Prescale data buffer queue
  //    					  std::ref(dstq)); // Distribute data buffer queue
    
  // Else if writing data to file
  // //   else if (writeData){
  // //     // Start write data monitor thread
  // //     write_monitor_thread[i] 
  // // 	= new std::thread(&queue_write<dataVars::off_event>::write_monitor_func,
  // // 			  ref(writeq),
  // // 			  i,
  // // 			  ref(udp[i]),
  // // 			  std::ref(writeq));       

  // //     // Start write data thread
  // //     write_thread[i] 
  // // 	= new std::thread (&queue_write<dataVars::off_event>::write_data_func,
  // // 			   ref(writeq),
  // // 			   i,
  // // 			   ref(udp[i]),
  // // 			   std::ref(writeq));
   
  // //     // Start write push thread
  // //     write_push_thread[i]
  // // 	= new std::thread (&FEthreads::write_data, // Push data to write function
  // // 			   ref(fe), // Frontend threads class
  // // 			   i, // Channel number
  // // 			   ref(udp[i]), // UDP socket class  
  // // 			   std::ref(dstq),
  // // 			   std::ref(writeq));
  // //   }
  // //   // Else distibute data
  // //   else{
  // //     // Start distibute data thread
  // //     dist_thread[i] = new std::thread (&FEthreads::distribute_data, // Distribute data function
  // // 					ref(fe), // Frontend threads class
  // // 					i, // Channel number
  // // 					dist_sock[i], // UDP socket number
  // // 					ref(udp[i]), // UDP socket class   
  // // 					ref(sw_udp[i]), // Software UDP socket class
  // // 					std::ref(dstq),
  // // 					event_queue); // Distribute data buffer queue

  std::cout << "DAQ stopped" << std::endl;
   
   
  // Check input/output data is identical 
  if (process[COMPARE]){
    testing.compare_data(chNum,numberhb,event_array_len,event_data,outData);
  }

  return 0;
  
}

 
