/////////////////////////////////////////////////////////////////////////
/// A header file containing all the data size informtion
/////////////////////////////////////////////////////////////////////////

#ifndef DATAVARS_hh
#define DATAVARS_hh

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <bits/stdc++.h>

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// XML interface
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

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

// Signal firmware IP/PORT with 0
static const int FW = 0;
// Signal software IP/PORT with 1
static const int SW = 1;
// Signal DQM IP/PORT with 2
static const int DQM = 2;

// Maxiumum packet size in bytes
// (Maximum packet size - UDP checksum)
static const uint16_t MAX_PACKET_SIZE = 8198;
// Max length of packet in number of int16_t values
static const uint16_t MAX_PACKET_LEN = MAX_PACKET_SIZE/2;

// // The theoretical maximum udp datagram size
// static const uint32_t MAX_UDP_SIZE = 65535;
// // The optimised SO_RCVBUF size
// static const uint64_t RCVBUF_SZ = 1e5*pow(2,21);
// // The optimised SO_SNDBUF soize
// static const uint64_t SNDBUF_SZ = 1e2*pow(2,21);

// // Recvfrom non-blocking timeout time 
// static const uint TIMEOUT_SECS = 0; // Secs
// static const uint TIMEOUT_USECS = 500; // usecs
// // The number of non-blocking timeouts to wait before exit
// static const uint TIMEOUT_MAX = 5e7;

// Number of slices per packet
// NOT TO BE HARD-CODED. JUST UNTIL ERDEM FIXES THE SLICES.
static const uint slicesPerPacket = 2;

static const uint on_spill_len = 510;
static const uint on_spill_size = 2*on_spill_len;

static const uint off_spill_len = 30000;
static const uint off_spill_size = 2*off_spill_len;
  
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

struct test{
  static constexpr int testArr[2] = {1,2};
};

// Firmware trigger header elements
struct fw_tHdr{

  // Size in bytes of the trigger header
  static const uint size = fw_tHdr_Size;
  // Length in int16_t values of the trigger header
  static const uint len = fw_tHdr_Len;

  // Index positions of trigger header components
  static const uint HdrStart_0 = 0; // 0xCAFE
  static const uint HdrStart_1 = 1; // 0xBBBB
  static const uint HdrStart_2 = 2; // 0xAAAA
  static const uint HdrStart_3 = 3; // 0xCAFE
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
  static const uint EvInPacket = 18; // Portion of event in packet
  static const uint Beef = 19; // 0xBEEF
  static const uint SubRunNum_0 = 20; // Incrementing subrun number from FW 1
  static const uint SubRunNum_1 = 21; // Incrementing subrun number from FW 2  
  static const uint ZSflag_PreVal = 22; // ZS flag + prescale value
  static const uint EvLen = 23; // Total length of event
  static const uint RunNum_0 = 24; // Run number from DCS write 1
  static const uint RunNum_1 = 25; // Run number from DCS write 2
  static const uint RunNum_2 = 26; // Run number from DCS write 3
  static const uint RunNum_3 = 27; // Run number from DCS write 4
  static const uint Ch_DTCclk_0 = 28; // Channel [0/1] + DTC Clock (200 MHz) 1
  static const uint DTCclk_1 = 29; // DTC Clock (200 MHz) 1
  static const uint DTCclk_2 = 30; // DTC Clock (200 MHz) 2
  static const uint DTCclk_3 = 31; // DTC Clock (200 MHz) 3

  // Size in bytes of the header end
  static const uint HdrStart_size = 8;
  // Length in int16_t values of the header end
  static const uint HdrStart_len = HdrStart_size/2;
  // Starting index of the header end
  static const uint HdrStart_start = 0;

  // Trigger header end data
  static const int16_t HdrStart_data[HdrStart_len];
   
};

// Define trigger header struct
const fw_tHdr tHdr_vars;

// Struct containing experimental configuration info from xml
struct expConfig{

  // Accelerator clock frequency
  double trigger_time_clock; // MHz 
  // Macropulse width
  double macropulse_width; // ms
  // Macropulse frequency
  double macropulse_freq; // Hz
  
  // ADC clock frequency 
  double adc_time_clock; // MHz 
  // Sample period of ADC (microsec) 
  double adc_time_clock_period; // us

  // ADC offset clock frequency
  double adc_offset_time_clock; // MHz

  // ADC prescale
  uint16_t prescale;

  // External mode ADC offset
  uint32_t ext_ADC_offset; // Multiple of fADCoffset
  // Slice length - external mode      
  uint32_t ext_slice_length; // Number of int16_t values
  // Number of slices - external mode  
  uint16_t ext_slice_num;

  // The time T after a trigger to wait for a new trigger
  // before switching to external mode
  uint32_t ext_trig_timeout;

  // The offset in time to wait after external mode 
  // before sending the first internal trigger.
  uint32_t ext_int_delay;

  // The number of internal triggers to generate.
  uint16_t int_trig_num;
  // Internal mode ADC offset
  uint32_t int_ADC_offset; // Multiple of fADCoffset
  // Slice length - internal mode      
  uint32_t int_slice_length; // Number of int16_t values
  // Number of slices - internal mode  
  uint16_t int_slice_num;

  // ADC sampling time in external mode    
  double ext_sample_time; // us 
  // ADC sampling time in internal mode 
  double int_sample_time; // us
  


};

class dataVars{

public:

  // Standard constructor - shouldn't be used
  dataVars();

  // Set experimental configuration values from xml
  expConfig getXMLvalues(expConfig exp);

  // Check experimental configuation values from xml
  int checkExpConfig(expConfig exp);
 
  // Form uin32_t out of 2 x int16_t 
  uint32_t make_uint32_t(int16_t p0, int16_t p1);

  // Form uin64_t out of 4 x int16_t 
  uint64_t make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3);

  // Split uin32_t into 2 x int16_t 
  int16_t* split_uint32_t(uint32_t value);

  // Split uin64_t into 4 x int16_t 
  int16_t* split_uint64_t(uint64_t value);

  // Get just the event number from the event header
  uint64_t get_event_number(int16_t *data, uint64_t hdr_start_loc);

  // Get just the EWT from the event header
  uint64_t get_EWT(int16_t *data, uint64_t hdr_start_loc);

  // Check the end of a packet for repeated 0xDEADBEEF
  uint16_t check_dead_beef(int16_t* data, uint64_t packet_start, uint16_t leftInPacket);

  // Data for a single event
  struct event{
    // Data size
    uint16_t size = 0;
    // Data array
    int16_t* data;
    // // Data size
    // uint16_t size = fw_tHdr_Size + off_spill_size;
    // // Data array
    // int16_t data[fw_tHdr_Len + off_spill_len] = {};


  };

  int16_t tpacket[MAX_PACKET_LEN] = {};
  //  int16_t tevent[MAX_UDP_SIZE/2] = {};

  // // Data for a single event
  // struct tevent{
  //   // Data size
  //   uint16_t size = 10;
  //   // Data array
  //   int16_t data[5] = {};
  // };

  // Data for a single on-spill event
  struct on_event{
    // Data size
    uint16_t size = fw_tHdr_Size + on_spill_size;
    // Data array
    int16_t data[fw_tHdr_Len + on_spill_len] = {};
  };

  // Data for a single off-spill event
  struct off_event{
    // Total payload size (kb)
    uint16_t size = 0;
    // Event number
    uint64_t number = 0;
    // Event length
    uint16_t length = 0;
    // Data array
    int16_t data[fw_tHdr_Len + off_spill_len] = {};
  };

  // Data for a single prescaled event (header only)
  struct ps_hdr_event{
    // Data size
    uint16_t size = fw_tHdr_Size;
    // Data array
    int16_t data[fw_tHdr_Len] = {};
  };



private:

};

#endif
