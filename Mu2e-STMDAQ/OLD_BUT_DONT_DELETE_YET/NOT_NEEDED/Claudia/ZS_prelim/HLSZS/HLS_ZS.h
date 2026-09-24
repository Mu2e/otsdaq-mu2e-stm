#ifndef _HLS_ZS_H_
#define _HLS_ZS_H_

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
#include "ap_int.h"

#include "/work/cgarcia/STMDAQ-TestBeam/Claudia/ZS_prelim/dataFIFOHeaders.hh"

using namespace std;

struct out_stream_data{
  int16_t data;
};

typedef hls::stream<int16_t> in_stream_t;
typedef hls::stream<out_stream_data> out_stream_t;

typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;
typedef ap_int<8> int8;


//Sampling time of ADC (microsec) 
const double tadc = 1.0/(fADC);

//store tbefore microseconds of data to the left of the trigger
const uint32_t prenumADCstored=int(tbefore/tadc);

//store tafter microseconds of data to the right of the trigger
const uint32_t postnumADCstored=int(tafter/tadc);

//total number of ADC values stored per trigger
const uint32_t totalnumADCstored=prenumADCstored+postnumADCstored;

//Theoretical number of ADC values per chunk
const double chunkdoub=NRAMblocks*ADCRAM;
const uint64_t chunk=NRAMblocks*ADCRAM;


//Long Clock
//Long Spill hardware clock
const uint64_t nADC_triggerSPILL_clock=fADC*hardware_longclockSPILL;
//Number of triggers per chunk to send in Spill
const uint64_t ntriggers_chunkSPILL=((chunkdoub/fADC)/hardware_longclockSPILL)-1; //-1 to not exceed the chunk due to trigg headers
//Number of ADC values per trigger in Spill
const uint64_t nADC_triggerSPILL=fADC*hardware_longclockSPILL;


/*
//Short Clock
//Short Spill hardware clock
const uint64_t nADC_triggerSPILL_clock=fADC*hardware_shortclockSPILL;
//Number of triggers per chunk to send in Spill
const uint64_t ntriggers_chunkSPILL=((chunkdoub/fADC)/hardware_shortclockSPILL);
//Number of ADC values per trigger in Spill
const uint64_t nADC_triggerSPILL=fADC*hardware_shortclockSPILL;
*/


//Number of ADC values per chunk according to triggers in Spill
const uint64_t nADC_SPILLchunk=nADC_triggerSPILL*ntriggers_chunkSPILL;



//Number of triggers per chunk to send in Gap
const uint64_t ntriggers_chunkGAP=(chunkdoub/fADC)/hardwareclockGAP;
//Number of ADC values per trigger in Gap
const uint64_t nADC_triggerGAP=fADC*hardwareclockGAP;
//Number of ADC values per chunk in Gap
const uint64_t nADC_GAPchunk=chunk-infifo_hdrs_Len;

void HLS_ZS(uint16_t allchunk[chunk], out_stream_t &output, uint64_t &ntriggers, uint32_t &zp_size);

#endif
