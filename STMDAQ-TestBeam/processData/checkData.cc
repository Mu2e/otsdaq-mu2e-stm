///////////////////////////////////////////////////////////////////////////////////
/// This module checks the data as part of the check thread (main).  
///////////////////////////////////////////////////////////////////////////////////
/********************************************************************/

#include <iostream>
#include <fstream>
#include <vector>

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// Process data header
#include "STMDAQ-TestBeam/processData/checkData.hh"

// Instance of data variables class
dataVars dv;

// Instance of UDPsocket class
UDPsocket udp;

// Standard constructor
checkData::checkData() {}

// Check packet number and for dropped packets
//void checkData::check_packets(int chan, int n_packets, uint64_t data_len, int16_t *data){
void checkData::check_packets(int chan, uint64_t data_len, int16_t *data){

  // First check packet sizes are correct
  if (data_len % MAX_PACKET_LEN != 0){
    Logger::Instance()->write(0,
                                "checkData::check_packets : Packet sizes incorrect! Channel = "
			      + udp.get_channel_name(chan)
			      + ". Data size = "
			      + std::to_string(data_len)
			      + " % MAX_PACKET_LEN = "
			      + std::to_string(data_len % MAX_PACKET_LEN));
  }

  // Get the last packet number to be read
  uint32_t last_packet = getLastPacketNum(chan);

  // If this is the first packet of the data run
  if (firstPacket){
    // Set the last packet to be the first packet to be read it
    last_packet = (uint16_t)data[1] << 16 | (uint16_t)data[0];
    // Set first packet boolean to false
    firstPacket = false;
  }

  // Loop through all elements
  uint64_t count = 0;
  while (count < data_len){

    // Get the packet start location
    uint64_t packet_start = count;
    // Reconstruct the packet number
    uint32_t packet_num  = (uint16_t) data[packet_start + 1] << 16 | 
      (uint16_t) data[packet_start];

    // Check the packet header checksum
    check_pHdr(packet_num,data[packet_start + 2]);

    // Check for dropped packets
    check_dropped_packets(chan,packet_num,last_packet);

    // Set the last packet number to the current packet number
    last_packet = packet_num;

    // Check the event/trigger headers in a packet
    //check_tHdrs(chan,data,packet_start);    

    // Increase count by packet length
    count += MAX_PACKET_LEN;

  }

  // Store the last packet number
  setLastPacketNum(chan,last_packet);

}

// Check for dropped packets
void checkData::check_dropped_packets(int chan, uint32_t packet_num, uint32_t last_packet){

    // Find the difference between this packet number and the last
    int diff = packet_num - last_packet;
    // Check if the difference between packets is negative
    if (diff < 0){
      Logger::Instance()->write(0,
				"Packet number mismatch! Channel = "
				+ udp.get_channel_name(chan)
				+ ", previous packet number = "
				+ std::to_string(last_packet)
				+ ", previous packet number = "
				+ std::to_string(packet_num));
      std::cout 
	<< "Packet number mismatch! Channel = "
	<< udp.get_channel_name(chan)
	<< ", previous packet number = "
	<< std::to_string(last_packet)
	<< ", previous packet number = "
	<< std::to_string(packet_num) << "\n";
    }

    // If the difference is larger than 1...
    if (diff > 1){
      // Increase the stored number of dropped packets
      dropped_packet_count[chan] += diff-1;
      // Loop over the number of dropped packets
      for (int j = 1; j < diff; j++){
	// Log the dropped packet
    	Logger::Instance()->write(2,
    				  "Dropped UDP packet warning: Channel = " 
    				  + udp.get_channel_name(chan) 
    				  + ", missing packet number = "
    				  + std::to_string(last_packet+j));
	std::cout 
	  << "Dropped UDP packet warning: Channel = " 
	  << udp.get_channel_name(chan) 
	  << ", missing packet number = "
	  << std::to_string(last_packet+j) << "\n";
      }
    }
            
}

// Check the packet header pHdrEnd                                     
void checkData::check_pHdr(uint32_t packet_num, int16_t pHdr_end){     

  // If the packet header check sum is incorrect                       
  if ((uint16_t)pHdr_end != fw_pHdr_end_data) {                         
    // Log the dropped packet                                          
    std::cout                                       
      << "ERROR! Packet number "                     
      << std::to_string(packet_num)                 
      << " has pHdrEnd "                           
      << std::to_string((uint16_t)pHdr_end)         
      << " instead of 65518 (0xFFEE)" << "\n";            
    
    Logger::Instance()->write(0,                                       
			      "ERROR! Packet number "                     
			      + std::to_string(packet_num)                 
			      + " has pHdrEnd "                           
			      + std::to_string((uint16_t)pHdr_end)         
			      + " instead of 65518 (0xFFEE)");            
  }                                                                    

}  

// Check headers
void checkData::check_tHdrs(int chan, int16_t* data, uint64_t packet_start){

  // Get the packet number
  uint32_t pnum = (uint16_t)data[packet_start+1] << 16 
    | (uint16_t)data[packet_start];
  
  // Calculate how much left in packet after packet header
  uint16_t leftInPacket = MAX_PACKET_LEN - fw_pHdr_Len;

  // While leftinPacket is more than a trigger header length
  while(leftInPacket > fw_tHdr_Len){

    // Get header start index location
    uint64_t hdr_start_loc = packet_start + MAX_PACKET_LEN - leftInPacket;
    // Get header end index location
    //uint64_t hdr_end_loc = hdr_start_loc + fw_tHdr_Len-1;

    // Get event length index location
    uint64_t eLen_loc = hdr_start_loc + fw_tHdr::EvLen;
    // Get event length
    uint16_t eLen = data[eLen_loc];

    // Check the trigger header and return event number
    uint64_t evNum = check_fw_tHdr(chan,data,hdr_start_loc);
    
    // Recalculate left in packet
    leftInPacket -= fw_tHdr_Len;      
  
    // If packet is filled with deadbeefs, break
    if (deadbeef[chan]) break;

    // Check end of trigger header is as expected, else throw critical error
    if (data[hdr_start_loc] != fw_tHdr::HdrStart_data[fw_tHdr::HdrStart_len-1]){
      Logger::Instance()->write(0,
				"ERROR! Event/trigger header mismatch!!\nPacket number = "+std::to_string(pnum)+". Event number = "+std::to_string(evNum));
    }

    // Recalculate left in packet
    leftInPacket -= eLen;      
    
  }

  // Check remainder of packet is filled with 0xDEADBEEF
  leftInPacket = dv.check_dead_beef(data,packet_start,leftInPacket);

}

// Check firmware trigger/event header                                    
uint64_t checkData::check_fw_tHdr(int chan, int16_t *data, uint64_t hdr_index){
  
  // Get the channel number
  uint16_t channel = data[hdr_index+tHdr_vars.Ch_DTCclk_0] & 0xFF;  

  // Check that the channel number is correct
  if (channel != chan){
    // Check to see if it is the last packet filled only with 0xDEADBEEFs
    for (uint i = 0; i < fw_tHdr_Len/2; i++){
      // If end of packet isn't filled with 0xDEADBEEF, throw critical error
      if ((uint16_t)(data[hdr_index+i*2] & 0xFFFF) == BEEF
          and (uint16_t)(data[hdr_index+i*2+1] & 0xFFFF) == DEAD){
        // is 0xDEADBEEF
	deadbeef[chan] = true;
      }
      else{
        // is not 0xDEADBEEF
	deadbeef[chan] = false;
      }
    }
    // If filled with deadbeef, then return
    if (deadbeef[chan]) return 0;
    Logger::Instance()->write(0,
                              "ERROR! Channel number "
                              + std::to_string(channel)
                              + " in event/trigger header does not match expected channel number "
                              + std::to_string(chan));
  }

  // // Get the DTC clock (200 MHz)
  // int16_t DTCclock_0 = (data[hdr_index+tHdr_vars.Ch_DTCclk_0] >> 8);
  // // uint64_t DTCclock = dv.make_uint64_t(DTCclock_0.
  // //                                data[hdr_index+tHdr_vars.DTCclk_1],
  // //                                data[hdr_index+tHdr_vars.DTCclk_2],
  // //                                data[hdr_index+tHdr_vars.DTCclk_3]);
  // uint64_t DTCclock = (uint64_t)data[hdr_index+tHdr_vars.DTCclk_3] << 40 |
  //   (uint64_t)data[hdr_index+tHdr_vars.DTCclk_2] << 24 |
  //   (uint64_t)data[hdr_index+tHdr_vars.DTCclk_1] << 8 |
  //   (uint64_t)DTCclock_0;

  // // Get the DTC clock (75 MHz)
  // uint64_t ADCclock = dv.make_uint64_t(data[hdr_index+tHdr_vars.ADCclk_0],
  //                                   data[hdr_index+tHdr_vars.ADCclk_1],
  //                                   data[hdr_index+tHdr_vars.ADCclk_2],
  //                                   data[hdr_index+tHdr_vars.ADCclk_3]);

  // Get the trigger/event number
  uint64_t EvNum = dv.make_uint64_t(data[hdr_index+tHdr_vars.EvNum_0],
                                 data[hdr_index+tHdr_vars.EvNum_1],
                                 data[hdr_index+tHdr_vars.EvNum_2],
                                 0);

  // Get the event window tag
  uint64_t EWT = dv.make_uint64_t(data[hdr_index+tHdr_vars.EWT_0],
                               data[hdr_index+tHdr_vars.EWT_1],
                               data[hdr_index+tHdr_vars.EWT_2],
                               0);

  // Check the EvNum and EWT are identical
  if (EvNum != EWT){
    Logger::Instance()->write(0,
                              "ERROR! Event number "
                              + std::to_string(EvNum)
                              + " does not match Event Window Tag "
                              + std::to_string(EWT));
  }

  // // Get the event mode
  // int16_t EM_2 = data[hdr_index+tHdr_vars.EM_2_DRTDC] & 0xFF;
  // uint64_t EM = dv.make_uint64_t(data[hdr_index+tHdr_vars.EM_0],
  //                             data[hdr_index+tHdr_vars.EM_1],
  //                             EM_2,
  //                             0);

  // // Get the Delivery Ring Marker TDC
  // uint16_t DRM = data[hdr_index+tHdr_vars.EM_2_DRTDC] >> 8 & 0xFF;

  // // Get the event start offset
  // uint16_t ESO = data[hdr_index+tHdr_vars.EvStart];

  // // Get the event length (to read)
  // uint16_t EvLen = data[hdr_index+tHdr_vars.EvLen];

  // Check if trigger header end is correct
  for (uint i = 0; i < tHdr_vars.HdrStart_len; i++){
    if(data[hdr_index+tHdr_vars.HdrStart_start+i] != tHdr_vars.HdrStart_data[i]){
      Logger::Instance()->write(0,
                                "ERROR! Event number "
                                + std::to_string(EvNum)
                                + " has event header end value "
                                + std::to_string(data[hdr_index+tHdr_vars.HdrStart_start+i])
                                + " instead of "
                                + std::to_string(tHdr_vars.HdrStart_data[i])
                                + " for header index "
                                + std::to_string(tHdr_vars.HdrStart_start+i));
    }
  }

  // cout << "Channel = " << channel <<  endl;
  // cout << "DTC clock = " << DTCclock << endl;
  // cout << "ADC clock = " << ADCclock << endl;
  // cout << "Event number = " << EvNum << endl;
  // cout << "Event window tag = " << EWT << endl;
  // cout << "Event mode = " << EM << endl;
  // cout << "Delivery ring marker = " << DRM << endl;
  // cout << "Event start offset = " << ESO << endl;
  // cout << "Event length = " << EvLen << endl;

  return EvNum;

}
