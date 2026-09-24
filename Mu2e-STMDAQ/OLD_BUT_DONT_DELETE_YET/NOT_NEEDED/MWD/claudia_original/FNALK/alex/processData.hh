// processData.hh

#ifndef PROCESSDATA_HH
#define PROCESSDATA_HH

// Load C++ libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>

#include "Hex.hh"
//#include "dataVars.hh"

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

//Load blinding libraries
class processData {
 
 public:

  processData();

  // Form uin64_t out of 4 x int16_t
  uint64_t make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3);

  // Read binary file
  std::vector<int16_t> readFile(char* fname);

};

#endif // PROCESSDATA_HH
