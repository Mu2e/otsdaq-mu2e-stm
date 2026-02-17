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

#include "STMDAQ-TestBeam/utils/ziggurat.hh"

// Extract data
#include "STMDAQ-TestBeam/processData/extractData.hh"

using namespace std;

// Extract data class  
extractData exData; 

// Boolean to set ADC emulation (incrementing counter)
static const bool ADCemulate = true;

// Boolean to write simulation data to file 
static const bool writeSimData = false;

// Boolean to generate ADC noise
static const bool noise = true;

// Boolean to use ziggurat algorithm to calculate gaussian noise
static const bool ziggurat = true;

// On-spill event width (us)
//static const double onEventWidth = 1.6949722; // 1695 ns
static const double onEventWidth1 = 1.700; // 1700 ns
static const double onEventWidth2 = 1.675; // 1675 ns
static const double onEventCycle = 4*onEventWidth1 + 1*onEventWidth2;
static const double onEventWidth = onEventCycle/5;

// Spill width (us)
static const double spillWidthInit = 43120; // 43.12 ms

// Number of events in an on-spill cycle
static const double onEventCycleNum = ceil(spillWidthInit/onEventCycle);

// Number of events per spill
static const uint onEventsSpill = onEventCycleNum*5;

static const double spillWidth = 4*onEventCycleNum*onEventWidth1
  +onEventCycleNum*onEventWidth2;

// Off-spill event width (us)
static const double offEventWidth = 100; // 100 us

// Regular spill gap(us)
static const double regSpillGap = 5000; // 5 ms

// Number of off-spill events per regular spill gap
static const uint offEventsRegGap = regSpillGap/offEventWidth;

// Gap between spill 4 and 5 (us)
//static const double spillGap4to5 = 41880; // 41.88 ms

// Number of off-spill events in gap between spill 4 and 5
//static const uint offEventsGap4to5 = spillGap4to5/offEventWidth;

// Gap between spill 8 and 1 (us)
static const double spillGap8to1 = 1020000; // 1020 ms
//static const double spillGap8to1 = 983000; // 983 ms

// Number of off-spill events in gap between spill 8 and 1
static const uint offEventsGap8to1 = spillGap8to1/offEventWidth;

// Number of spills per MI cycle
static const uint spillsPerMIcycle = 8;
 
// Total number of on and off spill events per MI cycle
static const uint totEventsMIcycle = onEventsSpill + offEventsRegGap + // Spill 1
  onEventsSpill + offEventsRegGap + // Spill 2
  onEventsSpill + offEventsRegGap + // Spill 3
  onEventsSpill + offEventsRegGap + // Spill 4
  onEventsSpill + offEventsRegGap + // Spill 5
  onEventsSpill + offEventsRegGap + // Spill 6
  onEventsSpill + offEventsRegGap + // Spill 7
  onEventsSpill + offEventsGap8to1; // Spill 8

// Total number of on-spill events per MI cycle
static const uint totOnEventsMIcycle = spillsPerMIcycle*onEventsSpill; 

// Total number of off-spill events per MI cycle
static const uint totOffEventsMIcycle = 7*offEventsRegGap + // 7 regular gaps per MI cycle
  offEventsGap8to1; // Gap between spills 8 and 1

// Number of MI cycles per super-cycle
//static const uint numMIcycles = 40;
static const uint numMIcycles = 1;

// Time for one test beam cycle (us)
static const double timeTBcycle = 4*1e6; // 4 secs

// Total time for each MI cycle (us)
static const double totTimeMIcycle = spillWidth + regSpillGap + // Spill 1
  spillWidth + regSpillGap + // Spill 2
  spillWidth + regSpillGap + // Spill 3
  spillWidth + regSpillGap + // Spill 4
  spillWidth + regSpillGap + // Spill 5
  spillWidth + regSpillGap + // Spill 6
  spillWidth + regSpillGap + // Spill 7
  spillWidth + spillGap8to1; // Spill 8

// Total time for each super cycle
static const double totTimeSuperCycle = numMIcycles*totTimeMIcycle // 40 MI cycles
  + timeTBcycle; // ... and one test beam cycle

// Number of distinct super cycles
//static const uint numSuperCycles = 60;
static const uint numSuperCycles = 1;

// Set all initial event modes in MI cycle to 0 (on-spill)
int16_t* eventModesPerMIcycle;

// On-spill mode
static const uint16_t onMode = 0;
// On-spill mode
static const uint16_t offMode = 1;


// Total number of defined sample periods
// 8*onEventsSpill + 7*offEventsRegGap + offEventsGap8to1 = 16
static const uint totSamplePeriods = 20; 

// Structs of periods and modes for 1 MI cycle
static const struct modeConfig pM[totSamplePeriods] = {
  {0,onMode,spillWidth,onEventsSpill}, // Spill 1, on-spill
  {1,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {2,onMode,spillWidth,onEventsSpill}, // Spill 2, on-spill
  {3,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {4,onMode,spillWidth,onEventsSpill}, // Spill 3, on-spill
  {5,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {6,onMode,spillWidth,onEventsSpill}, // Spill 4, on-spill
  {7,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  //  {7,offMode,spillGap4to5,offEventsGap4to5}, // Gap 4 --> 5, off-spill
  {8,onMode,spillWidth,onEventsSpill}, // Spill 5, on-spill
  {9,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {10,onMode,spillWidth,onEventsSpill}, // Spill 6, on-spill
  {11,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {12,onMode,spillWidth,onEventsSpill}, // Spill 7, on-spill 
  {13,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {14,onMode,spillWidth,onEventsSpill}, // Spill 4, on-spill
  {15,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part1], off-spill
  {16,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part2], off-spill
  {17,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part3], off-spill
  {18,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part4], off-spill
  {19,offMode,spillGap8to1/5,offEventsGap8to1/5}}; // Gap 8 --> 1 [part5] off-spill

static const uint16_t maxPacketSize = 8192; // 8.192 kBytes
static const uint16_t maxPacketLen = maxPacketSize/2;; 

uint MIcount = 0;
uint pTotPerMIcycle = 0;

// Standard constructor - shouldn't be used
HPGeSim::HPGeSim() {}

// Function to form arrays of events and packets 
void HPGeSim::formEventsAndPackets(pStruct &pData, expConfig exp, modeConfig md){

  // Get period number
  uint p = md.periodNum;

  // Get on-spill/off-spill mode
  uint16_t mode = md.mode;
  
  // Check for mode error
  checkMode(mode);
   
  // Total number of ADC values in that sampling period
  sampleLen[p] = 0;
  
  // Get the number of events in this sampling period
  uint eventNum = md.eventNum; 

  // On-spill mode
  if (mode == 0){
    uint eLen = onEventWidth/exp.adc_time_clock_period;
    uint eLen1 = onEventWidth1/exp.adc_time_clock_period;
    uint eLen2 = onEventWidth2/exp.adc_time_clock_period;
    sampleLen[p] = 4*onEventCycleNum*eLen1 + onEventCycleNum*eLen2;
  }
   // Off-spill mode
   else if (mode == 1) {
     uint eLen = offEventWidth/exp.adc_time_clock_period;
     sampleLen[p] = eventNum*eLen;
   }

  // Initialise cycle counter
  int cycleCount = 0;
  // Initialise event length variable
  eLen[p] = new uint [eventNum];
  // The event origin of each sample
  eOrigin[p] = new uint* [eventNum] ();
  // Sample index counter
  unsigned long int sampleCount = 0;
  // Loop over all events
  for (uint i = 0; i < eventNum; i++){
    // If on-spill
    if (mode == 0){
      // Increment on-spill cycle counter
      cycleCount++;
      // If at last event (5) of on-spill cycle..
      if (cycleCount == 5){	// Event length = 1675 ns
	eLen[p][i] = onEventWidth2/exp.adc_time_clock_period;
	// Set cycle counter to zero
	cycleCount = 0;
      }
      // If at event 1-4 of on-spill cycle
      else{
	// Event length = 1700 ns
	eLen[p][i] = onEventWidth1/exp.adc_time_clock_period;
      }
    }
    // If off-spill
    else if (mode == 1){
      // Event length = 100 us
      eLen[p][i] = offEventWidth/exp.adc_time_clock_period;
    }
    // Allocate array size
    eOrigin[p][i] = new uint[eLen[p][i]] ();
    // Loop through the length of the event
    for(uint j = 0; j < eLen[p][i]; j++){
      // Record the event origin
      eOrigin[p][i][j] = sampleCount;
      // Increment the sample index counter
      sampleCount++;
    }
  }

  // Initialise packet vectors of int16_ts
  vector<vector<int16_t>> packetVec;
  packetVec.push_back(vector<int16_t>());
  // Counter of what's left in the packet
  uint16_t leftInPacket = maxPacketLen;
  // Packet number counter
  uint16_t packetNum = 0;
  // Boolen for a new packet
  bool newPacket = true;
  // Loop over total number of events
  for (uint i = 0; i < eventNum; i++){
    // Get the length of the event
    uint eventToAdd = eLen[p][i];
    // While there is still some of the event to add
    while(eventToAdd != 0){
      // If remaining packet space is <= a trigger header...
      if (fw_tHdr_Len >= leftInPacket){
  	// PACKET COMPLETE
  	// Increase the packet number
  	packetNum++;
  	// Set new packet to true
  	newPacket = true;
  	// Push back the packet vector
  	packetVec.push_back(vector<int16_t>());
  	// Set left in packet to max
  	leftInPacket = maxPacketLen;
      }
      // If we're in a new packet
      if (newPacket){
  	// Set new packet to false
  	newPacket = false;
  	// Account for the packet header length
  	leftInPacket = maxPacketLen - fw_pHdr_Len;
  	// Add packet header length to packet vector	
  	packetVec[packetNum].resize(packetVec[packetNum].size() + fw_pHdr_Len);
      }       
      // Account for the trigger header length
      leftInPacket -= fw_tHdr_Len;
      // Add trigger header length to packet vector	
      packetVec[packetNum].resize(packetVec[packetNum].size() + fw_tHdr_Len);
      // If the event if larger then the packet remainder
      if (eventToAdd > leftInPacket){
  	// Get the start point of the event to add
  	uint ADCstart = eOrigin[p][i][eLen[p][i]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = eOrigin[p][i][eLen[p][i]-eventToAdd+leftInPacket-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// Add data length to packet vector	
	packetVec[packetNum].resize(packetVec[packetNum].size() + dataLen);
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
  	leftInPacket = maxPacketLen;
      }
      // If the event if less than the packet remainder
      else{
  	// Account for the event filling the packet
  	leftInPacket -= eventToAdd;
  	// Get the start point of the event to add
  	uint ADCstart = eOrigin[p][i][eLen[p][i]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = eOrigin[p][i][eLen[p][i]-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// Add data length to packet vector	
	packetVec[packetNum].resize(packetVec[packetNum].size() + dataLen);
       	// Subtract from the event left to add 
       	eventToAdd -= eventToAdd;
      }
    }
  }

  // Store packet number in struct to return
  pData.pNum = packetNum;
  // Convert vector of packets to array of packets
  // Intialise packect array in struct to return
  pData.pack = new packet [packetNum];
  // Loop over packets
  for (int i = 0; i < packetNum; i++){   
    // Initialise packet array in struct with packet size
    pData.pack[i].data = new int16_t [packetVec[i].size()] ();
    // Store packet size in struct to return
    pData.pack[i].size = 2*packetVec[i].size();
  }
  
  return;
  
}

// Function to calculate a generic pulse as an array
int16_t* HPGeSim::genPulse(expConfig exp){
  
  // Get the pulse length in number of int16_t values
  uint pulseLen = pulseLength/exp.adc_time_clock_period;
  
  // Allocate the temporary array for the pulse
  int16_t *pulseTmp = new int16_t[pulseLen];

  // Loop over the pulse length
  for (uint t = 0; t < pulseLen; t++){
    // Get the pulse time in us
    double time = t*exp.adc_time_clock_period;
    // Get the value of the pulse at that time
    pulseTmp[t] = pulseCalc(time);
  }

  // Return the pulse
  return pulseTmp;

}

// Function to generate ADC noise 
int16_t* HPGeSim::genNoise(modeConfig md){

  // Get period number
  uint p = md.periodNum;
  
  // // Get on-spill/off-spill mode
  uint16_t mode = md.mode;
  
  // // Check for mode error
  checkMode(mode);

  // Get the number of events in this sampling period
  uint eventNum = md.eventNum; 

  // Sample index counter
  unsigned long int sampleCount = 0;

  // Define the ziggurat algorithm variables
  float fn[128];
  uint32_t kn[128];
  int sample;
  float value;
  float wn[129];

  // Set up the ziggurat algorithm 
  r4_nor_setup ( kn, fn, wn );

  // Get a time-based random seed to use for the ziggurat algorithm
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

  // Define temp array for ADC values
  int16_t* ADCtmp = new int16_t [sampleLen[p]];

  // Randomly generate noise from gaussian distirbution about 0
  // The input parameter is noiseSD = 0.17 mV (standard deviation of gaussian in mv)
  //  double sigma_noise_ADC = noiseSD*38.5; // WHAT IS 38.5??
  double sigma_noise_ADC = 0.17*38.5; // WHAT IS 38.5???
  // Get generator
  default_random_engine generator;
  // Sample from guassian distribution
  normal_distribution<double> distribution(0,sigma_noise_ADC);

  // Loop over sample length
  for (uint i = 0; i < sampleLen[p]; i++){
    // If simulating incrementing ADC emulation
    if (ADCemulate){
      ADCtmp[i] = i;
    }
    // If not simulating incrementing ADC emulation
    else {
      // If generating noise
      if (noise){
	// If using ziggurat algortim to genrate noise
	if (ziggurat){
	  ADCtmp[i] = int(r4_nor ( seed, kn, fn, wn ) * sigma_noise_ADC);
	}
	// Else if generating noise from c++ normal distribution
	else{
	  //Generate the noise in ADC counts
	  ADCtmp[i] = distribution(generator);
	}
      }
    }
  }   
  
  return ADCtmp;

}
 
// Function to get a period's worth of data
int16_t* HPGeSim::genPulses(int16_t *ADCtmp, expConfig exp, modeConfig md) {

  // Get period number
  uint p = md.periodNum;
  
  // Get on-spill/off-spill mode
  uint16_t mode = md.mode;
  
  // Check for mode error
  checkMode(mode);

  // Get the number of events in this sampling period
  uint eventNum = md.eventNum; 

  // From mode, get average rate in detector (on-spill, off-spill, internal)
  double rateHz = 0;
  // On-spill mode
  if (mode == 0){
    rateHz = getOnSpillRate();
   }
  // Off-spill mode
  else if (mode == 1) {
    rateHz = getOffSpillRate();
  }

  // Get length in time of the sampling period (on-spill / off-spill gap)
  double sampleTime = sampleLen[p]*exp.adc_time_clock_period;;
  // Total sample size in bytes
  unsigned long int sampleSize = 2*sampleLen[p];

  // Rate in us
  double rate = rateHz * 1e-6; // 1kHz in us
  // Average number of pulses in the sample
  int pulseNum = rate*sampleTime;

  // Define temp array for ADC values
  //  int16_t* ADCtmp = new int16_t [sampleLen[p]];

  // If not simulating incrementing ADC emulation
  if (!ADCemulate){
    // Randomly generate pulse times for given rate within sample time
    random_device rd;
    // Sample from a poisson distributions
    poisson_distribution<int> pulseTime (1/rate) ;
    // Get the generator
    mt19937 rnd_gen(rd()) ;
    
    // Now, calculate and add pulses to ADC array
    int timeIndex = 0;
    // Get the pulse length in number of int16_t values
    uint pulseLen = pulseLength/exp.adc_time_clock_period;
    // Loop over average number of pulses
    for (int pn = 0; pn < pulseNum; pn++){
      // Randonly generate pulse times in clock ticks
      timeIndex += int(pulseTime(rnd_gen)/exp.adc_time_clock_period);
      // If the pulse will go over the edge of the sample, skip
      if((timeIndex+pulseLen)>=sampleLen[p]) continue;
      // Get the index value of the last element in the pulse
      uint max = timeIndex + pulseLen;
      // If the pulse exceeds the sample length, max out at the sample length
      if (timeIndex + pulseLen > sampleLen[p]) max = sampleLen[p]-1;
      // Loop over clock ticks
      for (uint t = timeIndex; t < max; t++){
	// Find the time starting from zero
	double time = (t-timeIndex)*exp.adc_time_clock_period;
	// Add pulse data and add to ADC array
	ADCtmp[t] += pulse[t-timeIndex];
      }
    }
  }

  return ADCtmp;
  
}

// Function to fill pakcets with ADC data 
void HPGeSim::fillPackets(int16_t *ADC, pStruct &pData, expConfig exp, modeConfig md) {

  // Get period number
  uint p = md.periodNum;

  // Get on-spill/off-spill mode
  uint16_t mode = md.mode;
  
  // Check for mode error
  checkMode(mode);

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
  uint16_t eventToAdd = eLen[p][0];
  // Loop over packets
  for (uint i = 0; i < packetNum; i++){
    // Count the total number of data entries per packet                                     
    unsigned long int count = 0;
    // Increment the global packet number
    globPacketNum++;
    // Get the packet length (half the packet size in bytes) 
    uint16_t packetLen = pData.pack[i].size/2;
    // Get packet header
    uint16_t *packetHeader = sim_fw_pHdr(uint32_t(globPacketNum));
    // Memcpy packet header to packet 	
    memcpy(pData.pack[i].data+count, packetHeader, fw_pHdr_Len * sizeof *packetHeader);
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
      // Get trigger header 	
      uint16_t *triggerHeader = sim_fw_tHdr(0, // Channel
					    mode,
					    0, // Event time
					    0, // ADC offset
					    globEventNum, 
					    0, // microCount
					    0, // Slice number
					    0); // Slice time
      // Memcpy trigger header to packet 	
      memcpy(pData.pack[i].data+count, triggerHeader, fw_tHdr_Len * sizeof *triggerHeader);
      // Account for the trigger header length
      leftInPacket -= fw_tHdr_Len;
      // Increment data counter  
      count += fw_tHdr_Len; 
      // If the event if larger then the packet remainder
      if (eventToAdd > leftInPacket){
  	// Get the start point of the event to add
  	uint ADCstart = eOrigin[p][eventCount][eLen[p][eventCount]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = eOrigin[p][eventCount][eLen[p][eventCount]-eventToAdd+leftInPacket-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// Find the size of that data to add in bytes
	uint dataSize = 2*dataLen;
	// Memcpy ADC data to packet 	
	memcpy(pData.pack[i].data+count, ADC+ADCstart, dataSize);
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
  	uint ADCstart = eOrigin[p][eventCount][eLen[p][eventCount]-eventToAdd];
  	// Get the end point of the event to add
        uint ADCend = eOrigin[p][eventCount][eLen[p][eventCount]-1];
	// Find the length of the data to add 
	uint dataLen = ADCend-ADCstart+1;
	// Find the size of that data to add in bytes
	uint dataSize = 2*dataLen;
	// Memcpy ADC data to packet 	
	memcpy(pData.pack[i].data+count, ADC+ADCstart, dataSize);
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
	eventToAdd = eLen[p][eventCount];
      }
    }   
  }
  
    return;

}

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
      exData._bf->write_raw_data(pData[i].pack[j].data,pData[i].pack[j].size/2);
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
      // for (uint k = 0; k < 10; k++){
      // 	if (i == 0 && j == 0) cout << k << " " << pData[i].pack[j].data[k] << endl;
      // }
      _UDPout->sendPacket(pData[i].pack[j],socket);
    }
    auto sendEnd = std::chrono::high_resolution_clock::now();
    double sendTime = chrono::duration <double,micro> (sendEnd-sendStart).count();
    // If the send time is less than the sample period
    if (sendTime < pM[i].period){
      // Sleep for the remainder of the sample period      
      double sleepTime = pM[i].period - sendTime;
      //      cout << pM[i].period << " - " << sendTime << " = " << sleepTime << " us " << endl;
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

// Threaded function to generate all data for MI cycle
void HPGeSim::genData(int16_t **ADCtmp, pStruct &pData, expConfig exp, modeConfig md) {

  // Get period number
  uint p = md.periodNum;
  
  // Generate noise
  ADCtmp[p] = genNoise(md);
  // Get pulse data
  if (!ADCemulate) ADCtmp[p] = genPulses(ADCtmp[p],exp,md);
  // Fill packets with ADC data
  fillPackets(ADCtmp[p],pData,exp,md);    
  
  return;
  
} 

// Function to run the main simulation
void HPGeSim::runSim(expConfig exp, bool output){

  // Initialise data generation threads
  thread *dataThread[totSamplePeriods];
  // Initialise send data thread
  thread *sendThread;
  // Initialise write to binary file thread
  thread *writeThread;

  // Boolen to signal no sending in first generation loop
  bool firstLoop = true;

  // The data generation index of thArrayNum as bool
  bool genInd = false;
  // The send data index of thArrayNum
  bool sendInd = true;

  // Get data generation start time
  auto MIstart = std::chrono::high_resolution_clock::now();

  // Infinite loop
  while(1){

    // If not the first loop, start the send thread loop
    if (!firstLoop) {
      sendThread = new thread (&HPGeSim::simMIcycle,this,MIcycle[sendInd],exp,output);
      if (writeSimData) writeThread = new thread (&HPGeSim::writeToFile,this,MIcycle[sendInd]);
    }

    // Get data generation start time
    auto dataStart = std::chrono::high_resolution_clock::now();
    
    // Calculate generic pulse
    //    cout << "Generating pulses in time and forming data packets..." << endl;
     
    // Loop over all sample periods
    for (uint i = 0; i < totSamplePeriods; i++){
      // Generate data for that sample period
      dataThread[i] = new thread (&HPGeSim::genData,this,ADC[genInd],ref(MIcycle[genInd][i]),exp,pM[i]);
      //genData(sim,ref(MIcycle[i]),exp,pM[i]);
      
    }
    
    // Loop over all sample periods
    for (uint i = 0; i < totSamplePeriods; i++){
      // Join all data generation threads back to main thread
      dataThread[i]->join();
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

// Function to initliase all simulation variables and processes
void HPGeSim::initSim(expConfig exp, bool output){

  // Initialise event variables
  // Total number of ADC values in a given sampling period
  sampleLen = new unsigned long int [totSamplePeriods] ();  
  // Event length variable
  eLen = new uint*[totSamplePeriods] ();
  // The event origin of each sample
  eOrigin = new uint**[totSamplePeriods] ();
  // ADC data array
  ADC = new int16_t**[thArrayNum] ();
  for (int i = 0; i < thArrayNum; i++) ADC[i] = new int16_t*[totSamplePeriods] ();
  // The MI cycle pStruct
  MIcycle = new pStruct*[thArrayNum];
  for (int i = 0; i < thArrayNum; i++) MIcycle[i] = new pStruct [totSamplePeriods];
  
  // Form MI cycle events for Mu2e smimulation 
  if (output) cout << "Forming events and packets..." << endl;
  for (uint i = 0; i < totSamplePeriods; i++){
    // Form events
    formEventsAndPackets(MIcycle[0][i],exp,pM[i]);
  }
  // Duplicate second gen/send MI cycle pStruct 
  MIcycle[1] = MIcycle[0];
    
  // Loop over sample periods
  for (uint i = 0; i < totSamplePeriods; i++){
    // Intialise data array of ADC values
    for (int j = 0; j < thArrayNum; j++) ADC[j][i] = new int16_t [sampleLen[i]] ();
    // Get total number of packets per MI cycle    
    pTotPerMIcycle += MIcycle[0][i].pNum;
  }

  // Calculate the generic pulse
  pulse = genPulse(exp);

}

// Function to set up all the configuration variables for the simulation
void HPGeSim::setupSim(Xml* xml_file, expConfig exp, bool output, int run_number){

  // Create the instance of the UDP socket to write data
  UDPsocket* UDPout = new UDPsocket();
  setUDPout(UDPout);

  // Create UDP socket to be sent to READ IP of frontend
  socket = UDPout->createClient(READ);

  // Create the instance to write the binary files
  BinaryFile* bf = new BinaryFile();
  exData.setBF(bf);

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

  // Set the maximum subrun file size
  exData._bf->set_subrun_filesize(max_binary_size);

  // Open the binary file to write data to
  exData._bf->open_raw_output_file(binary_file, run_number, 0);
  
  // Get external ADC offset in us
  extADCoffset = exp.ext_ADC_offset/exp.adc_offset_time_clock;
  
  // Get external trigger timeout in us
  extTrigTimeout = exp.ext_trig_timeout/exp.adc_offset_time_clock; // us
  
  // Get external/internal delay in us
  extIntDelay = exp.ext_int_delay/exp.adc_offset_time_clock; // us

  // Get internal ADC offset in us
  intADCoffset = exp.int_ADC_offset/exp.adc_offset_time_clock; // us

}



// Main function to run if not calling simulation from frontend
int main(){

  // Define instance of HPGeSim
  HPGeSim * sim = new HPGeSim();

  // Set boolean to ouput messages to screen to true
  bool output = true;
 
  // Open the XML file and instantiate the xml_file object
  string xml_path = EnvVars::expand("${STM_XML}");
  Xml* xml_file = new Xml(xml_path);

  // Set struct experimental configuration variables from xml file
  struct expConfig exp;
  exp = getXMLvalues(exp);

  // Check experimental configuration variables
  if (checkExpConfig(exp) == 0){
    cout << "Exiting due to bad value in " << xml_path << endl;
    exit(0);
  }

  // Print success to user
  cout << "Parameters in " << xml_path << " are okay!" << endl;

  // Get random run number
  int run_number = rand();

  // Setup simulation
  sim->setupSim(xml_file,exp,output,run_number);

  // Initliase simulation
  sim->initSim(exp,output);  

  // Run simulation
  sim->runSim(exp,output);  
    
  // Close the binary file
  exData._bf->close_output_file();

  return 1;
  

}
