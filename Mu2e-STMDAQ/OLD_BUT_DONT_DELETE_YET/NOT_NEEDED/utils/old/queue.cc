#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Queue buffer header
#include "STMDAQ-TestBeam/utils/queue.hh"

// Constructor 
queue_buffer::queue_buffer() {
  
  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    write[chan].store(0);
    read[chan].store(0);
    occupancy[chan].store(0);
    buffer[chan] = new int16_t [RING_BUFFER_SIZE];      
  }
  // // Get the maximum number of datagrams to pull based on the data type 
  // if (is_same<T,UDPsocket::packet>::value){
  //   pull_max = UDPsocket::RECVMMSG_NUM;
  // }
  // else if (is_same<T,dataVars::off_event>::value){
  //   pull_max = UDPsocket::SENDMMSG_NUM;
  // }


}

// Try to push data to queue
int queue_buffer::try_push(int chan, int16_t *data, int n){  
  
  // Initialise the return value
  uint64_t retval = 0;

  // Load the current tail
  current_tail[chan] = write[chan].load();

  // Load the amount in buffer
  write_num[chan] = occupancy[chan].load();

  // Get the available space in the buffer
  uint64_t space = size - write_num[chan];

  // If no space for data to push
  if(space <= n){
    // Return zero
    return retval;
  }
  // Else if space
  else if(space > n){
    // Push all n
    retval = n;
  }

  // If the increase wraps the buffer around
  if(increment(current_tail[chan],retval) < current_tail[chan]){

    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_SIZE - current_tail[chan];
    
    // Memcpy to the end of buffer
    memcpy(&buffer[chan][current_tail[chan]],
	   data,end*sizeof(int16_t));

    // Memcpy the rest to the start of the buffer
    memcpy(&buffer[chan][0],&data[end],(retval-end)*sizeof(int16_t));

  }
  // Else if if it doesn't wrap around
  else{

    // Memcpy retval datagrams
    memcpy(&buffer[chan][current_tail[chan]],
	   data,retval*sizeof(int16_t));
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_add(retval);

  // Update the write pointer
  write[chan].store(increment(current_tail[chan],retval));

  // Return retval
  return retval;
}
  
// Push data to queue
void queue_buffer::push(int chan, int16_t *data, int n){

  // If no packets to push, exit
  if (n == 0) return;

  // retval
  uint64_t retval = 0;
  
  // Wait until pushed..
  while(retval == 0) {
    // Try push
    retval = try_push(chan,data,n);
  };

}
  
// Try to pull data from queue
int queue_buffer::try_pull(int chan, int16_t *&data){

  // Initialise retval
  uint64_t retval = 0;
  
  // Get current read pointer
  current_head[chan] = read[chan].load();

  // Get number to read in buffer
  read_num[chan] = occupancy[chan].load();
  
  // If nothing to read in the buffer
  if ((retval = read_num[chan]) == 0){
    // Return unsuccesful pull
    return retval;
  }
  
  // If the increase wraps the buffer around
  if(increment(current_head[chan],retval) < current_head[chan]){ 

    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_SIZE - current_head[chan];

    // Memcpy from end of buffer
    memcpy(data,&buffer[chan][current_head[chan]],end*sizeof(int16_t));
    // Memcpy from the start of the buffer
    memcpy(&data[end],&buffer[chan][0],(retval-end)*sizeof(int16_t));
    
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy retval datagrams
    memcpy(data,&buffer[chan][current_head[chan]],retval*sizeof(int16_t));
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_sub(retval);

  // Store the updated read pointer
  read[chan].store(increment(current_head[chan],retval));

  // Return successful pull
  return retval;
  
}

// Pull data from queue
int queue_buffer::pull(bool *timeout, int chan, int16_t *&data){
  
  int retval = 0;
  
  // Wait until...
  while( (retval = try_pull(chan,data)) == 0 && !*timeout ) {};
  
  if (*timeout and (write[chan].load() - current_head[chan]) != 0){
    std::cout << "ERROR: Still data in queue!!!!" << std::endl;
    exit(0);
  }
  
  // Return pulled data
  return retval;
  
}
  
