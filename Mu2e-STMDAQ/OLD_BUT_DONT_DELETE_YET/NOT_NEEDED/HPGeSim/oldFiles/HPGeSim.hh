/////////////////////////////////////////////////A////////////////////////
/// This module provides the HPGe simulation (header)
/////////////////////////////////////////////////A////////////////////////

#ifndef HPGESIM_hh
#define HPGESIM_hh

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

// Data variables information
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// XML interface
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Binary file code
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// UDP
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

#ifndef HPGESIM_hh_DEFINED
#define HPGESIM_hh_DEFINED
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

// Accelator config parameter struct
struct modeConfig{

  // The period number
  uint periodNum;
  // Mode (on-spill/off-spill)
  uint16_t mode;
  // The period length
  double period;
  // Number of events in the period
  uint eventNum;

};

// Packet struct for a MI cycle containing 
// the UDP packet struct and the number of packets
struct pStruct{

  packet* pack;
  uint pNum;

};

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
  
  // Two threaded arrays for generation / sending
  static const uint16_t thArrayNum = 2; 

  // Event counter to add to trigger header
  uint64_t eventCount = 0; 
  // Total number of ADC values in a given sampling period
  unsigned long int* sampleLen;
  // Data array of ADC values
  int16_t*** ADC;
  // Event length variable
  uint** eLen;
  // The event origin of each sample
  uint*** eOrigin;
  // The generic pulse to be calculated
  int16_t* pulse;
  // The MI cycle pStruct
  pStruct **MIcycle;

  // Return average beam (on-spill) rate in detector
  double getOnSpillRate(){
    return onSpillRateHz;
  }
  // Return average beam (on-spill) rate in detector
  double getOffSpillRate(){
    return offSpillRateHz;
  }

  // Struct containing header info
  struct headerInfo{
    
    uint32_t macroCount;
    uint64_t macroTime;
    uint16_t mode;
    uint32_t microCount;
    uint16_t channel;
    uint32_t ADCoffset;  
    
  };

  // External (beam) [0] or internal (source) mode [1]
  uint16_t mode = 0;
  // Channel (HPGe [0] or LaBr [1])
  uint16_t channel = 0;

  // External ADC offset in us
  double extADCoffset = 0;

  // External trigger timeout in us
  double extTrigTimeout = 0; // us                            

  // External/internal delay in us
  double extIntDelay = 0; // us

  // Internal ADC offset in us
  double intADCoffset = 0; // us

  // Function that returns that value of a simulated pulse for a given time
  int16_t pulseCalc(double x){
    
    int16_t pulse = -(twiceA / (1 + exp(-(x - xshift)*invtaufall)) ) 
      * ( 1.0  - ( 1.0/ (1 + exp(-(x - xshift)*invtaudecay))));
    
    return pulse;
    
  }
  
  // **************************************************
  // Define main code functions below
  // **************************************************
      
  // Function to form arrays of events and packets
  void formEventsAndPackets(pStruct &pData, expConfig exp, modeConfig md);

  // Function to calculate a generic pulse as an array 
  int16_t *genPulse(expConfig exp);

  // Function to generate ADC noise
  int16_t* genNoise(modeConfig md);  
  
  // Function to generate pulses and add to noisy ADC data
  int16_t* genPulses(int16_t *ADCtmp, expConfig exp, modeConfig md);
  
  // Function to fill packets with ADC
  void fillPackets(int16_t *ADCtmp, pStruct &pData, expConfig exp, modeConfig md);

  // Function to write packet data to binary file
  void writeToFile(pStruct* pData);

  // Function that simulates the accelerator super cycle
  void simMIcycle(pStruct* pData, expConfig exp, bool output);

  // Threaded function to generate all data for MI cycle
  void genData(int16_t **ADCtmp, pStruct &pData, expConfig exp, modeConfig md);

  // Function to run the main simulation
  void runSim(expConfig exp, bool output);

  // Function to initliase all simulation variables and processes
  void initSim(expConfig exp, bool output);

  // Function to set up all the configuration variables for the simulation
  void setupSim(Xml* xml_file, expConfig exp, bool output, int run_number);

  // Function to check mode
  void checkMode(uint16_t mode){
    // If mode is not 0 or 1, throw error and exit
    if (mode != 0 and mode != 1){
      cout << "Error: Mode is not set to off-spill [0] or on-spill [1]..." << endl;
      cout << "Exiting...\n" << endl;
      exit(0);
    }
  }

  // Increment packet counter and set to zero if at UINT32_T_MAX
  void incrementPacket(){
    // Check if packet counter is at max and zero if so
    if (packetCount == UINT32_T_MAX) {
      packetCount = 0;
    }
    else{
      // Increment packet counter
      packetCount++;
    }

  }

  // **************************************************
  // Define firmware header simulation functions below
  // **************************************************

  // Create the packet header 6 bytes (3 int16)
  // Packet number 4 bytes (int16_t)
  uint16_t* sim_fw_pHdr(uint32_t pNum){
    
    // Split packet number (4 bytes) into (2 bytes)
    uint16_t pn1 = pNum & 0x0000FFFF;
    uint16_t pn2 = pNum >> 16;
    
    // Store packet header
    uint16_t* pH = new uint16_t[fw_pHdr_Len]();
    pH[fw_pHdr_pNum1] = pn1;
    pH[fw_pHdr_pNum2] = pn2;
    pH[fw_pHdr_checksum] = 0XFFEE;
    
    // Return array
    return pH;
    
  }

  // Create the trigger header (32 bytes), 4 x 64-bit numbers
  uint16_t* sim_fw_tHdr(uint8_t channel, uint8_t mode, uint64_t trigTime,
			uint32_t offset, uint32_t trigNo, uint16_t microCount,
			uint16_t sliceNo, uint32_t sliceSize){

    // Initialise word array
    uint16_t* tH = new uint16_t[fw_tHdr_Len]();

    // WORD 0
    uint8_t W0[8] = {0xAB,0xCD,0xEF,0x12,0xBE,0xEF,0x00,0x01};
    uint64_t WORD0 = (uint64_t)W0[0] << 56 | (uint64_t)W0[1] << 48 | (uint64_t)W0[2] << 40 | (uint64_t)W0[3] << 32 | (uint64_t)W0[4] << 24 | (uint64_t)W0[5] << 16 | (uint64_t)W0[6] << 8 | (uint64_t)W0[7];
    // Split into four 16-bit word
    tH[fw_tHdr_0] = WORD0 & 0x0000FFFF;
    tH[fw_tHdr_1] = WORD0 >> 16;
    tH[fw_tHdr_2] = WORD0 >> 32;
    tH[fw_tHdr_3] = WORD0 >> 48;  
  
    // WORD 1
    uint8_t chanMode = (uint8_t)channel << 4 | (uint8_t)mode;
    uint64_t WORD1 =  (uint64_t)chanMode << 56 | (uint64_t)trigTime;
    // Split into four 16-bit words
    tH[fw_tHdr_TrigTime1] = WORD1 & 0x0000FFFF;
    tH[fw_tHdr_TrigTime2] = WORD1 >> 16;
    tH[fw_tHdr_TrigTime3] = WORD1 >> 32;
    tH[fw_tHdr_ChMdTm4] = WORD1 >> 48;

    // WORD 2
    uint64_t WORD2 = (uint64_t)offset << 32  | (uint64_t)trigNo;
    tH[fw_tHdr_TrigNum1] = WORD2 & 0x0000FFFF;
    tH[fw_tHdr_TrigNum2] = WORD2 >> 16;
    tH[fw_tHdr_ADCoffset1] = WORD2 >> 32;
    tH[fw_tHdr_ADCoffset2] = WORD2 >> 48;

    // WORD 3
    uint64_t WORD3 = (uint64_t)microCount << 48  | (uint64_t)sliceNo << 32 | (uint64_t)sliceSize;
    tH[fw_tHdr_sLen1] = WORD3 & 0x0000FFFF;
    tH[fw_tHdr_sLen2] = WORD3 >> 16;
    tH[fw_tHdr_sNum] = WORD3 >> 32;
    tH[fw_tHdr_MicroCount] = WORD3 >> 48;

    // Return array
    return tH;
  
  }
  
  // Create the slice header (8 bytes), 1 x 64-bit number
  uint16_t* sim_fw_sHdr(uint64_t sliceNo){
    
    // Initialise word array
    uint16_t* sH = new uint16_t[fw_sHdr_Len]();
    
    // Split into four 16-bit words
    uint64_t WORD0 = sliceNo;
    sH[fw_sHdr_sNum0] = WORD0 & 0x0000FFFF;
    sH[fw_sHdr_sNum1] = WORD0 >> 16;
    sH[fw_sHdr_sNum2] = WORD0 >> 32;
    sH[fw_sHdr_sNum3] = WORD0 >> 48;
    
    // Return array
    return sH;
    
  }
  

private:

  // Average on-spill rate in detector
  //  const double onSpillRateHz = 2e3; // Hz
  const double onSpillRateHz = 100e3; // Hz
  // Average source (internal) rate in detector
  const double offSpillRateHz = 1e2; // Hz
  
  //------------Variables of the pulses generated-----------------------
  
  //The amplitude of the pulse is the half of this value
  const double twiceA=2370;
  //The x point in where we have the pulse (50 us)
  const double xshift=100;
  //fall from baseline time
  const double invtaufall=6;
  //decaytime(rise to baseline)
  const double invtaudecay=0.028;
  // Standard deviation of gaussian noise
  const double noiseSD = 10;
  //Pulse duration (microsec)
  const double pulseLength=xshift+220;
  
};

#endif
