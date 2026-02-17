#include <unistd.h>

#include "processData.hh"

// The total header length
static const uint hdrtot  = 41;
// The header length to skip
static const uint hdrskip = 9;  
// The event length header index
static const uint elen_loc = 18;  
// The event number header index
static const uint enum_loc = 8;
// The event header length
static const uint hdrlen = hdrtot-hdrskip;
// The event header size (bytes)
static const uint hdrsize = 2*hdrlen;
// The expected event length
static const uint expected_elength = 30000;

// Constructor
processData::processData() {};

// Form uin64_t out of 4 x int16_t                                                                       
uint64_t processData::make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3){

  uint64_t value = static_cast<uint16_t>(p3);
  value <<= 16;
  value |= static_cast<uint16_t>(p2);
  value <<= 16;
  value |= static_cast<uint16_t>(p1);
  value <<= 16;
  value |= static_cast<uint16_t>(p0);

  return value;

}

// Read data from binary file
std::vector<int16_t> processData::readFile(char* fname){

  // The data vecotr
  std::vector<int16_t> vec;

  // Read in the binary file
  std::ifstream is;
  is.open(fname, std::ios::binary);
  is.seekg(0, std::ios::end);
  size_t filesize=is.tellg();
  is.seekg(0, std::ios::beg);
  vec.resize(filesize/sizeof(int16_t));
  is.read((char *)vec.data(), filesize);

  // The data file length
  int file_length = vec.size();

  // A vector for only the adc data
  std::vector<int16_t> ADCdata;
  std::vector<std::vector<int16_t>> eventdata;
  ADCdata.clear();
  // A counter for all ADC values
  uint64_t adc_count = 0;

  // The start index of the ADC data of a new event
  uint event_start = 0;
  // The end index of the ADC data of a new event
  uint event_end = 0;
  // The event length (event_end - event_start)
  uint event_length = 0;
  // The event number
  uint64_t event_number = 0;
  // Stored event counter
  uint64_t event_count = 0;
  // Total event counter
  uint64_t event_tot = 0;
  
  // Boolean to signal the first header
  bool first_header = true;

  // Loop over all elements in the data vector
  for(long unsigned int i = 0; i < vec.size(); i++){

    // Assume we're not at the location of a header
    bool atHdr = false;
    // If we find what looks like the start of an event header
    if(vec[i] == HdrEnd_data[0]){
      // Pre-emptively assume we're at the location of a header
      atHdr = true;
      // Loop over the header length
      for(int j = i; j < (i + HdrEnd_len); j++){
	// If at any points the following data does not 
	// equal what is expected from an event header
	if(vec[j] != HdrEnd_data[j-i]){
	  // We're not at a header
	  atHdr = false;
	  break;
	}
      }
    }

    // If we've found the location of an event header
    if(atHdr){
     
      // Find the starting index of the event header
      int header_start = i - (hdrlen - HdrEnd_len);

      // If not the first event to have found
      if (!first_header){
	
	// Get the event end index of the previous event
	event_end = header_start - hdrskip - 1;	

	// Get the event length (event_end - event_start)
	event_length = event_end - event_start;
	
	// Apply data quality for dropped packets
	if (event_length < expected_elength){
	  std::cout << "Event " << event_number << " has length " << event_length << std::endl;
	  std::cout << "Must contain dropped packets. Cutting event..." << std::endl;
	}
	// Else store event
	else{	  

	  // Create vector for event
	  std::vector<int16_t> event;
	  // Reserve event size
	  event.resize(event_length);

	  // Memcpy the event data
	  memcpy(&event[0],&vec[event_start],event_length*sizeof(int16_t));
	  
	  // Push back the new event
	  eventdata.push_back(event);

	  // Add to single event vector
	  ADCdata.insert(std::end(ADCdata), std::begin(event), std::end(event));
	  
	  // Increase the adc counter
	  adc_count += event_length;
	  
	  // Incerement the event counter
	  event_count++;

	}

      } // End if !first_header
      
	// Set first event to false
      first_header = false;
      
      // Set the event start index
      event_start = header_start + hdrlen;
      
      // Get the event number
      event_number = make_uint64_t(vec[header_start+enum_loc+0],
				   vec[header_start+enum_loc+1],
				   vec[header_start+enum_loc+2],
				   0);     
      
    } // End if at header
          
  } // End loop over data
 
  // Now we need to store the last event in the file
  
  // Get the event end index of the last event (EOF)
  event_end = vec.size()-1;	
  
  // Get the event length (event_end - event_start)
  event_length = event_end - event_start;
  
  // Apply data quality for dropped packets
  if (event_length < expected_elength){
    std::cout << "Event " << event_number << " has length " << event_length << std::endl;
    std::cout << "Must contain dropped packets. Cutting event..." << std::endl;
  }
  // Else store event
  else{	  
    
    // Create vector for event
    std::vector<int16_t> event;
    // Reserve event size
    event.resize(event_length);
    
    // Memcpy the event data
    memcpy(&event[0],&vec[event_start],event_length*sizeof(int16_t));
    
    // Push back the new event
    eventdata.push_back(event);
    
    // Add to single event vector
    ADCdata.insert(std::end(ADCdata), std::begin(event), std::end(event));
    
    // Increase the adc counter
    adc_count += event_length;
    
    // Incerement the event counter
    event_count++;
    
  }  
  
  // Return data vector
  return ADCdata;
  
}
