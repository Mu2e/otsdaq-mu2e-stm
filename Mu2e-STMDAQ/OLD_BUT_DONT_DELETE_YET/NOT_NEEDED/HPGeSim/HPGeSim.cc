#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include <list>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <time.h>
#include <memory.h>
#include <vector>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// HPGeSim Header
#include "STMDAQ-TestBeam/HPGeSim/HPGeSim.hh"

using namespace std;

// ADC emulator class  
emulateADC emADC;

// UDP packet formation class
formPackets formPack;

// Boolean to write simulation data to file 
static const bool writeSimData = true;

// Main Injector Cycle Count
uint MIcount = 0;

// Standard constructor - shouldn't be used
HPGeSim::HPGeSim() {}

// Function to write packet data to binary file
void HPGeSim::writeToFile(pStruct* pData){

  // Time at start of macropulse
  auto writeStart = std::chrono::high_resolution_clock::now();

  // Write packet data to binary file
  for (uint i = 0; i < totSamplePeriods; i++){
    auto sendStart = std::chrono::high_resolution_clock::now();
    uint packetNum = pData[i].pNum;
    // Loop packets
    for (uint j = 0; j < packetNum; j++){
      //      exData._bf->write_raw_data(pData[i].pack[j].data,pData[i].pack[j].size/2);
    }
  }

  calcTime("\tWrite",writeStart);


  return;

}

// Function that simulates the accelerator super cycle 
void HPGeSim::simMIcycle(pStruct* pData, expConfig exp, bool output){

  // Time at start of macropulse
  auto MIcycleStart = std::chrono::high_resolution_clock::now();

  // Loop through sample period in MI cycle
  for (uint i = 0; i < totSamplePeriods; i++){
    auto sendStart = std::chrono::high_resolution_clock::now();
    uint packetNum = pData[i].pNum;
    // Loop packets
    for (uint j = 0; j < packetNum; j++){
      // Send packets
      //      _UDPout->sendPacket(pData[i].pack[j],socket);
    }
    auto sendEnd = std::chrono::high_resolution_clock::now();
    double sendTime = chrono::duration <double,micro> (sendEnd-sendStart).count();
    // If the send time is less than the sample period
    if (sendTime < pM[i].period){
      // Sleep for the remainder of the sample period      
      double sleepTime = pM[i].period - sendTime;
      usleep(sleepTime);
    }
    if (i == totSamplePeriods-1){
      auto MIcycleEnd = std::chrono::high_resolution_clock::now();
      double cycleTime = chrono::duration <double,micro> (MIcycleEnd-MIcycleStart).count();
      if (output) cout << "\tSimulated MI cycle (send) time = " << cycleTime << " us" << endl;
    }
  }

  // // Fill struct with firmware trigger header info
  // struct headerInfo extMode = {trigCount, // Trigger number
  // 			       macroTime, // Macropulse trigger time
  // 			       mode, // Trigger mode: external = 0
  // 			       microCount, // Micropulse counter
  // 			       channel, // HPGe/LaBr
  // 			       exp.ext_ADC_offset}; // ADC offset
 
  //   // Fill struct with firmware trigger header info
  //   struct headerInfo intMode = {trigCount, // Trigger number
  // 				 macroTime, // Macropulse trigger time
  // 				 mode, // Trigger mode: external = 0
  // 				 microCount, // Micropulse counter
  // 				 channel, // HPGe/LaBr
  // 				 exp.int_ADC_offset}; // ADC offset
 
  return;

}

// Function to run the main simulation
void HPGeSim::runSim(expConfig exp, bool output){

  // Initliase ADC
  emADC.initADC(exp,output);  

  // Initliase UDP packet formation
  formPack.initPackets(emADC,exp,output);

  // Initialise data generation threads
  thread *dataThread[totSamplePeriods];
  // Initialise send data thread
  thread *sendThread;
  // Initialise write to binary file thread
  thread *writeThread;

  // Boolen to signal no sending in first generation loop
  bool firstLoop = true;

  // The data generation index of genArrayNum as bool
  bool genInd = false;
  // The send data index of genArrayNum
  bool sendInd = true;

  // Get data generation start time
  auto MIstart = std::chrono::high_resolution_clock::now();

  // Infinite loop
  while(1){

    // If not the first loop, start the send thread loop
    if (!firstLoop) {
      sendThread = new thread (&HPGeSim::simMIcycle,this,formPack.MIcycle[sendInd],exp,output);
      if (writeSimData) writeThread = new thread (&HPGeSim::writeToFile,this,formPack.MIcycle[sendInd]);
    }

    // Get data generation start time
    auto dataStart = std::chrono::high_resolution_clock::now();
 
    // Calculate generic pulse
    //    cout << "Generating pulses in time and forming data packets..." << endl;
  
    // Loop over all sample periods
    for (uint i = 0; i < totSamplePeriods; i++){
      // Generate data for that sample period
      dataThread[i] = new thread (&emulateADC::genData,ref(emADC),emADC.ADC[genInd],exp,pM[i]);
    }

    // Loop over all sample periods
    for (uint i = 0; i < totSamplePeriods; i++){
      // Join all data generation threads back to main thread
      dataThread[i]->join();
      // Fill packets with ADC data
      formPack.fillPackets(emADC,MIcount,emADC.ADC[genInd][i],formPack.MIcycle[genInd][i],exp,pM[i]); 
    }
    
    // Print data generatins time
    string cycPrint = "MI cycle " + to_string(MIcount) + ": noise + pulse generation and packet allocation";
    if (output) calcTime(cycPrint,dataStart);
 
    // If not the first loop, join the send thread loop
    if (!firstLoop){
      sendThread->join();
      if (writeSimData) writeThread->join();
    }    

    // Set firstLoop to false
    firstLoop = false;

    // Swap genInd and sendInd
    genInd = !genInd;   // 0 <--> 1
    sendInd = !sendInd; // 1 <--> 0

    // Increment the counter
    MIcount++;

    if (MIcount == 10) break;

  } // End infinite loop

    // Print data generatis time
    string cycPrint = to_string(MIcount) + " x MI cycles, total";
    if (output) calcTime(cycPrint,MIstart);

}

// Function to set up all the configuration variables for the simulation
void HPGeSim::setupSim(Xml* xml_file, expConfig exp, bool output, int run_number){

  // Create the instance of the UDP socket to write data
  // UDPsocket* UDPout = new UDPsocket();
  // setUDPout(UDPout);

  // Create UDP socket to be sent to RECV IP of frontend
  //  socket = UDPout->createClient(UDPout->HPGe(),UDPout->recv());

  // // Create the instance to write the binary files
  // BinaryFile* bf = new BinaryFile();
  // exData.setBF(bf);

  // Get max subrun size from XML
  int max_binary_file_size = xml_file->int_value("stm.max_binary_file_size",100);

  // Calculate max binary size in MB
  int Mbytes = 1048576;
  unsigned int max_binary_size = (unsigned int) max_binary_file_size * (unsigned int) Mbytes;
  std::string binary_file = xml_file->value("stm.sim_binary_filename");

  // Write warning in case of max binary size being too large
  if (max_binary_size >= INT32_MAX) {
    std::cout << "Max binary file size has to be less than 2 Gb, else int32 issues, EXITING" << std::endl;
    exit(-1);
  }

  // // Set the maximum subrun file size
  // exData._bf->set_subrun_filesize(max_binary_size);

  // // Open the binary file to write data to
  // exData._bf->open_raw_output_file(binary_file, run_number, 0);

  // Get external ADC offset in us
  extADCoffset = exp.ext_ADC_offset/exp.adc_offset_time_clock;

  // Get external trigger timeout in us
  extTrigTimeout = exp.ext_trig_timeout/exp.adc_offset_time_clock; // us

  // Get external/internal delay in us
  extIntDelay = exp.ext_int_delay/exp.adc_offset_time_clock; // us

  // Get internal ADC offset in us
  intADCoffset = exp.int_ADC_offset/exp.adc_offset_time_clock; // us

}
