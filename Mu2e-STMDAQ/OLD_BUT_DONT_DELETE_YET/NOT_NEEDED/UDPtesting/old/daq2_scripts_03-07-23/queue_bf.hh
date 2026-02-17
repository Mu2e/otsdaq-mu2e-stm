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
      // Open first set of subrun files for each channel
      for (uint file = 0; file < file_num; file++){
	// Get file name
	std::string file_name = get_file_name(chan,file);
	// Store file name
	bf_name[chan][file] = file_name;
 	// Open file
	bf[chan][file].open(file_name,std::ios::out | std::ios::binary);
      }
    }

    // std::cout << "max_bf_size  = " << max_bf_size << " bytes" << std::endl;
    // std::cout << "bf_size  = " << bf_size << " bytes" << std::endl;
    // std::cout << "bf_len  = " << bf_len << std::endl;
    // std::cout << "write_packet_num  = " << write_packet_num << std::endl;     

  }

  // Get a new file name
  std::string get_file_name(int chan, int subrun){

    // Open binary file
    std::string filename = "data/channel" + std::to_string(chan)
      + "_subrun" + std::to_string(subrun) + ".bin";    

    // Return filename
    return filename;
    
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

      // Increment packet pointers
      push_packet_count[chan]++;

      // If reached the maximum number of packets to write out
      if (push_packet_count[chan] == write_packet_num){
      
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

    // Get current subrun
    current_subrun[chan] = subrun[chan].load();
    
    // Get subrun buffer number
    subrun_buff_index[chan] = current_subrun[chan] % file_num;
    // If a new file for this subrun is yet to be created
    if (new_file[chan][subrun_buff_index[chan]]){
      // Return unsuccesful write to file
      std::cout << "try_write_file: New file for subrun " << subrun_buff_index[chan] << " is yet to be created" << std::endl;
      return false;
    }

    // Write data to file
    bf[chan][subrun_buff_index[chan]]
      .write((char *)write_buffer[chan][current_head[chan]],write_size);
    
    // Increment the write counter
    write_count[chan]++;

    // Store the updated read pointer
    read[chan].store(increment(current_head[chan]));

    // Increase tota data written
    total_data[chan] += write_size; 

    // If the write count equals the number of writes per file
    if (write_count[chan] == writes_per_file){

      // Close file
      bf[chan][subrun_buff_index[chan]].close();
      
      // Signal a new file is to be created in this subrun buffer slot
      new_file[chan][subrun_buff_index[chan]].store(true);

      // Increment subrun counter
      subrun[chan].store(current_subrun[chan]+1);

      // Set write counter to zero
      write_count[chan] = 0;

    }

    // Return succeesful pull
    return true;

  }
  
  // Write the portion of the final subrun that doesn't fill
  // the maximum file size
  void write_end(int chan){

    // Get current subrun
    current_subrun[chan] = subrun[chan].load();

    // Get subrun buffer number
    subrun_buff_index[chan] = current_subrun[chan] % file_num;
    // If a new file for this subrun is yet to be created
    if (new_file[chan][subrun_buff_index[chan]]){
      // Return unsuccesful write to file
      std::cout << "write_end: New file for subrun " << subrun_buff_index[chan] << " is yet to be created" << std::endl;
      exit(0);
    }

    // Get the size of data to write
    uint64_t write_size = push_packet_count[chan]*P_SIZE;     
    
    // Write data to file
    bf[chan][subrun_buff_index[chan]]
      .write((char *)write_buffer[chan][current_head[chan]],write_size);
    
    // Increase tota data written
    total_data[chan] += write_size;
    
    // Close current subrun file
    bf[chan][subrun_buff_index[chan]].close();
        
    // If the push_packet_count ==0, no final data has been written to new subrun file
    // So, readjust subrun number
    if (push_packet_count[chan] == 0) current_subrun[chan]--;

    // Print for user
    std::cout << "Total CHANNEL " << chan 
	      << " data written to file = " << total_data[chan]*1e-9 
	      << " Gb split over " << current_subrun[chan]+1 << " files" << std::endl; 
    
    // Check for empty files (with zero size)
    for (uint file = 0; file < file_num; file++){
      // Get the file name
      std::string file_name = bf_name[chan][file];      
      // Get the file that has already been opened
      std::ifstream f(file_name, std::ifstream::ate | std::ifstream::binary);
      // Get the file size
      int file_size = int(f.tellg());
      // If the file size is zero, delete the empty file
      if (file_size == 0) std::remove(file_name.c_str()); // delete file
    }
    
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

  // Get the number of packets per file
  uint64_t get_packets_per_file(){
    return packets_per_file;
  }

  // Monitor and close/open binary files
  void monitor_files(int chan){
    
    // Loop over files in binary file buffer
    for (uint file = 0; file < file_num; file++){
      // If signal for a new file to be opened
      if (new_file[chan][file]){	  
	// std::cout << "Need to open file for subrun " 
	// 	  << (current_subrun_buff[chan]+1)*file_num + file << std::endl;
	// Get file name
	std::string file_name = get_file_name(chan,
					      (current_subrun_buff[chan]+1)*file_num + file);
	// Store file name
	bf_name[chan][file] = file_name;
	// Open file
	bf[chan][file].open(file_name,std::ios::out | std::ios::binary);
	// Set new_file boolean to false
	new_file[chan][file].store(false);
	// If reached end of file buffer
	if (file == file_num - 1) current_subrun_buff[chan]++;	
      }          
    }
  }
  
  // Signal that the data write is finished
  void signal_write_finished(int chan){
    write_finished[chan] = true;
  }

  // Check whether data write is finished
  bool check_write_finished(int chan){
    return write_finished[chan];
  }
  
private :

  // Maximum number of queue_buffers 
  static const int channels = 2;
  //  static const int channels = 1;

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

  // Packets in file counter
  uint64_t packets_in_file[channels] = {};

  // Subrun counter
  std::atomic<uint64_t> subrun[channels] = {};

  // Current head pointer
  uint64_t current_subrun[channels] = {};
  
  // The buffer size
  static const uint64_t size = BUFFER_SIZE;

  // Total data written
  uint64_t total_data[channels] = {};  

  // The maximum binary file size per subrun
  static const uint64_t max_bf_size = uint64_t(2e9); // 2 Gb
  // The multiple number of writes per file
  static const uint16_t writes_per_file = 100; 
  // The maximum size of data to write per write call
  static const uint64_t max_write_size = max_bf_size/writes_per_file; 
  // The actual write size per call as a integer multiple of packets
  static const uint64_t write_size = int(max_write_size/P_SIZE)*P_SIZE;
  // The write length in number of int16_t values
  static const uint64_t write_len = write_size/2;
  // The number of packets per write
  static const uint64_t write_packet_num = write_size/P_SIZE;
  // The number of packets per binary file
  static const uint64_t packets_per_file = writes_per_file*write_packet_num;

  // A counter of the number of writes per file
  uint16_t write_count[channels] = {};
  
  // The bf write buffer
  int16_t write_buffer[channels][BUFFER_SIZE][write_len] = {{{}}};

  // Increment function
  uint64_t increment(int n){
    return (n + 1) % size;
  }

  // Number of data files to cycle between (the data file ring buffer)
  static const uint file_num = 10;
  
  // The subrun_buff_index for each channel
  int subrun_buff_index[channels] = {};

  // Current head pointer
  uint64_t current_subrun_buff[channels] = {};
  
  // Data files
  std::ofstream bf[channels][file_num];

  // Data file names
  std::string bf_name[channels][file_num];

  // Booleans to signal new data file needed
  std::atomic<bool> new_file[channels][file_num] = {{}};

  // Boolean to signal whether the data write is finished
  bool write_finished[channels] = {};

};

#endif
