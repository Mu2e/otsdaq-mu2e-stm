// Data variables header
#include "dataVars.hh"

// Standard constructor - shouldn't be used
dataVars::dataVars() {}

const int16_t fw_tHdr::HdrStart_data[fw_tHdr::HdrStart_len] = {(int16_t)0xCAFE, // 0 = 0xCAFE
							   (int16_t)0xBBBB, // 1 = 0xBBBB
							   (int16_t)0xAAAA, // 2 = 0xAAAA
							   (int16_t)0xCAFE}; // 3 = 0xCAFE
 
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

// Split uint32_t into 2 x int16_t 
int16_t* dataVars::split_uint32_t(uint32_t value){
 
  int16_t* split_value = new int16_t[2];
  split_value[0] = value & 0xFFFF;
  split_value[1] = (value >> 16) & 0xFFFF;

  return split_value;
    
}

// Split uint64_t into 4 x int16_t 
int16_t* dataVars::split_uint64_t(uint64_t value){
 
  int16_t* split_value = new int16_t[4];
  split_value[0] = value & 0xFFFF;
  split_value[1] = (value >> 16) & 0xFFFF;
  split_value[2] = (value >> 32) & 0xFFFF;
  split_value[3] = (value >> 48) & 0xFFFF;

  return split_value;
    
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

// Get just the EWT from the event header
uint64_t dataVars::get_EWT(int16_t *data, uint64_t hdr_start_loc){

  // Get the trigger/event number
  uint64_t EWT = make_uint64_t(data[hdr_start_loc+tHdr_vars.EWT_0],
			       data[hdr_start_loc+tHdr_vars.EWT_1],
			       data[hdr_start_loc+tHdr_vars.EWT_2],
			       0);
  
  return EWT;

}


// Check the end of a packet for repeated 0xDEADBEEF
uint16_t dataVars::check_dead_beef(int16_t* data, uint64_t packet_start,
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
      std::cout << "ERROR! End of packet "
	+ std::to_string(pnum)
	+ " not filled with 0xDEADBEEF!!" << std::endl;
  }
    // Subtract from leftInPacket
    leftInPacket -= 2;
  }

  return leftInPacket;

}
