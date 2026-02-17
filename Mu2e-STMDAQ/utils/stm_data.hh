#ifndef STM_DATA_hh
#define STM_DATA_hh

#include <iterator>

// Hex reader
#include "Mu2e-STMDAQ/utils/Hex.hh"
// Async Logger code                                                     
#include "Mu2e-STMDAQ/utils/async_logger.hh"

// Max int16_t value
static constexpr uint INT16_T_MAX = 32767;
// Max uint16_t value
static constexpr uint16_t UINT16_T_MAX = 0xFFFF;
// Max uint32_t value
static constexpr uint32_t UINT32_T_MAX = 0XFFFFFFFF;

// Number of data channels
static constexpr int CHNUM = 2;
// Signal HPGE data as CHANNEL 0
static constexpr int HPGE = 0;
// Signal LABR data as CHANNEL 1
static constexpr int LABR = 1;

// Maxiumum packet size in bytes
static constexpr uint16_t MAX_PACKET_SIZE = 8198;
// Max length of packet in number of int16_t values
static constexpr uint16_t MAX_PACKET_LEN = MAX_PACKET_SIZE/2;

// Firmware packet header size in bytes
static constexpr uint16_t pHdr_Size = 6;
// Firmware packet header length
static constexpr uint16_t pHdr_Len = pHdr_Size/2;
// Firmware packet header elements
static constexpr uint16_t pHdr_pNum1 = 0; // Packet number 2
static constexpr uint16_t pHdr_pNum2 = 1; // Packet number 1
static constexpr uint16_t pHdr_end = 2; // Packet checksum
static constexpr uint16_t pHdr_check = 0xFFEE;

// 0xDEADBEEF
static constexpr uint16_t DEAD = 0xDEAD;
static constexpr uint16_t BEEF = 0xBEEF;

// Firmware event header size in bytes
static constexpr uint16_t eHdr_Size = 64;
// Firmware event header length
static constexpr uint16_t eHdr_Len = eHdr_Size/2;

// Event header elements
struct eHdr_info{

  // Size in bytes of the event header
  static constexpr uint16_t size = eHdr_Size;
  // Length in int16_t values of the event header
  static constexpr uint16_t len = eHdr_Len;

  // Size in bytes of the event header anchor
  static constexpr uint16_t anchor_size = 8;
  // Length in int16_t values of the header start
  static constexpr uint16_t anchor_len = anchor_size/2;
  // Starting index of the header anchor
  static constexpr uint16_t anchor_start = 0;

  // Event header anchor data
  static constexpr int16_t anchor_data[anchor_len]
  = {(int16_t)0xCAFE, // 0 = 0xCAFE
    (int16_t)0xBBBB, // 1 = 0xBBBB
    (int16_t)0xAAAA, // 2 = 0xAAAA
    (int16_t)0xCAFE}; // 3 = 0xCAFE

  // Index positions of event header components
  static constexpr uint16_t anchor_0 = 0; // 0xCAFE
  static constexpr uint16_t anchor_1 = 1; // 0xBBBB
  static constexpr uint16_t anchor_2 = 2; // 0xAAAA
  static constexpr uint16_t anchor_3 = 3; // 0xCAFE
  static constexpr uint16_t ADCclk_0 = 4; // ADC Clock (75 MHz) 0
  static constexpr uint16_t ADCclk_1 = 5; // ADC Clock (75 MHz) 1
  static constexpr uint16_t ADCclk_2 = 6; // ADC Clock (75 MHz) 2
  static constexpr uint16_t ADCclk_3 = 7; // ADC Clock (75 MHz) 3
  static constexpr uint16_t EvNum_0 = 8; // Event Number 0
  static constexpr uint16_t EvNum_1 = 9; // Event Number 1
  static constexpr uint16_t EvNum_2 = 10; // Event Number 2
  static constexpr uint16_t EWT_0 = 11; // DTC Event Window Tag 0
  static constexpr uint16_t EWT_1 = 12; // DTC Event Window Tag 1
  static constexpr uint16_t EWT_2 = 13; // DTC Event Window Tag 2
  static constexpr uint16_t EM_0 = 14; // DTC Event Mode 0
  static constexpr uint16_t EM_1 = 15; // DTC Event Mode 1
  static constexpr uint16_t EM_2_DRTDC = 16; // DTC Event Mode 2 + Delivery Ring TDC
  static constexpr uint16_t EvStart = 17; // Event Start Offset
  static constexpr uint16_t EvInPacket = 18; // Portion of event in packet
  static constexpr uint16_t Beef = 19; // 0xBEEF
  static constexpr uint16_t SubRunNum_0 = 20; // Subrun number 1
  static constexpr uint16_t SubRunNum_1 = 21; // Subrun number 2  
  static constexpr uint16_t ZSflag_PreVal = 22; // ZS flag + prescale value
  static constexpr uint16_t EvLen = 23; // Total length of event
  static constexpr uint16_t RunNum_0 = 24; // Run number from DCS write 1
  static constexpr uint16_t RunNum_1 = 25; // Run number from DCS write 2
  static constexpr uint16_t RunNum_2 = 26; // Run number from DCS write 3
  static constexpr uint16_t RunNum_3 = 27; // Run number from DCS write 4
  static constexpr uint16_t Ch_DTCclk_0 = 28; // Channel [0/1] + DTC Clock (200 MHz) 1
  static constexpr uint16_t DTCclk_1 = 29; // DTC Clock (200 MHz) 1
  static constexpr uint16_t DTCclk_2 = 30; // DTC Clock (200 MHz) 2
  static constexpr uint16_t DTCclk_3 = 31; // DTC Clock (200 MHz) 3

};

// Static instance of the event header info
static constexpr eHdr_info eHdr;

class STMdata{

private:
  
  // Async Logger                                                        
  const std::shared_ptr<AsyncLogger> logger;
  
public:

  // Constructor
  STMdata(const std::shared_ptr<AsyncLogger>& logger_);
  
  // Destructor                                                          
  ~STMdata() {
    std::cout << "STMdata destructor called.\n";
  }
  
  // Form uin32_t out of 2 x int16_t 
  uint32_t make_uint32_t(int16_t p0, int16_t p1);

  // Form uin64_t out of 4 x int16_t 
  uint64_t make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3);

  // Split uin32_t into 2 x int16_t 
  int16_t* split_uint32_t(uint32_t value);

  // Split uin64_t into 4 x int16_t 
  int16_t* split_uint64_t(uint64_t value);

  // Get just the event number from the event header
  uint64_t get_event_number(std::vector<int16_t>& data,
			    uint64_t hdr_start_loc);

  // Get just the EWT from the event header
  uint64_t get_EWT(std::vector<int16_t>& data,
		   uint64_t hdr_start_loc);

  // Get the event mode
  uint64_t get_event_mode(std::vector<int16_t>& data,
			  uint64_t hdr_index);
  
  // Check the end of a packet for repeated 0xDEADBEEF
  uint16_t check_dead_beef(std::vector<int16_t>& data,
			   uint64_t packet_start, uint16_t leftInPacket);

};

#endif
