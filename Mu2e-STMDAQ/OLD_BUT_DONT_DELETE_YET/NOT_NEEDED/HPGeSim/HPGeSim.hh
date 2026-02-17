/////////////////////////////////////////////////A////////////////////////
/// This module provides the HPGe simulation (header)
/////////////////////////////////////////////////A////////////////////////

#ifndef HPGESIM_MU2E_NEW_hh
#define HPGESIM_MU2E_NEW_hh

#include <iostream>
#include <stdlib.h>

#include <stdio.h>
#include <math.h>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include <list>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

// XML interface
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Binary file code
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

// UDP
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// ADC emulator
#include "STMDAQ-TestBeam/HPGeSim/emulate_ADC.hh"

// UDP Packet Formation Header                                                                         
#include "STMDAQ-TestBeam/HPGeSim/form_UDPpackets.hh"

#ifndef HPGESIM_MU2E_NEW_hh_DEFINED
#define HPGESIM_MU2E_NEW_hh_DEFINED
class HPGeSim;
#endif

// Define the XML reader instance
Xml *xml_file;

using namespace std;

// Clock
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

// Function to calculate and print the time of a process
void calcTime(string process, std::chrono::time_point<std::chrono::system_clock> start){
  auto end = std::chrono::high_resolution_clock::now();
  double time = chrono::duration <double,micro> (end-start).count();
  cout << process << " time = " << time << " us" << endl;
}

class HPGeSim{

public:
  
  // Standard constructor - shouldn't be used
  HPGeSim();

  // Initialise UDP socket
  void setUDPin(UDPsocket* UDPin) {_UDPin = UDPin;};
  // UDP socket instance
  UDPsocket *_UDPin;

  // Initialise UDP socket
  void setUDPout(UDPsocket* UDPout) {_UDPout = UDPout;};
  // UDP socket instnace
  UDPsocket *_UDPout;

  // UDP client
  int socket;

  // Packet counter  
  uint32_t packetCount = 0;
  
  // The MI cycle pStruct
  pStruct **MIcycle;

  // Struct containing header info
  struct headerInfo{
  
    uint32_t macroCount;
    uint64_t macroTime;
    uint16_t mode;
    uint32_t microCount;
    uint16_t channel;
    uint32_t ADCoffset;  
  
  };

  // External ADC offset in us
  double extADCoffset = 0;

  // External trigger timeout in us
  double extTrigTimeout = 0; // us                            

  // External/internal delay in us
  double extIntDelay = 0; // us

  // Internal ADC offset in us
  double intADCoffset = 0; // us
  
  // **************************************************
  // Define main code functions below
  // **************************************************

  // Function to write packet data to binary file
  void writeToFile(pStruct* pData);

  // Function that simulates the accelerator super cycle
  void simMIcycle(pStruct* pData, expConfig exp, bool output);

  // Function to run the main simulation
  void runSim(expConfig exp, bool output);

  // Function to set up all the configuration variables for the simulation
  void setupSim(Xml* xml_file, expConfig exp, bool output, int run_number);

private:
  
};

#endif
