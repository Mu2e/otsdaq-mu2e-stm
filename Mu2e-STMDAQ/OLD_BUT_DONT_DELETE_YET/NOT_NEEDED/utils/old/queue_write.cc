#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Queue buffer header
#include "STMDAQ-TestBeam/utils/queue_write.hh"

// Constructor 
template<typename T> queue_write<T>::queue_write(std::string date_time) {

  // Set the file date and time
  file_date_time = date_time;

  // Get the maximum number of datagrams to pull based on the data type 
  // if (is_same<T,UDPsocket::packet>::value){
  //   write_max = UDPsocket::RECVMMSG_NUM;
  // }
  // else if (is_same<T,dataVars::off_event>::value){
  //   write_max = UDPsocket::SENDMMSG_NUM;
  // }  
  
  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    write[chan].store(0);
    read[chan].store(0);
    occupancy[chan].store(0);
    buffer[chan] = new T [RING_BUFFER_SIZE];      
    write_data[chan] = new T [write_max];
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
template<typename T> int queue_write<T>::try_push(int chan, T *data, 
						   int n, int index){  
  
  // Initialise the return value
  int retval = 0;

  // Load the current tail
  current_tail[chan] = write[chan].load();

  // Load the amount in buffer
  write_num[chan] = occupancy[chan].load();

  // Get the available space in the buffer
  int space = RING_BUFFER_SIZE - write_num[chan];

  // If space is zero
  if(space == 0){
    // Return zero
    return retval;
  }
  // Else if n < space
  else if(n < space){
    // Push all n
    retval = n;
  }
  // Else if n > space
  else if(n > space){
    // Push only avaliable space
    retval = space;
  }

  // If the increase wraps the buffer around
  if(increment(current_tail[chan],retval) < current_tail[chan]){

    // Get the distance to the end of the buffer
    int end = RING_BUFFER_SIZE - current_tail[chan];
    
    // Memcpy to the end of buffer
    memcpy(&buffer[chan][current_tail[chan]],
	   &data[index],end*sizeof(data[0]));

    // Memcpy the rest to the start of the buffer
    memcpy(&buffer[chan][0],&data[index+end],(retval-end)*sizeof(data[0]));

  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy retval datagrams
    memcpy(&buffer[chan][current_tail[chan]],
	   &data[index],(retval*sizeof(data[0])));
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_add(retval);

  // Update the write pointer
  write[chan].store(increment(current_tail[chan],retval));

  // Return retval
  return retval;
}
  
// Push data to queue
template<typename T> void queue_write<T>::push(int chan, T *data, int n){   
  
  // We want to push n datagrams
  int retval = n;
  
  // While datagrams to push > 0
  while(retval > 0) {
    // Find the index in the array to push from
    int index = n - retval;    
    // Push, return the amount pushed and reclculate 
    // how many left to push
    retval -= try_push(chan,data,retval,index);
  };

}
  
// Try to write data
template<typename T> int queue_write<T>::try_write_file(int chan){

  // Initialise retval
  int retval = 0;
  
  // Get current read pointer
  current_head[chan] = read[chan].load();

  // Get number to read in buffer
  read_num[chan] = occupancy[chan].load();

  // Get current subrun                                                   
  current_subrun[chan] = subrun[chan].load();
  
  // If nothing to read in the buffer
  if ((retval = read_num[chan]) == 0){
    // Return unsuccesful write
    return retval;
  }
  
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
  if(retval > write_max) retval = write_max;

  // If the increase wraps the buffer around
  if(increment(current_head[chan],retval) < current_head[chan]){ 

    // Get the distance to the end of the buffer
    int end = RING_BUFFER_SIZE - current_head[chan];

    // Write data from end of buffer
    bf[chan][subrun_buff_index[chan]]
      .write((char *)&buffer[chan][current_head[chan]],
    	     end*sizeof(buffer[chan][current_head[chan]]));
    
    // Write data from start of buffer
    bf[chan][subrun_buff_index[chan]]
      .write((char *)&buffer[chan][0],
    	     (retval-end)*sizeof(buffer[chan][0]));
    
  }
  // Else if if it doesn't wrap around
  else{
    
    // Write data from end of buffer
    bf[chan][subrun_buff_index[chan]]
      .write((char *)&buffer[chan][current_head[chan]],
    	     retval*sizeof(buffer[chan][current_head[chan]]));
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_sub(retval);

  // Store the updated read pointer
  read[chan].store(increment(current_head[chan],retval));

  // Increase tota data written                                           
  total_data[chan] += retval*sizeof(buffer[chan][current_head[chan]]);

  // If the write count equals the number of writes per file              
  if (total_data[chan] >= max_bf_size){

    // Close file                                                         
    bf[chan][subrun_buff_index[chan]].close();

    // Signal a new file is to be created in this subrun buffer slot      
    new_file[chan][subrun_buff_index[chan]].store(true);
    
    // Increment subrun counter                                           
    subrun[chan].fetch_add(1);
    //    subrun[chan].store(current_subrun[chan]+1);
    
    // Reset total data
    total_data[chan] = 0;

  }
  
  // Return successful write
  return retval;
}

// Write data from queue
template<typename T> void queue_write<T>::write_file(bool *timeout, int chan){
  
  int retval = 0;
  
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
template<typename T> std::string 
queue_write<T>::get_file_name(int chan, int subrun){

  // Open binary file                                                     
  std::string filename = "/home/stm_mu2e/STMDAQ-TestBeam/data/"
    +file_date_time+"_"+(chan == 0 ? "HPGe" : "LaBr")+"_"
    + std::to_string(subrun) + ".bin";

  // Return filename                                                      
  return filename;

}

// Monitor and close/open binary files                                    
template<typename T> void queue_write<T>::monitor_files(int chan){

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
template<typename T> void queue_write<T>::write_monitor_func(int chan, UDPsocket &udp,
							      queue_write<T> &writeq){
  while(!udp.timeout) monitor_files(chan);

}

// Write data to file                                                                                    
template<typename T> void queue_write<T>::write_data_func(int chan, UDPsocket &udp,
							  queue_write<T> &writeq){

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

  
// // Instantiate UDPsocket::packet queue class
// template class queue_write<UDPsocket::packet>;
// Instantiate dataVars::event queue class
template class queue_write<dataVars::event>;
// Instantiate dataVars::event queue class
template class queue_write<dataVars::off_event>;
