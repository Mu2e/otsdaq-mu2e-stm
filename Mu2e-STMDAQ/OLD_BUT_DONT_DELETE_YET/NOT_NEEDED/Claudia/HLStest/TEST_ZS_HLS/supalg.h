#ifndef _SUPALG_H_
#define _SUPALG_H_

#include<iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cmath>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>
#include<cstdio>
#include <stdio.h>
#include <string.h>
#include "hls_stream.h"
#include <bitset>
using namespace std;

#define n 536870912 //filesize
#define hardwareclockSPILL 1.695 //us
#define hardwareclockGAP 100 //us
#define NRAMblocks 5
#define ADCRAM 2048 //ADC values per RAM block
#define headersize 19 //input header size in ADC values  
#define fADC 370 //MHz
#define window 100
#define threshold -100
#define tbefore 1 //us
#define tafter 2 //us


//typedef double double_data;
//typedef int16_t int16_data;
//typedef int64_t int64_data;
//typedef unsigned long int ulongint_data;


struct out_stream_data{
  int16_t data;
};

typedef hls::stream<int16_t> in_stream_t;
typedef hls::stream<out_stream_data> out_stream_t;


//Sampling time of ADC (microsec) 
const double tadc = 1.0/(fADC);

//store tbefore microseconds of data to the left of the trigger
const int prenumADCstored=int(tbefore/tadc);

//store tafter microseconds of data to the right of the trigger
const int postnumADCstored=int(tafter/tadc);

//total number of ADC values stored per trigger
const int totalnumADCstored=prenumADCstored+postnumADCstored;

//Theoretical number of ADC values per chunk
const double chunkdoub=NRAMblocks*ADCRAM;
const unsigned long int chunk=NRAMblocks*ADCRAM;



//Number of triggers per chunk to send in Spill
const unsigned long int ntriggers_chunkSPILL=(chunkdoub/fADC)/hardwareclockSPILL;

//Number of ADC values per trigger in Spill
const unsigned long int nADC_triggerSPILL=fADC*hardwareclockSPILL;

//Number of ADC values per chunk according to triggers in Spill
const unsigned long int nADC_chunk=nADC_triggerSPILL*ntriggers_chunkSPILL; 

//Free ADC spaces per chunk for ADC=0 in Spill and Gap
const unsigned long int restchunk=chunkdoub-nADC_chunk-headersize; 

//Number of ADC values per trigger in Gap
const unsigned long int nADC_triggerGAP=fADC*hardwareclockGAP;



void supalg(int16_t allchunk[chunk], out_stream_t &output, unsigned long int &ntriggers, int &zp_size, unsigned long int ADC_hardtrig);

#endif
