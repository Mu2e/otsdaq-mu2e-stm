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

// Extract data                                                        
#include "STMDAQ-TestBeam/processData/extractData.hh"

// Read data                                                           
#include "STMDAQ-TestBeam/processData/readData.hh"

// XML interface                                                       
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Binary file code                                                    
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

// HPGe Simulation                                                     
#include "STMDAQ-TestBeam/HPGeSim/HPGeSim.hh"

using namespace std;
using namespace uhal;

// Declare IPBusManager                                                
IPBusManager* hw = new IPBusManager();

// Define classes                                                      
HWcomms hw_comms; // Hardware class                              
dataVars inVars; // Data Variables class
readData rData; // Extract data class                                  
HPGeSim sim; // HPGe Simulator Class                                   

// Define the XML reader instance                                      
string xml_path;

// UDP socket number                                                   
int readSock;

// Emulating data boolean                                              
bool simulation = false;
bool emulateADC = true;

// Boolean to signal debugging the fw                                  
bool debug_fw = true;

// Experimental configuration variable struct                          
struct expConfig expC;

// Total data size written                                             
uint64_t totalDataSize = 0;

// Read counter                                                        
uint readCount = 0;
// Write counter                                                       
uint writeCount = 0;

// Boolean to signal exit FE                                           
bool exitFE = false;

// Define arrays for values for each loop
double eff = 0; // Efficiency array
double recvSpeed = 0; // Receive time array

// The maximum number of packets we expect to receive
static const uint32_t numberOfPackets = 0xffffff;
static const uint32_t numberOfPs = 0xffffffff;
static const uint maxPackets = 250000;

// Data arrays                                                         
packet readPacket[numberOfPackets];
//packet writePacket[numberOfPackets];

// Timers                                                              
auto start = std::chrono::system_clock::now();
auto now = std::chrono::system_clock::now();

using namespace std;

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

  // Create the instance to write the binary files                     
  BinaryFile* bf = new BinaryFile();
  rData.exData.setBF(bf);

  // Create the instance of the UDP socket to read data                
  UDPsocket* UDPin = new UDPsocket();
  rData.exData.setUDPin(UDPin);

  // Create the instance of the UDP socket to write data               
  UDPsocket* UDPout = new UDPsocket();
  rData.exData.setUDPout(UDPout);

  // Print to user
  printf("Initalising frontend...!\n");

  // Print to user
  printf("Acquiring data source...\n");

  // If not simulation...
  if (!simulation){

    // Print to user
    printf("Connecting device through IPBusManager...");
    
    // Get hardware connection
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

      // Create and bind UDP read socket                               
      readSock = UDPin->setupServer(READ);
      UDPin->flushPackets(readSock);

      // Succesful initialisation                                      
      printf("Hardware initialisation succesful!\n");

      return 1;

    }
  }
  // If simulation...
  else if (simulation){

    // Print to screen                                                 
    printf("Simulating data in software...\n");

    // Create and bind UDP read socket                                 
    readSock = UDPin->setupServer(READ);
    UDPin->flushPackets(readSock);

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

  // Get max subrun size from XML                                      
  int max_binary_file_size = xml_file->int_value("stm.max_binary_file_size",100); // in MB  
  std::cout << "Max max_binary_file_size (int) = " << max_binary_file_size << std::endl;
  int Mbytes = 1048576;
  unsigned int max_binary_size = (unsigned int) max_binary_file_size *(unsigned int) Mbytes;
  std::string binary_file = xml_file->value("stm.raw_binary_filename");
  std::cout << "Max Binary File Size  = " << max_binary_size << " byte\
s " << std::endl;
  std::cout << "Binary File           = " << binary_file << std::endl;

  if (max_binary_size >= INT32_MAX) {
    std::cout << "Max binary file size has to be less than 2 Gb, else int32 issues, EXITING" << std::endl;
    exit(-1);
  }

  rData.exData._bf->set_subrun_filesize(max_binary_size);
  rData.exData._bf->open_raw_output_file(binary_file, midas_run_number, 0);

  return 1;

}


/*-- Init FW packet sending ----------------------------------------*/
void init_packet_sending(){

  // Sleep this thread for 1 second                                    
  std::this_thread::sleep_for (std::chrono::seconds(1));

  // Sleep and countdown the start of this thread for 5 seconds        
  for (int i = 0; i < 3; i++){
    cout << "Starting packet sending in " << 3-i << "..." << endl;
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

  rData.exData._bf->close_output_file();

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
  delete rData.exData._bf;

  return 1;

}

// Server function to receive packets
void read_data(int socket){

  cout << "In read_thread " << endl;

  // Initalise receive speed timers
  std::chrono::time_point<std::chrono::system_clock> a;
  std::chrono::time_point<std::chrono::system_clock> b;

  // Ensure UDP timeout boolean is false                               
  rData.packetTimeout = false;

  // Enter infinite loop until exit FE is triggered in exit thread     
  while(!exitFE or rData.packetTimeout){

    if (readCount == 0) a = std::chrono::system_clock::now();

    // Get a packet's worth of data                                    
    readPacket[readCount] = rData.getOnePacket(readSock);

    // If the UDP socket has timed out...                              
    if (rData.packetTimeout){
      break;
    }

    // Increment the read counter                                      
    readCount++;

    if (readCount == numberOfPackets) b = std::chrono::system_clock::now();

  }

   // Find time taken to receive all packets
   auto diff = b - a;
   // Get time in nanoseconds
   double timeNano = chrono::duration <double, nano> (diff).count();
   if (readCount == numberOfPackets) cout << "Receive time = " << timeNano*1e-9 << " seconds" << endl;
   // Get packet size in bytes
   double pSize = 2*readPacket[readCount].size;
   // Store speed as Gbits/s
   recvSpeed = readCount*(pSize*8*1e-9)/(timeNano*1e-9); // Packet size [bytes] * Gbits in byte / time in secs

}


// Write data thread                                                   
void write_data(){
  
  cout << "In write_thread " << endl;

  // Enter infinite loop until exit FE is triggered in exit thread     
  while(!exitFE){// or (rData.packetTimeout && writeCount != readCount)){
    
    // If the write counter < read counter                             
    if (writeCount >= readCount){
      
    // Sleep this thread for 1 second                                
      std::this_thread::sleep_for (std::chrono::seconds(1));
      
    }
    else{
      
      // Get the data lengtht to writ                                  
      uint32_t dataLength = readPacket[writeCount].size;
       
      // Check data does not exceed the remaining binary file size     
      // Will start new file if it will exceed...                      
      //      rData.exData._bf->incoming_raw_data_size(dataLength);
      
      // Write data                                                    
      //      rData.exData._bf->
      //        write_raw_data(readPacket[writeCount].data,dataLength);
      
      // Calculate total data written so far                           
      totalDataSize += 2*dataLength;
      
      // Increase the write counter                                    
      writeCount++;
      
    }

    // Break clause if all packets are written
    if (writeCount == numberOfPackets){
      cout << "Written all packets!" << endl;
      exitFE = true;
      break;
    }      
    else if (rData.packetTimeout){
      exitFE = true;
      break;
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

  //  uint32_t numberOfPackets = 0xFFFFFF;
  hw->writeRegVal("Buffers.10g_readout_2.number_of_packets_to_send",numberOfPackets);

  // Define threads                                                    
  std::thread *simThread; // Simulation                                
  std::thread *sendThread; // Init sending fw packets                  
  std::thread *readThread; // Read data                     
  std::thread *writeThread; // Write data                   
  std::thread *exitThread; // Exit FE   

  // Initialise threads                                                
  // If using the simulation...                                        
  if (simulation){
    // Create the simulation thread                                    
    simThread = new std::thread (sim_thread,ref(run_number));
  }
  // Else if not simulation                                            
  else{
    // Start the firmware send thread                                  
    sendThread = new std::thread (init_packet_sending);
  }
  // Start the exit thread                                             
  exitThread = new std::thread (exit_thread,ref(run_number));
  // Start the read threads                                          
  readThread = new std::thread (read_data,readSock);
  // Start the write threads                                         
  writeThread = new std::thread (write_data);
  
  // Join threads                                                      
  if (simulation){
    // Simulation thread                                               
    simThread->join();
  }
  else{
    // Firmware send thread                                            
    sendThread->join();
  }  
  // Exit thread                                                       
  exitThread->join();
  // Read thread                                                    
  readThread->join();
  // Write thread
  writeThread->join();

  // Calculate efficiency
  eff = double(readCount)/numberOfPackets*100;
  
  // Print results 
  if (readCount == numberOfPackets){
    cout << "Read efficiency = " << readCount << "/" << numberOfPackets << " = " << eff << " %. " << "Receive speed = " << recvSpeed << " Gbit/s." << endl;
  }
  else{
    cout << "Read efficiency = " << readCount << "/" << numberOfPackets << " = " << eff << " %. " << endl;
  }
  cout << "Write efficiency = " << double(writeCount)/numberOfPackets*100 << " %. Written a total of " << totalDataSize*1e-6 << " Mb" <<  endl;
    
  return 0;
  
}



