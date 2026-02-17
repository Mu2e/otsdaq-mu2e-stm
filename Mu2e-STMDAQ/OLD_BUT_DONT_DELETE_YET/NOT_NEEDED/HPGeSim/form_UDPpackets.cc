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

// Header
#include "STMDAQ-TestBeam/HPGeSim/form_UDPpackets.hh"

using namespace std;

UDPsocket udp;

uint pTotPerMIcycle = 0;

// Standard constructor - shouldn't be used
formPackets::formPackets() {}

// Function to fill pakcets with ADC data 
void formPackets::fillPackets(emulateADC emADC, uint MIcount, int16_t *ADC, pStruct &pData, expConfig exp, modeConfig md) {

  // Get period number
  uint p = md.periodNum;

  // Get on-spill/off-spill mode
  uint16_t mode = md.mode;

  // Check for mode error
  emADC.checkMode(mode);

  // Get the number of events in this sampling period
  uint eventNum = md.eventNum;  

  // Get the number of packets for this MI cycle period
  uint16_t packetNum = pData.pNum;
  // Initialise an event counter
  uint16_t eventCount = 0;
  // Calculate the initial global packet num from the MI cycle number
  uint16_t globPacketNum = MIcount*pTotPerMIcycle;
  // Calculate the initial global event num from the MI cycle number
  uint64_t globEventNum = MIcount*totEventsMIcycle;
  // Loop over all MI cycles preceeding this one
  for (uint i = 0; i < p; i++){
    // Increase the global packet number by the
    // number of periods so far in this MI cycle
    globPacketNum += MIcycle[0][i].pNum;
    // Increase the global event number
    globEventNum += pM[i].eventNum;
  }
  // Counter of what's left in the packet
  uint16_t leftInPacket = 0;
  bool splitEvent = false;
  // Initialise variable for event to add
  uint16_t eventToAdd = emADC.eLen[p][0];
  // Loop over packets
  for (uint i = 0; i < packetNum; i++){
    // Count the total number of data entries per packet                                     
    unsigned long int count = 0;
    // Increment the global packet number
    globPacketNum++;
    // Get the packet length (half the packet size in bytes) 
    uint16_t packetLen = MAX_PACKET_LEN;
    // Get packet header
    uint16_t *packetHeader = sim_fw_pHdr(uint32_t(globPacketNum));
    // Memcpy packet header to packet 	
    //    memcpy(pData.pack[i].data+count, packetHeader, fw_pHdr_Len * sizeof *packetHeader);
    // Account for the packet header length
    leftInPacket = packetLen - fw_pHdr_Len;
    // Increment data counter  
    count += fw_pHdr_Len; 
    // Infinite loop
    while(1){
      // If remaining packet space is <= a trigger header...
      if (fw_tHdr_Len >= leftInPacket){
	// Check that the remainder in this packet is zero
	if (leftInPacket != 0){
	  cout << "Error 1! Packet remainder should be zero. Exiting..." << endl;
	  exit(0);
	}
  	// else, PACKET COMPLETE
	else{ 	  
	  // Exit loop and start new packet
	  break;
	}
      }
      // // Get trigger header 	
      // uint16_t *triggerHeader = sim_fw_tHdr(0, // Channel
      // 					    mode,
      // 					    0, // Event time
      // 					    0, // ADC offset
      // 					    globEventNum, 
      // 					    0, // microCount
      // 					    0, // Slice number
      // 					    0); // Slice time
      // // Memcpy trigger header to packet 	
      // memcpy(pData.pack[i].data+count, triggerHeader, fw_tHdr_Len * sizeof *triggerHeader);
      // Account for the trigger header length
      leftInPacket -= fw_tHdr_Len;
      // Increment data counter  
      count += fw_tHdr_Len; 
      // If the event if larger then the packet remainder
      if (eventToAdd > leftInPacket){
  	// Get the start point of the event to add
  	uint ADCstart = emADC.eOrigin[p][eventCount][emADC.eLen[p][eventCount]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = emADC.eOrigin[p][eventCount][emADC.eLen[p][eventCount]-eventToAdd+leftInPacket-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// Find the size of that data to add in bytes
	uint dataSize = 2*dataLen;
	// Memcpy ADC data to packet 	
	//	memcpy(pData.pack[i].data+count, ADC+ADCstart, dataSize);
	// Account for the data length
	leftInPacket -= dataLen;
	// Increment data counter  
	count += dataLen; 
	// Subtract trigger header from eventToAdd
	eventToAdd -= dataLen;
	// Check that the remainder in this packet is zero
	if (leftInPacket != 0){
	  cout << "Error 2! Packet remainder should be zero. Exiting..." << endl;
	  exit(0);
	}
	// Check that the counter is the same size as the packet
	if (count != packetLen){
	  cout << "Error 3! Packet counter not the same size as the packet. Exiting..." << endl;
	  exit(0);
	}
      }
      // If the event if less than the packet remainder
      else{
  	// Get the start point of the event to add
  	uint ADCstart = emADC.eOrigin[p][eventCount][emADC.eLen[p][eventCount]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = emADC.eOrigin[p][eventCount][emADC.eLen[p][eventCount]-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// Find the size of that data to add in bytes
	uint dataSize = 2*dataLen;
	// Memcpy ADC data to packet 	
	//	memcpy(pData.pack[i].data+count, ADC+ADCstart, dataSize);
	// Account for the data length
	leftInPacket -= dataLen;
	// Increment data counter  
	count += dataLen; 
	// Subtract trigger header from eventToAdd
	eventToAdd -= dataLen;
	// Check that the remainder of the event to add is zero
	if (eventToAdd != 0){
	  cout << "Error 4! Packet remainder should be zero. Exiting..." << endl;
	  exit(0);
	}
	// Increment the event counter
	eventCount++;
	// Increment the global event counter
	globEventNum++;
	// Set event to add to new event length
	eventToAdd = emADC.eLen[p][eventCount];
      }
    }   
  }

  return;

}


// Function to form arrays of packets 
void formPackets::createPackets(emulateADC emADC, pStruct &pData, expConfig exp, modeConfig md){

  // Get period number
  uint p = md.periodNum;

  // Get on-spill/off-spill mode
  uint16_t mode = md.mode;

  // Check for mode error
  emADC.checkMode(mode);

  // Get the number of events in this sampling period
  uint eventNum = md.eventNum; 

  // Initialise packet vectors of int16_ts
  vector<vector<int16_t>> packetVec;
  packetVec.push_back(vector<int16_t>());
  // Counter of what's left in the packet
  uint16_t leftInPacket = MAX_PACKET_LEN;
  // Packet number counter
  uint16_t packetNum = 0;
  // Boolen for a new packet
  bool newPacket = true;
  // Loop over total number of events
  for (uint i = 0; i < eventNum; i++){
    // Get the length of the event
    uint eventToAdd = emADC.eLen[p][i];
    // While there is still some of the event to add
    while(eventToAdd != 0){
      // If remaining packet space is <= a trigger header...
      if (fw_tHdr_Len >= leftInPacket){
	cout << "Left in packet = " << leftInPacket << endl;
  	// PACKET COMPLETE
  	// Increase the packet number
  	packetNum++;
  	// Set new packet to true
  	newPacket = true;
  	// Push back the packet vector
  	packetVec.push_back(vector<int16_t>());
  	// Set left in packet to max
  	leftInPacket = MAX_PACKET_LEN;
      }
      // If we're in a new packet
      if (newPacket){
  	// Set new packet to false
  	newPacket = false;
  	// Account for the packet header length
  	leftInPacket = MAX_PACKET_LEN - fw_pHdr_Len;
  	// // Add packet header length to packet vector	
  	// packetVec[packetNum].resize(packetVec[packetNum].size() + fw_pHdr_Len);
      }       
      // Account for the trigger header length
      leftInPacket -= fw_tHdr_Len;
      cout << "fw_tHdr_Len = " << fw_tHdr_Len << endl;
      exit(0);
      // Add trigger header length to packet vector	
      packetVec[packetNum].resize(packetVec[packetNum].size() + fw_tHdr_Len);
      // If the event if larger then the packet remainder
      if (eventToAdd > leftInPacket){
  	// Get the start point of the event to add
  	uint ADCstart = emADC.eOrigin[p][i][emADC.eLen[p][i]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = emADC.eOrigin[p][i][emADC.eLen[p][i]-eventToAdd+leftInPacket-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// // Add data length to packet vector	
	// packetVec[packetNum].resize(packetVec[packetNum].size() + dataLen);
       	// Subtract from the event left to add 
       	eventToAdd -= leftInPacket;
  	// PACKET COMPLETE
  	// Increase the packet number
  	packetNum++;
  	// Set new packet to true
  	newPacket = true;
  	// Push back the packet vector
  	packetVec.push_back(vector<int16_t>());
  	// Set left in packet to max
  	leftInPacket = MAX_PACKET_LEN;
      }
      // If the event if less than the packet remainder
      else{
  	// Account for the event filling the packet
  	leftInPacket -= eventToAdd;
  	// Get the start point of the event to add
  	uint ADCstart = emADC.eOrigin[p][i][emADC.eLen[p][i]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = emADC.eOrigin[p][i][emADC.eLen[p][i]-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// // Add data length to packet vector	
	// packetVec[packetNum].resize(packetVec[packetNum].size() + dataLen);
       	// Subtract from the event left to add 
       	eventToAdd -= eventToAdd;
      }
    }
  }

  // Store packet number in struct to return
  pData.pNum = packetNum;
  // Convert vector of packets to array of packets
  // Intialise packect array in struct to return
  //  pData.pack = new UDPsocket::packet [packetNum];
  // Loop over packets
  //  for (int i = 0; i < packetNum; i++){   
    // Initialise packet array in struct with packet size
    //    pData.pack[i].data = new int16_t [packetVec[i].size()] ();
    // // Store packet size in struct to return
    // pData.pack[i].size = 2*packetVec[i].size();
  //  }

  return;

}


// Function to initliase packet structure
void formPackets::initPackets(emulateADC emADC, expConfig exp, bool output){

  // The MI cycle pStruct
  MIcycle = new pStruct*[emADC.genArrayNum];
  for (int i = 0; i < emADC.genArrayNum; i++) MIcycle[i] = new pStruct [totSamplePeriods];

  // Form MI cycle events for Mu2e smimulation 
  if (output) cout << "Forming packets..." << endl;
  for (uint i = 0; i < totSamplePeriods; i++){
    // Create packets
    createPackets(emADC, MIcycle[0][i],exp,pM[i]);
  }
  // Duplicate second gen/send MI cycle pStruct 
  MIcycle[1] = MIcycle[0];

  // Loop over sample periods
  for (uint i = 0; i < totSamplePeriods; i++){
    // Get total number of packets per MI cycle    
    pTotPerMIcycle += MIcycle[0][i].pNum;
  }

}



