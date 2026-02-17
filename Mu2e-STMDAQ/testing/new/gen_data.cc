// Gen Data code                                                       
#include "gen_data.hh"

// Constructor
GenData::GenData(size_t event_num_, size_t event_len_,
		 const std::string& filepath) :
  event_num(static_cast<double>(event_num_)), event_len(event_len_) {
    
  // Calculate event length in ADCs
  size_t event_len_adc = (double)event_len*1e-9 * fADC*1e6;
  // Store the event size
  event_size = event_len_adc*sizeof(int16_t);
  // Calculate data length in ADCs
  data_len = event_len_adc*event_num;

  // Notify user
  std::cout << "Event len = " << event_len << " ns = " << event_len_adc << " ADC values = " << event_size << " bytes." << std::endl;
  std::cout << "Event num = " << event_num << " events = " << data_len << " ADC values = " << (double)data_len*sizeof(int16_t)*1e-9 << " GB." << std::endl;
   
  // Create the data vector
  data = std::make_shared<std::vector<int16_t>>();

  // Print string
  std::string print_string;
    
  // If no file provided, incrementin counter
  if (filepath.empty()){
    // Get all range of int16_t values
    for (int32_t i = std::numeric_limits<int16_t>::min();
	 i <= std::numeric_limits<int16_t>::max(); ++i){
      int16_t value = static_cast<int16_t>(i);
      // Push back the increment counter
      data->push_back(value);
    }
    // Save the data size    
    data_size = data->size()*sizeof(int16_t);
    // Save print string
    print_string = "Generated incrementing counter";
  }
  // Else load data from file
  else{
    // Load file
    std::ifstream file(filepath, std::ios::binary);
    // Throw error if file is absent
    if (!file) throw std::runtime_error("Failed to open file: " + filepath);
    // Get all values from file and push back to vector
    int16_t value;
    while (file.read(reinterpret_cast<char*>(&value), sizeof(int16_t))) {
      data->push_back(value);
    }
    // Throw error if file not read correctly
    if (!file.eof()) throw std::runtime_error("Error reading file: " + filepath);
    // Save print string
    print_string = "Loaded binary file";
    // Save the data size
    data_size = data->size()*sizeof(int16_t);
    // Offer the option to just send the contents in the file once
    double events_in_file = static_cast<double>(data_size)/static_cast<double>(event_size);
    std::cout << "There are " << events_in_file << " " << event_len << " ns events in filepath" << std::endl;
    std::cout << "Would you like to change the number of events to this value to send the file contents only once?" << std::endl;
    while (true) {
      char input;
      std::cout << "Input Y or N to continue: ";
      std::cin >> input;
      if (input == 'y' or input == 'Y') {
	// Update event num
	event_num = events_in_file;
	// Recalculate data length in ADCs
	data_len = event_len_adc*event_num;	
     	std::cout << "Event number changed to " << event_num << " events." <<  std::endl;
	// Notify user
	std::cout << "Event len = " << event_len << " ns = " << event_len_adc << " ADC values = " << event_size << " bytes." << std::endl;
	std::cout << "Event num = " << event_num << " events = " << data_len << " ADC values = " << (double)data_len*sizeof(int16_t)*1e-9 << " GB." << std::endl;
     	break;
      }
      else if (input == 'n' or input == 'N') {
	std::cout << "Event number unchanged." <<  std::endl;
	break;
      }
      else {
     	std::cout << "Invalid input. Please enter Y or N." << std::endl;
     	// Clear the fail state and discard the rest of the input line
 	std::cin.clear();
     	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
    }
  }
  
  // Print size of vector to user
  std::cout << print_string << " data, size = " << data_size*1e-9 << " GB." << std::endl;

}

// Get packets
void GenData::get_packets(){

  // Empty packet
  std::vector<int16_t> packet(MAX_PACKET_LEN);

  // Data counter
  size_t data_count = 0;
  
  // Store data left in packet
  size_t left_in_event = event_len;
  
  // This event size
  size_t this_event_len = event_len;

  // The EWT
  size_t EWT = 0;
  
  // Loop through all data
  while (data_count < data_len){

    // Store data left in packet
    size_t left_in_packet = MAX_PACKET_LEN;
    
    // Create a new packet header    
    uint32_t lowerHalf = packet_num & 0xFFFF; // Extract lower 16 bits     
    uint32_t upperHalf = (packet_num >> 16) & 0xFFFF; // Extract upper 16
    // Add packet header                                              
    packet[0] = lowerHalf;
    packet[1] = upperHalf;
    packet[2] = fw_pHdr_end_data;

    // Remove packet header from packet data
    left_in_packet -= pHdr_Len;

    // Loop through data left in packet
    while(left_in_packet > 0){

      // If remaining packet space is <= a trigger header...
      if (left_in_packet < fw_tHdr_Len){	
        // PACKET COMPLETE - fill curent packet with 0xDEADBEEF
    	// Get remaining in packet
    	uint16_t start = MAX_PACKET_LEN - left_in_packet;
    	// Fill remainder of packet with 0xDEADBEEF
    	for (int j = start; j < start+left_in_packet; j++){
    	  // If odd entry
    	  if (j % 2 != 0) packet[j] = BEEF;
    	  // If event entry
    	  if (j % 2 == 0) packet[j] = DEAD;
    	} // End j-loop	  
	// Break loop
	break;
      } // End if fw_tHdr_Len >= left_in_packet

      // Get the event length to copy
      size_t event_in_packet = 0;
      if (left_in_event >= left_in_packet - fw_tHdr_Len){
	event_in_packet = left_in_packet - fw_tHdr_Len;
      }
      else{
	event_in_packet = left_in_event;
      }
      // Calc event start
      size_t event_start = this_event_len - left_in_event;
      
      // Create event header
      std::vector<int16_t> header = form_event_header(EWT,length,
						      event_start,
						      event_in_packet);

      // Copy event header
      size_t copy_index = MAX_PACKET_LEN - left_in_packet;
      std::copy(header.begin(), header.end(), packet.begin() + copy_index);

      // Account for event header length                            
      left_in_packet -= fw_tHdr_Len;

      // Copy the event portion
      copy_index = MAX_PACKET_LEN - left_in_packet;
      size_t copy_start = data_count % data->size();
      size_t copy_end = (data_count + event_in_packet) % data->size();
      // If the modulus of the data has wrapped around
      if (copy_end < copy_start){
	// Copy portion to end of data
	std::copy(data.begin() + copy_start,
		  data.begin() + data.end(),
		  packet.begin() + copy_index);
	// Update left in packet and copy index
	left_in_packet -= data->size() - copy_start;
	copy_index = MAX_PACKET_LEN - left_in_packet;
	// Copy from start of data
	std::copy(data.begin(),
		  data.begin() + copy_end,
		  packet.begin() + copy_index);
	// Update left in packet 
	left_in_packet -= copy_end;
      }
      // Else if start and end are inside data
      else{
	// Copy data
	std::copy(data.begin() + copy_start,
		  data.begin() + copy_end,
		  packet.begin() + copy_index);	
	// Update left in packet 
	left_in_packet -= event_in_packet;
      }
      // Update the data counter
      data_count += event_in_packet;
      // Update left in event
      left_in_event -= event_in_packet;

      // If we've reached the end of the event
      if (left_in_event == 0){
	// Increment the EWT
	EWT++;
	// Check if the last event is an event portion
	if (event_num - double(EWT) < 1){
	  this_event_len = event_num - double(EWT);
	}
	// Else standard event size
	else{
	  this_event_len = event_len;
	}
	// Reset left in event
	left_in_event = this_event_len;
    } // end while(left_in_packet > 0)
    
    // Increase the packet number
    packetNum++;
    
    // If at end of data, ensure last packet is filled with 0xDEADBEEF
    if (data_count == data_len){
      // Fill final packet with 0xDEADBEEFS      
      // Get remaining in packet
      uint16_t start = MAX_PACKET_LEN - left_in-packet;      
      // Fill remainder of packet with 0xDEADBEEF
      for (int j = start; j < MAX_PACKET_LEN; j++){	
    	// If odd entry
    	if (j % 2 != 0) packet[j] = BEEF;
    	// If event entry
    	if (j % 2 == 0) packet[j] = DEAD;
    	// Decrement leftInPacket
    	left_in-packet--;
      }	  
    }
    
  } // End while (data_count < data_len)

}

std::vector<int16_t> GenData::form_event_header(EWT_,length,
						event_start,event_in_packet){

  // %%%%%%%%%%%%%%%%%%%%%%%%% //
  // Event header for event
  // %%%%%%%%%%%%%%%%%%%%%%%%% //
  // Get first event header inputs
  uint64_t eventNum = EWT_;
  uint64_t ADCclock = EWT_*75e6; // 75 MHz
  uint64_t EWT = EWT_;
  uint64_t EM = 0;
  uint64_t DRTDC = 0;
  uint16_t eventStart = event_start;
  uint16_t length_in_packet = event_in_packet;
  uint16_t subRunNum = 0;
  uint16_t ZSflag = 0;
  uint16_t PreVal = 0; 
  // Find event length to read
  uint16_t eventLength = length;
  uint64_t runNum = 0; 
  uint64_t DTCclock = EWT_*200e6; // 200 MHz

  // Initialise trigger header
  std::vector<int16_t> header(fw_tHdr_Len);

  // Add header start
  memcpy(&header[tHdr.HdrStart_start],
	 &(tHdr.HdrStart_data[0]),
	 tHdr.HdrStart_size);

  // ADC Clock 0-3
  header[tHdr.ADCclk_0] = ADCclock & 0xFFFF;
  header[tHdr.ADCclk_1] = ADCclock >> 16;
  header[tHdr.ADCclk_2] = ADCclock >> 32;
  header[tHdr.ADCclk_3] = ADCclock >> 48; 
  // Event Number 0-2
  header[tHdr.EvNum_0] = eventNum & 0xFFFF;
  header[tHdr.EvNum_1] = eventNum >> 16;
  header[tHdr.EvNum_2] = eventNum >> 32;
  // Event Window Tag 0-2
  header[tHdr.EWT_0] = EWT & 0xFFFF;
  header[tHdr.EWT_1] = EWT >> 16;
  header[tHdr.EWT_2] = EWT >> 32;
  // Event Mode 0-1
  header[tHdr.EM_0] = EM & 0xFFFF;
  header[tHdr.EM_1] = EM >> 16;
  // Event Mode 2 + Delivery Ring TDC  
  header[tHdr.EM_2_DRTDC] = (DRTDC & 0xFF) << 8 | (EM >> 32) & 0xFF;
  // Event Start Offset
  header[tHdr.EvStart] = eventStart;
  // Event Length (To Read)
  header[tHdr.EvInPacket] = eventLeninPacket;
  // Beef placeholder
  header[tHdr.Beef] = 0xBEEF;
  // Subrun number from FW
  header[tHdr.SubRunNum_0] = subRunNum & 0xFFFF;
  header[tHdr.SubRunNum_1] = subRunNum >> 16;
  // ZS flag and Prescale value
  header[tHdr.ZSflag_PreVal] = (ZSflag & 0xFF) << 8 | PreVal & 0xff;
  // Total event length
  header[tHdr.EvLen] = eventLength;
  // Run number [DCS write]
  header[tHdr.RunNum_0] = runNum & 0xFFFF;
  header[tHdr.RunNum_1] = runNum >> 16;
  header[tHdr.RunNum_2] = runNum >> 32;
  header[tHdr.RunNum_3] = runNum >> 48;
  // Channel + DTC Clock 0 
  header[tHdr.Ch_DTCclk_0] = (DTCclock & 0xFF) << 8 | chan & 0xff;
  // DTC Clock 1-3
  header[tHdr.DTCclk_1] = DTCclock >> 8;
  header[tHdr.DTCclk_2] = DTCclock >> 24;
  header[tHdr.DTCclk_3] = DTCclock >> 40;  
  
  // Return trigger header
  return header;




}
