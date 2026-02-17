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
#include "STMDAQ-TestBeam/HPGeSim/emulate_ADC.hh"
#include "STMDAQ-TestBeam/utils/ziggurat.hh"

#include "STMDAQ-TestBeam/HPGeSim/HPGe_genPulse.hh"

using namespace std;

// Create instance of HPGe_genPulse
HPGe_genPulse HPGe_pulse;

// Boolean to set ADC emulation (incrementing counter)
static const bool ADCemulate = false;

// Boolean to generate ADC noise
static const bool noise = true;

// Boolean to use ziggurat algorithm to calculate gaussian noise
static const bool ziggurat = true;

// Boolean to set full pulse simulation or not
static const bool simPulse = false;

// The pulse simulation energyx
static const double energy = 600;

// Standard constructor - shouldn't be used
emulateADC::emulateADC() {}

// Function to form arrays of events and packets 
void emulateADC::formEvents(expConfig exp, modeConfig md){

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

  return;

}

// Function to calculate a generic pulse as an array
int16_t** emulateADC::genPulseArray(expConfig exp){

  // Notify user
  cout << "Forming pulse array ";

  // Get the pulse length in number of int16_t values
  uint pulseLen = pulseLength/exp.adc_time_clock_period;

  // Rate in us
  double rate = 0;

  // If using main pulse simulation
  if (simPulse) {
    // From on-spill mode, get average rate in detector
    double rateHz = getOnSpillRate();
    // Get length in time of the on-spill sampling period
    double sampleTime = sampleLen[0]*exp.adc_time_clock_period;
    // Get rate in us
    rate = rateHz * 1e-6; // 1kHz in us
    // Average number of pulses in the sample
    genPulseNum = rate*sampleTime;
    // Tell user how many pulses
  }
  // Else if just using simple pulse...
  else{
    // There's only one pulse
    genPulseNum = 1;
  }

  cout << "with " << genPulseNum << " distinct pulses..." << endl;
  
  // Allocate the temporary array for the pulse
  int16_t **pulseTmp = new int16_t*[genPulseNum];

  // Loop over number of different pulses to generate
  for (int i = 0; i < genPulseNum; i++){
    // If using main pulse simulation
    if (simPulse){
      HPGe_pulse.fADC = exp.adc_time_clock;
      HPGe_pulse.tadc = exp.adc_time_clock_period;
      // Touch the poisson generator
      int dummy = int(Random::Instance()->PoissValue(rate)/HPGe_pulse.tadc);
      // Get the pulse
      pulseTmp[i] = HPGe_pulse.gen_pulse(energy,0,pulseLen);
      // Print pulse simulation counter to screen
      cout << "\r" << "Simulated " << i+1 << "/" << genPulseNum << " pulses" << std::flush;      
    }
    // Else if just using simple pulse...
    else{
      // Initialise pulse array with correct length
      pulseTmp[i] = new int16_t[pulseLen];
      // Loop over the pulse length
      for (uint t = 0; t < pulseLen; t++){
	// Get the pulse time in us
	double time = t*exp.adc_time_clock_period;
	// Get the value of the pulse at that time
	pulseTmp[i][t] = pulseCalc(time);
      }
    }
    
  }
  
  // End cout line after return/flush print
  if (simPulse) cout << endl;
  
  // Return the pulse
  return pulseTmp;

}

// Function to generate ADC noise 
int16_t* emulateADC::genNoise(modeConfig md){

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
	  //Generate Gaussian noise in ADC counts
	  ADCtmp[i] = Random::Instance()->GaussValue(0,sigma_noise_ADC,false);
	}
      }
    }
  }   

  return ADCtmp;

}

// Function to get a period's worth of data
int16_t* emulateADC::genPulses(int16_t *ADCtmp, expConfig exp, modeConfig md) {

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
  double sampleTime = sampleLen[p]*exp.adc_time_clock_period;
  // Total sample size in bytes
  unsigned long int sampleSize = 2*sampleLen[p];

  // Rate in us
  double rate = rateHz * 1e-6; // 1kHz in us
  // Average number of pulses in the sample
  int pulseNum = rate*sampleTime;

  // Get a time-based random seed to use for the ziggurat algorithm
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  // Initialise pulse index array
  int pulseIndex[pulseNum] = {};
  // If using main pulse simulation
  if (simPulse){
    // Loop over number of pulses and get random pulse index from ziggurat uniform
    for (int i = 0; i < pulseNum; i++) pulseIndex[i] = int(r4_uni ( seed ) * genPulseNum);
  }

  // If not simulating incrementing ADC emulation
  if (!ADCemulate){
 
    // Now, calculate and add pulses to ADC array
    int timeIndex = 0;
    // Get the pulse length in number of int16_t values
    uint pulseLen = pulseLength/exp.adc_time_clock_period;
    // Loop over average number of pulses
    for (int pn = 0; pn < pulseNum; pn++){
      // Randonly generate pulse times in clock ticks
      timeIndex += int(Random::Instance()->PoissValue(rate)/exp.adc_time_clock_period);
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
	ADCtmp[t] += pulseArray[pulseIndex[pn]][t-timeIndex];
      }
    }
  }

  return ADCtmp;

}

// Threaded function to generate all data for MI cycle
void emulateADC::genData(int16_t **ADCtmp, expConfig exp, modeConfig md) {

  // Get period number
  uint p = md.periodNum;

  // Generate noise
  ADCtmp[p] = genNoise(md);
  // Get pulse data
  if (!ADCemulate) ADCtmp[p] = genPulses(ADCtmp[p],exp,md);

  return;


} 

// Function to initliase all ADC variables and processes
void emulateADC::initADC(expConfig exp, bool output){

  // Initialise event variables
  // Total number of ADC values in a given sampling period
  sampleLen = new unsigned long int [totSamplePeriods] ();
  // Event length variable
  eLen = new uint*[totSamplePeriods] ();
  // The event origin of each sample
  eOrigin = new uint**[totSamplePeriods] ();
  // ADC data array
  ADC = new int16_t**[genArrayNum] ();
  for (int i = 0; i < genArrayNum; i++) ADC[i] = new int16_t*[totSamplePeriods] ();

  // Form MI cycle events for Mu2e simulation
  if (output) cout << "Forming events..." << endl;
  for (uint i = 0; i < totSamplePeriods; i++){
    // Form events
    formEvents(exp,pM[i]);
  }

  // Loop over sample periods
  for (uint i = 0; i < totSamplePeriods; i++){
    // Intialise data array of ADC values
    for (int j = 0; j < genArrayNum; j++) ADC[j][i] = new int16_t [sampleLen[i]] ();
  }

  // Calculate the generic pulse 
  pulseArray = genPulseArray(exp);

}
