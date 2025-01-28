#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <unistd.h>

// Queue buffer header
#include "STMDAQ-TestBeam/utils/queue.hh"

// Constructor 
queue_buffer::queue_buffer(uint64_t buffer_len) {
  
  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    // Initialise tail pointer
    tail[chan].store(0);
    // Initialise head pointer
    head[chan].store(0);
    // Initialise occupancy pointer
    occupancy[chan].store(0);
    // Initialise buffers
    RING_BUFFER_LEN = buffer_len;
    process_buffer[chan] = new int16_t [RING_BUFFER_LEN] ();
    data_buffer[chan] = new int16_t [RING_BUFFER_LEN] ();

  }

}

// Try to push data to queue
uint64_t queue_buffer::try_push(int chan, int16_t *data, uint64_t n){  
  
  // Initialise the push values
  uint64_t push_size = 0;
  uint64_t push_len = 0;

  // Load the current tail pointer
  current_tail[chan] = tail[chan].load();

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
  
  // Get data index, push size and push length
  push_size = push_len*sizeof(int16_t);
  
  // Copy to data buffer
  // If the increase wraps the data buffer around
  if(increment(current_tail[chan],push_len) < current_tail[chan]){
    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_LEN - current_tail[chan];
    // Memcpy to the end of buffer
    memcpy(&data_buffer[chan][current_tail[chan]],
 	   data,end*sizeof(int16_t));
     // Memcpy the rest to the start of the buffer
    memcpy(&data_buffer[chan][0],
	   &data[end],
	   (push_len-end)*sizeof(int16_t));
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy all datagrams
    memcpy(&data_buffer[chan][current_tail[chan]],
 	   data,push_size);
  }
  
  // Store the updated occupancy numbers
  occupancy[chan].fetch_add(push_len);
  
  // Update tail pointer
  tail[chan].store(increment(current_tail[chan],push_len));

  // Return push_len
  return push_len;

}
  
// Push data to queue
void queue_buffer::push(int chan, int16_t *data, uint64_t n){
  
  // If no packets to push, exit
  if (n == 0) return;
  
  // retval
  uint64_t retval = 0;
  
  // Wait until pushed..
  while(retval == 0) {
    // Try push
    retval = try_push(chan,data,n);
  };

  // We want to push n datagrams
  //  int push_num = n;
  
  // // While datagrams to push > 0
  // while(push_num > 0) {
  //   // Find the index in the array to push from
  //   int index = n - push_num;    
  //   // Push, return the amount pushed and reclculate 
  //   // how many left to push
  //   push_num -= try_push(chan,sizes,data,push_num,index);
  // };
    
}
  
// Try to pull data from queue
uint64_t queue_buffer::try_pull(int chan, int16_t *&data){

  // Initialise the pull values
  uint64_t pull_size = 0;
  uint64_t pull_len = 0;

  // Load the current head pointers
  current_head[chan] = head[chan].load();

  // Get number to read in buffer
  read_num[chan] = occupancy[chan].load();

  // If nothing to read in the buffer
  if (read_num[chan] == 0){    
    // Return unsuccesful pull
    return read_num[chan];
  }
  // Get pull length and pull size
  pull_len = read_num[chan];
  pull_size = pull_len*sizeof(int16_t);

  // Copy from data buffer  
  // If the increase wraps the buffer around
  if(increment(current_head[chan],pull_len) < current_head[chan]){ 

    // Get the distance to the end of the buffer     
    uint64_t end = RING_BUFFER_LEN - current_head[chan];
    
    // Memcpy from end of buffer
    memcpy(data,&data_buffer[chan][current_head[chan]],end*sizeof(int16_t));
    // Memcpy from the start of the buffer
    memcpy(&data[end],&data_buffer[chan][0],(pull_len-end)*sizeof(int16_t));
    
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy all datagrams
    memcpy(data,&data_buffer[chan][current_head[chan]],pull_size);
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_sub(pull_len);

  // Store the updated read pointer
  head[chan].store(increment(current_head[chan],pull_len));

  // Return successful pull
  return pull_len;
  
}

// Pull data from queue
uint64_t queue_buffer::pull(bool *timeout, int chan,  int16_t *&data){
  
  // Number of datagrams pulled  
  uint64_t pull_len = 0;
  // Wait until...
  while( (pull_len = try_pull(chan,data)) == 0 && !*timeout ) {};
  
  // if (*timeout and (write[chan].load() - current_head[chan]) != 0){
  //   std::cout << "ERROR: Still data in queue!!!!" << std::endl;
  //   exit(0);
  // }
  
  // Return pulled data
  return pull_len;
  
}
  
