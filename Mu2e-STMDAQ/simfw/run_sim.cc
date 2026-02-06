#include <cstdlib>

// Include config
#include "Mu2e-STMDAQ/config/config.hh"
// Include gen data code
#include "Mu2e-STMDAQ/simfw/gen_data.hh"
// Include UDP code                                            
#include "Mu2e-STMDAQ/processing/udp.hh"

// Consumer thread
void consume(const std::shared_ptr<UDP>& udp,
             const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& queue,
             const std::atomic<bool>& finished){
  
  // The packet number
  size_t packet_num = 0;

  // First call of the infinite loop
  bool first = true;
  
  // Target rate: 5 Gbit/s
  const double target_gbps = 5.0;
  const double target_bps  = target_gbps * 1e9; // bits per second
  
  std::uint64_t total_bytes = 0;
  
  // Start timing
  auto start_time = std::chrono::high_resolution_clock::now();
  
  // while packet generation has not finished
  while (true) {
    // Dummy packet
    std::shared_ptr<std::vector<int16_t>> packet = nullptr;
    
    // Pop packet from queue
    packet = queue->try_pop();
    
    if (packet) {
      // If first successful pop...
      if (first) {
        // Restart timer
        start_time = std::chrono::high_resolution_clock::now();
        first = false;
      }
      
      // Send packet
      udp->send_packet(*packet);
      packet_num++;
      total_bytes += MAX_PACKET_SIZE;
      
      // Throttle every N packets to reduce overhead
      constexpr std::uint64_t CHECK_INTERVAL = 1000;
      if ((packet_num % CHECK_INTERVAL) == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - start_time;
        
        // Total bits sent so far
        double bits_sent = static_cast<double>(total_bytes) * 8.0;
        
        // How long it *should* have taken at target_bps
        double ideal_time  = bits_sent / target_bps;      // seconds
        double actual_time = elapsed.count();             // seconds
        
        if (actual_time < ideal_time) {
          double sleep_sec = ideal_time - actual_time;
          if (sleep_sec > 0.0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_sec));
          }
        }
      }
    } else {
      // No packet: if packet generation has finished, exit loop
      if (finished.load(std::memory_order_acquire)) {
        break;
      }
      
      // Optional: small sleep to avoid busy-polling an empty queue
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  }
  
  // // while packet generation has not finished
  // while (true){
  //   // Dummy packet
  //   std::shared_ptr<std::vector<int16_t>> packet = nullptr;    
  //   // Pop packet from queue
  //   packet = queue->try_pop();
  //   // Increase the packet number                                                                        
  //   if (packet) {
  //     // If first successful pop...
  //     if (first) {
  //       // Restart timer
  //       start_time = std::chrono::high_resolution_clock::now();
  //       first = false;
  //     }
  //     // Send packet
  //     udp->send_packet(*packet);
  //     packet_num++;
  //     //      std::this_thread::sleep_for(std::chrono::nanoseconds(10)); // Mimic firmware speed
  //   }    
  //   // Else if no packet
  //   else{
  //     // If packet generation has finished
  //     if (finished){
  //       break;
  //     }
  //   }
  // }

  // End timing
  auto end_time = std::chrono::high_resolution_clock::now();

  // Calculate elapsed time
  std::chrono::duration<double> elapsed = end_time - start_time;
  // Convert bytes to Gbits
  double gbytes = packet_num*MAX_PACKET_SIZE / 1e9;
  double gbits = gbytes * 8.0;

  // Print sending speed
  std::cout << "Sent "
            << std::fixed << std::setprecision(2)
            << gbytes << " Gbytes ("
            << packet_num << " packets) at "
            << std::fixed << std::setprecision(2)
            << (gbits / elapsed.count()) << " Gbit/s" << std::endl;
  
}

// Main function
int main(int argc, char* argv[]){

  // Load the configuration                                               
  std::string xml_path = EnvVars::expand("${STM_XML}");
  Config& cfg = Config::getInstance(xml_path);

  // Initialise the STM data information
  std::shared_ptr<STMdata> stm = std::make_shared<STMdata>(cfg,nullptr);

  // Initialise the signal handler                                        
  std::shared_ptr<SignalHandler> signal = std::make_shared<SignalHandler>(nullptr,nullptr);
  
  // Argument variables
  size_t event_num = 0;
  size_t event_len = 0;
  std::string filename = "";

  // Print to user
  std::cout << "Executing packet generation programme with the following " << argc<< " inputs: " << std::endl;
  // If the accepted number of arguments
  if (argc == 3 or argc == 4){
    event_num = std::stoull(argv[1], nullptr, 0); // Get event number
    event_len = std::stoull(argv[2], nullptr, 0); // Get event lengh
    if (argc == 4) filename = argv[3];
    std::cout << "Number of events = " << event_num << "." << std::endl;
    std::cout << "Event length = " << event_len << " ns." << std::endl;
    if (argc == 3){
      std::cout << "Incrememting counter ADC data will be generated." << std::endl;
    }
    else{
      std::cout << "Input binary file = " << filename << "." << std::endl;
    }      
  }
  else{
    std::cout << "Incorrrect argument input. Please re-run with the following list:" << std::endl;
    std::cout << "./run.exe [number of events] [event length (ns)] [input binary file (OPTIONAL)]" << std::endl;
    return 0;
  }
  
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
  
  std::cout << "Acquiring data and storing in memory..." << std::endl;
  
  // Generate data
  std::shared_ptr<GenData> gen = std::make_shared<GenData>(event_num,event_len,filename);

  // Create ring buffer queue for packets
  std::shared_ptr<BufferQueue<std::vector<int16_t>>> queue
    = std::make_shared<BufferQueue<std::vector<int16_t>>>(10000);

  // Create UDP client to send packets
  std::shared_ptr<UDP> udp = std::make_shared<UDP>(nullptr,stm,signal,true);
  
  // Create boolean to say generation finished
  std::atomic<bool> finished = false;

  // Consumer thread
  std::thread consumer_thread(consume, std::ref(udp), std::ref(queue), std::ref(finished));
  
  // Wait for user to continue
  std::cout << "Press any key to send data...";
  std::cin.get();
  // Ignore the newline character left in the input buffer
  std::cin.ignore();
  std::cout << "Sending data..." << std::endl;

  // Generate packet data
  gen->gen_packets(queue,signal,finished);

  // Join thread
  consumer_thread.join();
  
  return 0;
  
}

 
