// TestFuncs header
#include "Mu2e-STMDAQ/utils/test_funcs.hh"
#include "Mu2e-STMDAQ/utils/Hex.hh" 


std::shared_ptr<AsyncLogger> tlog;
STMdata stm(tlog);

// Constructor
TestFuncs::TestFuncs() {

// FOR TESTING ONLY
  dummy_data.resize(len);
  for (size_t i = 0; i < len; ++i){
    dummy_data[i] = (10-i);
  }
  
  // Initialise function map for ProcessManager
  functionMap["genData"] = [this](std::shared_ptr<DataStruct>& buffer) {
    genData(buffer);
  };
  functionMap["addOne"] = [this](std::shared_ptr<DataStruct>& buffer) {
    addOne(buffer);
  };
  functionMap["confirmData"] = [this](std::shared_ptr<DataStruct>& buffer) {
    confirmData(buffer);
  };
  functionMap["doNothing"] = [this](std::shared_ptr<DataStruct>& buffer) {
    doNothing(buffer);
  };
  functionMap["print"] = [this](std::shared_ptr<DataStruct>& buffer) {
    print(buffer);
  };    
  functionMap["printPackets"] = [this](std::shared_ptr<DataStruct>& buffer) {
    print_packets(buffer);
  };
  functionMap["check_form_events"] = [this](std::shared_ptr<DataStruct>& buffer) {
    check_form_events(buffer);
  };  
  functionMap["check_prep_zs"] = [this](std::shared_ptr<DataStruct>& buffer) {
    check_zs(buffer);    
  };
  functionMap["check_zs"] = [this](std::shared_ptr<DataStruct>& buffer) {
    check_zs(buffer);    
  };
    
}

// FOR TESTING ONLY - Generate data 
void TestFuncs::genData(std::shared_ptr<DataStruct>& buffer){
  // Perform some processing
  // Simulate variable buffer size
  //    buffer->raw_len = (rand() % (60 * 1024 * 1024 / sizeof(int16_t))) + 1000;
  buffer->raw_len = len;//dummy_data.size();
  // Fill buffer with simulated data
  //    memset(buffer->raw.data(), rand() % 100, buffer->raw_len * sizeof(int16_t));
  buffer->raw = dummy_data;
}

// FOR TESTING ONLY - Add 1 to all data 
void TestFuncs::addOne(std::shared_ptr<DataStruct>& buffer){
  // Loop over all data in buffer
  for (size_t i = 0; i < buffer->raw_len; ++i) {
    // Increment each data element by 1
    buffer->raw[i] += 1; 
  }  
}


// FOR TESTING ONLY
void TestFuncs::confirmData(std::shared_ptr<DataStruct>& buffer){
  for (size_t i = 0; i < buffer->raw_len; ++i) {
    int16_t test = dummy_data[i]+3;
    if (buffer->raw[i] != test){
      std::cout << "BUFFER 1 ERROR: " << i << "," << buffer->raw[i] << " != " << test << std::endl;
      exit(0);
    }
    int16_t test2 = (test)*2;
    if (buffer->zs[i] != test2){
      std::cout << "BUFFER 2 ERROR: " << buffer->zs[i] << " != " << test2 << std::endl;
      exit(0);
    }
    int16_t test3 = static_cast<int16_t>(static_cast<double>(test2)/4.0);
    if (buffer->mwd[i] != test3){
      std::cout << "BUFFER 3 ERROR: " << buffer->mwd[i] << " != " << test3 << std::endl;
      exit(0);
    }
  }    
}

// FOR TESTING ONLY
void TestFuncs::doNothing(std::shared_ptr<DataStruct>& buffer){}


// FOR TESTING ONLY
void TestFuncs::print_packets(std::shared_ptr<DataStruct>& buffer){

  int packet_num = 10;
  int print_num = 50;

  if (buffer->buffer_num == 0){  
    std::cout << "Buffer number = " << buffer->buffer_num << "\n";
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Printing first " << print_num << " elements of first " << packet_num << " packets..." << std::endl;
    for (int i = 0; i < print_num; i ++){
      std::cout << i << "  ";
      for (int j = 0; j < packet_num; j ++){
	int index = j*MAX_PACKET_LEN + i;
	std::cout << HEX((buffer->raw[index] & 0xFFFF),2) << "  ";
      }
      std::cout << "\n";
    }

  }
    
}


// FOR TESTING ONLY
void TestFuncs::print(std::shared_ptr<DataStruct>& buffer){

  std::cout << "Buffer number = " << buffer->buffer_num << "\n";
  std::cout << "Raw data size = " << buffer->raw_len*sizeof(int16_t) << " bytes\n";
  std::cout << " Packets = " << buffer->packet_count << " (dropped = " << buffer->dropped_packet_count << ")\n";
  // for (int i = 0; i < buffer->dropped_packets.size();i++){
  //   std::cout << "  Dropped packet range = " << buffer->dropped_packets[i].first << ":" << buffer->dropped_packets[i].second << "\n";
  // }
  std::cout << " Events = " << buffer->EWT_count << " (lost = " << buffer->lost_EWT_count << ")\n";
  // for (int i = 0; i < buffer->lost_EWTs.size();i++){
  //   std::cout << "  Lost EWT range = " << buffer->lost_EWTs[i].first << ":" << buffer->lost_EWTs[i].second << "\n";
  // }
  for (int i = 0; i < 5;i++){
    for (int j = 0; j < 10;j++){
      std::cout << buffer->raw[i*10+j] << " ";
    }
    std::cout << "\n";
  }
    
  

  
}


// FOR TESTING ONLY
void TestFuncs::check_form_events(std::shared_ptr<DataStruct>& buffer){

  bool print = false;
  
  size_t max = 0;
  for (int i = 0; i < buffer->raw_header_num; i++){
    std::tuple raw = buffer->raw_header_map[i];
    if (print){
      if (i == 0 or i == 1 or i == buffer->raw_header_num-2 or i == buffer->raw_header_num-1){
	std::array<int16_t,eHdr_Len> hdr = std::get<hdrMap_hdrData>(raw);
	if (i == 0) std::cout << "---" << std::endl;
    	std::cout << std::get<hdrMap_EWT>(raw)
		  << "  " << std::get<hdrMap_adcIndex>(raw)
		  << "  " << std::get<hdrMap_dataLen>(raw)
		  << "  " << std::get<hdrMap_eventStart>(raw)
		  << "  " << std::get<hdrMap_eventLen>(raw)
		  << "  " << hdr[eHdr.EvStart]
		  << "  " << hdr[eHdr.EvInPacket]
		  << "  " << hdr[eHdr.EvLen];
	std::cout << "\n";
	if (i == buffer->raw_header_num-1) std::cout << "---" << std::endl;
      }
    }
    max += std::get<hdrMap_dataLen>(raw);
  }
  if (max != buffer->raw_len){    
    std::cout << "ERROR: Buffer header data count = " << std::to_string(max) << " != " << " buffer->raw_len = " << std::to_string(buffer->raw_len) << std::endl;
    std::cout << "EWT range = " << std::get<hdrMap_EWT>(buffer->raw_header_map[0])
	      << " --> "
	      << std::get<hdrMap_EWT>(buffer->raw_header_map[buffer->raw_header_num-1])
	      << std::endl;
    //exit(0);
  }
  
  for (int i = 0; i < buffer->raw_len; i++){
    if (first_counter){
      counter_check = buffer->raw[i];
      first_counter = false;
    }
    if (buffer->raw[i] != counter_check){
      int start = i-10;
      if (start < 0) start -= start;
      for (int j = start; j < i + 10; j++){
  	std::cout << j << " " << counter_check+j << " " << buffer->raw[j] << std::endl;
      }
      std::cout << i << "/" << buffer->raw_len << " ERROR in TestFuncs::check_form_events, incrementing counter = " << counter_check << ", data = " << buffer->raw[i] << std::endl;
      break;
      //    exit(0);
    }
    counter_check++;
  }


  
  
}

// FOR TESTING ONLY
void TestFuncs::check_prep_zs(std::shared_ptr<DataStruct>& buffer){
  
  // if (buffer->buffer_num < 3){  
  //   std::cout << "Buffer number = " << buffer->buffer_num << "\n";
  //   std::cout << "-------------------------------" << std::endl;    
  //   for (int i = 0; i < buffer->zs_header_map.size(); i++){
  //     std::tuple zs = buffer->zs_header_map[i];
  //     std::array<int16_t,eHdr_Len> zs_hdr = std::get<hdrMap_hdrData>(zs);
  //     if (i < buffer->raw_header_num){
  // 	std::tuple raw = buffer->raw_header_map[i];
  // 	std::array<int16_t,eHdr_Len> raw_hdr = std::get<hdrMap_hdrData>(raw);
  // 	std::cout << "raw: " << std::get<hdrMap_EWT>(raw)
  // 		  << "  " << std::get<hdrMap_adcIndex>(raw)
  // 		  << "  " << std::get<hdrMap_dataLen>(raw)
  // 		  << "  " << raw_hdr[eHdr.EvStart]
  // 		  << "  " << raw_hdr[eHdr.EvInPacket]
  // 		  << "  " << raw_hdr[eHdr.EvLen]
  // 		  << ",  zs: " << std::get<hdrMap_EWT>(zs)
  // 		  << "  " << std::get<hdrMap_adcIndex>(zs)
  // 		  << "  " << std::get<hdrMap_dataLen>(zs)
  // 		  << "  " << zs_hdr[eHdr.EvStart]
  // 		  << "  " << zs_hdr[eHdr.EvInPacket]
  // 		  << "  " << zs_hdr[eHdr.EvLen];
  //     }
  //     else{
  // 	std::cout << "raw: " << "  -  "
  // 		  << "  " << "  -  "
  // 		  << "  " << "  -  "
  // 		  << "  " << "  -  "
  // 		  << "  " << "  -  "
  // 		  << "  " << "  -  "
  // 		  << ",  zs: " << std::get<hdrMap_EWT>(zs)
  // 		  << "  " << std::get<hdrMap_adcIndex>(zs)
  // 		  << "  " << std::get<hdrMap_dataLen>(zs)
  // 		  << "  " << zs_hdr[eHdr.EvStart]
  // 		  << "  " << zs_hdr[eHdr.EvInPacket]
  // 		  << "  " << zs_hdr[eHdr.EvLen];
  //     }
  //     // std::cout << " " << hdr[eHdr.EvStart] << " " << hdr[eHdr.EvInPacket] << " " << hdr[eHdr.EvLen];
  //     std::cout << "\n";
  //   }

  // }
  // for (int i = 0; i < 5; i++){    
  //   std::cout << i << " " << buffer->raw[i] << " " << buffer->zs[i] << std::endl;
  // }
  // for (int i = 0; i < 5; i++){
  //   int index = 5-i;
  //   std::cout << i << " " << buffer->raw[buffer->raw_len-index] << " " << buffer->zs[buffer->zs_len-index] << std::endl;
  // }
  
  // for (int i = 0; i < 200; i++){    
  //   std::cout << i << " " << buffer->raw[i] << " " << buffer->zs[i] << std::endl;
  // }
  //     std::cout << i << "/" << buffer->raw_len << " ERROR in TestFuncs::check_form_events, incrementing counter = " << counter_check << ", data = " << buffer->raw[i] << std::endl;
  //     exit(0);
  //   }
  //   counter_check++;
  // }
  
}

void TestFuncs::check_zs(std::shared_ptr<DataStruct>& buffer){

  // The ZS grad window (CHECK IS SAME AS CONFIG FILE)
  const int window = 100;
  
  // The adc length
  const uint64_t n = buffer->raw_len;

  // Define a pointer to the adc data
  const std::vector<int16_t>* data = &buffer->raw;

  // The data index
  uint64_t i = 0;

  int low_print_count = 0;
  int high_print_count = 0;
  const int print_max = 10;

  // std::cout << "\nBUFFER = " << buffer_num << std::endl;
  // std::cout << "----------------" << std::endl;
  // std::cout << "n = " << n << ", buffer->zs_overflow_num = " << buffer->zs_overflow_num << ", n-buffer->zs_overflow_num = " << n-buffer->zs_overflow_num << std::endl;
  // std::cout << "START (i = " << i << ") low: check = " << low_check << ", data = " << (*data)[i] << "; high: check = " << high_check << ", data = " << (*data)[i+window] << std::endl;
  
  // Loop over all ADC values
  for (int i = 0; i < n-buffer->zs_overflow_num; i++){
    if (first_counter){
      low_check = (*data)[i];
      high_check = (*data)[i+window];
      first_counter = false;
    }
    if (low_check_failed) low_check = (*data)[i];
    if (high_check_failed) high_check = (*data)[i];
    // Check low counter
    if ((*data)[i] != low_check){
      low_check_failed = true;
      if (low_print_count < print_max){
	std::cout << "LOW DOESN'T MATCH: i = " << i << ", data = " << (*data)[i] << ", check = " << low_check << std::endl;
	low_print_count++;
      }
      else{	
	break;
      }
    }
    else{
      low_check_failed = false;
    }
    if ((*data)[i+window] != high_check){
      high_check_failed = true;
      if (high_print_count < print_max){
	std::cout << "HIGH DOESN'T MATCH: i = " << i << ", data = " << (*data)[i] << ", check = " << high_check << std::endl;
	high_print_count++;
      }
      else{	
	break;
      }
    }
    else{
      high_check_failed = false;
    }

    //   else
    // // Check high counter
    // if ((*data)[i+window] != high_check){
    //   std::cout << "HIGH DOESN'T MATCH: i = " << i << ", data = " << (*data)[i] << ", check = " << high_check << std::endl;
    //   break;
    // }
    // Increment check counters
    low_check++;
    high_check++;

  }

  i = n-buffer->zs_overflow_num-1;
  // std::cout << "END (i = " << i << ") low: check = " << low_check-1 << ", data = " << (*data)[i] << "; high: check = " << high_check-1 << ", data = " << (*data)[i+window] << std::endl;
  // std::cout << "-----" << std::endl;
  
  // Increase buffer counter
  buffer_num++;

}
