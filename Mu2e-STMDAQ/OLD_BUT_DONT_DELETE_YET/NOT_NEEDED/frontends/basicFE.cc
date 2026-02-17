/********************************************************************	\

  Name:         HPGe_frontend.cxx
  Created by:   Alex Keshavarzi

  Contents:     Experiment specific readout code (user part) of
                Midas frontend. 

\********************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>

#include <string.h>
#include <math.h>
#include <assert.h> // assert()

#include <chrono>
#include <ctime>
#include <time.h>

#include <memory.h>

#include <thread>
#include <mutex>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include<sys/socket.h>

// Clock                                             
using std::chrono::high_resolution_clock;
using std::chrono::seconds;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::system_clock;

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
class HWcomms hw_comms; // Hardware class   
//extractData getData; // Extract data class
readData rData; // Extract data class
HPGeSim sim; // HPGe Simulator Class

// Define the XML reader instance
string xml_path;
//Xml *xml_file;

// UDP socket number
int readSock;

// Emulating data boolean                                                     
bool simulation = true;
bool emulateADC = true;

// Boolean to signal debugging the fw
bool debug_fw = false;

// Data array (one for read, one for write)
static const int readWriteArrays = 2;

// Experimental configuration variable struct
struct expConfig expC;

// Struct array (one for read, one for write)
//int arrayNum = 0;
//struct trigData threadData[readWriteArrays];

struct trigData readArray;
struct trigData processArray;
struct trigData writeArray;

std::mutex rp_mutex; // Read + Process mutex
std::mutex pw_mutex; // Process + Write mutex

std::condition_variable rp_cv; // Read + Process conditional variable
std::condition_variable pw_cv; // Process + Write conditional variable

bool process_ready = false;
bool write_ready = false;

// Time since last data write
double lastWrite = 0;

// Total data size written
auto startTime = high_resolution_clock::now();
uint64_t totalDataSize = 0;

bool exitFE = false;

auto start = std::chrono::system_clock::now();
auto now = std::chrono::system_clock::now();

/*-- Frontend Init -------------------------------------------------*/

int frontend_init() {
  /* put any hardware initialization here */
 
  // Open the XML file and instantiate the xml_file object
  xml_path = EnvVars::expand("${STM_XML}");
  cout << "XML_PATH = " << xml_path << std::endl;
  xml_file = new Xml(xml_path);

  // Set struct experimental configuration variables from xml filer
  expC = getXMLvalues(expC);

  // Check experimental configuration variables                                                     
  if (checkExpConfig(expC) == 0){
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
  
  int status, size;
  char str[256];
  printf("frontend_init!\n");
  
  printf("Acquiring data source...\n");
      
  if (!simulation){
    
    printf("Connecting device through IPBusManager...");
    hw = new IPBusManager(hw->getConnectionFile(),hw->getDeviceID());
    
    // Initialise ADC
    //    if (hw_comms.initADC(hw) == 1){
    if (1){
      
      // Set firmware registers from xml config file
      hw_comms.setRegisters(hw);
      
      // Start and reset the external clock counter   
      hw_comms.startResetClock(hw);
      
      // Reset trigger FIFOs
      hw_comms.resetTriggerFIFOs(hw);
      
      // Set the 10G readout mode
      hw_comms.start10Greadout(hw);
      
      // Reset and initliaise DDR4 read
      hw_comms.reset_DDR4(hw);
      
      // If emulating data from the ADC, switch on ADC emulation
      if (emulateADC) {
	printf("Emulating ADC data in firmware...\n");
	hw_comms.emulateADC(hw);
      }
      
      // Enable triggered data-taking
      hw_comms.enable_trigData(hw);                                             
      // Start continous 10g packet sending
      hw_comms.start10Gpackets(hw);

      if (debug_fw){
	//DEBUG - Check write address
	while(1){
	  uint32_t temp = hw_comms.checkMemoryRollover(hw);
	  cout << HEX(temp,2) << endl;
	  if (temp == 1){
	    cout << "Break clause 1" << endl;
	    //break;
	  } 
	  
	  sleep(0.8);
	  temp = hw_comms.getDDR4addr(hw);
	  cout << "Write address: " <<  HEX(temp,2) << endl;
	}
      }

      // Create and bind UDP read socket
      //      readSock = UDPin->bindSocket(UDPin->createSocket(READ));
      readSock = UDPin->setupServer(READ);
      UDPin->flushPackets(readSock);
      
      // Succesful initialisation
      printf("Hardware initialisation succesful!\n");
      
      return 1;

    }
  }
  else if (simulation){
    
    // Print to screen
    printf("Simulating data in software...\n");
    
    // Create and bind UDP read socket
    //    readSock = UDPin->bindSocket(UDPin->createSocket(READ));
    readSock = UDPin->setupServer(READ);
    UDPin->flushPackets(readSock);
    
    return 1;
    
  }
  /* print message and return error frontend should not be started */
  printf("Error: data source not found! Exiting...\n");  
  exit(0);

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
  unsigned int max_binary_size = (unsigned int) max_binary_file_size * (unsigned int) Mbytes;
  std::string binary_file = xml_file->value("stm.raw_binary_filename");
  std::cout << "Max Binary File Size  = " << max_binary_size << " bytes " << std::endl;
  std::cout << "Binary File           = " << binary_file << std::endl;

  if (max_binary_size >= INT32_MAX) {
    std::cout << "Max binary file size has to be less than 2 Gb, else int32 issues, EXITING" << std::endl;
    exit(-1);
  }

  rData.exData._bf->set_subrun_filesize(max_binary_size);
  rData.exData._bf->open_raw_output_file(binary_file, midas_run_number, 0);

  // Set the external/internal data buffer lengths from the xml
  rData.exData.setBufferLengths();

  return 1; 

}

/*-- End of Run ----------------------------------------------------*/

int end_of_run(int run_number)
{

  rData.exData._bf->close_output_file();

   return 1;
}

/*------------------------------------------------------------------*/

// Read data thread
void read_data(){
  
  cout << "In read_thread" << endl;
  
  // Enter infinite loop until exit FE is triggered in exit thread
  while(!exitFE){
    
    // Define and lock the critical section of read thread with a mutex
    std:: unique_lock<std::mutex> ul_rp(rp_mutex);   

    // Get a packet's worth of data
    readArray = rData.getOnePacket(readSock,readArray);
    cout << "Reading packet number " << readArray.trigNum <<
      ". Dropped packets = " << readArray.droppedPackets << endl; 
    // Get a trigger's worth of data
    // readArray = rData.getOneTrigger(readSock,readArray);
    // cout << "Reading trigger number " << readArray.trigNum <<
    //   ". Dropped packets = " << readArray.droppedPackets << endl; 
    
    // Set the read/process predicate to true
    process_ready = true;
    // Unlock the critical section of read thread 
    ul_rp.unlock();
    // Send condition variable notification to process 
    // thread to access this read array
    rp_cv.notify_one();
    // Lock the mutex to wait for process data to finish
    // accessing read array
    ul_rp.lock();
    // Wait for condition variable notification from process
    // thread to indicate it has finised accessing read array
    rp_cv.wait(ul_rp, []() { return process_ready == false; });
    
  }
  
}

// Process data thread
void process_data(){

  cout << "In process_thread" << endl;

  // Enter infinite loop until exit FE is triggered in exit thread
  while(!exitFE){

    // Define mutex for read/process critical section
    std::unique_lock<std::mutex> ul_rp(rp_mutex);

    // Wait for condition variable notification from read
    // thread to indicate it has finised acquiring read array
    rp_cv.wait(ul_rp, [](){ return process_ready; });

    // Define mutex for process/write critical section
    std:: unique_lock<std::mutex> ul_pw(pw_mutex);   
    
    // Store process array as read array
    processArray = readArray;

    // Set the read/process predicate to true
    process_ready = false;
    // Unlock the critical section of read/process thread 
    ul_rp.unlock();
    // Send condition variable notification to read
    // thread to start reading next array
    rp_cv.notify_one();

    // Process and fornat a trigger's worth of data 
    // ready for writing to disk
    //data = rData.exData.processData(data);
          
    cout << "Processing trigger number " << processArray.trigNum <<
      ". Dropped packets = " << processArray.droppedPackets << endl; 
    
    // Set the process/write predicate to true
    write_ready = true;
    // Unlock the critical section of process/write thread 
    ul_pw.unlock();
    // Send condition variable notification to write 
    // thread to access this process array
    pw_cv.notify_one();
    // Lock the process/write mutex 
    ul_pw.lock();
    // Wait for condition variable notification from write
    // thread to indicate it has finised accessing process array
    pw_cv.wait(ul_pw, []() { return write_ready == false; });

    // Lock the read/process mutex 
    ul_rp.lock();

    
  }

}

// Write data thread
void write_data(){

  // Enter infinite loop until exit FE is triggered in exit thread
  while(!exitFE){
    
    // Define mutex for process/write critical section
    std::unique_lock<std::mutex> ul_pw(pw_mutex);
    
    // Wait for condition variable notification from process
    // thread to indicate it has finised forming proccessed array
    pw_cv.wait(ul_pw, [](){ return write_ready; });
    
    // Store write array as process array
    writeArray = processArray;
        
    // Set the process/write predicate to false
    write_ready = false;
    // Unlock the critical section of process/write thread 
    ul_pw.unlock();
    // Send condition variable notification to process
    // thread to start processing next array
    pw_cv.notify_one();

    // // Check that the software trigger header starts with 0xDEADBEEF
    // uint16_t deadbeef1 = (uint16_t)data[sw_tHdr_0];
    // uint16_t deadbeef2 = (uint16_t)data[sw_tHdr_1];
    // uint32_t deadbeef = (uint32_t)deadbeef2 << 16  | (uint32_t)deadbeef1;
    // if (deadbeef != 0xDEADBEEF){
    //   cout << "ERROR: The first 2 elements of the trigger header has been read as: " << endl;
    //   cout << HEX(deadbeef2,2) << " << 16 | " << HEX(deadbeef1,2) << " = "
    //        << HEX(deadbeef,2);
    //   cout << "\nThe trigger header is expected to start with: 0xDEADBEEF" << endl;
    //   cout << "\nExiting..." << endl;
    //   exit(0);
    // }
    
    // // Get the trigger number
    // uint16_t trigNum1 = (uint16_t)data[sw_tHdr_TrigNum1];
    // uint16_t trigNum2 = (uint16_t)data[sw_tHdr_TrigNum2];
    // uint32_t trigNum = (uint32_t)trigNum2 << 16  | (uint32_t)trigNum1;
    // cout << "Writing trigger number: " << trigNum << endl;
    
    // // Get the length of the data array
    uint32_t dataLength = writeArray.dataSize/2;
    
    // Check writing the trigger data                                          
    // does not exceed the remaining binary file size                                            
    // Will start new file if it will exceed...                                                  
    rData.exData._bf->incoming_raw_data_size(dataLength);    
    
    // Write out trigger data
    rData.exData._bf->write_raw_data(writeArray.data,dataLength); 
    
    // Print to screen how much data is being written
    totalDataSize += writeArray.dataSize;
    //  cout << "Written " << dataLength*2*1e-6 << " Mb. Total = " << totalDataSize*1e-6 << " Mb" << endl;
    
    cout << "Writing trigger number " << writeArray.trigNum <<
      ". Dropped packets = " << writeArray.droppedPackets << ". Data written = " << totalDataSize*1e-6\
 << " Mb" << endl;

    exit(0);

    // Lock the process/write mutex 
    ul_pw.lock();
    
  }

}


// Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int signum) {

  cout << "\nExiting..." <<  endl;; //print this line       
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
  exit(SIGINT);


}

// Simulation thread function
void sim_thread(int run_number){
  
  cout << "In sim_thread" << endl;

  //  sleep(5);
  std::this_thread::sleep_for (std::chrono::seconds(5));
  // Set boolean to ouput messages to screen to true
  bool output = false;

  // If simulating Mu2e
  if (simulation) {
    // Set variables for simulation
    sim.setupSim(xml_file,expC,output,run_number);
    // Initliase simulation
    sim.initSim(expC,output);
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


void monitor_thread(int run_number){

  cout << "In monitor_thread" << endl;

  std::time_t runStart = std::chrono::system_clock::to_time_t(start);
    
  uint32_t prevTrigNum = -1;

  while(!exitFE){
  }
    
  //    if (threadData[0].trigNum != prevTrigNum){
      
  // Get time from start
  now = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = now-start;
  int n = elapsed_seconds.count();
  // Calulate elapsed days
  int day = n / (24 * 3600); 
  string day_str = to_string(day);
  if (day < 10) day_str = "0"+day_str;
  // Calulate elapsed hours
  n = n % (24 * 3600);
  int hour = n / 3600; 
  string hour_str = to_string(hour);
  if (hour < 10) hour_str = "0"+hour_str;
  // Calulate elapsed minutes
  n %= 3600;
  int minutes = n / 60 ;
  string minutes_str = to_string(minutes);
  if (minutes < 10) minutes_str = "0"+minutes_str;
  // Calulate elapsed minutes
  n %= 60;
  int seconds = n;
  string seconds_str = to_string(seconds);
  if (seconds < 10) seconds_str = "0"+seconds_str;
  
  sleep(2);
  system("clear");      
  sleep(2);
  cout << endl;
  cout << "-----------------------------------------------------------------------------------------------------" << endl;
  cout << "                                  ** Frontend Running **" << endl;
  cout << "                           [ Press Ctrl+C ONCE to safely exit...]" << endl;
  //    cout << "           ** Run " << run_number << " **" << endl;
  cout << "-----------------------------------------------------------------------------------------------------" << endl;
  cout << "Run number: " << run_number << ". Data dir: " <<  xml_file->value("stm.raw_binary_filename") << ". Started: " << ctime(&runStart);

  while(!exitFE){
    
    // Get time from start
    now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = now-start;
    int n = elapsed_seconds.count();
    // Calulate elapsed days
    int day = n / (24 * 3600); 
    string day_str = to_string(day);
    if (day < 10) day_str = "0"+day_str;
    // Calulate elapsed hours
    n = n % (24 * 3600);
    int hour = n / 3600; 
    string hour_str = to_string(hour);
    if (hour < 10) hour_str = "0"+hour_str;
    // Calulate elapsed minutes
    n %= 3600;
    int minutes = n / 60 ;
    string minutes_str = to_string(minutes);
    if (minutes < 10) minutes_str = "0"+minutes_str;
    // Calulate elapsed minutes
    n %= 60;
    int seconds = n;
    string seconds_str = to_string(seconds);
    if (seconds < 10) seconds_str = "0"+seconds_str;

    cout << "Time elapsed: " << day_str << ":" << hour_str << ":" << minutes_str << ":" << seconds_str <<
      ". Trigger number: " << readArray.trigNum 
     	 << ". Dropped packets = " << readArray.droppedPackets << ". Data written = " << totalDataSize*1e-6 << " Mb\r";
    // cout << "------------------------------------- " << endl;
    // cout << "--> Press Ctrl+C ONCE to safely exit... " << endl;
    // cout << "------------------------------------- " << endl;
    fflush(stdout);
    sleep(1);
    
  }

}

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
  std::thread *readThread; // Read data
  std::thread *processThread; // Process data
  std::thread *writeThread; // Write data
  std::thread *monitorThread; // Screen output
  std::thread *exitThread; // Exit FE

  // Initialise threads
  simThread = new std::thread (sim_thread,ref(run_number)); 
  monitorThread = new std::thread (monitor_thread,ref(run_number)); 
  exitThread = new std::thread (exit_thread,ref(run_number)); 
  readThread = new std::thread (read_data); 
  processThread = new std::thread (process_data); 
  writeThread = new std::thread (write_data); 

  // Join threads
  simThread->join();
  monitorThread->join(); 
  exitThread->join(); 
  readThread->join(); 
  processThread->join(); 
  writeThread->join(); 

  return 0;                                                             
  
}
