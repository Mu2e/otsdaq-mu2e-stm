// STM data header
#include "Mu2e-STMDAQ/utils/stm_data.hh"

// Constructor
STMdata::STMdata(const std::shared_ptr<AsyncLogger>& logger_) : logger(logger_) {}

// Form uin32_t out of 2 x int16_t 
uint32_t STMdata::make_uint32_t(int16_t p0, int16_t p1){
 
  uint64_t value = static_cast<uint16_t>(p1);
  value <<= 16;
  value |= static_cast<uint16_t>(p0);

  return value;
    
}  
// Form uin64_t out of 4 x int16_t 
uint64_t STMdata::make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3){
 
  uint64_t value = static_cast<uint16_t>(p3);
  value <<= 16;
  value |= static_cast<uint16_t>(p2);
  value <<= 16;
  value |= static_cast<uint16_t>(p1);
  value <<= 16;
  value |= static_cast<uint16_t>(p0);

  return value;
    
}

// Split uint32_t into 2 x int16_t 
int16_t* STMdata::split_uint32_t(uint32_t value){
 
  int16_t* split_value = new int16_t[2];
  split_value[0] = value & 0xFFFF;
  split_value[1] = (value >> 16) & 0xFFFF;

  return split_value;
    
}

// Split uint64_t into 4 x int16_t 
int16_t* STMdata::split_uint64_t(uint64_t value){
 
  int16_t* split_value = new int16_t[4];
  split_value[0] = value & 0xFFFF;
  split_value[1] = (value >> 16) & 0xFFFF;
  split_value[2] = (value >> 32) & 0xFFFF;
  split_value[3] = (value >> 48) & 0xFFFF;

  return split_value;
    
}

// Get just the event number from the event header
uint64_t STMdata::get_event_number(std::vector<int16_t>& data,
				   uint64_t hdr_start_loc){

  // Get the trigger/event number
  uint64_t event_number = make_uint64_t(data[hdr_start_loc+eHdr.EvNum_0],
					data[hdr_start_loc+eHdr.EvNum_1],
					data[hdr_start_loc+eHdr.EvNum_2],
					0);
  return event_number;

}

// Get just the EWT from the event header
uint64_t STMdata::get_EWT(std::vector<int16_t>& data,
			  uint64_t hdr_start_loc){

  // Get the trigger/event number
  uint64_t EWT = make_uint64_t(data[hdr_start_loc+eHdr.EWT_0],
			       data[hdr_start_loc+eHdr.EWT_1],
			       data[hdr_start_loc+eHdr.EWT_2],
			       0);
  return EWT;

}


// Check the end of a packet for repeated 0xDEADBEEF
uint16_t STMdata::check_dead_beef(std::vector<int16_t>& data,
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
uint64_t STMdata::get_event_mode(std::vector<int16_t>& data,
				 uint64_t hdr_index){
  
  // Get event mode data
  uint16_t EM0 = static_cast<uint16_t>(data[hdr_index + eHdr.EM_0]);    
  uint16_t EM1 = static_cast<uint16_t>(data[hdr_index + eHdr.EM_1]); 
  uint16_t EM2 = static_cast<uint16_t>(data[hdr_index + eHdr.EM_2_DRTDC]); 

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
