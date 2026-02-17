#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Queue buffer header
#include "queue.hh"

// Constructor 
template<typename T> queue_buffer<T>::queue_buffer() {
  
  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    write[chan].store(0);
    read[chan].store(0);
    occupancy[chan].store(0);
    buffer[chan] = new T [RING_BUFFER_SIZE];      
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
template<typename T> int queue_buffer<T>::try_push(int chan, T *data, 
						   int n, int index){  
  
  // Initialise the return value
  int retval=0;

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
template<typename T> void queue_buffer<T>::push(int chan, T *data, int n){   
  
  // We want to push n datagrams
  int retval = n;
  
  // While datagrams to push > 0
  while(retval > 0) {
    // Find the index in the array to push from
    int index = n - retval;    
    // Push, return the amount pushed and reclculate 
    // how many left to push
    std::cout << "Attemping to push " << retval << std::endl;
    retval -= try_push(chan,data,retval,index);
  };

}
  
// Try to pull data from queue
template<typename T> int queue_buffer<T>::try_pull(int chan, T *&data){

  // Initialise retval
  int retval = 0;
  
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
    int end = RING_BUFFER_SIZE - current_head[chan];

    // Memcpy from end of buffer
    memcpy(data,&buffer[chan][current_head[chan]],end*sizeof(data[0]));
    // Memcpy from the start of the buffer
    memcpy(&data[end],&buffer[chan][0],(retval-end)*sizeof(data[0]));
    
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy retval datagrams
    memcpy(data,&buffer[chan][current_head[chan]],retval*sizeof(data[0]));
  }
  
  // Store the updated number in buffer
  occupancy[chan].fetch_sub(retval);

  // Store the updated read pointer
  read[chan].store(increment(current_head[chan],retval));

  // Return successful pull
  return retval;
}

// Pull data from queue
template<typename T> int queue_buffer<T>::pull(bool *timeout, int chan, T *&data){
  
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
  
// Instantiate UDPsocket::packet queue class
template class queue_buffer<int16_t>;
