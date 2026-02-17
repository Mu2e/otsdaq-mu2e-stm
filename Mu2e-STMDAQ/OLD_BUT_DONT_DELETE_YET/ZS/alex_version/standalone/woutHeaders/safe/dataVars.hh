/////////////////////////////////////////////////////////////////////////
/// A header file containing all the data size informtion
/////////////////////////////////////////////////////////////////////////

#ifndef DATAVARS_hh
#define DATAVARS_hh

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#ifndef DATAVARS_hh_DEFINED
#define DATAVARS_hh_DEFINED
class dataVars;
#endif

// Max int16_t value
static const uint INT16_T_MAX = 32767;
// Max uint16_t value
static const uint16_t UINT16_T_MAX = 0xFFFF;
// Max uint32_t value
static const uint32_t UINT32_T_MAX = 0XFFFFFFFF;

// Number of data channels
static const int CHNUM = 2;
// Signal HPGE data as CHANNEL 0
static const int HPGE = 0;
// Signal LABR data as CHANNEL 1
static const int LABR = 1;

// Firmware packet header size in bytes
static const uint fw_pHdr_Size = 6;
// Firmware packet header length
static const uint fw_pHdr_Len = fw_pHdr_Size/2;
// Firmware packet header elements
static const uint fw_pHdr_pNum1 = 0; // Packet number 2
static const uint fw_pHdr_pNum2 = 1; // Packet number 1
static const uint fw_pHdr_end = 2; // Packet checksum
static const uint16_t fw_pHdr_end_data = 0xFFEE;

// 0xDEADBEEF
static const uint16_t DEAD = 0xDEAD;
static const uint16_t BEEF = 0xBEEF;

// Firmware trigger header size in bytes
static const uint fw_tHdr_Size = 64;
// Firmware trigger header length
static const uint fw_tHdr_Len = fw_tHdr_Size/2;

// Firmware trigger header elements
struct fw_tHdr{

  // Size in bytes of the trigger header
  static const uint size = fw_tHdr_Size;
  // Length in int16_t values of the trigger header
  static const uint len = fw_tHdr_Len;

  // Index positions of trigger header components
  static const uint Ch_DTCclk_0 = 0; // Channel + DTC Clock (200 MHz) 0
  static const uint DTCclk_1 = 1; // DTC Clock (200 MHz) 1
  static const uint DTCclk_2 = 2; // DTC Clock (200 MHz) 2
  static const uint DTCclk_3 = 3; // DTC Clock (200 MHz) 3
  static const uint ADCclk_0 = 4; // ADC Clock (75 MHz) 0
  static const uint ADCclk_1 = 5; // ADC Clock (75 MHz) 1
  static const uint ADCclk_2 = 6; // ADC Clock (75 MHz) 2
  static const uint ADCclk_3 = 7; // ADC Clock (75 MHz) 3
  static const uint EvNum_0 = 8; // Event Number 0
  static const uint EvNum_1 = 9; // Event Number 1
  static const uint EvNum_2 = 10; // Event Number 2
  static const uint EWT_0 = 11; // DTC Event Window Tag 0
  static const uint EWT_1 = 12; // DTC Event Window Tag 1
  static const uint EWT_2 = 13; // DTC Event Window Tag 2
  static const uint EM_0 = 14; // DTC Event Mode 0
  static const uint EM_1 = 15; // DTC Event Mode 1
  static const uint EM_2_DRTDC = 16; // DTC Event Mode 2 + Delivery Ring TDC
  static const uint EvStart = 17; // Event Start Offset
  static const uint EvLen = 18; // Event Length (To Read)
  static const uint HdrEnd_0 = 19; // 0xBEEF
  static const uint HdrEnd_1 = 20; // 0xCAFE
  static const uint HdrEnd_2 = 21; // 0xBEEF
  static const uint HdrEnd_3 = 22; // 0xCAFE
  static const uint HdrEnd_4 = 23; // 0xAAAA
  static const uint HdrEnd_5 = 24; // 0x5678
  static const uint HdrEnd_6 = 25; // 0x1234
  static const uint HdrEnd_7 = 26; // 0xFFFF
  static const uint HdrEnd_8 = 27; // 0xEEEE
  static const uint HdrEnd_9 = 28; // 0xDDDD
  static const uint HdrEnd_10 = 29; // 0xCCCC
  static const uint HdrEnd_11 = 30; // 0xBBBB
  static const uint HdrEnd_12 = 31; // 0xAAAA

  // Size in bytes of the header end
  static const uint HdrEnd_size = 26;
  // Length in int16_t values of the header end
  static const uint HdrEnd_len = HdrEnd_size/2;
  // Starting index of the header end
  static const uint HdrEnd_start = fw_tHdr_Len - HdrEnd_len;

  // Trigger header end data
  static const int16_t HdrEnd_data[HdrEnd_len];
   
};

// Define trigger header struct
const fw_tHdr tHdr_vars;

class dataVars{

public:

  // Standard constructor - shouldn't be used
  dataVars();

  // Form uin32_t out of 2 x int16_t 
  uint32_t make_uint32_t(int16_t p0, int16_t p1);

  // Form uin64_t out of 4 x int16_t 
  uint64_t make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3);

  // Get just the event number from the event header
  uint64_t get_event_number(int16_t *data, uint64_t hdr_start_loc);

  // Check the end of a packet for repeated 0xDEADBEEF
  uint16_t check_dead_beef(int16_t* data, uint64_t packet_start, uint16_t leftInPacket);


private:

};

#endif
