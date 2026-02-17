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

#include "HPGeSimbin.h"

using namespace std;

int16_t** getData(struct headerInfo trigInfo) {

  // Get external/internal mode
  uint16_t mode = trigInfo.mode;

  // Get sampling time and average rate in detector depending on whether we're in external or internal mode
  double rateHz = 0;
  double timeSample = 0;
  // External mode
  if (mode == 0){
    timeSample =  extSampleTime;
    rateHz = beamRateHz;
  }
  // Internal mode
  else if (mode == 1) {
    timeSample =  intSampleTime;
    rateHz = sourceRateHz;
  }
  else{
    cout << "Error: Mode is not set to external [0] or internal [1]..." << endl;
    cout << "Exiting...\n" << endl;
    exit(0);
  }
  
  // Remaining size in bytes in packet after headers
  uint adcSpace = pSize - pHdrSize - tHdrSize;
  // Size in bytes of each slice
  uint sliceSize = (adcSpace-sliceNum*sHdrSize)/sliceNum;
  // Length of each slice in ADC elements
  uint sliceLen = floor(sliceSize/2);
  std::cout<<"SliceLen: "<<sliceLen<<std::endl;
  double sliceTime = sliceLen*tadc;
  // Find integer multiple number of 8kb packets
  int packetNum = round(timeSample/(sliceNum*sliceTime));
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

  // Rate in us
  double rate = rateHz * 1e-6; // 1kHz in us
  // Average number of pulses in the sample
  int pulseNum = rate*sampleTime;
    
  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleNum] ();

  // Randomly generate pulse times for given rate within sample time
  random_device rd;
  exponential_distribution<double> pulseTime ( rate ) ;
  mt19937 rnd_gen(rd()) ;
  
  // Now, calculate and add pulses to ADC array
  // Loop over average number of pulses
  int timeIndex = 0;
  for (int i = 0; i < pulseNum; i++){
    // Randomly generate pulse times in clock ticks
    timeIndex += int(pulseTime(rnd_gen)/tadc);
    // Get the index value of the last element in the pulse
    uint max = timeIndex + pulseLength;
    // If the pulse exceeds the sample length, max out at the sample length
    if (timeIndex + pulseLength > sampleNum) max = sampleNum;
    // Print the pulse time
    // Loop over the number of ADC values for a pulse, starting from the randolmy generated pulse time
    for (uint j = timeIndex; j < max; j++){
      // Find the time starting from zero
      double time = (j-timeIndex)*tadc;
      // Generate pulse data with noise and add to ADC array
      ADC[j] += pulseCalc(time);
    }
  }

  // Intialise data array of packets
  int16_t** data = new int16_t*[packetNum] ();			       
  
  // Count the number of adc values
  unsigned long int adcCount = 0;
  // Count the number of slices
  int sliceCount = 0;

  // Get trigger header info
  uint32_t macroCount = trigInfo.macroCount;
  uint64_t macroTime = trigInfo.macroTime;
  uint32_t microCount = trigInfo.microCount;
  uint16_t channel = trigInfo.channel;
  uint16_t ADCoffset = trigInfo.ADCoffset;
  
  // Loop over total number of packets
  for (int p = 0; p < packetNum; p++){
    // Count the total number of data entries per packet
    unsigned long int count = 0;  
    // Intitliase packet array with packet legnth
    data[p] = new int16_t[packetLen];
    //   // Insert packet header	
    uint16_t *packetHeader = pHeader(uint32_t(p));
    memcpy(data[p]+count, packetHeader, pHdrLen * sizeof *packetHeader);
    count += pHdrLen; // Increment data counter
    //   // Insert trigger header
    uint16_t *triggerHeader = tHeader(macroCount,macroTime,mode,microCount,channel,ADCoffset,sliceNum);
    memcpy(data[p]+count, triggerHeader, tHdrLen * sizeof *triggerHeader);
    count += tHdrLen; // Increment data counter
    // Loop over number of slices per packet
    for (int s = 0; s < sliceNum; s++){
      // Insert slice header
      uint32_t sliceNo = sliceCount;



      // Get external/internal delay time
      double ext_int_delay = 0;
      if (extSampleTime <= macroWidth) ext_int_delay = extSampleTime;
      if (extSampleTime > macroWidth) ext_int_delay = extSampleTime - macroWidth;

      double intSampleTot = macroGapWidth - ext_int_delay - int_trig_num*int_ADC_offset;
      double intTrigLen = intSampleTot/int_trig_num;

      //uint64_t adc1 = adcCount; //Number of each ADC values written, to get the time, we have to multiply this number by tadc
      uint64_t adc1;
      //External
      //cout<<"Macro period:"<<macroPeriod<<" 1 slice: "<<sliceTime<<endl;
      //Slice time is not an integer in us, store it as nanoseconds
      if(mode==0){adc1 =(macroPeriod*macroCount + sliceNo*sliceTime)*1000;
        //adc1=((extSampleTime+int_trig_num*intSampleTime)*macroCount + sliceNo*sliceTime)*1000;}
      }
	//Internal//microcount is the internal trigger number
	if(mode==1){
	  //adc1 =((macroCount+1)*extSampleTime + microCount*intSampleTime + sliceNo*sliceTime)*1000;
	  adc1 =((macroCount+1)*macroWidth + (macroCount+1)*ext_int_delay + microCount*intTrigLen + (microCount+1)*int_ADC_offset + sliceNo*sliceTime)*1000;
	}




      std::cout<<"sliceTime: "<<adc1<<" ns"<<endl;
      uint32_t sliceTm = sliceTime/tadc; //Number of ADC values in each slice, constant
      uint16_t *sliceHeader = sHeader(sliceNo,adc1,sliceTm);
      memcpy(data[p]+count, sliceHeader, sHdrLen * sizeof *sliceHeader);
      count += sHdrLen; // Increment data counter     
       // Loop over slice length and fill with ADC data
      for (uint j = 0; j < sliceLen; j++) {
	// Insert ADC data with random noise
	double time = adcCount*tadc;
	double rd = (double)rand() / RAND_MAX;
	double noise = -noiseSD + rd*(2*noiseSD); // Random noise
	data[p][count] = ADC[adcCount]+noise;
	count += 1; // Increment data counter
	adcCount += 1; // Increment ADC counter
      }
      // Increment slice counte
      sliceCount += 1;
    } // End loop over number of slices per packet
  } // End loop over number of packets
  
  return data;
   
}

int main(){
  
  // Micropulse counter
  uint32_t microCount = 0;
  // Macropulse counter
  uint32_t macroCount = 0;
  // Internal trigger counter
  uint32_t intTrigCount = 0;
  // Channel (HPGe [0] or LaBr [1])
  uint16_t channel = 0;
  
  // Get external/internal delay time
  double ext_int_delay = 0;
  if (extSampleTime <= macroWidth) ext_int_delay = extSampleTime;
  if (extSampleTime > macroWidth) ext_int_delay = extSampleTime - macroWidth;

  // Ensure sampling times don't overshoot macropulse configuration
  double totSampleTime = extSampleTime + ext_int_delay + ext_ADC_offset + int_trig_num*(ext_ADC_offset+intSampleTime);
  if (totSampleTime > macroPeriod){
    cout << "\nERROR! Summed sampling times are larger than macropulse period: "<< endl;
    cout << "-" << endl;
    cout << "External sampling time = " << extSampleTime << " us" << endl;
    cout << "External ADC offset = " << ext_ADC_offset << " us" << endl;
    cout << "Macropulse width = " << macroWidth << " us " << endl;
    cout << "-" << endl;
    cout << "Internal sampling time = " << intSampleTime << " us" << endl;
    cout << "Internal ADC offset = " <<int_ADC_offset << " us" << endl;
    cout << "Number of internal triggers = " << int_trig_num << endl;
    cout << "Macropulse gap = " << macroGapWidth << " us" << endl;
    cout << "---" << endl;
    cout << "Total summed sampling time per macropulse = " << totSampleTime << " us" <<endl;
    cout << "Macropulse cycle period = " << macroPeriod << " us" << endl;
    cout << "Difference = " << macroPeriod-totSampleTime << " us\n" << endl;
    exit(0);
  }



  ////////Add
  uint16_t mode;
  double rateHz = 0;
  double timeSample = 0;
  // Remaining size in bytes in packet after headers
  uint adcSpace;
  // Size in bytes of each slice                                                                                        
  uint sliceSize;
  // Length of each slice in bytes                                                                          
  uint sliceLen;
  double sliceTime;
  // Find integer multiple number of 8kb packets
  int packetNum; //Need                                                  
  // Calculate number of slices                                                                                           
  uint sliceTot;
  double sampleTime;
  // Total number of ADC values                                                               
  unsigned long int sampleNum;
  // Calculate total length of data array including headers
  long unsigned int dataNumTot; //Need
  // Calculat array length of each packet
  unsigned int packetLen; //Need      
  ///////////////
 




  // Loop over macropulses
  // while (1){
  //4 macropulse
  while(macroCount<4){
    std::cout<<"Macropulse: "<<macroCount<<std::endl;
    auto macroStart = std::chrono::high_resolution_clock::now();

    // EXTERNAL MODE
    // Set mode to external
    mode = 0;
  

    timeSample =  extSampleTime;
    rateHz = beamRateHz;
    // Remaining size in bytes in packet after headers    
    adcSpace = pSize - pHdrSize - tHdrSize;
    // Size in bytes of each slice
    sliceSize = (adcSpace-sliceNum*sHdrSize)/sliceNum;
    // Length of each slice in bytes
    sliceLen = floor(sliceSize/2);
    sliceTime = sliceLen*tadc;
    // Find integer multiple number of 8kb packets
    packetNum = round(timeSample/(sliceNum*sliceTime)); //Need
    // Calculate number of slices 
    sliceTot = packetNum*sliceNum;
    sampleTime = sliceTot*sliceTime;
    // Total number of ADC values
    sampleNum = floor(sampleTime/tadc);
    // Calculate total length of data array including headers
    dataNumTot = sampleNum
    +sliceTot*sHdrLen
    +packetNum*(pHdrLen+tHdrLen); //Need                                                                                      
  // Calculat array length of each packet                                                                         
  packetLen = pHdrLen+tHdrLen+sliceNum*(sHdrLen+sliceLen); //Need  








  // Get macropulse trigger time (SHOULD CONVERT TO CLOCK TICKS)
    uint64_t macroTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); // ms 

    // Fill struct with header info
    struct headerInfo extMode = {macroCount,macroTime,mode,microCount,channel,ext_ADC_offset};
    std::cout<<macroTime<<endl;
    
    // Check ADC offset isn't within one order of magnitude of macropulse width
    if (ext_ADC_offset >= 0.1*macroWidth){
      cout << "Error: External ADC offset is within one order of magnitude of the marcopulse width!" << endl;
      cout << "Exiting...\n" << endl;
      exit(0);
    }
    // Wait for ADC offset
    usleep(ext_ADC_offset);

    // Receive external beam trigger - sample data
    auto extStart = std::chrono::high_resolution_clock::now(); // Time before ext data
    int16_t** extData = getData(extMode);
    auto extEnd = std::chrono::high_resolution_clock::now(); // Time after ext data
    double extTime = std::chrono::duration<double, std::milli>(extEnd-extStart).count()*1e3; // Ext data time
    
    //---------------------------------Write Binary file with data in the macropulse
    std::cout<<"External mode"<<std::endl;
    std::cout<<"dataNumTotExt: "<<dataNumTot<<std::endl;
    std::cout<<"packet num: "<<packetNum<<std::endl;
    std::cout<<"packet len: "<<packetLen<<std::endl;
   
    CreateBinary(extData,dataNumTot,packetNum,packetLen,pHdrLen,tHdrLen);


    // SEND DATA TO SOCKET HERE
    //

    // Now wait for length of macropulse width + external/internal delay before internal data sampling
    double extWait = macroWidth-ext_ADC_offset+ext_int_delay;
    // Subtract the time to simulate data from the wait time... 
    if (extTime < extWait){
      extWait -= extTime;
    }
    else{
      extWait = 0;
    }
    // Wait
    usleep(extWait);

   // Increase micropulse counter by number of micropulse per macropulse
    microCount += microPerMacro;
    
    // Find total amount of time remaining for internal sampling
    double intSampleTot = macroGapWidth - ext_int_delay - int_trig_num*int_ADC_offset;
    // Ensure number of internal triggers and sampling period 
    // does not exceed remaining time in macropulse gap
    if (int_trig_num*intSampleTime > intSampleTot){
      cout << "\nERROR! Summed internal sampling times are larger than remaining time in macropulse gap: "<< endl;
      cout << "-" << endl;
      cout << "External/internal delay = " << ext_int_delay << " us" << endl;
      cout << "Internal sampling time = " << intSampleTime << " us" << endl;
      cout << "Internal ADC offset = " << int_ADC_offset << " us" << endl;
      cout << "Number of internal triggers = " << int_trig_num << endl;
      cout << "Macropulse gap = " << macroGapWidth << " us" << endl;
      cout << "---" << endl;
      cout << "Total internal sampling time per macropulse gap = " << int_trig_num*intSampleTime << " us" <<endl;
      cout << "Reamining time available for sampling in macropulse gap = " << intSampleTot << " us" << endl;
      cout << "Difference = " << intSampleTot-int_trig_num*intSampleTime << " us\n" << endl;
      exit(0);
    }
    
    // Calculate internal trigger length to be the macropulse gap time
    // divided by the number of internal triggers
    double intTrigLen = intSampleTot/int_trig_num;    
    
    // INTERNAL MODE
    // Set mode to external
    mode = 1;


    timeSample =  intSampleTime;
    rateHz = sourceRateHz;


    // Remaining size in bytes in packet after headers
    adcSpace = pSize - pHdrSize - tHdrSize;
    // Size in bytes of each slice
    sliceSize = (adcSpace-sliceNum*sHdrSize)/sliceNum;
    // Length of each slice in bytes
    sliceLen = floor(sliceSize/2);
    sliceTime = sliceLen*tadc;
    // Find integer multiple number of 8kb packets
    packetNum = round(timeSample/(sliceNum*sliceTime)); //Need
    // Calculate number of slices
    sliceTot = packetNum*sliceNum;
    sampleTime = sliceTot*sliceTime;
    // Total number of ADC values                                          
    sampleNum = floor(sampleTime/tadc);
    // Calculate total length of data array including headers
    dataNumTot = sampleNum
    +sliceTot*sHdrLen
    +packetNum*(pHdrLen+tHdrLen); //Need                                                    
  // Calculat array length of each packet
  packetLen = pHdrLen+tHdrLen+sliceNum*(sHdrLen+sliceLen); //Need    





    // Loop over number of internal triggers
  for (int t = 0; t < int_trig_num; t++){
    std::cout<<"Internal trigger "<<t<<std::endl;
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

      // Receive external beam trigger - sample data
      auto intStart = std::chrono::high_resolution_clock::now();
      int16_t** intData = getData(intMode);
      auto intEnd = std::chrono::high_resolution_clock::now();
      double intTime = std::chrono::duration<double, std::milli>(intEnd-intStart).count()*1e3;
      
      //---------------------------------Write Binary file with data for each internal trigger
      std::cout<<"Internal mode"<<std::endl;
      std::cout<<"dataNumTotInt: "<<dataNumTot<<std::endl;
      std::cout<<"packet num: "<<packetNum<<std::endl;
      std::cout<<"packet len: "<<packetLen<<std::endl;

      CreateBinary(intData,dataNumTot,packetNum,packetLen,pHdrLen,tHdrLen);


      // SEND DATA TO SOCKET HERE
      //
      
      // Now wait for length of macropulse width + external/internal delay before internal data sampling    
      double intWait = intTrigLen-int_ADC_offset;
      // Subtract the time to simulate data from the wait time... 
      if (intTime < intWait){
	intWait -= intTime;
      }
      else{
	intWait = 0;
      }
      // Wait
      usleep(intWait);
      
      // Increment internal trigger counter
      intTrigCount += 1;
      }

    auto macroEnd = std::chrono::high_resolution_clock::now();
    double elapsed_time_ms = std::chrono::duration<double, std::milli>(macroEnd-macroStart).count();
    	   
    cout << macroCount << " " << elapsed_time_ms << endl;

    // Increment macropulse counter
    macroCount += 1;

  } // End macropulse cycle loop

  return 1;
  
}
