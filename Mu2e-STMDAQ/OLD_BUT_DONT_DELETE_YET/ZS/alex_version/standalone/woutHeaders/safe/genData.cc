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

// Convert Gbits to bytes
static const double bytes2Gbits = 8e-9;

// Maximum number of Gbits to send 
static const uint64_t maxGbit = 1e5; // Gbits
static const double maxbytes = maxGbit/bytes2Gbits; // byte

// The number of Gbits to send
double numGbits = 0;

// Total number of packets with data to generate
static const uint64_t genNum = 0xFFFF;

// // Size in bytes of the header end                                                                   
// static const uint HdrEnd_size = 26;
// // Length in int16_t values of the header end                                                        
// static const uint HdrEnd_len = HdrEnd_size/2;
// // Starting index of the header end                                                                  
// static const uint HdrEnd_start = fw_tHdr_Len - HdrEnd_len;

// // Trigger header end data                                                                           
// static const int16_t HdrEnd_data[HdrEnd_Len] = {(int16_t)0xBEEF, // 19 = 0xBEEF
// 						(int16_t)0xCAFE, // 20 = 0xCAFE
// 						(int16_t)0xBEEF, // 21 = 0xBEEF
// 						(int16_t)0xCAFE, // 22 = 0xCAFE
// 						(int16_t)0xAAAA, // 23 = 0xAAAA
// 						(int16_t)0x5678, // 24 = 0x5678
// 						(int16_t)0x1234, // 25 = 0x1234
// 						(int16_t)0xFFFF, // 26 = 0xFFFF
// 						(int16_t)0xEEEE, // 27 = 0xEEEE
// 						(int16_t)0xDDDD, // 28 = 0xDDDD
// 						(int16_t)0xCCCC, //  29 = 0xCCCC
// 						(int16_t)0xBBBB, //  30 = 0xBBBB
// 						(int16_t)0xAAAA}; //  31 = 0xAAAA

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
  
  // Total event size including header (bytes)
  //  event_size_tot = fw_tHdr_Size + deltahb_len*sizeof(int16_t);
  event_size_tot = deltahb_len*sizeof(int16_t);
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

// Form an event/trigger header
int16_t* genData::form_tHdr(uint16_t chan, uint64_t DTCclock,
                            uint64_t ADCclock, uint64_t eventNum,
                            uint64_t EWT, uint64_t EM,
                            uint16_t DRTDC, uint16_t eventStart,
                            uint16_t eventLength){

  // Initialise trigger header
  int16_t* tH = new int16_t[fw_tHdr_Len]();

  // Channel + DTC Clock 0
  tH[tHdr.Ch_DTCclk_0] = (DTCclock & 0xFF) << 8 | chan & 0xff;
  // DTC Clock 1-3
  tH[tHdr.DTCclk_1] = DTCclock >> 8;
  tH[tHdr.DTCclk_2] = DTCclock >> 24;
  tH[tHdr.DTCclk_3] = DTCclock >> 40;
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
  tH[tHdr.EM_2_DRTDC] = (DRTDC & 0xFF) << 8| (EM >> 32) & 0xFF;
  // Event Start Offset
  tH[tHdr.EvStart] = eventStart;
  // Event Length (To Read)
  tH[tHdr.EvLen] = eventLength;
  // Add header end
  memcpy(&tH[tHdr.HdrEnd_start],&(tHdr.HdrEnd_data[0]),tHdr.HdrEnd_size);

  // Return trigger hesder
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
    uint16_t eventNum = start_event_num + i;
    uint64_t DTCclock = eventNum*200e6; // 200 MHz
    uint64_t ADCclock = eventNum*75e6; // 75 MHz
    uint64_t EWT = eventNum;
    uint64_t EM = 0;
    uint64_t DRTDC = 0;
    uint16_t eventStart = 0;
    // Find event length to read
    uint16_t eventLength = deltahb_len;
    // Form event header
    //    int16_t *tH_event = form_tHdr(c,DTCclock,ADCclock,eventNum,EWT,EM,DRTDC,eventStart,eventLength);
    // Copy event header to start of events
    //    memcpy(&events[event_start],tH_event,fw_tHdr_Size);
    // // Find event length to read
    // uint16_t eventLength = deltahb_len;
    // // Form event header
    // int16_t* tH_event = new int16_t [fw_tHdr_Len] ();
    // tH_event[fw_tHdr_eNum] = eventNum;
    // tH_event[fw_tHdr_eLen] = eventLength;
    // tH_event[fw_tHdr_endloc] = fw_tHdr_end;
    // Copy event header to start of events
    //    memcpy(&events[event_start],tH_event,fw_tHdr_Size);              
    // Add ADC data
    for (int j = 0; j < eventLength; j++){
      // If using data from input binary file
      if (dataFromFile){
	//	events[event_start+fw_tHdr_Len+j] = binary_data[adc_count % binary_len];
	events[event_start+j] = binary_data[adc_count % binary_len];
	adc_count++;
      }
      // Else in generating incrmenting data
      else{
	//	events[event_start+fw_tHdr_Len+j] = adc++;
	events[event_start+j] = adc++;
      }
    } // End j-loop over adc data

  } // End i-loop over events
  
  // Print information for user
  if (c == 0){ // only once
    std::cout << "Number of events = " << " = " << numberhb << std::endl;
    std::cout << "Event length = " << " = " << deltahb 
	      << " = " << deltahb_ns << " ns = " << deltahb_s << " s"
	      << " = " << deltahb_len << " counts" <<  std::endl;
    // if (dataFromFile){
    //   std::cout << "Input binary file repeated " << double(adc_count*sizeof(int16_t))/(double)binary_size << " times" << std::endl;
    // }
    std::cout << "Total data size = " << event_array_len*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl;
  }

  // Release binary file memeory once stored as events
  if (c==1) delete[] binary_data;

  return {event_array_len,events};

}
