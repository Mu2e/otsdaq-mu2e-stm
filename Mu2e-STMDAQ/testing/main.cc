// Test functions
#include "test_funcs.hh"

// Gen data
#include "genData.hh"

// !!!! CHANGE THESE ONLY !!! //
// Number of heartbeats generated in memory
//static const uint32_t numberhb = 0x3AF61C; // NOT MORE THAN 0xFFFF if 0x30D4
static const uint32_t numberhb = 0xFFFF; // NOT MORE THAN 0xFFFF if 0x30D4
// Heartbeat length (multiple of 8ns) 
//static const uint32_t deltahb = 0xD4;
static const uint32_t deltahb = 0x30D4;
// How many times to send this data on repeat
//static const uint32_t send_num = 11641;
static const uint32_t send_num = 1;
// !!!!!!!!!!!!!!!!!!!!!!!!!! //0

// Instance of UDP socket class
UDPsocket udp;
  
// Generate data
genData gen(numberhb,deltahb);
//genData gen("/data1/claudia_simulation_binary_files/662keV_0.32mV/genData662keV_20kHz.bin");

// Instance of test functions class
test_funcs testing;

// The number of packets to send
uint64_t packetNum = 0;

// Define packet arrays for each channel
int16_t* send_packets;

// Initialise events
uint64_t event_array_len = {};
int16_t *event_data;

// Main function
int main(){
  
  std::cout << "Generating data and storing in memory..." << std::endl;
  
  // Generate events
  std::pair<uint64_t,int16_t*> events = gen.genEvents(0,numberhb);      
  // Store event array length
  event_array_len = events.first;
  // Store event data
  event_data = events.second;
  // Generate packets from events
  std::pair<uint64_t,int16_t*> packets = gen.genPackets(0,event_data,numberhb);
  // Store number of packets
  packetNum = packets.first;
  // Store packets to send
  send_packets = packets.second;

  std::cout << "Press any key to continue...";
  std::cin.get();
  std::cout << "Continuing..." << std::endl;

  // Ignore the newline character left in the input buffer
  std::cin.ignore();
  
  // Print message about data to be sent
  std::cout << "Generated " << packetNum*MAX_PACKET_SIZE*1e-9 << " Gbytes as " << packetNum << " packets of size " << MAX_PACKET_SIZE*1e-3 << " kbytes" << std::endl;

  std::cout << "Sending this " << send_num<< " times = " << packetNum*send_num*MAX_PACKET_SIZE*1e-9 << " Gbytes as " << packetNum*send_num << " packets" << std::endl;
  
    // Send client socket (mimics fw)
  int send_sock = udp.setupClient();
  
  // Start send thread
  std::thread send_thread(&test_funcs::client,
  			  std::ref(testing),
  			  std::ref(udp),
  			  send_sock,
  			  send_packets,
  			  packetNum,
  			  numberhb,
  			  send_num);

  // Join thread
  send_thread.join();

  return 0;
  
}

 
