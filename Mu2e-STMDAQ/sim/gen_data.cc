// Gen Data code                                                       
#include "gen_data.hh"

// Constructor
GenData::GenData(size_t channel_, size_t event_num_, size_t event_period_,
                 const std::string& filepath) :
  channel(channel_),
  event_num(static_cast<double>(event_num_)),
  event_period(event_period_),
  event_len((double)event_period*1e-9 * fADC*1e6){
  
  // Store the event size
  event_size = event_len*sizeof(int16_t);
  // Calculate data length in ADCs
  data_len = event_len*event_num;

  // Notify user
  std::cout << "Event len = " << event_period << " ns = " << event_len << " ADC values = " << event_size << " bytes." << std::endl;
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
    // Calculate repetition number of file
    double nfiles_in_data = static_cast<double>(data_len*sizeof(int16_t))/static_cast<double>(data_size);
    std::cout << "There are " << events_in_file << " " << event_len << " ns events in filepath" << std::endl;
    std::cout << "Choose from the following two options:" << std::endl;
    std::cout << "A) Send the user input number of events: " << event_num << " events = " << nfiles_in_data << " * the input file." << std::endl;
    std::cout << "B) Send the input file a multiple number of times." << std::endl;
    char input = 'a';


    while (true) {
      char input;
      std::cout << "Input A or B to continue: ";
      std::cin >> input;
      if (input == 'a' or input == 'A'){
        std::cout << "Event number unchanged." <<  std::endl;
        std::cout << "Input file will be repeated " << nfiles_in_data << " times." << std::endl;
        break;
	
      }	
      else if (input == 'b' or input == 'B') {
        double num = 0;
        while(true){
          std::cout << "Input the number times the file be repeated:" << std::endl;
          std::string n;
          std::cin >> n;
          try {
            num = std::stod(n);
            std::cout << "Double value: " << num << std::endl;
            break;
          } catch (const std::invalid_argument&) {
            std::cout << "Invalid input: not a number." << std::endl;
          }
        }
        // Update event num
        event_num = num*events_in_file;
        // Recalculate data length in ADCs
        data_len = event_len*event_num;	
        std::cout << "Event number changed to " << event_num << " events." <<  std::endl;
        // Notify user
        std::cout << "Event len = " << event_period << " ns = " << event_len << " ADC values = " << event_size << " bytes." << std::endl;
        std::cout << "Event num = " << event_num << " events = " << data_len << " ADC values = " << (double)data_len*sizeof(int16_t)*1e-9 << " GB." << std::endl;
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

  // Make sure the data size is larger than the event size
  if (data->size() < event_len){
    std::cout << "ERROR: Please ensure generated data length ("
              << data->size() << " ADC values) is >= the event length " 
      	      << event_len << " ADC values). Exiting..." << std::endl;
    exit(0);
  }

}

// Get packets
void GenData::gen_packets(const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& queue,
                          const std::shared_ptr<SignalHandler>& signal,
                          std::atomic<bool>& finished){

  // The packet number
  size_t packet_num = 0;
  
  // Data counter
  size_t data_count = 0;
  
  // Store data left in packet
  size_t left_in_event = event_len;
  
  // This event size
  size_t this_event_len = event_len;

  // The EWT
  size_t EWT = 0;

  // Does the null hb require a new packet?
  bool new_packet_for_nullhb = false;
  
  // Loop through all data
  while (data_count < data_len && !stop::should_stop()){

    // Empty packet
    std::shared_ptr<std::vector<int16_t>> packet = std::make_shared<std::vector<int16_t>>(MAX_PACKET_LEN);
    
    // Store data left in packet
    size_t left_in_packet = MAX_PACKET_LEN;
    
    // Create a new packet header    
    uint32_t lowerHalf = packet_num & 0xFFFF; // Extract lower 16 bits     
    uint32_t upperHalf = (packet_num >> 16) & 0xFFFF; // Extract upper 16
    // Add packet header                                              
    (*packet)[0] = lowerHalf;
    (*packet)[1] = upperHalf;
    (*packet)[2] = pHdr_check;

    // Remove packet header from packet data
    left_in_packet -= pHdr_Len;

    // Loop through data left in packet
    while(left_in_packet > 0){

      // If remaining packet space is <= a event header...
      if (left_in_packet < fw_eHdr_len){	
        // PACKET COMPLETE - fill curent packet with 0xDEADBEEF
        // Get remaining in packet
        uint16_t start = MAX_PACKET_LEN - left_in_packet;
        // Fill remainder of packet with 0xDEADBEEF
        for (int j = start; j < start+left_in_packet; j++){
          // If odd entry
          if (j % 2 != 0) (*packet)[j] = BEEF;
          // If event entry
          if (j % 2 == 0) (*packet)[j] = DEAD;
        } // End j-loop	  
        // Break loop
        break;
      } // End if fw_eHdr_len >= left_in_packet

      // Get the event length to copy
      size_t event_in_packet = 0;
      if (left_in_event >= left_in_packet - fw_eHdr_len){
        event_in_packet = left_in_packet - fw_eHdr_len;
      }
      else{
        event_in_packet = left_in_event;
      }
      // Calc event start
      size_t event_start = this_event_len - left_in_event;
      
      // Create event header
      std::vector<int16_t> header = form_event_header(channel,EWT,1,this_event_len,
                                                      event_start,
                                                      event_in_packet);

      // Copy event header
      size_t copy_index = MAX_PACKET_LEN - left_in_packet;
      std::copy(header.begin(), header.end(), packet->begin() + copy_index);

      // Account for event header length                            
      left_in_packet -= fw_eHdr_len;

      // Copy the event portion
      copy_index = MAX_PACKET_LEN - left_in_packet;
      size_t copy_start = data_count % data->size();
      size_t copy_end = (data_count + event_in_packet) % data->size();
      if (data_count + event_in_packet == data->size()) copy_end = data->size();
      // If the modulus of the data has wrapped around
      if (copy_end < copy_start){
        // Copy portion to end of data
        std::copy(data->begin() + copy_start,
                  data->end(),
                  packet->begin() + copy_index);
        // Update left in packet and copy index
        left_in_packet -= data->size() - copy_start;
        copy_index = MAX_PACKET_LEN - left_in_packet;
        // Copy from start of data
        std::copy(data->begin(),
                  data->begin() + copy_end,
                  packet->begin() + copy_index);
        // Update left in packet 
        left_in_packet -= copy_end;
      }
      // Else if start and end are inside data
      else{
        // Copy data
        std::copy(data->begin() + copy_start,
                  data->begin() + copy_end,
                  packet->begin() + copy_index);	
        // Update left in packet 
        left_in_packet -= event_in_packet;
      }
      // Update the data counter
      data_count += event_in_packet;
      // Update left in event
      left_in_event -= event_in_packet;

      // If at end of data...
      if (data_count == data_len){
        // If remaining packet space is <= a event header
        if (left_in_packet < fw_eHdr_len){
          // Signal a new packet needed for the nullhb header
          new_packet_for_nullhb = true;
        }
        // Else, if there is a space for the nullhb header in this packet
        else{
          // Create event header
          std::vector<int16_t> header = form_event_header(channel,
                                                          0,
                                                          0,
                                                          0,
                                                          0,
                                                          0);
	  
          // Copy event header
          size_t copy_index = MAX_PACKET_LEN - left_in_packet;
          std::copy(header.begin(), header.end(), packet->begin() + copy_index);
          // Account for event header length                            
          left_in_packet -= fw_eHdr_len;	  
        }
        // Fill final packet with 0xDEADBEEFS      
        // Get remaining in packet
        uint16_t start = MAX_PACKET_LEN - left_in_packet;      
        // Fill remainder of packet with 0xDEADBEEF
        for (int j = start; j < MAX_PACKET_LEN; j++){	
          // If odd entry
          if (j % 2 != 0) (*packet)[j] = BEEF;
          // If event entry
          if (j % 2 == 0) (*packet)[j] = DEAD;
          // Decrement left_in_packet
          left_in_packet--;
        }
        // Increment the EWT
        EWT++;
      }      
      // If we've reached the end of the event
      else if (left_in_event == 0){
        // Increment the EWT
        EWT++;
        // Check if the last event is an event portion
        if (event_num - double(EWT) < 1){
          this_event_len = data_len - data_count;
        }
        // Else standard event size
        else{
          this_event_len = event_len;
        }
        // Reset left in event
        left_in_event = this_event_len;
      }
      
    } // end while(left_in_packet > 0)

    // Push packet to queue
    queue->push(packet);
    
    // Increase the packet number
    packet_num++;

    // Print final EWT
    if (EWT == std::ceil(event_num)) std::cout << "Final EWT = " << EWT << std::endl;
    
  } // End while (data_count < data_len)

  // If a new packet is needed for the nullhb
  if (new_packet_for_nullhb){

    // Empty packet
    std::shared_ptr<std::vector<int16_t>> packet = std::make_shared<std::vector<int16_t>>(MAX_PACKET_LEN);

    // Store data left in packet
    size_t left_in_packet = MAX_PACKET_LEN;
    
    // Create a new packet header    
    uint32_t lowerHalf = packet_num & 0xFFFF; // Extract lower 16 bits     
    uint32_t upperHalf = (packet_num >> 16) & 0xFFFF; // Extract upper 16
    // Add packet header                                              
    (*packet)[0] = lowerHalf;
    (*packet)[1] = upperHalf;
    (*packet)[2] = pHdr_check;

    // Remove packet header from packet data
    left_in_packet -= pHdr_Len;

    // Create event header
    std::vector<int16_t> header = form_event_header(channel,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    0);
    
    // Copy event header
    size_t copy_index = MAX_PACKET_LEN - left_in_packet;
    std::copy(header.begin(), header.end(), packet->begin() + copy_index);
    // Account for event header length                            
    left_in_packet -= fw_eHdr_len;
    
    // Fill final packet with 0xDEADBEEFS      
    // Get remaining in packet
    uint16_t start = MAX_PACKET_LEN - left_in_packet;      
    // Fill remainder of packet with 0xDEADBEEF
    for (int j = start; j < MAX_PACKET_LEN; j++){	
      // If odd entry
      if (j % 2 != 0) (*packet)[j] = BEEF;
      // If event entry
      if (j % 2 == 0) (*packet)[j] = DEAD;
      // Decrement left_in_packet
      left_in_packet--;
    }

    // Push packet to queue
    queue->push(packet);
    
    // Increase the packet number
    packet_num++;
    
  }
  
  // Signal finish
  finished = true;
  
  // Notify user
  std::cout << "Generated " << EWT << " events (" << packet_num << " packets)." << std::endl;

 
}

// Generate a new event header
std::vector<int16_t> GenData::form_event_header(size_t channel_,
                                                size_t EWT_,
                                                size_t EM_,
                                                size_t len,
                                                size_t event_start,
                                                size_t event_in_packet){

  // Get first event header inputs
  uint16_t chan = channel_;
  uint64_t eventNum = EWT_;
  uint64_t ADCclock = EWT_*75e6; // 75 MHz
  uint64_t EWT = EWT_;
  uint64_t EM = EM_;
  uint64_t DRTDC = 0;
  uint16_t eventStart = event_start;
  uint16_t len_in_packet = event_in_packet;
  uint16_t subRunNum = 0;
  uint16_t ZSflag = 0;
  uint16_t PreVal = 0; 
  // Find event length to read
  uint16_t eventLen = len;
  uint64_t runNum = 0; 
  uint64_t DTCclock = EWT_*40e6; // 40 MHz

  // Initialise event header
  std::vector<int16_t> header(fw_eHdr_len);

  // Copy header anchor
  std::copy(std::begin(fw_eHdr.anchor_data),
            std::end(fw_eHdr.anchor_data),
            header.begin());   
  // ADC Clock 0-3
  header[fw_eHdr.ADCclk_0] = ADCclock & 0xFFFF;
  header[fw_eHdr.ADCclk_1] = ADCclock >> 16;
  header[fw_eHdr.ADCclk_2] = ADCclock >> 32;
  header[fw_eHdr.ADCclk_3] = ADCclock >> 48; 
  // Event Number 0-2
  header[fw_eHdr.EvNum_0] = eventNum & 0xFFFF;
  header[fw_eHdr.EvNum_1] = eventNum >> 16;
  header[fw_eHdr.EvNum_2] = eventNum >> 32;
  // Event Window Tag 0-2
  header[fw_eHdr.EWT_0] = EWT & 0xFFFF;
  header[fw_eHdr.EWT_1] = EWT >> 16;
  header[fw_eHdr.EWT_2] = EWT >> 32;
  // Event Mode 0-1
  header[fw_eHdr.EM_0] = EM & 0xFFFF;
  header[fw_eHdr.EM_1] = EM >> 16;
  // Event Mode 2 + Delivery Ring TDC  
  header[fw_eHdr.EM_2_DRTDC] = (DRTDC & 0xFF) << 8 | (EM >> 32) & 0xFF;
  // Event Start Offset
  header[fw_eHdr.EvStart] = eventStart;
  // Event Length (To Read)
  header[fw_eHdr.EvInPacket] = len_in_packet;
  // Beef placeholder
  header[fw_eHdr.Beef] = int16_t(0xBEEF);
  // Subrun number from FW
  header[fw_eHdr.SubRunNum_0] = subRunNum & 0xFFFF;
  header[fw_eHdr.SubRunNum_1] = subRunNum >> 16;
  // ZS flag and Prescale value
  header[fw_eHdr.ZSflag_PreVal] = (ZSflag & 0xFF) << 8 | PreVal & 0xff;
  // Total event length
  header[fw_eHdr.EvLen] = len;
  // Run number [DCS write]
  header[fw_eHdr.RunNum_0] = runNum & 0xFFFF;
  header[fw_eHdr.RunNum_1] = runNum >> 16;
  header[fw_eHdr.RunNum_2] = runNum >> 32;
  header[fw_eHdr.RunNum_3] = runNum >> 48;
  // Channel + DTC Clock 0 
  header[fw_eHdr.Ch_DTCclk_0] = (DTCclock & 0xFF) << 8 | chan & 0xff;
  // DTC Clock 1-3
  header[fw_eHdr.DTCclk_1] = DTCclock >> 8;
  header[fw_eHdr.DTCclk_2] = DTCclock >> 24;
  header[fw_eHdr.DTCclk_3] = DTCclock >> 40;  
  
  // Return event header
  return header;




}
