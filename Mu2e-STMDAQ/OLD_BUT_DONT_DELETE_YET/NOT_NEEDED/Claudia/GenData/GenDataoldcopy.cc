#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include<list>

using namespace std;

template <typename BidirectionalIterator, typename T>
BidirectionalIterator getClosest(BidirectionalIterator first, 
                                 BidirectionalIterator last, 
                                 const T & value)
{
  BidirectionalIterator before = std::lower_bound(first, last, value);

  if (before == first) return first;
  if (before == last)  return --last; // iterator must be bidirectional

  BidirectionalIterator after = before;
  --before;

  return (*after - value) < (value - *before) ? after : before;
}

template <typename BidirectionalIterator, typename T>
std::size_t getClosestIndex(BidirectionalIterator first, 
                            BidirectionalIterator last, 
                            const T & value)
{
  return std::distance(first, getClosest(first, last, value));
}


//------------Variables of the pulses generated-----------------------
//The amplitude of the pulse is the half of this value
const double twiceA=2370;
//The x point in where we have the pulse (50 us)
const double xshift=100;
//fall from baseline time
const double invtaufall=6;
//decaytime(rise to baseline)
const double invtaudecay=0.028;
//Amplitude of noise
const double Anoise=10;

// ADC samling frequency (Hz)
const double fADC = 320*1e6;
//Sampling time of ADC (microsec)
const double tadc=1/(fADC*1e-6);
//Pulse duration (microsec)
const double pulselength=xshift+220;
//Number of ADC values in one pulse
const unsigned long int pulseLength=(xshift+220)/tadc;
//Number of bytes in one pulse
unsigned long int pulseBytes=2*pulseLength;

// Bytes in 8kb 10G packet
const uint pSize = 8240;
// Packet header size in bytes
const uint pHdrSize = 6;
// Packet header legnth
const uint pHdrLen = pHdrSize/2;
// Trigger header size in bytes
const uint tHdrSize = 32;
// Trigger header legnth
const uint tHdrLen = tHdrSize/2;
// Slice header size in bytes
const uint sHdrSize = 16;
// Slice header legnth
const uint sHdrLen = sHdrSize/2;


// Create the packet header 6 bytes (3 int16)
// Packet number 4 bytes (2 int16_t)
uint16_t* pHeader(uint32_t pNum){
  
  // Split packet number (4 bytes) into (2 bytes)
  uint16_t pn1 = pNum >> 16;
  uint16_t pn2 = pNum & 0x0000FFFF;

  // Store packet header
  static uint16_t p[pHdrLen] = {pn1,pn2,0};
  
  // Return array
  return p;
  
}

// Create the trigger header 32 bytes (16 int16)
int16_t* tHeader(int16_t triggernumber, int64_t triggertime,
		 int16_t triggertype_channel_datatype, int32_t triggertimeoffset,
		 int16_t baselinemean, int16_t baselineRMS){
  
  //Split triggertime (8 bytes) into (2 bytes)
  int16_t tt1 = triggertime >> 48;
  int16_t tt2 = triggertime >> 32;
  int16_t tt3 = triggertime >> 16;
  int16_t tt4 = triggertime & 0x0000FFFF;

  //Split triggertimeoffset (4 bytes) into (2 bytes)
  int16_t ttof1 = triggertimeoffset >> 16;
  int16_t ttof2 = triggertimeoffset & 0x0000FFFF;

  //Store the header into an array
  static int16_t t[tHdrLen]={triggernumber,
			     tt1,tt2,tt3,tt4,
			     triggertype_channel_datatype,
			     ttof1,ttof2,
			     baselinemean,baselineRMS,
			     0,0,0,0,0,0};
  
  // Return array
  return t;
  
}

// Create the slice header 16 bytes (8 int16)
int16_t* sHeader(int32_t sliceNo, int64_t timefirstadc, int32_t sizeslice){
  
  // SliceNum (4 bytes) into (2 bytes)
  int16_t sn1 = sliceNo >> 16;
  int16_t sn2 = sliceNo & 0x0000FFFF;
  
  //Time first ADC (8 bytes) into (2 bytes)
  int16_t tfadc1 = timefirstadc >> 48;
  int16_t tfadc2 = timefirstadc >> 32;
  int16_t tfadc3 = timefirstadc >> 16;
  int16_t tfadc4 = timefirstadc & 0x0000FFFF;

  // Slice slice (4 bytes) into (2 bytes)
  int16_t ss1 = sizeslice >> 16;
  int16_t ss2 = sizeslice & 0x0000FFFF;

  //Store the header into an array
  static int16_t s[sHdrLen]={sn1,sn2,tfadc1,tfadc2,tfadc3,tfadc4,ss1,ss2};

  // Return array
  return s;     
      
}


void GenDataHeaders() {

  // User input variables
  // --------------------------
  
  // Average rate in detector
  double ratekHz = 1e+3; //1kHz-----I think this should be 1------  
  //Duration of the sample generated (microseconds)
  long double timesample= 10000;
  // Total number of slices per packet
  int sliceNum = 2; 
  // --------------------------

  // Remaining size in bytes in packet after headers
  uint adcSpace = pSize - pHdrSize - tHdrSize;
  // Size in bytes of each slice
  uint sliceSize = (adcSpace-sliceNum*sHdrSize)/sliceNum;
  // Length of each slice in ADC values
  uint sliceLen = floor(sliceSize/2);
  double sliceTime = sliceLen*tadc;
  // Find integer multiple number of 8kb packets
  int packetNum = round(timesample/(sliceNum*sliceTime));
  // Calculate number of slices
  uint sliceTot = packetNum*sliceNum;
  // Total time of the sample
  double sampleTime = sliceTot*sliceTime;
  // Total number of ADC values  
  unsigned long int sampleNum = floor(sampleTime/tadc);
  // Total sample size in bytes
  unsigned long int sampleSize = sampleNum*2;
  
  // Calculate total length of data array including headers
  long unsigned int dataNumTot = sampleNum
    +sliceTot*sHdrLen
    +packetNum*(pHdrLen+tHdrLen);
  // Calcuate full data size in bytes
  long unsigned int dataSizeTot = 2*dataNumTot;

  
  // Rate in us
  double rate = ratekHz * 1e-6; // 1kHz in us --------------(I think this is 1e+3/ratekHz = us)--------------------
  // Average number of pulses in the sample
  int pulseNum = rate*sampleTime;
  
  // Print variables to screen
  cout << "Rate in detector = " << rate << " kHz" << endl;//I think this is ratekHz-------
  cout << "Total sample length =  " << sampleTime << " us" << endl;
  cout << "Total number ADC values = " << sampleNum << endl;
  cout << "Number of slices per 8kb packet = " << sliceNum << endl;
  cout << "Slice length = " << sliceTime << " us" << endl;
  cout << "Slice size = " << sliceSize << " bytes" << endl;
  cout << "Total number of slices = " << sliceTot << endl;
  cout << "Total number of packets = " << packetNum << endl;
  cout << "Total data size = " << dataSizeTot*1e-9 << " Gb" << endl;
  
  // Store times of each ADC sample
  double* adcTime = new double [sampleNum];
  for (unsigned long int i = 0; i < sampleNum; i++) adcTime[i] = tadc*i;

  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleNum];
  // Add noise to each ADC value around zero
  for (unsigned long int i = 0; i < sampleNum; i++) ADC[i] = Anoise*sin(adcTime[i]);
  
  // Randomly generate pulse times for given rate within sample time
  random_device rd;
  exponential_distribution<double> pulseTime ( rate ) ;
  mt19937 rnd_gen(rd()) ;
  double time = 0;
  // Array of pulse times
  double* pulseTimes = new double [pulseNum];
  // Loop over number of pulses and randomly generate pulse times
  for (int i = 0; i < pulseNum; i++){
    time += pulseTime(rnd_gen);
    pulseTimes[i] = time;
  }

  // Now, calculate and add pulses to ADC array
  // Loop over average number of pulses
  for (int i = 0; i < pulseNum; i++){
    // For each randomly generated pulse time, find the index of the closest ADC time
    int index = getClosestIndex(adcTime, adcTime + sampleNum, pulseTimes[i]);
    // Get the index value of the last element in the pulse
    uint max = index + pulseLength;
    // If the pulse exceeds the sample length, max out at the sample length
    if (index + pulseLength > sampleNum) max = sampleNum;
    // Print the pulse time
    //    cout << "Pulse occuring at " << adcTime[index] << endl;
    // Loop over the number of ADC values for a pulse, starting from the randolmy generated pulse time
    for (uint j = index; j < max; j++){
      // Find the time starting from zero
      double time = (j-index)*tadc;
      // Generate pulse data with noise and add to ADC array
      ADC[j] += -(twiceA / (1 + exp(-(time - xshift)*invtaufall)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(time - xshift)*invtaudecay))));
    }
  }

  // Intialise data array of headers and ADC values
  int16_t* data = new int16_t[dataNumTot];
  
  // Count the total number of data entries
  unsigned long int count = 0;
  // Count the number of adc values
  unsigned long int adcCount = 0;
  // Count the number of packets
  uint32_t packetCount = 0;
  // Count the number of slices
  int sliceCount = 0;
  // Boolen that tells us to start a new packet
  bool newPacket = true;

  // Loop over the total number of slices
  for (int i = 0; i < sliceTot; i++){
    // Increment the slice counter
    sliceCount += 1;

    // If in a new packet...
    if (newPacket){
      // Loop over packet header length and fill with packet header
      uint16_t *packetHeader = pHeader(packetCount);          
      for (uint j = 0; j < pHdrLen; j++) {
	// Insert packet header
	data[count] = *(packetHeader+j);
	count += 1; // Increment data counter
      }
      // Fill trigger header
      int16_t trignumber = packetCount;
      int64_t trigtime=0; // Set this number
      int16_t trigtype_channel_datatype = 0;
      int32_t trigtimeoffset = 0;
      int16_t baselinemean = 0;
      int16_t baselineRMS = 0;
      int16_t* triggerHeader = tHeader(trignumber, trigtime,
				       trigtype_channel_datatype, trigtimeoffset,
				       baselinemean, baselineRMS);

      // Loop over trigger header length and fill with trigger header
      for (uint j = 0; j < tHdrLen; j++) {
	// Insert trigger header
	data[count] = *(triggerHeader+j);
	count += 1; // Increment data counter
      }
      packetCount += 1;
    } // End if new packet


    // Loop over slice header length and fill with slice header
    int32_t sliceNo = i;
    int64_t adc1 = adcTime[adcCount]/tadc;
    int32_t sliceTm = sliceTime; //this is in time (sliceTime is a double)
    int16_t *sliceHeader = sHeader(sliceNo,adc1,sliceTm);    
    for (uint j = 0; j < sHdrLen; j++) {
      // Insert slice header
      data[count] = *(sliceHeader+j);
      count += 1; // Increment data counter
    }
    // Loop over slice length and fill with ADC data
    for (uint j = 0; j < sliceLen; j++) {
      // Insert ADC data
      data[count] = ADC[adcCount];
      count += 1; // Increment data counter
      adcCount += 1; // Increment ADC counter
    }
    // If slice counter is less than number of slice per packet...
    if (sliceCount < sliceNum) {
      // Stay in same packet
      newPacket = false;
    }
    // If slice counter = the number of slices per packet...
    else{
      // Start new packet
      newPacket = true;
      // Set slice counter to zero
      sliceCount = 0;
    }
  }
  




  // Store data as vector - FOR DRAWING ONLY
  std::vector<double> data_time;
  std::vector<double> data_header;
  for (int i = 0; i < sampleNum; i++){
    data_time.push_back(adcTime[i]);
    data_header.push_back(ADC[i]);
  }
  // Draw
  TGraph *g = new TGraph(sampleNum, &data_time[0], &data_header[0]);
  g->Draw("A*");  
  
}
