#ifndef _EXAMPLE_H_
#define _EXAMPLE_H_

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

#define chunk 10

typedef int16_t int16_data;


struct out_stream_data{
  int16_t data;
};


/*struct in_stream_data{
  int16_t data1;
};*/


typedef hls::stream<out_stream_data> out_stream_t;

void example(int16_data allchunk[chunk], out_stream_t &output);


#endif
