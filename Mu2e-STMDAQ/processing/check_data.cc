// Hex reader
#include "Mu2e-STMDAQ/utils/Hex.hh"

// Process data header
#include "Mu2e-STMDAQ/processing/check_data.hh"

// Constructor
CheckData::CheckData(const std::shared_ptr<AsyncLogger>& logger_,
                     const std::shared_ptr<STMdata>& stm_) :
  logger(logger_), stm(stm_) {

  // Register operations for OperationManager
  register_operation("check_packets", [this](auto& b){ check_packets(b); });
  
}

// Check packet number and for dropped packets
void CheckData::check_packets(std::shared_ptr<DataStruct>& buffer){

  // Ensure new buffer boolean is true
  new_buffer = true;
  
  // The data length                                                                           
  size_t n = buffer->zs_len;
  
  // Define a pointer to the raw data buffer                                                  
  int16_t* data_ptr = buffer->zs.data();
  
  // First check packet sizes are correct
  if (n % MAX_PACKET_LEN != 0){
    logger->log("CheckData::check_packets : Packet sizes incorrect!  Data size = " + std::to_string(n)
                + " % MAX_PACKET_LEN = "
                + std::to_string(n % MAX_PACKET_LEN),0);
    return;
  }


  // If this is the first packet of the data run or a null_hb has been detected
  if (firstPacket or is_null_hb){
    // Set the last packet to be the first packet to be read in
    last_packet = (uint16_t)data_ptr[1] << 16 | (uint16_t)data_ptr[0];
    // Set the last EWT to be the first event to be read in
    last_EWT = stm->make_uint64_t(data_ptr[pHdr_Len+fw_eHdr.EWT_0],
				  data_ptr[pHdr_Len+fw_eHdr.EWT_1],
				  data_ptr[pHdr_Len+fw_eHdr.EWT_2],
				  0);
    // Start the EWT counter
    checked_EWT_count += 1;
    // Set first packet boolean to false
    firstPacket = false;
    // Set deadbeef to false
    deadbeef = false;        
    // If a null hb has been detected
    if (is_null_hb){
      // Warn user
      logger->log("CheckData: storing first packet number after null_hb ("
		  + std::to_string(last_packet)
		  + ") as last detected packet number.",2);
      logger->log("CheckData: storing first EWT after null_hb ("
		  + std::to_string(last_EWT)
		  + ") as last detected EWT.",2);
    }
    // Signal no longer null hb
    is_null_hb = false;
    ewt_offset_known = false;   // re-detect after reset/null HB
    ewt_is_evnum_minus_one = false;
  }

  // Counter for all elements
  uint64_t count = 0;  
  
  // Loop through all elements
  while (count < n){

    // Get the packet start location
    uint64_t packet_start = count;
    // Reconstruct the packet number
    uint32_t packet_num  = (uint16_t) data_ptr[packet_start + 1] << 16 | 
      (uint16_t) data_ptr[packet_start];
    
    // Check the packet header checksum
    check_pHdr(packet_num,data_ptr[packet_start + 2]);

    // Check the event headers in a packet
    check_eHdrs(buffer,packet_start);    

    // Increase count by packet length
    count += MAX_PACKET_LEN;

    // Return if a stop signal has been triggered
    if (stop::should_critical_stop()) return;
    
  }

}

// Check headers
void CheckData::check_eHdrs(std::shared_ptr<DataStruct>& buffer, uint64_t packet_start){

  // The data length                                                                           
  size_t n = buffer->zs_len;
  
  // Define a pointer to the raw data buffer                                                  
  int16_t* data_ptr = buffer->zs.data();
  
  // Get the packet number
  uint32_t pnum = (uint16_t)data_ptr[packet_start+1] << 16 
    | (uint16_t)data_ptr[packet_start];
  
  // Check for dropped packets
  bool dropped_packets = check_dropped_packets(buffer,pnum,last_packet);

  // Return if a stop signal has been triggered
  if (stop::should_critical_stop()) return;
  
  // Set the last packet number to the current packet number
  last_packet = pnum;  
  
  // Increment the checked packet counters
  checked_packet_count++;
  buffer->packet_count++;
  
  // Calculate how much left in packet after packet header
  uint16_t leftInPacket = MAX_PACKET_LEN - pHdr_Len;

  // While leftinPacket is more than a event header length
  while(leftInPacket > fw_eHdr_len){
    
    // Get header start index location
    uint64_t hdr_start_loc = packet_start + MAX_PACKET_LEN - leftInPacket;
    // Get header end index location
    uint64_t hdr_end_loc = hdr_start_loc + fw_eHdr_len-1;

    // Get event length index location
    uint64_t eLen_loc = hdr_start_loc + fw_eHdr.EvInPacket;
    // Get event length
    uint16_t eLen = data_ptr[eLen_loc];

    // Check the event header and return event number
    uint64_t EWT = check_eHdr(buffer,hdr_start_loc);

    // Return if a stop signal has been triggered
    if (stop::should_critical_stop()) return;
    
    // Recalculate left in packet
    leftInPacket -= fw_eHdr_len;      
  
    // If packet is filled with deadbeefs, break
    if (deadbeef) break;

    // Check end of event header is as expected, else throw critical error
    if (data_ptr[hdr_start_loc] != fw_eHdr.anchor_data[fw_eHdr.anchor_len-1]){
      logger->log("CheckData: ERROR! Event header mismatch!!\nPacket number = " +
		  std::to_string(pnum) +
		  ". EWT = " +
		  std::to_string(EWT),0);
      return;
    }

    // Recalculate left in packet
    leftInPacket -= eLen;      
    
  }

  // Check remainder of packet is filled with 0xDEADBEEF
  leftInPacket = stm->check_dead_beef(data_ptr,packet_start,leftInPacket);

}

// Check the packet header 
void CheckData::check_pHdr(uint32_t packet_num, int16_t pHdr_end){     

  // If the packet header check sum is incorrect                       
  if ((uint16_t)pHdr_end != pHdr_check) {
    // Log the dropped packet                                          
    logger->log("CheckData: ERROR! Packet number "                     
		+ std::to_string(packet_num)                 
		+ " has pHdrEnd "                           
		+ std::to_string((uint16_t)pHdr_end)         
		+ " instead of 65518 (0xFFEE).\n Last packet num = "
		+ std::to_string(last_packet),1);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }                                                                    
  
}  

// Check for dropped packets
bool CheckData::check_dropped_packets(std::shared_ptr<DataStruct>& buffer,
				      uint32_t packet_num, uint32_t last_packet){

  
  // Find the difference between this packet number and the last
  int diff = packet_num - last_packet;    
  
  // Check if the difference between packets is negative
  if (diff < 0){
    logger->log("CheckData: Packet number mismatch! Previous packet = "
		+ std::to_string(last_packet)
		+ ", new packet = "
		+ std::to_string(packet_num),0);
    return 0;
  }
  
  // Are there dropped packets?
  bool dropped_packets = false;
  
  // If the difference is larger than 1...
  if (diff > 1){
    // Increase the stored number of dropped packets
    dropped_packet_count += diff-1;
    buffer->dropped_packet_count += diff-1;
    // Log the dropped packet
    logger->log("CheckData: Dropped " +
		std::to_string(diff-1) +
		" UDP packets. Missing packet numbers = "  +
		std::to_string(last_packet+1) +
		" --> "  +
		std::to_string(packet_num-1),2);
    // Store the range of dropped packets
    buffer->dropped_packets.push_back({last_packet+1,packet_num-1});
    // Signal dropped packets
    dropped_packets = true;
  }
  
  // Return whether there are dropped packets
  return dropped_packets;
  
}


// Check event header                                    
uint64_t CheckData::check_eHdr(std::shared_ptr<DataStruct>& buffer, uint64_t hdr_index){

  // The data length                                                                           
  size_t n = buffer->zs_len;
  
  // Define a pointer to the raw data buffer                                                  
  int16_t* data_ptr = buffer->zs.data();
  
  // Get the channel number
  uint16_t channel = data_ptr[hdr_index+fw_eHdr.Ch_DTCclk_0] & 0xFF;  
  // Check that the channel number is correct
  if (channel != stm->master_config.ch_num){
    // Check to see if it is the last packet filled only with 0xDEADBEEFs
    for (size_t i = 0; i < fw_eHdr_len/2; i++){
      // If end of packet isn't filled with 0xDEADBEEF, throw critical error
      if ((uint16_t)(data_ptr[hdr_index+i*2] & 0xFFFF) == BEEF
          and (uint16_t)(data_ptr[hdr_index+i*2+1] & 0xFFFF) == DEAD){
        // is 0xDEADBEEF
	deadbeef = true;
      }
      else{
        // is not 0xDEADBEEF
	deadbeef = false;
      }
    }
    // If filled with deadbeef, then return
    if (deadbeef) return 0;
    logger->log("CheckData: ERROR! Event header channel = " +
		std::to_string(channel) +
		" does not match expected channel number",0);
    return 0;
  }

  
  // Get the event number
  uint64_t EvNum = stm->get_event_number(data_ptr,hdr_index);
  
  // Get the event window tag
  uint64_t EWT = stm->get_EWT(data_ptr,hdr_index);

  // Check event mode
  uint64_t EM = stm->get_event_mode(data_ptr,hdr_index);
  
  // Check for null heartbeat (event mode = 0)
  if (EM == 0){
    is_null_hb = true;
    buffer->has_null_hb = true;
    logger->log("CheckData: Null heartbeat detected in channel " +
		std::to_string(channel) +
		".",2);    
  }
  
  // Check the EvNum and EWT are as expected
  //if ((EvNum != EWT && EvNum != EWT+1) && !is_null_hb){
  if (!is_null_hb){
    if (!ewt_offset_known) {
      // Determine the offset
      if (EWT == EvNum) {
          ewt_is_evnum_minus_one = false;
          ewt_offset_known = true;
          logger->log("CheckData: EWT offset mode = EWT == EventNum (offset 0)", 1);
      } 
      else if (EWT == EvNum - 1) {
        ewt_is_evnum_minus_one = true;
        ewt_offset_known = true;
        logger->log("CheckData: EWT offset mode = EWT == EventNum - 1 (offset -1)", 1);
      } 
      else {
        logger->log("CheckData: ERROR! First event EWT = " +
                     std::to_string(EWT) + " EventNum = " +
                     std::to_string(EvNum) +
                     " — unexpected offset, cannot determine mode.", 0);
	return 0;
      }
    } 
    else {
      uint64_t expected_EWT = ewt_is_evnum_minus_one
			? static_cast<uint64_t>(EvNum) - 1
			: static_cast<uint64_t>(EvNum);
      if (EWT != expected_EWT) {
        logger->log("CheckData: ERROR! Event number "
	  	  + std::to_string(EvNum) + " does not match EWT "
	  	  + std::to_string(EWT) + ". Expected EWT " 
		  + std::to_string(expected_EWT), 0);
	return 0;
      }
    }
  }

  // Find the difference between this EWT and the last
  int diff = EWT - last_EWT;

  // Check if the difference between packets is negative
  if (diff < 0 && !is_null_hb){
    logger->log("CheckData: ERROR - EWT mismatch! Previous EWT = "
		+ std::to_string(last_EWT)
		+ ", new EWT = "
		+ std::to_string(EWT),0);
    return 0;
  }
  
  // If the difference is larger than 1...
  if (diff > 1 && !is_null_hb){
    // Increase the stored number of lost EWTs
    lost_EWT_count += diff-1;
    buffer->lost_EWT_count += diff-1;
    // Log the lost EWTs
    logger->log("CheckData: Lost " +
		std::to_string(diff-1) +
		" EWTs. Missing EWTs = "  +
		std::to_string(last_EWT+1) +
		" --> "  +
		std::to_string(EWT-1),2);
    // Store the range of lost EWTs
    buffer->lost_EWTs.push_back({last_EWT+1,EWT-1});
  }

  // Increment the checked EWT counters
  if (EWT != last_EWT && !is_null_hb){
    ++checked_EWT_count;
  }
  
  // Ensure new buffer is now false
  new_buffer = false;
  
  // Set the last EWT to the current EWT
  last_EWT = EWT; 

  // Check if event header end is correct
  for (size_t i = 0; i < fw_eHdr.anchor_len; i++){
    if(data_ptr[hdr_index+fw_eHdr.anchor_start+i] != fw_eHdr.anchor_data[i]){
      logger->log("CheckData: ERROR! EWT "
		  + std::to_string(EvNum)
		  + " has event header end value "
		  + std::to_string(data_ptr[hdr_index+fw_eHdr.anchor_start+i])
		  + " instead of "
		  + std::to_string(fw_eHdr.anchor_data[i])
		  + " for header index "
		  + std::to_string(fw_eHdr.anchor_start+i),0);
      return 0;
    }
  }

  return EWT;

}


