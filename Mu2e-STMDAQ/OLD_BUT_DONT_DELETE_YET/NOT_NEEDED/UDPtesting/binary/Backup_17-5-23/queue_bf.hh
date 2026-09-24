#ifndef QUEUE_BF_hh
#define QUEUE_BF_hh

#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <string>
#include <algorithm>
#include <fstream>

// Ring buffer size
#define BUFFER_SIZE 32  // power of 2 for efficient %
//#define BUFFER_SIZE 4  // power of 2 for efficient %

// PACKET
#define P_SIZE 8198
#define P_LEN 4099

// #define P_SIZE 4
// #define P_LEN 2

class queue_bf_buffer{

public :

  // Constructor
  queue_bf_buffer(){

    // For each channel, set read and write pointers to zero
    for (int chan = 0; chan < channels; chan++){
      write[chan].store(0);
      read[chan].store(0);
    }

    // std::cout << "max_bf_size  = " << max_bf_size << " bytes" << std::endl;
    // std::cout << "bf_size  = " << bf_size << " bytes" << std::endl;
    // std::cout << "bf_len  = " << bf_len << std::endl;
    // std::cout << "bf_packet_num  = " << bf_packet_num << std::endl;
    
  }

  // Try to push data to queue_buffer
  bool try_push(int chan, int16_t* data){
    
    // Get the current write pointer
    const_cast<uint64_t&>(current_tail[chan]) = write[chan].load();

    // Increment the next write pointer
    const_cast<uint64_t&>(next_tail[chan]) = increment(current_tail[chan]);

    // If the next write pointer != the current read pointer
    if (next_tail[chan] != read[chan].load()){
      
      // Get buffer index to write data to
      uint loc = push_packet_count[chan]*P_LEN;
      // Add data to write_buffer
      memcpy(write_buffer[chan][current_tail[chan]] + loc,
	     data,P_SIZE);

      //      std::cout << "Pushed packet " << push_packet_count[chan] << " / " << bf_packet_num << " to index " << loc << " in buffer " << current_tail[chan] << std::endl;
      
      // Increment packet pointer
      push_packet_count[chan]++;

      // If reached the maximum number of packets to write out
      if (push_packet_count[chan] == bf_packet_num){
      
	// Reset the updated current pointer
	push_packet_count[chan] = 0;
	
	// Store the updated write pointer
	write[chan].store(next_tail[chan]);

      }

      // Return succesful push
      return true;

    }

    // Return unsuccesful push
    return false;  

  }
  
  // Push data to queue_buffer
  void push(int chan, int16_t* data){
    
    // Try to push data to queue_buffer
    while( ! try_push(chan,data) ){};

  }
  
  // Try to write data to file
  bool try_write_file(int chan){
    
    // Get current read pointer
    current_head[chan] = read[chan].load();

    // If the current read pointer == write pointer
    if (current_head[chan] == write[chan].load()){
      // Return unsuccesful write to file
      return false;
    }

    // Open binary file
    std::string filename = "channel_" + std::to_string(chan)
      + "_subrun" + std::to_string(subrun_count[chan]) + ".bin";
    std::ofstream bf(filename, std::ios::out | std::ios::binary);

    // Write data to file
    bf.write((char *)write_buffer[chan][current_head[chan]],max_bf_size);

    // Increase tota data written
    total_data += max_bf_size;
    
    // Close file
    bf.close();

    // Store the updated read pointer
    read[chan].store(increment(current_head[chan]));

    // Increment subrun counter
    subrun_count[chan]++;

    // Return succeesful pull
    return true;

  }
  
  // Write the portion of the final subrun that doesn't fill
  // the maximum file size
  void write_end(int chan){

    // If packets have been pushed to a buffer
    // that hasn't been written...
    if (push_packet_count[chan] != 0){

      // Get the size of data to write
      uint64_t write_size = push_packet_count[chan]*P_SIZE;     

      // Open binary file
      std::string filename = "channel_" + std::to_string(chan)
	+ "_subrun" + std::to_string(subrun_count[chan]) + ".bin";
      std::ofstream bf(filename, std::ios::out | std::ios::binary);
      //      std::ofstream test[2];
      
    // Write data to file
      bf.write((char *)write_buffer[chan][current_head[chan]],write_size);

      // Increase tota data written
      total_data += max_bf_size;
            
      // Close file
      bf.close();

    }

    std::cout << "Total data written to file = " << total_data*1e-9 << " Gb split over " << subrun_count[chan] << " files" << std::endl; 
    
  }
  
  // Write data to file
  void write_file(int chan, bool exitFE){

    // Wait until...

    //    while( ! try_write_file(chan) ) {};
    while( !exitFE ) {
      try_write_file(chan);
    };

    write_end(chan);
    
  }

  uint64_t get_bf_packet_num(){
    return bf_packet_num;
  }
  
private :

  // Maximum number of queue_buffers 
  static const int channels = 2;

  // Atomic write pointer
  std::atomic<uint64_t> write[channels];
  // Atomic read pointer
  std::atomic<uint64_t> read[channels];
  
  // Current tail pointer
  uint64_t current_tail[channels] = {};
  // Next tail pointer
  uint64_t next_tail[channels] = {};
  // Current head pointer
  uint64_t current_head[channels] = {};

  // Pushed packet counter
  uint64_t push_packet_count[channels] = {};

  // Subrun counter
  uint64_t subrun_count[channels] = {};
  
  // The buffer size
  static const uint64_t size = BUFFER_SIZE;

  // Total data written
  uint64_t total_data = 0;  

  // The binary file size size
  // NEED TO FIGURE OUT HOW TO WRITE 2GB FILES
  // ARRAY IS TOO LARGE FOR MEMORY ALLOCATION
  static const uint64_t max_bf_size = uint64_t(2e7); // 2 Gb
  //  static const uint64_t max_bf_size = 8; // 2 Gb
  static const uint64_t bf_size = int(max_bf_size/P_SIZE)*P_SIZE;
  static const uint64_t bf_len = bf_size/2;
  static const uint64_t bf_packet_num = bf_size/P_SIZE;
  
  // The bf write buffer
  int16_t write_buffer[channels][BUFFER_SIZE][bf_len] = {{{}}};

  // Increment function
  uint64_t increment(int n){
    return (n + 1) % size;
  }

};

#endif
 

