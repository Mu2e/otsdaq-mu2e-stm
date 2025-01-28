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
    buffer[chan] = new int16_t [RING_BUFFER_LEN];      
    // pull_max = UDPsocket::RECVMMSG_NUM*UDPsocket::MAX_UDP_LEN;
  }


}

// Try to push data to queue
uint64_t queue_buffer::try_push(int chan, int16_t *data, uint64_t n, uint64_t index){  
  
  // Initialise the return value
  uint64_t retval = 0;

  // Load the current tail
  current_tail[chan] = write[chan].load();

  // Load the amount in buffer
  write_num[chan] = occupancy[chan].load();

  // Get the available space in the buffer
  uint64_t space = RING_BUFFER_LEN - write_num[chan];

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
    uint64_t end = RING_BUFFER_LEN - current_tail[chan];
    
    // Memcpy to the end of buffer
    memcpy(&buffer[chan][current_tail[chan]],
	   &data[index],end*sizeof(int16_t));

    // Memcpy the rest to the start of the buffer
    memcpy(&buffer[chan][0],&data[index+end],(retval-end)*sizeof(int16_t));

  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy retval datagrams
    memcpy(&buffer[chan][current_tail[chan]],
	   &data[index],retval*sizeof(int16_t));
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_add(retval);

  // Update the write pointer
  write[chan].store(increment(current_tail[chan],retval));

  // Return retval
  return retval;
}
  
// Push data to queue
void queue_buffer::push(int chan, int16_t *data, uint64_t n){   
  
  // We want to push n datagrams
  uint64_t retval = n;
  
  // While datagrams to push > 0
  while(retval > 0) {
    // Find the index in the array to push from
    uint64_t index = n - retval;    
    // Push, return the amount pushed and reclculate 
    // how many left to push
    retval -= try_push(chan,data,retval,index);
    std::cout << "Pushed " << n-retval << "/" << n << std::endl;
  };

}
  
// Try to pull data from queue
uint64_t queue_buffer::try_pull(int chan, int16_t *&data){

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
  
  // Make sure we don't try and pull more than the push_max
  if(retval > pull_max) retval = pull_max;

  // If the increase wraps the buffer around
  if(increment(current_head[chan],retval) < current_head[chan]){ 

    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_LEN - current_head[chan];

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
uint64_t queue_buffer::pull(bool *timeout, int chan, int16_t *&data){
  
  uint64_t retval = 0;
  
  // Wait until...
  while( (retval = try_pull(chan,data)) == 0 && !*timeout ) {};
  
  if (*timeout and (write[chan].load() - current_head[chan]) != 0){
    cout << "ERROR: Still data in queue!!!!" << endl;
    exit(0);
  }
  
  // Return pulled data
  return retval;
  
}


