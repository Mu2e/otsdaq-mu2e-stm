#ifndef STM_DATA_hh
#define STM_DATA_hh

#include <iterator>
#include <cmath>

// Hex reader
#include "Mu2e-STMDAQ/utils/Hex.hh"
// Async Logger code                                                     
#include "Mu2e-STMDAQ/utils/async_logger.hh"

// Config code
#include "Mu2e-STMDAQ/config/include/master_config.hh" // master
#include "Mu2e-STMDAQ/config/include/fw_config.hh" // firmware
#include "Mu2e-STMDAQ/config/include/mu2e_config.hh" // mu2e
#include "Mu2e-STMDAQ/config/include/udp_config.hh" // udp
#include "Mu2e-STMDAQ/config/include/zs_config.hh" // zero suppression
#include "Mu2e-STMDAQ/config/include/prescale_config.hh" // prescale
#include "Mu2e-STMDAQ/config/include/write_config.hh" // file writing
#include "Mu2e-STMDAQ/config/include/buffer_config.hh" // buffers
#include "Mu2e-STMDAQ/config/include/baseline_config.hh" // adc baseline
#include "Mu2e-STMDAQ/config/include/mwd_config.hh" // moving window algorithm
#include "Mu2e-STMDAQ/config/include/pulseheight_config.hh" // labr pulse-height algorithm
#include "Mu2e-STMDAQ/config/include/dqm_config.hh" // dqm
#include "Mu2e-STMDAQ/config/include/tcp_config.hh" // tcp

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
static constexpr uint16_t fw_eHdr_size = 64;
// Firmware event header length
static constexpr uint16_t fw_eHdr_len = fw_eHdr_size/sizeof(int16_t);

// Firmware event header elements
struct fw_eHdr_info{
  
  // Size in bytes of the event header
  static constexpr uint16_t size = fw_eHdr_size;
  // Length in int16_t values of the event header
  static constexpr uint16_t len = fw_eHdr_len;

  // Size in bytes of the event header anchor
  static constexpr uint16_t anchor_size = 8;
  // Length in int16_t values of the header start
  static constexpr uint16_t anchor_len = anchor_size/2;
  // Starting index of the header anchor
  static constexpr uint16_t anchor_start = 0;

  // Event header anchor data
  static constexpr int16_t anchor_data[anchor_len]
  = {static_cast<int16_t>(0xCAFE), // 0 = 0xCAFE
    static_cast<int16_t>(0xBBBB), // 1 = 0xBBBB
    static_cast<int16_t>(0xAAAA), // 2 = 0xAAAA
    static_cast<int16_t>(0xCAFE)}; // 3 = 0xCAFE
  
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

// Static instance of the firmware event header info
static constexpr fw_eHdr_info fw_eHdr;

// Software event header size in bytes
static constexpr uint16_t sw_eHdr_size = 42;
// Software event header length
static constexpr uint16_t sw_eHdr_len = sw_eHdr_size/sizeof(int16_t);

// Software event header elements
struct sw_eHdr_info{
  
  // Size in bytes of the event header
  static constexpr uint16_t size = sw_eHdr_size;
  // Length in int16_t values of the event header
  static constexpr uint16_t len = sw_eHdr_len;

  // Software event header anchor
  static constexpr int16_t anchor = static_cast<int16_t>(0xCAFE);
  
  // Index positions of event header components
  static constexpr uint16_t anchor_start = 0; // 0xCAFE
  static constexpr uint16_t EWT_0 = 1; // DTC Event Window Tag 0
  static constexpr uint16_t EWT_1 = 2; // DTC Event Window Tag 1
  static constexpr uint16_t EWT_2 = 3; // DTC Event Window Tag 2
  static constexpr uint16_t ADCclk_0 = 4; // ADC Clock (75 MHz) 0
  static constexpr uint16_t ADCclk_1 = 5; // ADC Clock (75 MHz) 1
  static constexpr uint16_t ADCclk_2 = 6; // ADC Clock (75 MHz) 2
  static constexpr uint16_t ADCclk_3 = 7; // ADC Clock (75 MHz) 3
  static constexpr uint16_t Ch_DTCclk_0 = 8; // Channel [0/1] + DTC Clock (200 MHz) 1
  static constexpr uint16_t DTCclk_1 = 9; // DTC Clock (200 MHz) 1
  static constexpr uint16_t DTCclk_2 = 10; // DTC Clock (200 MHz) 2
  static constexpr uint16_t DTCclk_3 = 11; // DTC Clock (200 MHz) 3
  static constexpr uint16_t EM_0 = 12; // DTC Event Mode 0
  static constexpr uint16_t EM_1 = 13; // DTC Event Mode 1
  static constexpr uint16_t EM_2_DRTDC = 14; // DTC Event Mode 2 + Delivery Ring TDC
  static constexpr uint16_t PRESCALE = 15; // Raw Prescale On/Off + Raw Prescale Num +
                                           // ZS Prescale On/Off + ZS Prescale Num
  static constexpr uint16_t RAW_LEN = 16; // Raw Data Len
  static constexpr uint16_t ZS_REGIONS = 17; // Number of ZS data regions
  static constexpr uint16_t ZS_LEN = 18; // ZS Data Len
  static constexpr uint16_t PH_NUM = 19; // Number of pulse height values
  static constexpr uint16_t anchor_end = 20; // 0xCAFE
};

// Static instance of the software event header info
static constexpr sw_eHdr_info sw_eHdr;

// Define software event header type
using sw_event_header = std::array<int16_t, sw_eHdr_info::len>;

class STMdata{

private:

  // Store reference to the Config instance
  Config& cfg;
  
  // Async Logger                                                        
  const std::shared_ptr<AsyncLogger> logger;  
  
public:

  // Constructor
  STMdata(Config& cfg,
          const std::shared_ptr<AsyncLogger>& logger_);
  
  // Destructor                                                          
  ~STMdata() {
    std::cout << "STMdata destructor called.\n";
  }

  // Form uint32_t out of 2 x int16_t 
  uint32_t make_uint32_t(int16_t p0, int16_t p1);

  // Form uint64_t out of 4 x int16_t 
  uint64_t make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3);

  // Split uin32_t into 2 x int16_t 
  std::vector<int16_t> split_uint32_t(uint32_t value);

  // Split uin64_t into 4 x int16_t 
  std::vector<int16_t> split_uint64_t(uint64_t value);

  // Get just the event number from the event header
  uint64_t get_event_number(int16_t* data, uint64_t hdr_index);

  // Get just the ADC clock time from the event header
  uint64_t get_ADCclock(int16_t* data, uint64_t hdr_index);
  
  // Get just the EWT from the event header
  uint64_t get_EWT(int16_t* data, uint64_t hdr_index);
  
  // Get the event mode
  uint64_t get_event_mode(int16_t* data, uint64_t hdr_index);
  
  // Get the channel number and DTC clock word from the event header
  uint64_t get_Ch_DTCclk(int16_t* data, uint64_t hdr_index);
  
  // Get just the channel number from the event header
  uint16_t get_channel(int16_t* data, uint64_t hdr_index);
  
  // Get just the DTC clock time from the event header
  uint64_t get_DTCclock(int16_t* data, uint64_t hdr_index);
  
  // Check the end of a packet for repeated 0xDEADBEEF
  uint16_t check_dead_beef(int16_t* data, uint64_t packet_start, uint16_t leftInPacket);

  // Create software event header from buffer
  sw_event_header create_sw_eHdr(int16_t* data, uint64_t hdr_index);

  // Create software event header from individual inputs
  sw_event_header create_sw_eHdr(uint64_t EWT,
                                 uint64_t ADCclk,
                                 uint64_t Ch_DTCclk,
                                 uint64_t EM);

  // STMDAQ master config
  const master_info master_config;
  
  // Firmware configuration
  const fw_info fw_config;

  // Mu2e configuration
  const mu2e_info mu2e_config;

  // UDP configuration
  const udp_info udp_config;
  
 // Zero suppression configuration
  const zs_info zs_config;
  
  // Prescale configuration
  const prescale_info prescale_config;
  
  // MWD configuration
  const write_info write_config;
  
  // Buffer configuration
  const buffer_info buffer_config;

  // ADC baseline configuration
  const baseline_info baseline_config;

  // MWD configuration
  const mwd_info mwd_config;

  // Pulse height configuration
  const pulseheight_info pulseheight_config;

  // TCP configuration
  const tcp_cfg_info tcp_config;
  
  // DQM configuration
  const dqm_info dqm_config;
  
};

#endif
