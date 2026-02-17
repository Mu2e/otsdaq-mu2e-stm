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
using namespace std;

#define n 536870912 //filesize
#define window 100
#define hardwareclock 1.695 //us 
#define chunk 10000 
#define fADC 370 //MHz
#define threshold -100
#define tbefore 2 //us
#define tafter 5 //us


typedef double double_data;
typedef int16_t int16_data;
typedef unsigned long int ulongint_data;


struct out_stream_data{
  double_data time;
  int16_data data;
};

typedef hls::stream<int16_data> in_stream_t;
typedef hls::stream<out_stream_data> out_stream_t;

void supalg(in_stream_t &input, out_stream_t &output, ulongint_data &ntriggers, ulongint_data &last_trigerstored, double_data topcalls,  ulongint_data nADC_chunk, int16_data ADC_hardtrig[]);

//Sampling time of ADC (microsec) 
const double tadc = 1.0/(fADC);

//store tbefore microseconds of data to the left of the trigger
const int prenumADCstored=int(tbefore/tadc);

//store tafter microseconds of data to the right of the trigger
const int postnumADCstored=int(tafter/tadc);

//total number of ADC values stored per trigger
const int totalnumADCstored=prenumADCstored+postnumADCstored;
#endif
