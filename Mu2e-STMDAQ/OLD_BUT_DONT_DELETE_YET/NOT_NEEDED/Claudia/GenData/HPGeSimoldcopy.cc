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
#include <list>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <time.h>

#include "HPGeSimoldcopy.h"

using namespace std;

int16_t** getData(struct headerInfo trigInfo) {
  
  // Remaining size in bytes in packet after headers
  uint adcSpace = pSize - pHdrSize - tHdrSize;
  // Size in bytes of each slice
  uint sliceSize = (adcSpace-sliceNum*sHdrSize)/sliceNum;
  // Length of each slice in ADC values(I've changed this??????)
  uint sliceLen = floor(sliceSize/2);
  double sliceTime = sliceLen*tadc;
  // Find integer multiple number of 8kb packets
  int packetNum = round(timesample/(sliceNum*sliceTime));
  // Calculate number of slices
  uint sliceTot = packetNum*sliceNum;
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
  // Calculat array length of each packet
  unsigned int packetLen = pHdrLen+tHdrLen+sliceNum*(sHdrLen+sliceLen);
  // Calcuate actual data size of each packet in bytes
  uint packetSize = 2*packetLen;

  // Get average rate in detector depending on whether we're in external or internal mode
  int mode = trigInfo.mode; //(uint16_(t)??????????????????????)
  double rateHz = 0;
  // External mode
  if (mode == 0){
    rateHz = beamRateHz;
  }
  // Internal mode
  else if (mode == 1) {
    rateHz = sourceRateHz;
  }
  else{
    cout << "Error: Mode is not set to external [0] or internal [1]..." << endl;
    cout << "Exiting...\n" << endl;
    exit(0);
  }

  // Rate in us
  double rate = rateHz * 1e-6; // 1kHz in us
  // Average number of pulses in the sample
  int pulseNum = rate*sampleTime;
  
  // Print variables to screen
  // cout << "Rate in detector = " << rate*1e3 << " kHz" << endl;
  // cout << "Total sample length =  " << sampleTime << " us" << endl;
  // cout << "Total number ADC values = " << sampleNum << endl;
  // cout << "Number of slices per 8kb packet = " << sliceNum << endl;
  // cout << "Slice length = " << sliceTime << " us" << endl;
  // cout << "Slice size = " << sliceSize << " bytes" << endl;
  // cout << "Total number of slices = " << sliceTot << endl;
  // cout << "Packet size = " << packetSize << endl;
  // cout << "Total number of packets = " << packetNum << endl;
  // cout << "Total data size = " << dataSizeTot*1e-9 << " Gb" << endl;
  
  // Store times of each ADC sample
  double* adcTime = new double [sampleNum];
  for (unsigned long int i = 0; i < sampleNum; i++) adcTime[i] = tadc*i;

  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleNum];
  // Add random noise to each ADC value around zero
  random_device rd1;
  default_random_engine eng(rd1());
  uniform_real_distribution<double> distr(0,1);
  double rand2pi = 2*M_PI*distr(eng);
  for (unsigned long int i = 0; i < sampleNum; i++) ADC[i] = Anoise*sin(adcTime[i] + rand2pi);
  
  // Randomly generate pulse times for given rate within sample time
  random_device rd;
  exponential_distribution<double> pulseTime ( rate ) ;
  mt19937 rnd_gen(rd()) ;
  double time = 0;
  // Array of pulse times
  double* pulseTimes = new double [pulseNum];
  // Loop over number of pulses and randonly generate pulse times
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
      ADC[j] += pulseCalc(time);
    }
  }


  // Intialise data array of packets
  int16_t** data = new int16_t*[packetNum];       
  
  // Count the number of adc values
  unsigned long int adcCount = 0;
  // Count the number of slices
  int sliceCount = 0;
  // Loop over total number of packets
  for (int p = 0; p < packetNum; p++){
    // Count the total number of data entries per packet
    unsigned long int count = 0;  
    // Intialise packet array with packet legnth
    data[p] = new int16_t[packetLen];
    // Loop over packet header length and fill with packet header
    uint16_t *packetHeader = pHeader(uint32_t(p));          
    for (uint j = 0; j < pHdrLen; j++) {
      // Insert packet header
      data[p][count] = *(packetHeader+j);
      count += 1; // Increment data counter
    }
    // Loop over trigger header length and fill with trigger header
    uint32_t macroCount = trigInfo.macroCount;
    uint64_t macroTime = trigInfo.macroTime;
    uint32_t microCount = trigInfo.microCount;
    uint16_t channel = trigInfo.channel;
    uint16_t ADCoffset = trigInfo.ADCoffset;
    uint16_t *triggerHeader = tHeader(macroCount,macroTime,mode,microCount,channel,ADCoffset,sliceNum);    
    for (uint j = 0; j < tHdrLen; j++) {
      // Insert trigger header
      data[p][count] = *(triggerHeader+j);
      count += 1; // Increment data counter
    }
    // Loop over number of slices per packet
    for (int s = 0; s < sliceNum; s++){
      // Loop over slice header length and fill with slice header
      uint32_t sliceNo = sliceCount;
      uint64_t adc1 = adcTime[adcCount]/tadc;
      uint32_t sliceTm = sliceTime/tadc
	;      uint16_t *sliceHeader = sHeader(sliceNo,adc1,sliceTm);
      for (uint j = 0; j < sHdrLen; j++) {
	// Insert slice header
	data[p][count] = *(sliceHeader+j);
	count += 1; // Increment data counter
      }
      // Loop over slice length and fill with ADC data
      for (uint j = 0; j < sliceLen; j++) {
	// Insert ADC data
	data[p][count] = ADC[adcCount];
	count += 1; // Increment data counter
	adcCount += 1; // Increment ADC counter
      }
      // Increment slice counter
      sliceCount += 1;
    } // End loop over number of slices per packet
  } // End loop over number of packets

  // cout << " -- " << endl;
  // for (int i = 0; i < 10; i++){

  //   for (int j = 0; j < 60; j++){
  //     cout << data[i][j] << ",";
  //   }
  //   cout << endl;
  // }
  
  // // Store data as vector - FOR DRAWING ONLY
  // std::vector<double> data_time;
  // std::vector<double> data_header;
  // for (int i = 0; i < sampleNum; i++){
  //   data_time.push_back(adcTime[i]);
  //   data_header.push_back(ADC[i]);
  // }
  // // Draw
  // TGraph *g = new TGraph(sampleNum, &data_time[0], &data_header[0]);
  // g->Draw("A*");  

  return data;
  
}









//void HPGeSim(){
//void main (){
int main (void){

  // Micropulse counter
  uint32_t microCount = 0;
  // Macropulse counter
  uint32_t macroCount = 0;
  // Internal trigger counter
  uint32_t intTrigCount = 0;
  // External (beam) [0] or internal (source) mode [1]
  uint16_t mode = 0;
  // Channel (HPGe [0] or LaBr [1])
  uint16_t channel = 0;
  
  // Loop over macropulses
  while (1){
    
    // EXTERNAL MODE
    // Set mode to external
    mode = 0;
    // Get macropulse trigger time (SHOULD CONVERT TO CLOCK TICKS)
    uint64_t macroTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); // ms 

    // Fill struct with header info
    struct headerInfo extMode = {macroCount,macroTime,mode,microCount,channel,ext_ADC_offset};
    
    // Check ADC offset isn't within one order of magnitude of macropulse width
    if (ext_ADC_offset >= 0.1*macroWidth){
      cout << "Error: External ADC offset is within one order of magnitude of the marcopulse width!" << endl;
      cout << "Exiting...\n" << endl;
      exit(0);
    }
    // Wait for ADC offset
    usleep(ext_ADC_offset);
    // Receive external beam trigger - sample data
    int16_t** data1 = getData(extMode);
      
    // SEND DATA TO SOCKET HERE
    //
    // Now wait for length of macropulse width
    usleep(macroWidth-ext_ADC_offset);
    // Increase micropulse counter by number of micropulse per macropulse
    microCount += microPerMacro;
    
    
    // INTERNAL MODE
    // Set mode to internal
    mode = 1;
    // Loop over number of internal triggers
    for (int t = 0; t < int_trig_num; t++){

      // Calculate internal trigger length to be the macropulse gap time
      // divided by the number of internal triggers
      double intTrigLen = macroGapWidth/int_trig_num;
      
      // Fill struct with header info
      struct headerInfo intMode = {macroCount,macroTime,mode,intTrigCount,channel,int_ADC_offset};
      
      // Check ADC offset isn't within one order of magnitude of interal trigger length
      if (int_ADC_offset >= 0.1*intTrigLen){
	cout << "Error: Internal ADC offset is within one order of magnitude of the trigger window!" << endl;
	cout << "Exiting...\n" << endl;
	exit(0);
      }
      // Wait for ADC offset
      usleep(int_ADC_offset);
      // Receive internal trigger - sample source data
      int16_t** data2 = getData(intMode);
      // SEND DATA TO SOCKET HERE
      //
      // Now wait for length of macropulse width
      usleep(intTrigLen-int_ADC_offset);
      
      // Increment internal trigger counter
      intTrigCount += 1;
    }
       
    // Increment macropulse counter
    macroCount += 1;
     
  }
  
}
