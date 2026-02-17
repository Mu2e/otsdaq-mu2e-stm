///////////////////////////////////////////////////////////////////////////////////
/// Test FE module
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

#include<iostream>
#include<fstream>
#include <vector>

#include <thread>
#include <mutex>

#include<string.h> //memset 
#include<arpa/inet.h>
#include <sys/types.h>
#include<sys/socket.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close 

#include <time.h>
#include <math.h>

// Data variables information                                          
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// Hex reader                                                          
#include "STMDAQ-TestBeam/utils/Hex.hh"

// uhal                                                                
#include "uhal/uhal.hpp"
#include "uhal/ConnectionManager.hpp"

// Hardware scripts                                                    
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"
#include "STMDAQ-TestBeam/hardwareScripts/HWcomms.hh"

// UDP
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// XML interface                                                       
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Binary file code                                                    
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

// HPGe Simulation                                                     
#include "STMDAQ-TestBeam/HPGeSim/HPGeSim.hh"

// Circular buffer queue
#include "STMDAQ-TestBeam/utils/queue.hh"

using namespace uhal;

// Emulating data boolean                                              
bool simulation = true;
bool emulateADC = true;
// Boolean to signal debugging the fw                                  
bool debug_fw = true;

// Number of data channels (Maximum of 2: HPGe = 0, LaBr = 1)                
static const uint chNum = 2;

// Declare IPBusManager                                                
IPBusManager* hw = new IPBusManager();

// Define class instances                      
HWcomms hw_comms; // Hardware Communications
dataVars inVars; // Data Variables
HPGeSim sim; // HPGe Simulator
BinaryFile bf[chNum]; // Binary File
UDPsocket UDPin[chNum]; // UDP socket (in / receive data)
UDPsocket UDPout[chNum]; // UDP socket (out / send data)
queue_buffer cbq[chNum]; // Circular buffer queue class

// Define the XML reader instance                                      
string xml_path;

// UDP socket number                                                   
int readSock[chNum];

// Experimental configuration variable struct                          
struct expConfig expC;

// Total data size written                                             
uint64_t totalDataSize[chNum] = {};

// Put counter                                                        
uint64_t recvCount[chNum] = {};
// Pull counter                                                       
uint64_t pullCount[chNum] = {};

// Boolean to signal exit FE                                           
bool exitFE = false;

// Timers                                                              
auto start = std::chrono::system_clock::now();
auto now = std::chrono::system_clock::now();

/*-- Frontend Init -------------------------------------------------*/

int frontend_init() {

  // Open the XML file and instantiate the xml_file object             
  xml_path = EnvVars::expand("${STM_XML}");
  cout << "XML_PATH = " << xml_path << std::endl;
  xml_file = new Xml(xml_path);

  // Set struct experimental configuration variables from xml filer    
  expC = inVars.getXMLvalues(expC);

  // Check experimental configuration variables                        
  if (inVars.checkExpConfig(expC) == 0){
    cout << "Exiting due to bad value in " << xml_path << endl;
    exit(0);
  }
  cout << "Parameters in " << xml_path << " are okay!" << endl;

  // Print to user
  printf("Initalising frontend...!\n");

  // Print to user
  printf("Acquiring data source...\n");

  // If not simulation...
  if (!simulation){

    // Print to user
    printf("Connecting device through IPBusManager...");
  
    // Pull hardware connection
    hw = new IPBusManager(hw->getConnectionFile(),hw->getDeviceID());

    // Initialise ADC                                                  
    //    if (hw_comms.initADC(hw) == 1)                               
    //{                                                                
    if (1){

      // Ping firmware interfaces on initialisation                    
      hw_comms.pingInterfaces();

      // Set firmware registers from xml config file                   
      hw_comms.setRegisters(hw);

      // Start and reset the external clock counter                    
      hw_comms.startResetClock(hw);

      // Reset trigger FIFOs                                           
      hw_comms.resetTriggerFIFOs(hw);

      // Create and bind UDP read socket for each channel
      for (uint i = 0; i < chNum; i++){
	readSock[i] = UDPin[i].setupServer(i,UDPin[i].recv());
	UDPin[i].flushPackets(readSock[i]);
      }

      // Succesful initialisation                                      
      printf("Hardware initialisation succesful!\n");

      return 1;

    }
  }
  // If simulation...
  else if (simulation){

    // Print to screen                                                 
    printf("Simulating data in software...\n");
    
    // Create and bind UDP read socket for each channel
    for (uint i = 0; i < chNum; i++){      
      readSock[i] = UDPin[i].setupServer(i,UDPin[i].recv());
      UDPin[i].flushPackets(readSock[i]);
    }

    return 1;

  }

  /* print message and return error frontend should not be started */
  printf("Error: data source not found! Exiting...\n");
  exit(0);

}

/*-- Begin of Run --------------------------------------------------*/

int begin_of_run(int run_number)
{
  /* put here clear scalers etc. */
  int midas_run_number = run_number;

  // Save xml run to odb directory with run number                     
  string runXML = "$STMDAQ_ROOT/odb/stm_run"+to_string(run_number)+".xml";
  string saveXMLcmd = "cp "+xml_path+" "+runXML;
  const char* saveXML = saveXMLcmd.c_str();
  system(saveXML);

  // Pull max subrun size from XML                                      
  int max_binary_file_size = xml_file->int_value("stm.max_binary_file_size",100); // in MB  
  std::cout << "Max max_binary_file_size (int) = " << max_binary_file_size << std::endl;
  int Mbytes = 1048576;
  unsigned int max_binary_size = (unsigned int) max_binary_file_size *(unsigned int) Mbytes;
  std::cout << "Max Binary File Size  = " << max_binary_size << " bytes " << std::endl;

  if (max_binary_size >= INT32_MAX) {
    std::cout << "Max binary file size has to be less than 2 Gb, else int32 issues, EXITING" << std::endl;
    exit(-1);
  }

  // Create binary file output for each channel
  std::string binary_file[chNum];
  for (uint i = 0; i < chNum; i++){          
    binary_file[i] = xml_file->value("stm.raw_binary_filename") + UDPin[i].get_channel_name(i);
    std::cout << "Binary File " << i << " = " << binary_file[i] << std::endl;
    bf[i].set_subrun_filesize(max_binary_size);
    bf[i].open_raw_output_file(binary_file[i], midas_run_number, 0);
  }
  
  return 1;

}


/*-- Init FW packet sending ----------------------------------------*/
void init_packet_sending(){

  // Sleep this thread for 1 second                                    
  std::this_thread::sleep_for (std::chrono::seconds(1));

  // Sleep and countdown the start of this thread for 5 seconds        
  for (int i = 0; i < 3; i++){
    cout << "Sending " << " packets in " << 3-i << "..." << endl;
    std::this_thread::sleep_for (std::chrono::seconds(1));
  }

  // Set the 10G readout mode                                          
  hw_comms.start10Greadout(hw);

  // Reset and initliaise DDR4 read                                    
  hw_comms.reset_DDR4(hw);

  // Stop DDR4 overwrite                                               
  hw_comms.stop_DDR4overwrite(hw);

  // If emulating data from the ADC, switch on ADC emulation           
  if (emulateADC) {
    printf("Emulating ADC data in firmware...\n");
    hw_comms.emulateADC(hw);
  }

  // Enable triggered data-taking                                      
  hw_comms.enable_trigData(hw);

  // Ensure continuous data-taking is off                              
  hw_comms.disable_contData(hw);

  // Start continous 10g packet sending                                
  hw_comms.start10Gpackets(hw);

  // Set number of packets to send
  //  hw->writeRegVal("Buffers.10g_readout_2.number_of_packets_to_send",numberOfPackets);

  cout << "Sending packets now!" << endl;

  // If in debug firmware mode...                                      
  if (debug_fw){
    //DEBUG - Check write address                                      
    int n = 1;
    uint32_t temp_old = 0;
    while(n > 0){
      uint32_t temp = hw_comms.checkMemoryRollover(hw);
      cout << HEX(temp,2) << endl;
      uint32_t temp2 = temp >> 31;
      // Print the write address to the screen                         
      std::this_thread::sleep_for (std::chrono::milliseconds(400));
      temp = hw_comms.getDDR4addr(hw);
      cout << "Write address: " <<  HEX(temp,2) << endl;
      if (temp_old != 0 && temp_old == temp){
	cout << "Break clause 1" << endl;
	break;
      }
      n++;
      temp_old = temp;
    }

    // Stop ADC emulation                                              
    if (emulateADC) hw_comms.stopEmulateADC(hw);

    // Disable triggered data taking                                   
    hw_comms.disable_trigData(hw);

    // Stop 10G packet sending                                         
    hw_comms.stop10Gpackets(hw);

    // Stop 10G readout                                                
    hw_comms.stop10Greadout(hw);

  }

  return;

}

// Simulation thread function                                          
void sim_thread(int run_number){

  // Tell the user the simulation thread is active                     
  cout << "In sim_thread" << endl;

  // Sleep this thread for 1 second                                    
  std::this_thread::sleep_for (std::chrono::seconds(1));

  // Sleep and countdown the start of this thread for 5 seconds        
  for (int i = 0; i < 5; i++){
    cout << "Starting simulation in " << 5-i << "..." << endl;
    std::this_thread::sleep_for (std::chrono::seconds(1));
  }
  // Set boolean to ouput messages to screen to true                   
  bool output = false;

  // If simulating Mu2e                                                
  if (simulation) {
    // Set variables for simulation                                    
    sim.setupSim(xml_file,expC,output,run_number);
    // Initliase simulation                                            
    //    sim.initSim(expC,output);
  }
  int count = 0;

  // Start loop until exit frontend is initliased                      
  while(!exitFE){

    if (simulation) sim.runSim(expC,output);

    // For tests only - only 1 trigger then exit                       
    count++;
    //    if (count == 3) exitFE = true;                               

  }

}

/*-- End of Run ----------------------------------------------------*/
int end_of_run(int run_number)
{

  // Close binary file for each channel
  for (uint i = 0; i < chNum; i++) bf[i].close_output_file();

  return 1;
}

/*-- Frontend Exit -------------------------------------------------*/
int frontend_exit()
{

  // If running with hardware                                          
  if (!simulation){

    // Stop ADC emulation                                              
    if (emulateADC) hw_comms.stopEmulateADC(hw);

    // Disable triggered data taking                                   
    hw_comms.disable_trigData(hw);

    // Stop 10G packet sending                                         
    hw_comms.stop10Gpackets(hw);

    // Stop 10G readout                                                
    hw_comms.stop10Greadout(hw);

  }

  delete xml_file;
  //  delete bf;

  return 1;

}

// Server function to receive packets
void recv_data(int chan, int socket){

  // Notify user
  cout << "In " << UDPin[chan].get_channel_name(chan) << " receive data thread..." << endl;

  // Define data struct to receive                                           
  UDPsocket::packet data;
  data.size = UDPin[chan].getMaxPacketSize(); // Packet size in bytes
  data.data = new int16_t [UDPin[chan].getMaxPacketLength()] (); // Packet length  

  // Timeout counter                                                         
  int timeout_count = 0;
  // Maximum number of timeouts                                              
  int timeout_max = 5e7;

  // Enter infinite loop until exit FE is triggered in exit thread     
  while(!exitFE && timeout_count < timeout_max){
    // Pull packet and check if it times out (returns 0)                      
    if (UDPin[chan].getPacket(data,socket,chan) <= 0){
      // Increment timeout counters                                          
      timeout_count++;
    }
    // If not timed out...                                                   
    else{
      // Place packet in queue                     
      cbq[chan].push(chan,data.data);
      // Increment the received packet counter                               
      recvCount[chan]++;
      // Reset timeout counter                                               
      timeout_count = 0;
    } // End if timeout                                                      
  } // End infinite loop   

  // If socket has timed out, exit frontend...
  if (!exitFE && timeout_count == timeout_max){
    cout << "UDP server for channel " << chan << "timed out..." << endl;
    cout << "Closing frontend..." << endl;
    exitFE = true;
  }
  
}


// Pull data thread                                                   
void pull_data(int chan){

  // Notify user
  cout << "In " << UDPin[chan].get_channel_name(chan) << " queue pull thread " << endl;
  
  // Initalise data array 
  const uint16_t packetSize = UDPin[chan].getMaxPacketSize();
  const uint16_t packetLength = packetSize/2;
  int16_t* pullData = new int16_t [UDPin[chan].getMaxPacketLength()] ();

  // Enter infinite loop until exit FE is triggered in exit thread     
  while(!exitFE){
  
    // If the write counter >= read counter                             
    if (pullCount[chan] >= recvCount[chan]){
      
      // Sleep this thread for 1 second                                
      std::this_thread::sleep_for (std::chrono::milliseconds(1));
      
    }
    // If the write counter < read counter                             
    else{
      
      // Pull data packet from queue and increase pull counter
      memcpy(pullData,cbq[chan].pull(chan),packetSize);
      
      // Check writing the trigger data
      // does not exceed the remaining binary file siz
      // Will start new file if it will exceed...
      //      bf[chan].incoming_raw_data_size(packetLength);
      // Write out packet data
      //      bf[chan].write_raw_data(pullData,packetLength);

      // Calculate total data written so far                           
      totalDataSize[chan] += packetSize;
      
      // Increase the pull counter                                    
      pullCount[chan]++;
      
    }
    
  }
  
}

// Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int signum) {

  // Set exitFE to true                                                
  exitFE = true;

}

// Exit frontend thread function                                       
void exit_thread(int run_number){

  cout << "In exit_thread" << endl;

  // Register signal and signal handler                                
  signal(SIGINT, signal_callback_handler);

  while(!exitFE){
    sleep(1);
  }

  // Sleep for 3 seconds to make sure main_loop has exited             
  sleep(3); 
  // End the run                                                       
  cout << "Ending run " << run_number  << "..." << endl;
  end_of_run(run_number);

  // Exit frontend commands                                            
  cout << "Closing frontend..." << endl;
  frontend_exit();

  // Terminate program                                                 
  //  exit(SIGINT);                                                    

}


// Main function
int main(){

  int run_number = 1;

  // Initialise frontend                                               
  frontend_init();

  // Being the run                                                     
  begin_of_run(run_number);
  // Store start time of run                                           
  start = std::chrono::system_clock::now();

  // Define threads                                                    
  std::thread *simThread; // Simulation                                
  std::thread *sendThread; // Init sending fw packets                  
  std::thread *recvThread[chNum]; // Receive and put data into queue
  std::thread *pullThread[chNum]; // Pull data from queue
  std::thread *exitThread; // Exit FE   

  // Initialise threads                                                
  // If using the simulation...                                        
  if (simulation){
    // Create the simulation thread                                    
    //    simThread = new std::thread (sim_thread,ref(run_number));
  }
  // Else if not simulation                                            
  else{
    // Start the firmware send thread                                  
    sendThread = new std::thread (init_packet_sending);
  }
  // Start the exit thread                                             
  exitThread = new std::thread (exit_thread,ref(run_number));
  // Loop over channels
  for (uint i = 0; i < chNum; i++){
    // Start the read threads                                          
    recvThread[i] = new std::thread (recv_data,i,readSock[i]);
    // Start the write threads                                         
    pullThread[i] = new std::thread (pull_data,i);
  }
  
  // Join threads                                                      
  if (simulation){
    // Simulation thread                                               
    //    simThread->join();
  }
  else{
    // Firmware send thread                                            
    sendThread->join();
  }  
  // Exit thread                                                       
  exitThread->join();
  // Loop over channels
  for (uint i = 0; i < chNum; i++){    
    // Read thread                                                    
    recvThread[i]->join();
    // Write thread
    pullThread[i]->join();
  }

  // Loop over channels
  for (uint i = 0; i < chNum; i++){
    cout << "Channel " << i << " [" << UDPin[i].get_channel_name(i) 
	 << "]: received  = " << recvCount[i] 
	 << ", pulled = " << pullCount[i] 
	 << ", written = " << totalDataSize[i]*1e-9 << "Gb." << endl;
  }

  return 0;
  
}



