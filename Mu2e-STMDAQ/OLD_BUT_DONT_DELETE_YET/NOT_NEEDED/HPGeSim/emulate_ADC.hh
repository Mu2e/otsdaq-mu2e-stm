/////////////////////////////////////////////////////
////////////////////////////////////////
/// This module provides the ADC emulation (header)
/////////////////////////////////////////////////////////////////////////

#ifndef EMULATE_ADC_hh
#define EMULATE_ADC_hh

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

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// Mu2e configuration information
#include "STMDAQ-TestBeam/HPGeSim/mu2e_config.hh"

#ifndef EMULATE_ADC_hh_DEFINED
#define EMULATE_ADC_hh_DEFINED
class emulateADC;
#endif

using namespace std;

class emulateADC{

public:
  
  // Standard constructor - shouldn't be used
  emulateADC();
  
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
  int16_t** pulseArray;
  
  // The number of simulation pulses to generate
  int genPulseNum = 0;
  
  // Two generation arrays 
  static const uint16_t genArrayNum = 2;
  
  // Return average beam (on-spill) rate in detector
  double getOnSpillRate(){
    return onSpillRateHz;
  }
  // Return average beam (on-spill) rate in detector
  double getOffSpillRate(){
    return offSpillRateHz;
  }
  
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
 void formEvents(expConfig exp, modeConfig md);

 // Function to calculate a generic pulse as an array 
 int16_t **genPulseArray(expConfig exp);

 // Function to generate ADC noise
 int16_t* genNoise(modeConfig md);  
 
 // Function to generate pulses and add to noisy ADC data
 int16_t* genPulses(int16_t *ADCtmp, expConfig exp, modeConfig md);
 
 // Threaded function to generate all data for MI cycle
 void genData(int16_t **ADCtmp, expConfig exp, modeConfig md);

 // Function to initliase all ADC variables and processes
 void initADC(expConfig exp, bool output);

 // Function to check mode
 void checkMode(uint16_t mode){
   // If mode is not 0 or 1, throw error and exit
   if (mode != 0 and mode != 1){
     cout << "Error: Mode is not set to off-spill [0] or on-spill [1]..." << endl;
     cout << "Exiting...\n" << endl;
     exit(0);
   }
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
