/********************************************************************/

#include<iostream>
#include<fstream>
#include <vector>
#include <string>

// Gen data
#include "genData.hh"

// Boolean to indicate sources packet data from file
bool dataFromFile = false;
// Binary file containing the packet data
static std::string binary_file = "/data1/run00109.new.bin_00";
// Binary file size (bytes)
uint64_t binary_size = 0;
// Binary file length (int16_t values)
uint64_t binary_len = 0;
// The binary file data packets
int16_t* binary_data; 
// The first packet number
uint32_t start_packet_num = 0;
// The first event number
uint32_t start_event_num = 0;

// Firmware trigger header struct from dataVars.hh
fw_tHdr tHdr;

// Constructor
genData::genData(const uint32_t number, const uint32_t delta){
  
  // Number of heartbeats
  numberhb = number;
  // Heartbeat length (multiple of 8ns) 
  deltahb = delta; 
  
  // Event length (ns)
  deltahb_ns = deltahb*8;
  // Event length (s)
  deltahb_s = deltahb_ns*1e-9;
  // Event length (counts)
  deltahb_len = deltahb_s*sampsPerSec;
  
  // Total event size including header (bytes)
  event_size_tot = fw_tHdr_Size + deltahb_len*sizeof(int16_t);
  // Total event length including header
  event_len_tot = event_size_tot/sizeof(int16_t);
    
}

// Constructor - from file
genData::genData(std::string file){		 
  
  // Tell the code we're reading data from file
  dataFromFile = true;

  // If getting data from input binary file
  readBinaryFile();

  // Heartbeat length (multiple of 8ns)
  deltahb = 0x30D4; // 100 us

  // Event length (ns)
  deltahb_ns = deltahb*8;
  // Event length (s)
  deltahb_s = deltahb_ns*1e-9;
  // Event length (counts)
  deltahb_len = deltahb_s*sampsPerSec;
  
  // Total event size including header (bytes)
  event_size_tot = fw_tHdr_Size + deltahb_len*sizeof(int16_t);
  // Total event length including header
  event_len_tot = event_size_tot/sizeof(int16_t);

  // Number of heartbeats
  double number = (double)binary_len/(double)deltahb_len;
  numberhb = number;

  std::cout << numberhb << " " << deltahb*8*1e-3 << " us events in file" << std::endl;
  
}

// Constructor - from file
genData::genData(int16_t* data, uint64_t data_len){

  // Tell the code we're reading data from file
  dataFromFile = true;

  binary_data = data;
  binary_len = data_len;

  // Heartbeat length (multiple of 8ns)
  deltahb = 0x30D4; // 100 us
  
  // Event length (ns)
  deltahb_ns = deltahb*8;
  // Event length (s)
  deltahb_s = deltahb_ns*1e-9;
  // Event length (counts)
  deltahb_len = deltahb_s*sampsPerSec;

  // Total event size including header (bytes
  event_size_tot = fw_tHdr_Size + deltahb_len*sizeof(int16_t);
  // Total event length including header
  event_len_tot = event_size_tot/sizeof(int16_t);

  // Number of heartbeats
  double number = (double)data_len/(double)deltahb_len;
  numberhb = number;

  std::cout << numberhb << " " << deltahb*8*1e-3 << " us events in file" << std::endl;

}

// Function to read input binary file                                      
void genData::readBinaryFile(){

  std::cout << "Importing binary file data from " << binary_file << std::endl; 

  // Open file                                                             
  std::ifstream ipfile;
  ipfile.open(binary_file, std::ios::in | std::ios::binary);
  ipfile.seekg(0, std::ios::end);
  binary_size = ipfile.tellg();
  ipfile.seekg(0, std::ios::beg);

  // Define array    
  binary_len = binary_size/2;

  // Read file into array
  binary_data = new int16_t[binary_len];      
  ipfile.read( (char*) binary_data, binary_len*sizeof(int16_t));

  // Print file info                                                       
  std::cout << "Input data file size = " 
       << binary_size*1e-9 << " Gbytes = " 
       << binary_len << " elements " << std::endl;


}

// Form an event/trigger headder
int16_t* genData::form_tHdr(uint16_t chan, uint64_t ADCclock, uint64_t eventNum, 
			    uint64_t EWT, uint64_t EM, 
			    uint16_t DRTDC, uint16_t eventStart, 
			    uint16_t eventLeninPacket, uint16_t subRunNum,
			    uint16_t ZSflag, uint16_t PreVal, 
			    uint16_t eventLength, uint64_t runNum, uint64_t DTCclock){
  
  // Initialise trigger header
  int16_t* tH = new int16_t[fw_tHdr_Len]();

  // Add header start
  memcpy(&tH[tHdr.HdrStart_start],&(tHdr.HdrStart_data[0]),tHdr.HdrStart_size);

  // ADC Clock 0-3
  tH[tHdr.ADCclk_0] = ADCclock & 0xFFFF;
  tH[tHdr.ADCclk_1] = ADCclock >> 16;
  tH[tHdr.ADCclk_2] = ADCclock >> 32;
  tH[tHdr.ADCclk_3] = ADCclock >> 48; 
  // Event Number 0-2
  tH[tHdr.EvNum_0] = eventNum & 0xFFFF;
  tH[tHdr.EvNum_1] = eventNum >> 16;
  tH[tHdr.EvNum_2] = eventNum >> 32;
  // Event Window Tag 0-2
  tH[tHdr.EWT_0] = EWT & 0xFFFF;
  tH[tHdr.EWT_1] = EWT >> 16;
  tH[tHdr.EWT_2] = EWT >> 32;
  // Event Mode 0-1
  tH[tHdr.EM_0] = EM & 0xFFFF;
  tH[tHdr.EM_1] = EM >> 16;
  // Event Mode 2 + Delivery Ring TDC  
  tH[tHdr.EM_2_DRTDC] = (DRTDC & 0xFF) << 8 | (EM >> 32) & 0xFF;
  // Event Start Offset
  tH[tHdr.EvStart] = eventStart;
  // Event Length (To Read)
  tH[tHdr.EvInPacket] = eventLeninPacket;
  // Beef placeholder
  tH[tHdr.Beef] = 0xBEEF;
  // Subrun number from FW
  tH[tHdr.SubRunNum_0] = subRunNum & 0xFFFF;
  tH[tHdr.SubRunNum_1] = subRunNum >> 16;
  // ZS flag and Prescale value
  tH[tHdr.ZSflag_PreVal] = (ZSflag & 0xFF) << 8 | PreVal & 0xff;
  // Total event length
  tH[tHdr.EvLen] = eventLength;
  // Run number [DCS write]
  tH[tHdr.RunNum_0] = runNum & 0xFFFF;
  tH[tHdr.RunNum_1] = runNum >> 16;
  tH[tHdr.RunNum_2] = runNum >> 32;
  tH[tHdr.RunNum_3] = runNum >> 48;
  // Channel + DTC Clock 0 
  tH[tHdr.Ch_DTCclk_0] = (DTCclock & 0xFF) << 8 | chan & 0xff;
  // DTC Clock 1-3
  tH[tHdr.DTCclk_1] = DTCclock >> 8;
  tH[tHdr.DTCclk_2] = DTCclock >> 24;
  tH[tHdr.DTCclk_3] = DTCclock >> 40;  
  
  // Return trigger header
  return tH;

}

// Generate packet data
std::pair<uint64_t,int16_t*> genData::genEvents(int c, int eventNum){

  // Initialise events
  uint64_t event_array_len = eventNum * event_len_tot;
  int16_t *events = new int16_t [event_array_len];

  // Incrementing counter for ADC values
  int16_t adc = c;

  // Count how many adc values added 
  uint64_t adc_count = 0;
    
  // Loop over total number of events
  for (uint i = 0; i < eventNum; i++){
    // Get the starting index in the array of the event
    uint64_t event_start = i*event_len_tot;
    // %%%%%%%%%%%%%%%%%%%%%%%%% //
    // Event header for event
    // %%%%%%%%%%%%%%%%%%%%%%%%% //
    // Get first event header inputs
    uint64_t eventNum = start_event_num + i;
    uint64_t ADCclock = eventNum*75e6; // 75 MHz
    uint64_t EWT = eventNum;
    uint64_t EM = 0;
    uint64_t DRTDC = 0;
    uint16_t eventStart = event_start;
    uint16_t eventLeninPacket = deltahb_len;
    uint16_t subRunNum = 0;
    uint16_t ZSflag = 0;
    uint16_t PreVal = 0; 
    // Find event length to read
    uint16_t eventLength = deltahb_len;
    uint64_t runNum = 0; 
    uint64_t DTCclock = eventNum*200e6; // 200 MHz
    // Form event header
    int16_t *tH_event = form_tHdr(c,ADCclock,eventNum,EWT,EM,DRTDC,eventStart,eventLeninPacket,subRunNum,ZSflag,PreVal,eventLength,runNum, DTCclock);
    // Copy event header to start of events
    memcpy(&events[event_start],tH_event,fw_tHdr_Size);              
    // Add ADC data
    for (int j = 0; j < eventLength; j++){
      // If using data from input binary file
      if (dataFromFile){
	events[event_start+fw_tHdr_Len+j] = binary_data[adc_count % binary_len];
	adc_count++;
      }
      // Else in generating incrmenting data
      else{
	events[event_start+fw_tHdr_Len+j] = adc++;
      }
    } // End j-loop over adc data

  } // End i-loop over events
  
  // Release binary file memeory once stored as events
  delete[] binary_data;

  return {event_array_len,events};

}

// Generate packet data
std::pair <uint64_t,int16_t*>  genData::genPackets(int c, int16_t *events, int eventNum){
    
  // Initialise packet variables
  std::vector<std::vector<int16_t>> packetVec;
  std::vector<int16_t> current_packet(MAX_PACKET_LEN);
  fill(current_packet.begin(), current_packet.end(), 0);
  
  // Counter of what's left in the packet
  uint16_t leftInPacket = MAX_PACKET_LEN;
  // Packet number counter
  int packetNum = 0;
  // Boolen for a new packet
  bool newPacket = true;

  // Loop over total number of events
  for (uint i = 0; i < eventNum; i++){

    // Get the starting index in the event array of the event
    uint64_t event_start = i*event_len_tot;

    // Initialise event counter
    uint event_counter = 0;

    // Get event header
    int16_t *tH = new int16_t[fw_tHdr_Len]();
  
    // Copy event header to start of events
    memcpy(tH,&events[event_start],fw_tHdr_Size);
     
    // Get the length of the events
    uint eventToAdd = deltahb_len;
    // While there is still some of the event to add
    while(eventToAdd != 0){

      // If remaining packet space is <= a trigger header...
      if (fw_tHdr_Len >= leftInPacket){
    	
        // PACKET COMPLETE  
    	// Fill curent packet with 0xDEADBEEF
    	// Get remaining in packet
    	uint16_t start = MAX_PACKET_LEN - leftInPacket;
    	// Fill remainder of packet with 0xDEADBEEF
    	for (int j = start; j < start+leftInPacket; j++){
    	  // If odd entry
    	  if (j % 2 != 0) current_packet[j] = BEEF;
    	  // If event entry
    	  if (j % 2 == 0) current_packet[j] = DEAD;
    	} // End j-loop	  
    	// Push back the packet std::vector
    	packetVec.push_back(current_packet);	  
    	// Increase the packet number
    	packetNum++;

    	// Set new packet to true
    	newPacket = true;
    	// Set left in packet to max
    	leftInPacket = MAX_PACKET_LEN;
      } // End if fw_tHdr_Len >= leftInPacket
  
      // If we're in a new packet
      if (newPacket){
    	// Set new packet to false
    	newPacket = false;
    	// %%%%%%%%%%%%%%%%%%%%%%%%% //
    	// Packet header 
    	// %%%%%%%%%%%%%%%%%%%%%%%%% //
    	// Get the adjusted starting packet number
    	uint32_t p_num = start_packet_num + packetNum;
    	uint32_t lowerHalf = p_num & 0xFFFF; // Extract lower 16 bits
    	uint32_t upperHalf = (p_num >> 16) & 0xFFFF; // Extract upper 16

    	// Add packet header
    	current_packet[0] = lowerHalf;
    	current_packet[1] = upperHalf;
    	current_packet[2] = fw_pHdr_end_data;	
    	
    	// Account for the packet header length
    	leftInPacket = MAX_PACKET_LEN - fw_pHdr_Len;
      } // End if newPacket
   
      // Find event start offset
      uint16_t eventStart = deltahb_len - eventToAdd;	
      // Change event start offset in header
      tH[fw_tHdr::EvStart] = eventStart;
      // Find event length to read
      uint16_t eventLength = 0;
      if (eventToAdd >= leftInPacket - fw_tHdr_Len){
    	eventLength = leftInPacket - fw_tHdr_Len;
      }
      else{
    	eventLength = eventToAdd;
      } // End if eventToAdd >= leftInPacket - fw_tHdr_Len
      // Change event length in header
      tH[fw_tHdr::EvInPacket] = eventLength;
      // Copy event header to start of packet
      memcpy(&current_packet[MAX_PACKET_LEN - leftInPacket],
    	     tH,fw_tHdr_Size);	

      // Account for the trigger header length
      leftInPacket -= fw_tHdr_Len;
   
      // If the event if larger then the packet remainder
      if (eventToAdd >= leftInPacket){
    	// Get remaining in packet
    	uint16_t start = MAX_PACKET_LEN - leftInPacket;
    	// Fill remainder of packet with incrementing ADC adata
    	memcpy(&current_packet[start],
    	       &events[event_start+fw_tHdr_Len+event_counter],
    	       leftInPacket*sizeof(int16_t));
    	// Increase event counter
    	event_counter += leftInPacket;

    	// Push back the packet std::vector
    	packetVec.push_back(current_packet);
    	
    	// Subtract from the event left to add
    	eventToAdd -= leftInPacket;
    	// PACKET COMPLETE	
    	// Increase the packet number
    	packetNum++;

    	// Set new packet to true
    	newPacket = true;
    	// Set left in packet to max
    	leftInPacket = MAX_PACKET_LEN;	
      } // End if eventToAdd >= leftInPacket
   
      // If the event if less than the packet remainder
      else{	
    	// Get remaining in packet
    	uint16_t start = MAX_PACKET_LEN - leftInPacket;
    	// Fill remainder of packet with incrementing ADC data
    	memcpy(&current_packet[start],
    	       &events[event_start+fw_tHdr_Len+event_counter],
    	       eventToAdd*sizeof(int16_t));
    	// Increase event counter
    	event_counter += eventToAdd;
    	// Account for the event filling the packet
    	leftInPacket -= eventToAdd;
    	// Subtract from the event left to add
    	eventToAdd -= eventToAdd;
      } // End if eventToAdd < leftInPacket
   
    } // End while(eventToAdd != 0)
  
    // If at final event and packet is not full...
    if (i == eventNum-1 && eventToAdd == 0 && leftInPacket != 0){
      // Fill final packet with 0xDEADBEEFS      
      // Get remaining in packet
      uint16_t start = MAX_PACKET_LEN - leftInPacket;      
      // Fill remainder of packet with 0xDEADBEEF
      for (int j = start; j < MAX_PACKET_LEN; j++){	
    	// If odd entry
    	if (j % 2 != 0) current_packet[j] = BEEF;
    	// If event entry
    	if (j % 2 == 0) current_packet[j] = DEAD;
    	// Decrement leftInPacket
    	leftInPacket--;
      }	  
      // Push back the packet std::vector
      packetVec.push_back(current_packet);	  
      // Increase the packet number
      packetNum++;      
    }

    
    // Check event_counter == event length
    if (event_counter != deltahb_len){
      std::cout << "ERROR in genPackets: event_counter != deltahb_len!!!" << std::endl;
      exit(0);
    }
     
  } // End i-loop over events

  // Initialise packets
  int16_t* packets = new int16_t [packetNum*MAX_PACKET_LEN] ();

  // Loop over and copy packets
  for (uint i = 0; i < packetNum; i++){    
    int16_t* temp = &packetVec[i][0];
    memcpy(&packets[i*MAX_PACKET_LEN],temp,MAX_PACKET_SIZE);    
  }    
  
  // Print information for user
  if (c == 0){ // only once
    std::cout << "Number of generated packets = " << packetNum << std::endl;
  }
  
  // Return packet data    
  return {packetNum,packets};

}
		   

// Update event header
void genData::update_tHdr(int16_t* data, uint64_t hdr_start, 
			  uint16_t chan, uint64_t event_num){
  
  // Update clocks
  uint64_t DTCclock = event_num*200e6; // 200 MHz
  uint64_t ADCclock = event_num*75e6; // 75 MHz
  
  // Channel + DTC Clock 0 
  data[hdr_start+tHdr.Ch_DTCclk_0] = (DTCclock & 0xFF) << 8 | chan & 0xff;
  // DTC Clock 1-3
  data[hdr_start+tHdr.DTCclk_1] = DTCclock >> 8;
  data[hdr_start+tHdr.DTCclk_2] = DTCclock >> 24;
  data[hdr_start+tHdr.DTCclk_3] = DTCclock >> 40;
  // ADC Clock 0-3
  data[hdr_start+tHdr.ADCclk_0] = ADCclock & 0xFFFF;
  data[hdr_start+tHdr.ADCclk_1] = ADCclock >> 16;
  data[hdr_start+tHdr.ADCclk_2] = ADCclock >> 32;
  data[hdr_start+tHdr.ADCclk_3] = ADCclock >> 48;
  // Event Number 0-2
  data[hdr_start+tHdr.EvNum_0] = event_num & 0xFFFF;
  data[hdr_start+tHdr.EvNum_1] = event_num >> 16;
  data[hdr_start+tHdr.EvNum_2] = event_num >> 32;
  // Event Window Tag 0-2
  data[hdr_start+tHdr.EWT_0] = event_num & 0xFFFF;
  data[hdr_start+tHdr.EWT_1] = event_num >> 16;
  data[hdr_start+tHdr.EWT_2] = event_num >> 32;

}
