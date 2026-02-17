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

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// HW
#include "STMDAQ-TestBeam/hardwareScripts/initHW.hh"

// Gen data
#include "STMDAQ-TestBeam/UDPtesting/genData.hh"

// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)
static const uint chNum = 2;

// The number of threads
static uint thread_num;

// Send, Receive, Check, Form events, ZS, Prescale, Write, Distribute
static const int process_num = 8;

static const int SEND = 0;
static const int RECEIVE = 1;
static const int CHECK = 2;
static const int EVENTS = 3;
static const int ZS = 4;
static const int PRESCALE = 5;
static const int WRITE = 6;
static const int DISTRIBUTE = 7;


// Get the number of data queues (number of channels * processes [not send])
static const uint queue_num = chNum*(process_num-1);
// Get number of queue buffers
static const uint queue_buffer_num = queue_num*2;

// Doing those processes
static const bool process[process_num] = {true, // Send
					  true, // Receive
					  true, // Check
					  true, // Form Events
					  false, // ZS
					  false, // Prescale
					  false, // Write
					  false}; // Distribute

// Boolean to indicate receiving packets from firmware
bool fw = false;

// !!!! CHANGE THESE ONLY !!! //
// Number of heartbeats
static const uint32_t numberhb = 0xFFFF;
// Heartbeat length (multiple of 8ns) 
static const uint32_t deltahb = 0x30D4; 
// !!!!!!!!!!!!!!!!!!!!!!!!!! //

// The first packet number
uint32_t start_packet_num = 0;
// The first event number
uint32_t start_event_num = 0;
// Boolean to compare input/output data
bool compareOutput = true;
// Boolean to write data
bool writeData = false;
// Boolean to send data to ArtDaq over socket
bool sendToArtDaq = true;

// Instance of UDP socket class
UDPsocket udp[chNum];

// Instance of UDP socket class
UDPsocket sw_udp[chNum];
  
// Instance of UDP class for DQM
UDPsocket udp_dqm;


// // Instance of circular buffer queue to check data
// queue_buffer chkq;

// // Instance of circular buffer queue to form packets into events
// queue_buffer fevq;

// // Instance of zero suppresion queue to ZS event data
// queue_buffer zsuq;

// // Instance of circular buffer queue to prescale events
// queue_buffer pscq;

// // Instance of (final) circular buffer queue to distribute data
// queue_buffer dstq;

dataVars dvar;
// // Pointers to distribution event shared memeory queues
// bip::managed_shared_memory* priority_shm; // Event priority queue
// shm_queue *priority_shm_q;
// bip::managed_shared_memory* on_shm; // On-spill
// shm_queue *on_shm_q;
// bip::managed_shared_memory* off_shm; // Off-spill
// shm_queue *off_shm_q;
// bip::managed_shared_memory* ps_shm; // Prescaled event header
// shm_queue *ps_shm_q;

// The pointer to the shared memory segment                                
bip::managed_shared_memory* event_shm;
// The pointer to the shared queue                                         
shm_event_queue *event_queue;

// Generate data
//genData gen(numberhb,deltahb);
genData gen("/data1/claudia_simulation_binary_files/662keV_0.32mV/genData662keV_20kHz.bin");

// Instance of process data class
checkData chData;

// Instance of frontend threads class
FEthreads fe;

// Instance of IPBus Manager
IPBusManager* hw = new IPBusManager();

// Instance of HW class
initHW initHW_;

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

// The data pulled from the queue
int16_t *outData[chNum];

// Client function to send packets
void client(int chan, int16_t *data, int socket){

  // Notify user
  cout << "In send thread: channel " << chan << ", socket " << socket << endl;

  // Send data counter
  uint64_t sendCount = 0;

  // Define receive buffer as array of packets
  int16_t *snd_buffer[udp[chan].SENDMMSG_NUM];
  uint16_t *size_buffer = new uint16_t [udp[chan].SENDMMSG_NUM] ();
  for (int i = 0; i < udp[chan].SENDMMSG_NUM; i++){
    snd_buffer[i] = new int16_t [MAX_PACKET_LEN] ();
    size_buffer[i] = MAX_PACKET_SIZE;
  }

  // Get start time before packet sending
  auto start = chrono::steady_clock::now();

  // Define a message counter
  int msgNum = 0;

  // Number of cycles through the generated data
  int genCount = 0;
  
  // Loop over all packets to be sent 
  for (uint i = 0; i < packetNum; i++) {
    //    if (i != 0 && i % genNum == 0) genCount++;
    // Memcpy packet to buffer
    //    memcpy(&snd_buffer[msgNum],&data[i % genNum],sizeof(data[0]));
    memcpy(snd_buffer[msgNum],&data[i*MAX_PACKET_LEN],MAX_PACKET_SIZE);
    // if (genCount > 0){
    //   // Get the adjusted starting packet number
    //   uint32_t p_num = start_packet_num + i;
    //   // Adjust packet number
    //   snd_buffer[msgNum].data[0] = (uint32_t)p_num & 0xFFFF; // Extract lower 16 bits
    //   snd_buffer[msgNum].data[1] = ((uint32_t)p_num >> 16) & 0xFFFF; // Extract upper 16      
    // }
    // Increment message counter
    msgNum++;
    // Check if SENDMMMSG_NUM has been reached
    if (msgNum == udp[chan].SENDMMSG_NUM){
      // Sleep to mimic fw sending
      //      this_thread::sleep_for(5ms);
      // Send buffer
      udp[chan].send(snd_buffer,size_buffer,UDPsocket::SENDMMSG_NUM,socket);
      // Increase counter by number of sent messages
      sendCount += udp[chan].SENDMMSG_NUM;
      // Reset message counter
      msgNum = 0;
    }
    // Send any leftover packets at end of loop
    else if (i == packetNum-1){
      // Send remaining buffer
      udp[chan].send(snd_buffer,size_buffer,msgNum,socket);
      // Increase counter by number of sent messages
      sendCount += msgNum;
    }
  }

  // Get end time after packet sending
  auto end = chrono::steady_clock::now();
  // Find time taken to send all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = sendCount*MAX_PACKET_SIZE*1e-9;

  // Store speed as Gbits/s
  double sendSpeed = data_size*8/(timeNano*1e-9);

  // Notify user
  cout << "Channel = " << chan 
       << " [" << udp[chan].get_channel_name(chan)
       << "] sent " << sendCount
       << " packets [" << data_size
       << " Gbytes] at "<< sendSpeed << " Gbit/s" << endl;
  
}

// Function for final data pull to compare input data for test
void pull_data(int chan, UDPsocket &udp, queue_buffer &pullq){

  // Notify user
  cout << "In pull / compare thread: channel " << chan << endl;

  // Pull data counters                                                
  uint64_t count = 0;

  // Data buffer
  int16_t *data_buffer = new int16_t [queue_buffer::RING_BUFFER_LEN] ();

  // Define the return value of number of messages got from queue           
  uint64_t pull_len = 0; // Total length of data

  // Get start time before packet receiving                                 
  auto start = chrono::steady_clock::now();
  // Get end time after event distribution                                  
  auto end = chrono::steady_clock::now();

  // Infinite loop                                                          
  while(1){
    // End condition                                                        
    if (udp.timeout){
      // Exit infinite loop                                                 
      break;
    }
    // Else pull data from queue                                      
    pull_len = pullq.pull(&udp.timeout,chan,data_buffer);
    //    std::cout << "Pulled = " << pull_len*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl;
    // Restart the timing clock when first messages is pulled                                               
    if (count == 0) start = chrono::steady_clock::now();
    // Memcpy data to output arrays
     memcpy(&outData[chan][count],data_buffer,pull_len*sizeof(int16_t));
    // Increment distribute event counters
    count += pull_len;
    //    std::cout << "Total data pulled = " << count*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl;
    // Store new end time
    if (pull_len > 0) end = chrono::steady_clock::now();  
  }
  // Find time taken to distribute all events                               
  auto diff = end - start;
  // Get time in nanoseconds                                                
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Calculate the total data size in Gbytes                                                                
  double data_size = count*sizeof(int16_t)*1e-9;

  // Delay cout                                                             
  this_thread::sleep_for(4s);

  // Store speed as Gbits/s                                                                                 
  double speed = data_size*8/(timeNano*1e-9);

  // Notify user                                                            
  Logger::Instance()->write(1,"Channel = "
                            + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
                            + "] pulled "
                            + std::to_string(data_size)
                            + " Gbytes at "
                            + std::to_string(speed)
                            + " Gbit/s");

}

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

  // Write event data
  //  queue_write<dataVars::off_event> writeq(stm_file);

  // Get log file name
  std::string stm_log_file = stm_file+".log";

  // Logger
  Logger::Instance(Logger::DEBUG);
  Logger::Instance()->setStylePlain();
  Logger::Instance()->LogToFile("log/"+stm_log_file);
  Logger::Instance()->write(1,"Logger initialised");

  // Initialise all hardware
  //  initHW_.init(hw);

  // If generating data in software
  if (!fw){    
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


  // Get available cores
  cpu_set_t cpuset;
  sched_getaffinity(0, sizeof(cpuset), &cpuset);
  static const uint cores = CPU_COUNT(&cpuset) - 2; // (leave two for main + DQM)
  std::cout << "System detected " << cores << " available cores" << std::endl;

  // Number of threads (number of channels * number of processes + dqm thread)
  int thread_count = 1; // DQM thread
  for (int i = 0; i < chNum; i++){
    for (int j = 0; j < process_num; j++){
      if (process[j]){
  	thread_count++;
      }
    }
  }

  // Check if thread_count is <= number of available cores 
  if (thread_count > cores){
    std::cout << "ERROR: More threads to be initialised that available cores!" << std::endl;
    std::cout << "Exiting..." <<  std::endl;
    exit(0);
  }
  // Set as static const
  thread_num = thread_count;
  std::cout << "Initialising " << thread_num << " threads..." << std::endl;

  // Get the available memeory of the system
  std::cout << "Calculating memory allocation..." << std::endl;
  static const uint64_t free_mem = sysconf(_SC_AVPHYS_PAGES)*sysconf(_SC_PAGE_SIZE);

  // Leave 20% of free memory
  uint64_t buffer_mem = free_mem*0.8; 
  std::cout << "Available system memory  = " << free_mem*1e-9 << " Gbytes" << std::endl;
  std::cout << "Allocating 80% of available memory (" << buffer_mem*1e-9 << " Gbytes) to DAQ operations and buffers" << std::endl;
    
  // Initiliase sockets and get memory used for UDP kernel buffers
  int send_sock[chNum] = {}; // Send sockets (mimic fw)
  int recv_sock[chNum] = {}; // Receive sockets (from fw socket)
  int dist_sock[chNum] = {}; // Distribution socket (to ArtDAQ)x
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
     if (process[DISTRIBUTE]){
       dist_sock[i] = sw_udp[i].setupClient(i,SW);
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

  // Calculate the buffer size of each queue
  uint64_t queue_buffer_mem = buffer_mem/queue_buffer_num;
  std::cout << "Allocating " << queue_buffer_mem*1e-9 << " Gbytes to each queue buffer" << std::endl;

  // Now create and allocate queues
  for (int i = 0; i < chNum; i++){
    for (int j = 0; j < queue_num/chNum; j++){
      queue_buffer data_queue(queue_buffer_mem);
    }
  }
  exit(0);

  // std::cout << "Entered producer..." << std::endl;

  // // Remove any previous instance of shared memory
  // bip::shared_memory_object::remove(SHARED_MEMORY_NAME);

  // // Get shared memory (open only)                             
  // event_shm = new bip::managed_shared_memory(bip::open_or_create,
  // 					     SHARED_MEMORY_NAME,
  // 					     4294967296);

  // // Allocate the shared memory segment
  // shm_event_queue::allocator_type alloc(event_shm->get_segment_manager());

  // // Construct the shared memory queue
  // event_queue = event_shm->construct<shm_event_queue>(SHARED_QUEUE_NAME)(alloc);


  // bip::managed_shared_memory mysegment(bip::open_or_create,SHARED_MEMORY_NAME, 65536);

  // std::cout << "Created memory segment" << std::endl;

  // MySynchronisedQueue::allocator_type alloc(mysegment.get_segment_manager());
  // std::cout << "Allocated segment" << std::endl;
  // MySynchronisedQueue *myQueue = mysegment.construct<MySynchronisedQueue>(SHARED_QUEUE_NAME)(alloc);

  // std::cout << "Constructed queue" << std::endl;

  // int eventNum = 100;
  // // int eventSize = 10;
  // // int eventLen = eventSize/2;
  // dataVars::tevent testEvents [eventNum];
  // for (int i = 0; i < eventNum; i++){
  //   //    testEvents[i].size = eventSize;
  //   //    testEvents[i].data = new int16_t[eventLen];
  //   for (int j = 0; j < testEvents[i].size/2; j++){
  //     testEvents[i].data[j] = i + j;
  //   }
  // }
  
  // for(int i = 0; i < eventNum; ++i){
  //   std::cout << "Pushed " << i << " events" << std::endl;
  //   myQueue->push(testEvents[i]);
  // }

  // // Wait until the queue is empty: has been processed by client(s)                                               
  // while(myQueue->sizeOfQueue() > 0)
  //   continue;

  // exit(0);


  
  // Define threads
  std::thread *send_thread[chNum];
  // Standard threads
  std::thread *dqm_thread;
  std::thread *receive_thread[chNum];
  std::thread *check_thread[chNum];      
  std::thread *event_thread[chNum];      
  std::thread *zero_sup_thread[chNum];      
  std::thread *prescale_thread[chNum];      
  // std::thread *dist_thread[chNum];      
  // // Test threads
  // std::thread *write_monitor_thread[chNum];
  // std::thread *write_thread[chNum];
  // std::thread *write_push_thread[chNum];
  // Compare the output data against the input
  std::thread *pull_thread[chNum];      

  // Start DQM thread
  dqm_thread = new std::thread (&FEthreads::dqm_client, // Server funtion
				ref(fe), // Frontend threads class 
				dqm_sock, // UDP socket number
				std::ref(udp[0]), // UDP socket class 
				std::ref(udp_dqm)); // Check data buffer queue 
    
  // Loop over number of channels and start receive/getthreads
  for (uint i = 0; i < chNum; i++){   
    
    // Start receive data thread
    receive_thread[i] = new std::thread (&FEthreads::server, // Server funtion
    					 ref(fe), // Frontend threads class 
    					 i, // Channel number
    					 recv_sock[i], // UDP socket number
    					 std::ref(udp[i]), // UDP socket class 
    					 std::ref(chkq)); // Check data buffer queue 
    
    // Start check data thread
    check_thread[i] = new std::thread (&FEthreads::check_data, // Check data function
    				       ref(fe), // Frontend threads class
    				       i, // Channel number
    				       std::ref(udp[i]), // UDP socket class 
    				       std::ref(chkq), // Check data buffer queue
    				       std::ref(fevq)); // Form events buffer queue
    
    // Start form events thread
    event_thread[i] = new std::thread (&FEthreads::form_events, // Check data function
       				       ref(fe), // Frontend threads class
       				       i, // Channel number
       				       std::ref(udp[i]), // UDP socket class 
       				       std::ref(fevq), // Form events buffer queue
       				       std::ref(dstq)); // Zero suppress buffer queue

    // Start form events thread
    zero_sup_thread[i] = new std::thread (&FEthreads::zs_data, // Check data function
					  ref(fe), // Frontend threads class
					  i, // Channel number
					  std::ref(udp[i]), // UDP socket class 
					  std::ref(zsuq), // Zero suppress buffer queue
					  std::ref(pscq)); // Prescale buffer queue

    
    // Start precale data thread
    prescale_thread[i] = new std::thread (&FEthreads::prescale_data, // Check data function
     					  ref(fe), // Frontend threads class
     					  i, // Channel number
     					  std::ref(udp[i]), // UDP socket class 
     					  std::ref(pscq), // Prescale data buffer queue
     					  std::ref(dstq)); // Distribute data buffer queue
    
    // If running tests and comparing outut
    if (compareOutput){
      // Start pull data thread
      pull_thread[i] = new std::thread (&pull_data, // Pull data function
    					i, // Channel number
    					ref(udp[i]), // UDP socket class   
    					std::ref(dstq));
    }
    //   // Else if writing data to file
  //   else if (writeData){
  //     // Start write data monitor thread
  //     write_monitor_thread[i] 
  // 	= new std::thread(&queue_write<dataVars::off_event>::write_monitor_func,
  // 			  ref(writeq),
  // 			  i,
  // 			  ref(udp[i]),
  // 			  std::ref(writeq));       

  //     // Start write data thread
  //     write_thread[i] 
  // 	= new std::thread (&queue_write<dataVars::off_event>::write_data_func,
  // 			   ref(writeq),
  // 			   i,
  // 			   ref(udp[i]),
  // 			   std::ref(writeq));
   
  //     // Start write push thread
  //     write_push_thread[i]
  // 	= new std::thread (&FEthreads::write_data, // Push data to write function
  // 			   ref(fe), // Frontend threads class
  // 			   i, // Channel number
  // 			   ref(udp[i]), // UDP socket class  
  // 			   std::ref(dstq),
  // 			   std::ref(writeq));
  //   }
  //   // Else distibute data
  //   else{
  //     // Start distibute data thread
  //     dist_thread[i] = new std::thread (&FEthreads::distribute_data, // Distribute data function
  // 					ref(fe), // Frontend threads class
  // 					i, // Channel number
  // 					dist_sock[i], // UDP socket number
  // 					ref(udp[i]), // UDP socket class   
  // 					ref(sw_udp[i]), // Software UDP socket class
  // 					std::ref(dstq),
  // 					event_queue); // Distribute data buffer queue
   
   
  }    

  // }
  
  // If not using firmware, start send thread
  if (!fw){
    // Pause and countdown to sending
    for (int i = 0; i < 3; i++){
      cout << "Sending data in " << 3-i << endl;
      sleep(1);
    }	    
    // Loop over number of channels and send threads
    for (uint i = 0; i < chNum; i++){          
      send_thread[i] =  new std::thread (&client,i,send_packets[i],send_sock[i]);
    }    
  }
  
  // Join dqm thread
  dqm_thread->join();

  // Loop over number of channels and join threads
  for (uint i = 0; i < chNum; i++){    
  
    // Join client thread
    if (!fw) send_thread[i]->join();    

    // Join server thread
    receive_thread[i]->join();
    // Join check data thread
    check_thread[i]->join();    
    // Join form events thread
    event_thread[i]->join();    
    // Join form events thread
    zero_sup_thread[i]->join();    
    // Join prescale data thread
    prescale_thread[i]->join();   
    // If running tests and comparing outut
    if (compareOutput){
      pull_thread[i]->join();
    }
  //   // Else if writing data
  //   else if (writeData){
  //     // Join the write push thread
  //     write_push_thread[i]->join();
  //     // Join write thread
  //     write_thread[i]->join();
  //     // Join the monitor thread
  //     write_monitor_thread[i]->join();
  //   }
  //   // Else distribute data
  //   else{
  //     // Join distribute data thread
  //     dist_thread[i]->join();   
  //   }

  }

  // Check input/output data is identical if using queue
  // and not a stupidly high number of packets
  if (compareOutput){
    // Boolean to signal success
    bool success[chNum];
    // Loop over channels
    for (uint c = 0; c < chNum; c++){
      // Initialise as success
      success[c] = true;
      // Error counter
      uint64_t false_count = 0;
      // Counters
      uint64_t out_count = 0;
      uint64_t count = 0;      
      uint64_t event_count = 0;
      // Loop over all elements in input array
      while(count < event_array_len[c]){
	// Input array
	uint64_t event_start = count; // Event start index
	uint64_t event_num = dvar.get_event_number(event_data[c],count);
	uint16_t event_len = event_data[c][event_start+fw_tHdr::EvLen];
	// Output array
	uint64_t out_event_start = out_count;
	uint64_t out_event_num = dvar.get_event_number(outData[c],out_count);
	uint16_t out_event_len = outData[c][out_event_start+fw_tHdr::EvLen];
	// Check event numbers are the same
	if (out_event_num != event_num){
	  std::cout << "Error in compare output data!! Event numbers do not match in  channel " << c << std::endl;
	  std::cout << "Input event number = " << event_num << std::endl; 
	  std::cout << "Output event number = " << out_event_num << std::endl; 
	  std::cout << "Exiting..." << std::endl;
	  exit(0);
	}
 	// Loop over all header elements before length
	for (int i = 0; i < fw_tHdr::EvLen; i++){
	  // If not equal...
	  if (event_data[c][count+i] != outData[c][out_count+i]){
	    // Unsuccesful...
	    false_count++;
	    success[c] = false;
	  }
	}
	// Check event lengths are the same (if non-zero)
	if (out_event_len != event_len && out_event_len != 0){
	  std::cout << "Error in compare output data!! Event lengths do not match for event " << out_event_num << " in  channel " << c << std::endl;
	  std::cout << "Input data event length = " << event_len << std::endl; 
	  std::cout << "Output data event length = " << out_event_len << std::endl; 
	  std::cout << "Exiting..." << std::endl;
	}
 	// Loop over all header elements after length
	for (int i = fw_tHdr::EvLen+1; i < fw_tHdr_Len; i++){
	  // If not equal...
	  if (event_data[c][count+i] != outData[c][out_count+i]){
	    // Unsuccesful...
	    false_count++;
	    success[c] = false;
	  }
	}
 	// Loop over output event length
	if (out_event_len != 0){
	  for (int i = fw_tHdr_Len; i < fw_tHdr_Len+event_len; i++){
	    // If not equal...
	    if (event_data[c][count+i] != outData[c][out_count+i]){
	      // Unsuccesful...
	      false_count++;
	      success[c] = false;
	    }
	  }
	  // Increment event counter
	  event_count++;
	}
	// Increase counters
	count += fw_tHdr_Len + event_len;
	out_count += fw_tHdr_Len + out_event_len;
      }
      // Print results for each channel
      if (success[c]) std::cout << "CHANNEL " << c 
				<< " SUCCESS: Identical input/output for " << event_count << " events (" << numberhb << " headers). Prescale ~ " << round((double)numberhb/(double)event_count) << std::endl;
      if (!success[c]) std::cout << "CHANNEL " << c 
				 << " ERROR: Input/Output data not identical! " 
				 << false_count << "/" << event_array_len[c] 
				 << " elements = " << (double)false_count/(double)event_array_len[c]*100 
				 << "% incorrect..." << endl;
      
    }
  }
  
  return 0;
  
}

 
