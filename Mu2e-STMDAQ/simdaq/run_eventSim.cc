// Include config
#include "Mu2e-STMDAQ/config/config.hh"
// Include gen data code
#include "Mu2e-STMDAQ/simdaq/gen_eventData.hh"
// Include gen data code
#include "Mu2e-STMDAQ/simdaq/send_TCP.hh"


// Ring buffer queue sizes
constexpr size_t queue_size = 100000;

//Send TCP class
std::shared_ptr<SendTCP> hdr_TCP;

bool writeEvent = 1;
bool writeRaw = 1;
bool writeZS = 1;
bool writePH = 1;


bool sendTCP = 0;


// Sender thread
void send_thread(const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& input_queue,
	     const std::atomic<bool>& add_headers_finished,size_t nevents){
        

  // The buffer number
  size_t buffer_num = 0;

  // Total size
  double gbytes = 0;
  
  // First call of the infinite loop
  bool first = true;
  
  // Start timing
  auto start_time = std::chrono::high_resolution_clock::now();
  
  // While adding headers has not finished and there is still data in input queue
  while (!add_headers_finished || !input_queue->is_empty()){    
    // Dummy buffer
    std::shared_ptr<std::vector<int16_t>> buffer = nullptr;    
    // Pop buffer from queue
    buffer = input_queue->try_pop();
    if (buffer) {
      // If first successful pop...
      if (first) {
	// Restart timer1
	start_time = std::chrono::high_resolution_clock::now();
	first = false;
      }
      

      // Reference event_data
      auto& event = *buffer;


      //write to file in event format with headers
      if (writeEvent == 1){

          const std::string filename = "data.bin";
          std::ofstream file(filename, std::ios::binary);
          if (!file.is_open()) {
           throw std::runtime_error("Failed to open output file: " + filename);
                  }


      for (int i = 0; i < event.size(); i++){
         file.write(reinterpret_cast<const char*>(&event[i]), sizeof(int16_t));          
	 //std::cout<< i << " : "<<event[i]<<std::endl;
         }
        file.close();
      }
      //event write


      //write only raw binary data, no headers, for input into DAQ
      if (writeRaw == 1){
	  int rawlen = event[16];


	  const std::string filename = "data_raw.bin";
	  std::ofstream file(filename, std::ios::binary);
	  if (!file.is_open()) {
 	   throw std::runtime_error("Failed to open output file: " + filename);
		  }

      for (int i = 21; i < rawlen+21; i++){
         file.write(reinterpret_cast<const char*>(&event[i]), sizeof(int16_t));
         //std::cout<< i << " : "<<event[i]<<std::endl;
         }
        file.close();
      }
      //raw write


      //write ZS, with zs headers 
      if (writeZS == 1){
	      int zslen = event[18]+2*event[17];
	      int zsstartindex = 21+event[16];

          const std::string filename = "data_ZS.bin";
          std::ofstream file(filename, std::ios::binary);
          if (!file.is_open()) {
           throw std::runtime_error("Failed to open output file: " + filename);
                  }

      for (int i = zsstartindex; i < zsstartindex+zslen; i++){
         file.write(reinterpret_cast<const char*>(&event[i]), sizeof(int16_t));
         }
      }
      //ZS write


      //write out pulse times and heights
      if (writePH == 1){
              int phlen = event[19]*2;
              int phstartindex = 21+event[16]+event[18]+2*event[17];

          const std::string filename = "data_PH.bin";
          std::ofstream file(filename, std::ios::binary);
          if (!file.is_open()) {
           throw std::runtime_error("Failed to open output file: " + filename);
                  }

      for (int i = phstartindex; i < phstartindex+phlen; i++){
         file.write(reinterpret_cast<const char*>(&event[i]), sizeof(int16_t));
	 std::cout<<"PH: "<<event[i]<<std::endl;
         }
      }
      //PH write

   
}
    // Else if no buffer
    else{
      continue;
    }
  }

  // End timing
  auto end_time = std::chrono::high_resolution_clock::now();

  // Close the file after loop
  //file.close();
  
  // Calculate elapsed time
  std::chrono::duration<double> elapsed = end_time - start_time;
  // Convert bytes to Gbits
  double gbits = gbytes * 8.0;
/*
  // Print sending speed
  std::cout << "Sent "
	    << std::fixed << std::setprecision(2)
	    << gbytes << " Gbytes ("
	    << buffer_num << " buffers) at "
	    << std::fixed << std::setprecision(2)
	    << (gbits / elapsed.count()) << " Gbit/s" << std::endl;
*/  
}

// Add headers thread
void hdr_TCP_thread(const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& input_queue,
		const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& output_queue,
		const std::atomic<bool>& gen_events_finished,
		std::atomic<bool>& add_headers_finished){
  
  // The buffer number
  size_t buffer_num = 0;

  // Total size
  double gbytes = 0;

  // First call of the infinite loop
  bool first = true;
  
  // Start timing
  auto start_time = std::chrono::high_resolution_clock::now();
  
  // While event generation has not finished and there is still data in input queue
  while (!gen_events_finished || !input_queue->is_empty()){
    // Dummy buffer
    std::shared_ptr<std::vector<int16_t>> buffer = nullptr;    
    // Pop buffer from queue
    buffer = input_queue->try_pop();
    if (buffer) {
      // If first successful pop...
      if (first) {
	// Restart timer1
	start_time = std::chrono::high_resolution_clock::now();
	first = false;
      }
      // Call function to add headers to buffer

    
      hdr_TCP->send_TCP(buffer);
      // Push buffer to output queue
      output_queue->push(buffer);
      // Increase the buffer number
      buffer_num++;
//      gbytes += buffer_num*buffer->raw_len / 1e9;      
    }
    // Else if no buffer
    else{
      continue;
    }
  }

  // Tell sender thread that adding headers is finished
  add_headers_finished = true;
  
  // End timing
  auto end_time = std::chrono::high_resolution_clock::now();

  // Calculate elapsed time
  std::chrono::duration<double> elapsed = end_time - start_time;
  // Convert bytes to Gbits
  double gbits = gbytes * 8.0;
/*
  // Print sending speed
  std::cout << "Add headers processed "
	    << std::fixed << std::setprecision(2)
	    << gbytes << " Gbytes ("
	    << buffer_num << " buffers) at "
	    << std::fixed << std::setprecision(2)
	    << (gbits / elapsed.count()) << " Gbit/s" << std::endl;
*/  
}

// Main function
int main(int argc, char* argv[]){

  //size of ZS slices
  int adcCountsAroundPeakZS = 20;

  // Load the configuration                                               
  std::string xml_path = EnvVars::expand("${STM_XML}");
  Config& cfg = Config::getInstance(xml_path);

  // Initialise the STM data information
  const std::shared_ptr<STMdata> stm = std::make_shared<STMdata>(cfg,nullptr);

  // Initialise the signal handler                                        
  std::shared_ptr<SignalHandler> signal = std::make_shared<SignalHandler>(nullptr,nullptr);
  
  // Argument variables
  size_t event_num = 0;
  size_t event_len = 0;
  size_t prescale = 0;
  
  // Print to user
  std::cout << "Executing STMDAQ data simulation with the following " << argc<< " inputs: " << std::endl;

  if (argc != 4){
    std::cout << "Incorrrect argument input. Please re-run with the following list:" << std::endl;
    std::cout << "./build/bin/simdaq.exe [number of events] [event length (ns)] [raw_prescale]" << std::endl;
    return 0;
  }

  // If the accepted number of arguments
  event_num = std::stoull(argv[1], nullptr, 0); // Get event number
  event_len = std::stoull(argv[2], nullptr, 0); // Get event lengh
  prescale = std::stoull(argv[3], nullptr, 0); // Get prescale value
  std::cout << "Number of events = " << event_num << "." << std::endl;
  std::cout << "Event length = " << event_len << " ns." << std::endl;
  std::cout << "Raw prescale value = " << prescale << "." << std::endl;
  
/*  
  // Infinite loop for user inut
  while (true) {
    char input;
    std::cout << "Continue? Input Y or N to continue: ";
    std::cin >> input;
    if (input == 'y' or input == 'Y') {	  
      std::cout << "Continuing..." << std::endl;
      break;
    }
    else if (input == 'n' or input == 'N') {
      std::cout << "You chose not to continue. Exiting..." << std::endl;
      return 0;
    }
    else {
      std::cout << "Invalid input. Please enter Y or N." << std::endl;
      // Clear the fail state and discard the rest of the input line
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }
*/
 
  int CHAN = 0;

  // Add headers
  hdr_TCP = std::make_shared<SendTCP>(cfg,stm,CHAN,event_num,sendTCP);
  
  // Generate data
  std::shared_ptr<GenEvents> gen = std::make_shared<GenEvents>(stm,event_num,event_len,prescale,adcCountsAroundPeakZS);

  // Create ring buffer queue for adding headers
  std::shared_ptr<BufferQueue<std::vector<int16_t>>> queue_gen_to_hdr
    = std::make_shared<BufferQueue<std::vector<int16_t>>>(queue_size);

  // Create ring buffer queue for sending data
  std::shared_ptr<BufferQueue<std::vector<int16_t>>> queue_hdr_to_send
    = std::make_shared<BufferQueue<std::vector<int16_t>>>(queue_size);
    
  // Create boolean to say generation finished
  std::atomic<bool> gen_events_finished = false;

  // Create boolean to say add headers finished
  std::atomic<bool> add_headers_finished = false;

  // Add header thread
  std::thread header_thread(hdr_TCP_thread, std::ref(queue_gen_to_hdr), std::ref(queue_hdr_to_send), std::ref(gen_events_finished), std::ref(add_headers_finished));
  
  // Sender thread
  std::thread sender_thread(send_thread, std::ref(queue_hdr_to_send), std::ref(add_headers_finished),event_num);
  
// std::thread consumer_thread(consume, std::ref(udp), std::ref(queue), std::ref(finished));

  // Wait for user to continue
  //std::cout << "Press any key to send data...";
  //std::cin.get();
  // Ignore the newline character left in the input buffer
  std::cin.ignore();
  std::cout << "Sending data..." << std::endl;

  // Generate packet data
  gen->gen_events(queue_gen_to_hdr,signal,gen_events_finished);

  // Join thread
  header_thread.join();
  sender_thread.join();
  
  return 0;
  
}

 
