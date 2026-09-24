// calcFuncs.h
#ifndef CALCFUNCS_H
#define CALCFUNCS_H

// Load C++ libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <random>
#include <list>
#include <bitset>
//#include "Hex.hh"
//#include "dataVars.hh"
//Load blinding libraries



class calcFuncs {
 
 public:

  std::vector<int16_t> readFile(char* fname);
  std::vector<unsigned> returnChannel_Num() {return Channel_Num;}
  std::vector<unsigned long int> returnDTCClock_Num() {return DTCClock_Num;}
  std::vector<unsigned long int> returnADCClock_Num() {return ADCClock_Num;}
  std::vector<unsigned long int> returnEvent_Num() {return Event_Num;}
  std::vector<unsigned long int> returnStartdata_index() {return Startdata_index;}

 private:

  std::vector<unsigned> Channel_Num;
  std::vector<unsigned long int> DTCClock_Num;
  std::vector<unsigned long int> ADCClock_Num;
  std::vector<unsigned long int> Event_Num;

  std::vector<unsigned long int> Startdata_index;

};



std::vector<int16_t> calcFuncs::readFile(char* fname){

  bool debugout = false;

  const int HdrEnd_len = 13;
  const int16_t HdrEnd_data[HdrEnd_len] = {(int16_t)0xBEEF, // 19 = 0xBEFF
							   (int16_t)0xCAFE, // 20 = 0xCAFE
							   (int16_t)0xBEEF, // 21 = 0xBEFF
							   (int16_t)0xCAFE, // 22 = 0xCAFE
							   (int16_t)0xAAAA, // 23 = 0xAAAA
							   (int16_t)0x5678, // 24 = 0x5678
							   (int16_t)0x1234, // 25 = 0x1234
							   (int16_t)0xFFFF, // 26 = 0xFFFF
							   (int16_t)0xEEEE, // 27 = 0xEEEE
							   (int16_t)0xDDDD, // 28 = 0xDDDD
							   (int16_t)0xCCCC, //  29 = 0xCCCC
							   (int16_t)0xBBBB, //  30 = 0xBBBB
							   (int16_t)0xAAAA}; //  31 = 0xAAAA   
  //skip 8 adc values  before header
  static const uint hdrtot  = 40;
  static const uint hdrskip = 8;
  //skip 9 adc values before the header
  /*static const uint hdrtot  = 41;
  static const uint hdrskip = 9;*/
  static const uint channel_loc = 0;
  static const uint ADCclock_loc = 4;
  static const uint evNum_loc = 8;
  static const uint elen_loc = 18;
  static const uint hdrlen = hdrtot-hdrskip;
  static const uint hdrsize = 2*hdrlen;
  //static const uint bufferLen = 30003;
  std::vector<int16_t> vec;
  std::ifstream is;
  is.open(fname, std::ios::binary);
  is.seekg(0, std::ios::end);
  size_t filesize=is.tellg();
  is.seekg(0, std::ios::beg);
  vec.resize(filesize/sizeof(int16_t));
  is.read((char *)vec.data(), filesize);

 
  std::vector<int16_t> ADCdata;
  ADCdata.clear();

  std::vector<unsigned long int> Header_Pos;
  Header_Pos.clear();


  Channel_Num.clear();
  DTCClock_Num.clear();
  ADCClock_Num.clear();
  Event_Num.clear();
  Startdata_index.clear();

  int cnt = vec.size();
  unsigned long int countpos=0;
  
  for(long unsigned int i = 0; i < vec.size(); i++){
    bool atHdr = false;
    if(vec[i] == HdrEnd_data[0]){
      atHdr = true;
      for(long unsigned int j=i;j < i+HdrEnd_len; j++){
	if(vec[j] != HdrEnd_data[j-i]){
	  atHdr = false;
	  break;
	}
      }
    }
    if(atHdr){
      Header_Pos.push_back(i);
      //std::cout<<"HEADER POS: "<<i<<std::endl;

      int hdrStartLoc = i - (hdrlen - HdrEnd_len);
      // initialise temporary arrays for memcpy ease
      int16_t headerTmp[hdrlen];
      // copy header to array
      memcpy(headerTmp,&vec[hdrStartLoc],hdrsize);    
      // Header 
      int elen = vec[hdrStartLoc+elen_loc];
      int eNum = vec[hdrStartLoc+evNum_loc];

      //channel
      uint64_t WORD1[4];
      WORD1[0] = (uint64_t) vec[hdrStartLoc+channel_loc+0] << 0  & (uint64_t) 0x000000000000FFFF;
      WORD1[1] = (uint64_t) vec[hdrStartLoc+channel_loc+1] << 16 & (uint64_t) 0x00000000FFFF0000;
      WORD1[2] = (uint64_t) vec[hdrStartLoc+channel_loc+2] << 32 & (uint64_t) 0x0000FFFF00000000;
      WORD1[3] = (uint64_t) vec[hdrStartLoc+channel_loc+3] << 48 & (uint64_t) 0xFFFF000000000000;
      uint8_t channel = (uint8_t) (WORD1[0]);
      uint64_t DTC_clock = (uint64_t)( WORD1[3] | WORD1[2] | WORD1[1] | WORD1[0] >> 8);
      Channel_Num.push_back(channel);
      DTCClock_Num.push_back(DTC_clock);

      /*std::bitset<64> x70(WORD1[0]);
      std::bitset<8> x71((uint8_t) WORD1[0]);
      std::bitset<64> x72(WORD1[0] >> 8);
      std::bitset<64> x73(WORD1[1]);
      std::bitset<64> x74(WORD1[2]);
      std::bitset<64> x75(WORD1[3]);
      std::bitset<64> x76(DTC_clock);
      std::cout<<"x70: "<<x70<<std::endl;
      std::cout<<"x71: "<<x71<<std::endl;
      std::cout<<"x72: "<<x72<<std::endl;
      std::cout<<"x73: "<<x73<<std::endl;
      std::cout<<"x74: "<<x74<<std::endl;
      std::cout<<"x75: "<<x75<<std::endl;
      std::cout<<"DTC_clock: "<<x76<<std::endl;*/
      
      //ADC clock
      uint64_t WORD2[4];
      WORD2[0] = (uint64_t) vec[hdrStartLoc+ADCclock_loc+0] << 0  & (uint64_t) 0x000000000000FFFF;
      WORD2[1] = (uint64_t) vec[hdrStartLoc+ADCclock_loc+1] << 16 & (uint64_t) 0x00000000FFFF0000;
      WORD2[2] = (uint64_t) vec[hdrStartLoc+ADCclock_loc+2] << 32 & (uint64_t) 0x0000FFFF00000000;
      WORD2[3] = (uint64_t) vec[hdrStartLoc+ADCclock_loc+3] << 48 & (uint64_t) 0xFFFF000000000000;
      uint64_t ADCclock = (uint64_t)( WORD2[3] | WORD2[2] | WORD2[1] | WORD2[0] );
      ADCClock_Num.push_back(ADCclock);

      //event number and 1st ADC of EWT
      uint64_t WORD3[4];
      WORD3[0] = (uint64_t) vec[hdrStartLoc+evNum_loc+0] << 0  & (uint64_t) 0x000000000000FFFF;
      WORD3[1] = (uint64_t) vec[hdrStartLoc+evNum_loc+1] << 16 & (uint64_t) 0x00000000FFFF0000;
      WORD3[2] = (uint64_t) vec[hdrStartLoc+evNum_loc+2] << 32 & (uint64_t) 0x0000FFFF00000000;
      WORD3[3] = (uint64_t) vec[hdrStartLoc+evNum_loc+3] << 48 & (uint64_t) 0xFFFF000000000000;
      uint64_t eventNum = (uint64_t)( WORD3[2] | WORD3[1] | WORD3[0] );
      Event_Num.push_back(eventNum);

      int esize = 2*elen;
      // modify counter and index
      if(debugout==true){
	std::cout << "Channel= " <<(unsigned)channel << std::endl;
	std::cout << "DTC_clock= " << DTC_clock <<std::endl;
	std::cout << "ADC_clock= " << ADCclock <<std::endl;
	std::cout << "Event num= " << eNum << " My event num: "<<eventNum<<std::endl;
      }
     /*for(unsigned long int k =0;k<bufferLen;k++){
	ADCdata.push_back(vec[i+HdrEnd_len+k]);
       }*/
     
    }


  }

  unsigned long int totalevents=0;
  unsigned long int removeevents=0;
  unsigned long int index_startEvent=0;

  unsigned long int data_start, data_end;
  //std::cout<<"Start copying data"<<std::endl;
  for(long unsigned int i = 0; i < Header_Pos.size(); i++){

    unsigned long int k = i+1;
   
    //skip last event //Last event k==Header_Pos.size()   
    if(k <= Header_Pos.size()){
      data_start = Header_Pos.at(i)+HdrEnd_len;

      if(k==Header_Pos.size()){ data_end = vec.size(); }
      else{data_end = Header_Pos.at(k)-(hdrtot-HdrEnd_len+1);}

      unsigned long int data_per_event = data_end - data_start;
      
      if(data_end>data_start){

	if(debugout==true){
	  std::cout<<"EVENT NUMBER : "<<Event_Num.at(i)<<std::endl; 
	  std::cout<<"Data in this event: "<<data_per_event<<std::endl;
	  std::cout<<"data_start "<<data_start<<" data_end "<<data_end<<std::endl;
	}
	totalevents++;
	if(data_per_event<30000){
	  removeevents++;
	}
	else{

	  for(unsigned long int j=data_start ; j<data_end;j++){
	    ADCdata.push_back(vec[j]);	
	    //std::cout<<vec[j]<<std::endl;
	  }
	  //Fill start index of events 
	  Startdata_index.push_back(index_startEvent);

	  index_startEvent = index_startEvent+data_per_event;
	}//else(data_per_event>30000)
      }//dataend
    }// if k

  }// for Header_Pos.size()

  std::cout<<"Total events: "<<totalevents<<" Events with less than 30,000 ADCs: "<<removeevents<<std::endl;

  return ADCdata;
}






#endif // CALCFUNCS_H
