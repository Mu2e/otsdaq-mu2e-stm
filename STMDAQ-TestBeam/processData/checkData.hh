////////////////////////////////////////////////////////////////////////////////// 
/// This module checks the data as part of the check thread (header)
///////////////////////////////////////////////////////////////////////////////////

#ifndef CHECKDATA_hh
#define CHECKDATA_hh

#include<iostream>
#include<fstream>
#include<vector>

// Hex reader  
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// Logger
#include "STMDAQ-TestBeam/utils/Logger.hh"

// UDPsocket
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Queue
#include "STMDAQ-TestBeam/utils/queue.hh"

class checkData {

private:

  // The last packet number
 uint32_t last_packet[CHNUM] = {};

  // Booleans to check for 0xDEADBEEF
  bool deadbeef[CHNUM] = {false,false};

public:

  // Standard constructor - shouldn't be used
  checkData();

  // Boolean whether this is the first packet of the data run
  bool firstPacket = true;

  // A dropped packet counter
  uint64_t dropped_packet_count[CHNUM] = {};
  
  // Set the stored last packet number
  void setLastPacketNum(int chan, uint32_t lastPacket){
    last_packet[chan] = lastPacket;
  }
  
  // Get the stored last packet number
  uint32_t getLastPacketNum(int chan){
    return last_packet[chan];
  }
  
  // Process packet data
  //  void check_packets(int chan, int packet_num, uint64_t data_len, int16_t *data);
  void check_packets(int chan, uint64_t data_len, int16_t *data);

  // Check for dropped packets 
  void check_dropped_packets(int chan, uint32_t packet_num, uint32_t last_packet);

  // Check the packet header end
  void check_pHdr(uint32_t packet_num, int16_t pHdr_end);

  // Check for correct header format
  void check_tHdrs(int chan, int16_t *data, uint64_t packet_start);

  // Check firmware trigger/event header 
  uint64_t check_fw_tHdr(int chan, int16_t *data, uint64_t hdr_index);


};

#endif
