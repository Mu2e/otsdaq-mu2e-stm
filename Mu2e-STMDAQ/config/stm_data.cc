// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"

// Constructor
STMdata::STMdata(Config& cfg_,
                 const std::shared_ptr<AsyncLogger>& logger_) :
  cfg(cfg_), logger(logger_),
  master_config(cfg,logger), // STMDAQ master config
  fw_config(cfg,logger), // firmware config
  mu2e_config(cfg,logger,fw_config), // mu2e config
  udp_config(cfg,logger,master_config), // udp config
  zs_config(cfg,logger,master_config,fw_config,mu2e_config), // zero suppression config
  prescale_config(cfg,logger), // prescale config  
  write_config(cfg,logger), // file writing config
  buffer_config(cfg,logger,fw_config,mu2e_config,zs_config,MAX_PACKET_SIZE), // buffer config
  baseline_config(cfg,logger,fw_config,buffer_config,MAX_PACKET_LEN - pHdr_Len - fw_eHdr_len), // adc baseline config
  mwd_config(cfg,logger,fw_config,baseline_config), // mwd config  
  tcp_config(cfg,logger,master_config), // tcp config  
  dqm_config(cfg,logger,fw_config) // dqm config
{}


// Form uint32_t out of 2 x int16_t 
uint32_t STMdata::make_uint32_t(int16_t p0, int16_t p1){
  uint32_t value = static_cast<uint32_t>(static_cast<uint16_t>(p1)) << 16;
  value |= static_cast<uint16_t>(p0);
  return value;
}


// Form uint64_t out of 4 x int16_t
uint64_t STMdata::make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3){
    uint64_t value = (static_cast<uint64_t>(static_cast<uint16_t>(p3)) << 48)
                   | (static_cast<uint64_t>(static_cast<uint16_t>(p2)) << 32)
                   | (static_cast<uint64_t>(static_cast<uint16_t>(p1)) << 16)
                   |  static_cast<uint64_t>(static_cast<uint16_t>(p0));
    return value;
}

// Split uint32_t into 2 x int16_t
std::vector<int16_t> STMdata::split_uint32_t(uint32_t value){
  std::vector<int16_t> split_value(2);
  split_value[0] = static_cast<int16_t>((value >>  0) & 0xFFFF);
  split_value[1] = static_cast<int16_t>((value >> 16) & 0xFFFF);
  return split_value;
}

// Split uint64_t into 4 x int16_t 
std::vector<int16_t> STMdata::split_uint64_t(uint64_t value){
  std::vector<int16_t> split_value(4);  
  split_value[0] = static_cast<int16_t>((value >>  0) & 0xFFFF);
  split_value[1] = static_cast<int16_t>((value >> 16) & 0xFFFF);
  split_value[2] = static_cast<int16_t>((value >> 32) & 0xFFFF);
  split_value[3] = static_cast<int16_t>((value >> 48) & 0xFFFF);  
  return split_value;
}


// Get just the ADC clock time from the event header
uint64_t STMdata::get_ADCclock(int16_t* data, uint64_t hdr_index){  
  uint64_t ADCclock = make_uint64_t(data[hdr_index+fw_eHdr.ADCclk_0],
                                     data[hdr_index+fw_eHdr.ADCclk_1],
                                     data[hdr_index+fw_eHdr.ADCclk_2],
                                     data[hdr_index+fw_eHdr.ADCclk_3]);
   return ADCclock;  
}
 

// Get just the internal event number from the event header
uint64_t STMdata::get_event_number(int16_t* data, uint64_t hdr_index){
  uint64_t event_number = make_uint64_t(data[hdr_index+fw_eHdr.EvNum_0],
                                        data[hdr_index+fw_eHdr.EvNum_1],
                                        data[hdr_index+fw_eHdr.EvNum_2],
                                        0);
  return event_number;  
}

// Get just the EWT from the event header
uint64_t STMdata::get_EWT(int16_t* data, uint64_t hdr_index){
  uint64_t EWT = make_uint64_t(data[hdr_index+fw_eHdr.EWT_0],
                               data[hdr_index+fw_eHdr.EWT_1],
                               data[hdr_index+fw_eHdr.EWT_2],
                               0);
  return EWT;
}


// // Check the end of a packet for repeated 0xDEADBEEF
uint16_t STMdata::check_dead_beef(int16_t* data,
				  uint64_t packet_start,
				  uint16_t leftInPacket){

  // Get the packet number
  uint32_t pnum = (uint16_t)data[packet_start+1] << 16 
    | (uint16_t)data[packet_start];

  // Check remainder of packet is filled with 0xDEADBEEF
  while (leftInPacket != 0){
    // Find location to check
    int loc = packet_start + MAX_PACKET_LEN - leftInPacket;
    // If end of packet isn't filled with 0xDEADBEE, throw critical error
    if ((uint16_t)(data[loc] & 0xFFFF) != BEEF
        and (uint16_t)(data[loc+1] & 0xFFFF) != DEAD){
      if (logger){
        logger->log("ERROR! End of packet "
                    + std::to_string(pnum)
                    + " not filled with 0xDEADBEEF!!",0);
        return leftInPacket;
      }
      else{
        std::cout << "ERROR! End of packet " << pnum << " not filled with 0xDEADBEEF!!" << std::endl;
        exit(0);
      }
    }
    // Subtract from leftInPacket
    leftInPacket -= 2;
  }

  return leftInPacket;

}

// Get the event mode
uint64_t STMdata::get_event_mode(int16_t* data, uint64_t hdr_index){
  
  // Get event mode data
  uint16_t EM0 = static_cast<uint16_t>(data[hdr_index + fw_eHdr.EM_0]);    
  uint16_t EM1 = static_cast<uint16_t>(data[hdr_index + fw_eHdr.EM_1]); 
  uint16_t EM2 = static_cast<uint16_t>(data[hdr_index + fw_eHdr.EM_2_DRTDC]); 

  // Extract each byte
  uint8_t byte0 = EM0 & 0xFF;
  uint8_t byte1 = (EM0 >> 8) & 0xFF;
  uint8_t byte2 = EM1 & 0xFF;
  uint8_t byte3 = (EM1 >> 8) & 0xFF;
  uint8_t byte4 = EM2 & 0xFF;
  
  // Combine into a 40-bit value (stored in 64-bit int)
  uint64_t EM = 0;
  EM |= static_cast<uint64_t>(byte3) << 32;
  EM |= static_cast<uint64_t>(byte2) << 24;
  EM |= static_cast<uint64_t>(byte1) << 16;
  EM |= static_cast<uint64_t>(byte0) << 8;
  EM |= static_cast<uint64_t>(byte4);
  
  return EM;
  
}

// Get just the DTC clock time from the event header
uint64_t STMdata::get_Ch_DTCclk(int16_t* data, uint64_t hdr_index){
  uint64_t Ch_DTCclk = make_uint64_t(data[hdr_index+fw_eHdr.Ch_DTCclk_0],
                                       data[hdr_index+fw_eHdr.DTCclk_1],
                                       data[hdr_index+fw_eHdr.DTCclk_2],
                                       data[hdr_index+fw_eHdr.DTCclk_3]);
  return Ch_DTCclk;  
}

// Get just the channel number from the event header
uint16_t  STMdata::get_channel(int16_t* data, uint64_t hdr_index){
  return static_cast<uint16_t>(data[hdr_index + fw_eHdr.Ch_DTCclk_0]) & 0x00FF;
}

// Get just the DTC clock time from the event header
uint64_t STMdata::get_DTCclock(int16_t* data, uint64_t hdr_index){
  uint64_t Ch_DTCclk = get_Ch_DTCclk(data,hdr_index);
  uint64_t DTCclock = Ch_DTCclk >> 8;  // bits [63:8] → now in [55:0]
  return DTCclock;  
}


// Create software event header form buffer
sw_event_header STMdata::create_sw_eHdr(int16_t* data, uint64_t hdr_index){
  
  // Create empty header
  sw_event_header hdr{};

  // Starting anchor = 0xCAFE
  hdr[sw_eHdr.anchor_start] = sw_eHdr.anchor;

  // EWT
  hdr[sw_eHdr.EWT_0] = data[hdr_index+fw_eHdr.EWT_0];    
  hdr[sw_eHdr.EWT_1] = data[hdr_index+fw_eHdr.EWT_1];
  hdr[sw_eHdr.EWT_2] = data[hdr_index+fw_eHdr.EWT_2];
  
  // ADC clock
  hdr[sw_eHdr.ADCclk_0] = data[hdr_index+fw_eHdr.ADCclk_0];
  hdr[sw_eHdr.ADCclk_1] = data[hdr_index+fw_eHdr.ADCclk_1];
  hdr[sw_eHdr.ADCclk_2] = data[hdr_index+fw_eHdr.ADCclk_2];
  hdr[sw_eHdr.ADCclk_3] = data[hdr_index+fw_eHdr.ADCclk_3];
  
  // Channel number and DTC clock
  hdr[sw_eHdr.Ch_DTCclk_0] = data[hdr_index+fw_eHdr.Ch_DTCclk_0];
  hdr[sw_eHdr.DTCclk_1] = data[hdr_index+fw_eHdr.DTCclk_1];
  hdr[sw_eHdr.DTCclk_2] = data[hdr_index+fw_eHdr.DTCclk_2];
  hdr[sw_eHdr.DTCclk_3] = data[hdr_index+fw_eHdr.DTCclk_3];
  
  // Event mode
  hdr[sw_eHdr.EM_0] = data[hdr_index + fw_eHdr.EM_0];    
  hdr[sw_eHdr.EM_1] = data[hdr_index + fw_eHdr.EM_1];
  hdr[sw_eHdr.EM_2_DRTDC] = data[hdr_index + fw_eHdr.EM_2_DRTDC]; 

  // Ending anchor = 0xCAFE
  hdr[sw_eHdr.anchor_end] = sw_eHdr.anchor;
  
  return hdr;
}

// Create software event header from individual inputs
sw_event_header STMdata::create_sw_eHdr(uint64_t EWT,
                                        uint64_t ADCclk,
                                        uint64_t Ch_DTCclk,
                                        uint64_t EM) {

  // Create empty header
  sw_event_header hdr{};
    
  hdr[sw_eHdr.anchor_start] = sw_eHdr.anchor;

  auto ewt_words   = split_uint64_t(EWT);
  auto adc_words   = split_uint64_t(ADCclk);
  auto chdtc_words = split_uint64_t(Ch_DTCclk);
  auto em_words    = split_uint64_t(EM);

  hdr[sw_eHdr.EWT_0] = ewt_words[0];
  hdr[sw_eHdr.EWT_1] = ewt_words[1];
  hdr[sw_eHdr.EWT_2] = ewt_words[2];

  hdr[sw_eHdr.ADCclk_0] = adc_words[0];
  hdr[sw_eHdr.ADCclk_1] = adc_words[1];
  hdr[sw_eHdr.ADCclk_2] = adc_words[2];
  hdr[sw_eHdr.ADCclk_3] = adc_words[3];

  hdr[sw_eHdr.Ch_DTCclk_0] = chdtc_words[0];
  hdr[sw_eHdr.DTCclk_1]    = chdtc_words[1];
  hdr[sw_eHdr.DTCclk_2]    = chdtc_words[2];
  hdr[sw_eHdr.DTCclk_3]    = chdtc_words[3];

  hdr[sw_eHdr.EM_0]       = em_words[0];
  hdr[sw_eHdr.EM_1]       = em_words[1];
  hdr[sw_eHdr.EM_2_DRTDC] = em_words[2];

  hdr[sw_eHdr.anchor_end] = sw_eHdr.anchor;

  return hdr;
}
