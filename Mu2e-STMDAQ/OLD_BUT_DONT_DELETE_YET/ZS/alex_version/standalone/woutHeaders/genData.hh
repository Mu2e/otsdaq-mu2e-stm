//////////////////////////////////////////////////////////////////
/// This module generates data to test the STMDAQ code
//////////////////////////////////////////////////////////////////

/********************************************************************/

#ifndef GENDATA_hh
#define GENDATA_hh

#include<iostream>
#include<fstream>
#include <cstring>

// Data variables header
#include "dataVars.hh"

class genData {		   

public:
  
  // Standard constructor
  genData(const uint32_t number, const uint32_t delta);

  // Constructor - with input file
  genData(std::string file);

  // Constructor - with input data
  genData(int16_t* data, uint64_t data_len);
  
  // static const uint16_t fw_tHdr_Size = 64;//6;
  // static const uint16_t fw_tHdr_Len = fw_tHdr_Size/2; 
  
  // // static const uint fw_tHdr_eNum = 0;
  // static const uint fw_tHdr_eLen = 18;
  // // static const uint fw_tHdr_endloc = 2;
  // // static const int16_t fw_tHdr_end = 0xFFEE;
  
  // // Number of heartbeats
  uint32_t numberhb = 0;
  // Heartbeat length (multiple of 8ns)
  uint32_t deltahb = 0;
  // Event length (ns)
  uint32_t deltahb_ns = 0;
  // Event length (s)
  double deltahb_s = 0;
  // 300 Msamples per second
  static const uint64_t sampsPerSec = 3e8;
  // Event length (counts)
  uint64_t deltahb_len = 0;
  // Total event size including header (bytes)
  uint64_t event_size_tot = 0;
  // Total event length including header
  uint64_t event_len_tot = 0;

  // Get data from binary file
  void readBinaryFile();

  // Form an event header
  int16_t *form_tHdr(uint16_t chan, uint64_t DTCclock, uint64_t ADCclock,
		     uint64_t eventNum, uint64_t EWT, uint64_t EM, 
		     uint16_t DRTDC, uint16_t eventStart, 
		     uint16_t eventLength);
  
  // Generate events
  std::pair<uint64_t,int16_t*> genEvents(int c, int eventNum);

  // Generate packets
  std::pair<uint64_t,int16_t*> genPackets(int c, int16_t *events, int eventNum);

  // Update event headers with new event number
  void update_tHdr(int16_t* data, uint64_t hdr_start, uint16_t chan, uint64_t event_num);

};

#endif
