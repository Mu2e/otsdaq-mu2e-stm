/////////////////////////////////////////////////////////////////////////
/// This module forms UDP packets to be sent (header)
/////////////////////////////////////////////////////////////////////////

#ifndef FORM_UDPPACKETS_hh
#define FORM_UDPPACKETS_hh

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

// UDP send/receive
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Data variables
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// ADC emulator
#include "STMDAQ-TestBeam/HPGeSim/emulate_ADC.hh"

#ifndef FORM_UDPPACKETS_hh_DEFINED
#define FORM_UDPPACKETS_hh_DEFINED
class formPackets;
#endif

using namespace std;

// Packet struct for a MI cycle containing 
// the UDP packet struct and the number of packets
struct pStruct{

  //  UDPsocket::packet* pack;
  uint pNum;

};

class formPackets{

public:
  
  // Standard constructor - shouldn't be used
  formPackets();

  // Packet counter  
  uint32_t packetCount = 0;

  // The MI cycle pStruct
  pStruct **MIcycle;

  // **************************************************
  // Define main code functions below
  // **************************************************      

  // Function to fill packets with ADC
  void fillPackets(emulateADC emADC, uint MIcount, 
		   int16_t *ADCtmp, pStruct &pData, 
		   expConfig exp, modeConfig md);
  
  // Function to create arrays of packets
  void createPackets(emulateADC emADC, pStruct &pData, expConfig exp, modeConfig md);

  // Function to initliase packet structure
  void initPackets(emulateADC emADC, expConfig exp, bool output);

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
    pH[fw_pHdr_end] = fw_pHdr_end_data;

    // Return array
    return pH;

  }
  

private:
  
};

#endif
