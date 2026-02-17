#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Queue buffer header
#include "STMDAQ-TestBeam/utils/queue_write.hh"

// Constructor 
queue_write::queue_write(std::string date_time, uint64_t buffer_len) {

  // Set the file date and time
  file_date_time = date_time;

  // Get the maximum number of datagrams to pull based on the data type 
  write_max = UDPsocket::RECVMMSG_NUM;

  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    write[chan].store(0);
    read[chan].store(0);
    occupancy[chan].store(0);
    RING_BUFFER_LEN = buffer_len;
    buffer[chan] = new int16_t[RING_BUFFER_LEN];      
    write_data[chan] = new int16_t[write_max];
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

}

// Try to push data to queue
uint64_t queue_write::try_push(int chan, int16_t *data, int n){  
  // Initialise the push values
  uint64_t push_size = 0;
  uint64_t push_len = 0;

  // Load the current tail
  current_tail[chan] = write[chan].load();

  // Load the amount in buffer
  write_num[chan] = occupancy[chan].load();

  // Get the available space in the buffer
  uint64_t space = RING_BUFFER_LEN - write_num[chan];

  // If space is zero
  if(space <= n){
    // Return zero
    return push_len;
  }
  // Else if n > space
  else {
    // Push only avaliable space
    push_len = n;
  }

  push_size = push_len*sizeof(int16_t);

  // Copy to data buffer
  // If the increase wraps the data buffer around
  if(increment(current_tail[chan],push_len) < current_tail[chan]){
    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_LEN - current_tail[chan];
    // Memcpy to the end of buffer
    memcpy(&buffer[chan][current_tail[chan]],
 	   data,end*sizeof(int16_t));
     // Memcpy the rest to the start of the buffer
    memcpy(&buffer[chan][0],
	   &data[end],
	   (push_len-end)*sizeof(int16_t));
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy all datagrams
    memcpy(&buffer[chan][current_tail[chan]],
 	   data,push_size);
  }

  // Store the updated occupancy numbers
  occupancy[chan].fetch_add(push_len);
  
  // Update tail pointer
  write[chan].store(increment(current_tail[chan],push_len));

  // Return push_len
  return push_len;
}
  
// Push data to queue
void queue_write::push(int chan, int16_t *data, int n){   
  
  // If no packets to push, exit
  if (n == 0) return;
  
  // pull_len
  uint64_t push_len = 0;
  
  // Wait until pushed..
  while(push_len == 0) {
    // Try push
    push_len = try_push(chan,data,n);
  };
}

// Try to write data
uint64_t queue_write::try_write_file(int chan){

  // Initialise the pull values
  uint64_t pull_size = 0;
  uint64_t pull_len = 0;
  
  // Load the current head pointers
  current_head[chan] = read[chan].load();

  // Get number to read in buffer
  read_num[chan] = occupancy[chan].load();

  //std::cout << "Trying to pull: " << read_num[chan] << " events from queue \n";

  // Get current subrun                                                   
  current_subrun[chan] = subrun[chan].load();
  // If nothing to read in the buffer
  if (read_num[chan] == 0){    
    // Return unsuccesful pull
    return read_num[chan];
  }
  // Get pull length and pull size
  pull_len = read_num[chan];
  pull_size = pull_len*sizeof(int16_t);
  
  // Get subrun buffer number                                             
  subrun_buff_index[chan] = current_subrun[chan] % file_num;
  // If a new file for this subrun is yet to be created                   
  if (new_file[chan][subrun_buff_index[chan]]){
    // Return unsuccesful write to file                                   
    std::cout << "try_write_file: New file for subrun " 
	      << subrun_buff_index[chan] 
	      << " is yet to be created" << std::endl;
    return 0;
  }

  // Make sure we don't try and write more than the push_max
  if(pull_len > write_max) pull_len = write_max;

  // If the increase wraps the buffer around
  if(increment(current_head[chan],pull_len) < current_head[chan]){ 

    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_LEN - current_head[chan];

    // Write data from end of buffer
    bf[chan][subrun_buff_index[chan]]
      .write((char *)&buffer[chan][current_head[chan]],
    	     end*sizeof(int16_t));
    
    // Write data from start of buffer
    bf[chan][subrun_buff_index[chan]]
      .write((char *)&buffer[chan][0],
    	     (pull_len-end)*sizeof(int16_t));
    
  }
  // Else if if it doesn't wrap around
  else{
    
    // Write data from end of buffer
    bf[chan][subrun_buff_index[chan]]
      .write((char *)&buffer[chan][current_head[chan]],
    	     pull_len*sizeof(int16_t));
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_sub(pull_len);

  // Store the updated read pointer
  read[chan].store(increment(current_head[chan],pull_len));

  // Increase tota data written                                           
  total_data[chan] += pull_len*sizeof(int16_t);

  // If the write count equals the number of writes per file              
  if (total_data[chan] >= max_bf_size){

    // Close file                                                         
    bf[chan][subrun_buff_index[chan]].close();

    // Signal a new file is to be created in this subrun buffer slot      
    new_file[chan][subrun_buff_index[chan]].store(true);
    
    // Increment subrun counter                                           
    subrun[chan].fetch_add(1);
    
    // Reset total data
    total_data[chan] = 0;

  }
  
  // Return successful write
  return pull_len;
}

// Write data from queue
void queue_write::write_file(bool *timeout, int chan){
  
  int pull_len = 0;
  
  // Wait until...
  while( !*timeout ) {
    try_write_file(chan);
  };
  
  if (*timeout and (write[chan].load() - current_head[chan]) != 0){
    cout << "ERROR: Still data to be written!!!!" << endl;
    exit(0);
  }
  

}

// Get a new file name                                                    
std::string queue_write::get_file_name(int chan, int subrun){

  // ADD TO XML 
  // Open binary file                                                     
  std::string filename = data_dir
    //    +file_date_time+"_"+(chan == 0 ? "HPGe" : "LaBr")+"_"
    +file_date_time+"_ch"+std::to_string(chan)+"_"
    + std::to_string(subrun) + ".bin";

  // Return filename                                                      
  return filename;

}

// Monitor and close/open binary files                                    
void queue_write::monitor_files(int chan){

  // Loop over files in binary file buffer                                
  for (uint file = 0; file < file_num; file++){
    // If signal for a new file to be opened                              
    if (new_file[chan][file].load()){
      // Get file name                      
      std::string file_name = get_file_name(chan,
					    (current_subrun_buff[chan]+1)\
					    *file_num + file);
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

// Monitor files to open/close
void queue_write::write_monitor_func(int chan, UDPsocket &udp){
  while(!udp.timeout) monitor_files(chan);

}

// Write data to file                                                           
void queue_write::write_data_func(int chan, UDPsocket &udp){

  // Infinte loop                          
  while(1){
    // Try to write file   
    try_write_file(chan);
    // If exit has been signalled
    if (udp.timeout){
      // Break infinite loop
      break;
    }
  }

}

