// Test functions
#include "test_funcs.hh"

//Standard constructor
test_funcs::test_funcs() {}

// Event counter
uint32_t event_count = 0;
// The current event
uint32_t current_event = 0;

// Firmware trigger header struct from dataVars.hh
fw_tHdr tHdr2;

// Data variables class
dataVars dv;

// Client function to send packets
void test_funcs::client(UDPsocket &udp ,int socket, 
			int16_t* data, uint64_t packet_num,
			uint32_t number_hb, uint32_t send_num){

  // Notify user
  std::cout << "In send thread:  socket " << socket << std::endl;

  // Send data counter
  uint64_t sendCount = 0;

  // Define receive buffer as array of packets
  int16_t *snd_buffer[UDPsocket::SENDMMSG_NUM];
  uint16_t *size_buffer = new uint16_t [UDPsocket::SENDMMSG_NUM] ();
  for (int i = 0; i < UDPsocket::SENDMMSG_NUM; i++){
    snd_buffer[i] = new int16_t [MAX_PACKET_LEN] ();
    size_buffer[i] = MAX_PACKET_SIZE;
  }

  // Get start time before packet sending
  auto start = std::chrono::steady_clock::now();

  // Define a message counter
  int msgNum = 0;

  // If sending multiple times, calculate packets to send
  if (send_num < 1){
    send_num = 1;
  }
  uint to_send = send_num*packet_num;
  std::cout << "Sending " << (send_num-1)*(packet_num-1) + packet_num  << " packets" << std::endl;

  // The offset for skipping packets
  int offset = 0;

  // Last event of a multiple send? 
  bool last_event = false;
  
  // Loop over all packets to be sent
  for (uint i = 0; i < to_send; i++) {

    // If in the first packet of a repeated send
    if (send_num > 1  && i % packet_num == 0){
      // Set last event to false
      last_event = false;
      // Print cycle to user
      std::cout << i/packet_num+1 << "/" << send_num << std::endl;
    }
    // If in the last packet of a multiple send AND not the last overall packet
    // OR if we're in the last event of a multiple send
    if ((i % packet_num == packet_num-1 and i != to_send-1) or last_event){
      // Increase the skip offset
      offset++;
      // Skip this packet
      continue;
    }
    // Memcpy packet to buffer
    memcpy(snd_buffer[msgNum],
	   &data[(i % packet_num)*MAX_PACKET_LEN],
	   MAX_PACKET_SIZE);
    // Adjust the packet and event numbers
    // and update whether this is the last event of a multiple send
    last_event = update_headers(snd_buffer[msgNum],packet_num,i,offset,number_hb,send_num);
    // Increment message counter
    msgNum++;
    // Check if SENDMMMSG_NUM has been reached
    if (msgNum == UDPsocket::SENDMMSG_NUM){
      // Sleep to mimic fw sending
      // this_thread::sleep_for(5ms);
      // Send buffer
      int retval = udp.send(snd_buffer,size_buffer,UDPsocket::SENDMMSG_NUM,socket);
        // Increase counter by number of sent messages
      sendCount += udp.SENDMMSG_NUM;
      // Reset message counter
      msgNum = 0;
    }
    // Send any leftover packets at end of loop
    else if (i == to_send-1){
      // Send remaining buffer
      int retval = udp.send(snd_buffer,size_buffer,msgNum,socket);
        // Increase counter by number of sent messages
      sendCount += msgNum;
    }
  }

  // Get end time after packet sending
  auto end = std::chrono::steady_clock::now();
  // Find time taken to send all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = std::chrono::duration <double, std::nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = sendCount*MAX_PACKET_SIZE*1e-9;

  // Store speed as Gbits/s
  double sendSpeed = data_size*8/(timeNano*1e-9);

  // Notify user
  std::cout << "Sent " << sendCount
	    << " packets [" << data_size
	    << " Gbytes] at "<< sendSpeed << " Gbit/s" << std::endl;
  
}

// Update event headers to send more than generated in memory
bool test_funcs::update_headers(int16_t* &data, uint64_t packet_num, uint i,
				int offset, uint32_t number_hb, uint32_t send_num){

  // Set last event of multiple send to false
  bool last_event = false;
  
  // Get the packet number
  uint32_t pnum = (uint16_t)data[1] << 16 
    | (uint16_t)data[0];

  // Check for error
  if (i % packet_num != pnum){
    std::cout << "You fucked up the packet count in test_funcs::update_headers" << std::endl;
    exit(0);
  }
  
  // Adjust the packet number
  uint32_t new_pnum = std::floor(i/packet_num)*packet_num + i % packet_num - offset;  
  uint32_t lowerHalf = new_pnum & 0xFFFF; // Extract lower 16 bits
  uint32_t upperHalf = (new_pnum >> 16) & 0xFFFF; // Extract upper 16 bits
  data[0] = lowerHalf;
  data[1] = upperHalf;
  
  // Calculate how much left in packet after packet header
  uint16_t leftInPacket = MAX_PACKET_LEN - fw_pHdr_Len;

  // While leftinPacket is more than a event header length
  while(leftInPacket > fw_tHdr_Len){

    // Get header start index location
    uint64_t hdr_start_loc = MAX_PACKET_LEN - leftInPacket;
    // Get header end index location
    uint64_t hdr_end_loc = hdr_start_loc + fw_tHdr_Len-1;

    // Get event length index location
    uint64_t eLen_loc = hdr_start_loc + tHdr2.EvInPacket;
    // Get event length
    uint16_t eLen = data[eLen_loc];
    
    // Check to see if it is the last packet filled only with 0xDEADBEEFS
    bool deadbeef = false;
    for (size_t i = 0; i < fw_tHdr_Len/2; i++){
      // If end of packet isn't filled with 0xDEADBEEF, throw critical error        
      if ((uint16_t)(data[hdr_start_loc+i*2] & 0xFFFF) == BEEF
          and (uint16_t)(data[hdr_start_loc+i*2+1] & 0xFFFF) == DEAD){
        // is 0xDEADBEEF                                                            
        deadbeef = true;
      }
    }
    if (deadbeef) break;
        
    // Get the event number
    uint64_t evNum = dv.make_uint64_t(data[hdr_start_loc+tHdr2.EvNum_0],
				      data[hdr_start_loc+tHdr2.EvNum_1],
				      data[hdr_start_loc+tHdr2.EvNum_2],
				      0);

    // If a new event
    if (evNum != current_event){
      // INcrement event counter
      event_count++;
      // Set current event to new event number
      current_event = evNum;
    }
    
    // Check for error
    if (event_count % number_hb != evNum){
      std::cout << "You fucked up the event count in test_funcs::update_headers" << std::endl;
      std::cout << event_count << " " << number_hb << " " << event_count % number_hb << " " << evNum << std::endl;
      exit(0);
    }
    
    // Adjust the packet number
    uint64_t new_evNum = std::floor(event_count/number_hb)*number_hb + event_count % number_hb;
    
    // Get new event header inputs
    uint64_t eventNum = new_evNum;
    uint64_t ADCclock = eventNum*75e6; // 75 MHz
    uint64_t EWT = eventNum;
    uint64_t DTCclock = eventNum*200e6; // 200 MHz
    
    // ADC Clock 0-3
    data[hdr_start_loc+tHdr2.ADCclk_0] = ADCclock & 0xFFFF;
    data[hdr_start_loc+tHdr2.ADCclk_1] = ADCclock >> 16;
    data[hdr_start_loc+tHdr2.ADCclk_2] = ADCclock >> 32;
    data[hdr_start_loc+tHdr2.ADCclk_3] = ADCclock >> 48; 
    // Event Number 0-2
    data[hdr_start_loc+tHdr2.EvNum_0] = eventNum & 0xFFFF;
    data[hdr_start_loc+tHdr2.EvNum_1] = eventNum >> 16;
    data[hdr_start_loc+tHdr2.EvNum_2] = eventNum >> 32;
    // Event Window Tag 0-2
    data[hdr_start_loc+tHdr2.EWT_0] = EWT & 0xFFFF;
    data[hdr_start_loc+tHdr2.EWT_1] = EWT >> 16;
    data[hdr_start_loc+tHdr2.EWT_2] = EWT >> 32;
    // DTC Clock 1-3
    data[hdr_start_loc+tHdr2.DTCclk_1] = DTCclock >> 8;
    data[hdr_start_loc+tHdr2.DTCclk_2] = DTCclock >> 24;
    data[hdr_start_loc+tHdr2.DTCclk_3] = DTCclock >> 40;  

    // Get the event length values
    size_t eventLen = data[hdr_start_loc+tHdr2.EvInPacket];     
    size_t tot_event_len = data[hdr_start_loc+tHdr2.EvLen];
    
    // Fix the event length of the last event if sending multiples
    if (send_num > 1 && current_event % number_hb == number_hb-1 && new_evNum != send_num*number_hb-1){
      // Adjsut total event length to just first part of the event
      data[hdr_start_loc+tHdr2.EvLen] = eventLen;
      // Signal last event of multiple sending is true
      last_event = true;
    }
    
    // Recalculate left in packet
    leftInPacket -= fw_tHdr_Len;      
    
    // Recalculate left in packet
    leftInPacket -= eLen;      
    
  }

  // Return if last event of multiple sending
  return last_event;

}


