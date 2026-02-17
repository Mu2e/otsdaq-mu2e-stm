// Test functions
#include "test_funcs.hh"

//Standard constructor
test_funcs::test_funcs() {}

// Firmware trigger header struct from dataVars.hh
fw_tHdr tHdr2;

// Data variables class
dataVars dv;

// Client function to send packets
void test_funcs::client(UDPsocket &udp , const int socket, int16_t* data, const uint32_t number_hb){

  // Notify user
  std::cout << "In send thread:  socket " << socket << std::endl;

  // Create send buffer vector
  std::vector<int16_t> snd_buffer[UDPsocket::SENDMMSG_NUM][MAX_PACKET_LEN];
  // Initialise with zeors
  std::fill
  
  // Get start time before packet sending
  auto start = std::chrono::steady_clock::now();

  // First send?
  bool first_send = true;

  // The event window tag
  uint64_t EWT = 0;

  // The send number
  size_t send_count = 0;
  
  // Infinite loop
  while(1){
   
    // Create new packet with data
    {snd_buffer[packet_num % UDPsocket::SENDMMSG_NUM],EWT} = gen_packet(packet_num,data);
    // Increment the packet number
    packet_num++;

    // If reach last eWT
    if (EWT == number_hb){
      // Send remaining buffer
      int retval = udp.send(snd_buffer,size_buffer,msgNum,socket);
      // Increase counter by number of sent messages
      send_count += msgNum;
    }

    
    // Check if SENDMMMSG_NUM has been reached
    if (packet_num % UDPsocket::SENDMMSG_NUM == 0){
      // Sleep to mimic fw sending
      // this_thread::sleep_for(5ms);
      // Send buffer
      int retval = udp.send(snd_buffer,size_buffer,UDPsocket::SENDMMSG_NUM,socket);
        // Increase counter by number of sent messages
      send_count += UDPsocket::SENDMMSG_NUM;
      // Reset message counter
      msgNum = 0;
    }
    // Send any leftover packets at end of loop
    else if (i == to_send-1){
      // Send remaining buffer
      int retval = udp.send(snd_buffer,size_buffer,msgNum,socket);
        // Increase counter by number of sent messages
      send_count += msgNum;
    }
  }

  // Get end time after packet sending
  auto end = std::chrono::steady_clock::now();
  // Find time taken to send all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = std::chrono::duration <double, std::nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = send_count*MAX_PACKET_SIZE*1e-9;

  // Store speed as Gbits/s
  double sendSpeed = data_size*8/(timeNano*1e-9);

  // Notify user
  std::cout << "Sent " << send_count
	    << " packets [" << data_size
	    << " Gbytes] at "<< sendSpeed << " Gbit/s" << std::endl;
  
}

