/////////////////////////////////////////////////////////////////////////
/// 
/////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

// Data variables header
#include "dataVars.hh"

// Standard constructor - shouldn't be used
dataVars::dataVars() {}

// // Define trigger header struct
// const fw_tHdr tHdr_vars;

const int16_t fw_tHdr::HdrEnd_data[fw_tHdr::HdrEnd_len] = {(int16_t)0xBEEF, // 19 = 0xBEFF
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

 
// Form uin32_t out of 2 x int16_t 
uint32_t dataVars::make_uint32_t(int16_t p0, int16_t p1){
 
  uint64_t value = static_cast<uint16_t>(p1);
  value <<= 16;
  value |= static_cast<uint16_t>(p0);

  return value;
    
}
  
// Form uin64_t out of 4 x int16_t 
uint64_t dataVars::make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3){
 
  uint64_t value = static_cast<uint16_t>(p3);
  value <<= 16;
  value |= static_cast<uint16_t>(p2);
  value <<= 16;
  value |= static_cast<uint16_t>(p1);
  value <<= 16;
  value |= static_cast<uint16_t>(p0);

  return value;
    
}

// Get just the event number from the event header
uint64_t dataVars::get_event_number(int16_t *data, uint64_t hdr_start_loc){

  // Get the trigger/event number
  uint64_t event_number = make_uint64_t(data[hdr_start_loc+tHdr_vars.EvNum_0],
					data[hdr_start_loc+tHdr_vars.EvNum_1],
					data[hdr_start_loc+tHdr_vars.EvNum_2],
					0);
  return event_number;

}

