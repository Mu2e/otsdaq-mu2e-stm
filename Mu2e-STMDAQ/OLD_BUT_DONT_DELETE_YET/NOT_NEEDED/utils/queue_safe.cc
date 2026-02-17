#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <unistd.h>

// Queue buffer header
#include "STMDAQ-TestBeam/utils/queue.hh"

// Constructor 
queue_buffer::queue_buffer() {
  
  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    // Initialise tail pointer
    //    tail_[chan].store({0,0,0,0});
    tail[chan].store(0);
    // Initialise head pointers
    //    head_[chan].store({0,0,0,0});
    head[chan].store(0);
    // Initialise occupancy pointer
    occupancy[chan].store(0);
    // occupancy_size[chan].store(0);
    // occupancy_data[chan].store(0);
    // Initialise buffers
    //    size_buffer[chan] = new uint64_t [RING_BUFFER_NUM];
    //   header_buffer[chan] = new packet_header [RING_BUFFER_NUM];
    data_buffer[chan] = new int16_t [RING_BUFFER_LEN];

  }

}

// Try to push data to queue
int queue_buffer::try_push(int chan, int16_t *data, int n){  
  
  // Initialise the push values
  uint64_t push_size = 0;
  uint64_t push_len = 0;

  // Load the current tail pointers
  current_tail[chan] = tail[chan].load();
  // // Current size buffer pointer
  // current_tail_size[chan] = make_int32_t(current_tail[chan].size_0,
  // 					 current_tail[chan].size_1);
  // // Current data buffer pointer
  // current_tail_data[chan] = make_int32_t(current_tail[chan].data_0,
  // 					 current_tail[chan].data_1);  

  // Load the amount in buffer
  write_num[chan] = occupancy[chan].load();

  // Get the available space in the buffer
  uint64_t space = RING_BUFFER_LEN - write_num[chan];

  // If space is zero
  if(space < n){
    std::cout << "Space full" << std::endl;
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
  
  // // Copy to size buffer
  // // If the increase wraps the size buffer around
  // if(increment(current_tail_size[chan],push_num,RING_BUFFER_NUM) < current_tail_size[chan]){
  //   // Get the distance to the end of the buffer
  //   uint64_t end = RING_BUFFER_NUM - current_tail_size[chan];
  //   // Memcpy to the end of buffer
  //   memcpy(&size_buffer[chan][current_tail_size[chan]],
  // 	   &sizes[SIZE_INDEX][index],
  // 	   end*sizeof(uint64_t));
  //    // Memcpy the rest to the start of the buffer
  //   memcpy(&size_buffer[chan][0],
  // 	   &sizes[SIZE_INDEX][index+end],
  // 	   (push_num-end)*sizeof(uint64_t));
  // }
  // // Else if if it doesn't wrap around
  // else{
  //   // Memcpy all datagrams
  //   memcpy(&size_buffer[chan][current_tail_size[chan]],
  // 	   &sizes[SIZE_INDEX][index],push_num*sizeof(uint64_t));
  // }

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
  //  occupancy_size[chan].fetch_add(push_num);

  // // Get new current size buffer pointer values
  // current_tail_size[chan] = increment(current_tail_size[chan],push_num,RING_BUFFER_NUM);
  // int16_t size0 = current_tail_size[chan] & 0xFFFF;
  // int16_t size1 = current_tail_size[chan] >> 16;;

  // // Get new current data buffer pointer
  // current_tail_data[chan] = increment(current_tail_data[chan],push_len,RING_BUFFER_LEN); 
  // int16_t data0 = current_tail_data[chan] & 0xFFFF;
  // int16_t data1 = current_tail_data[chan] >> 16;;
  
  // // Update tail pointer
  // tail_[chan].store({size0,size1,data0,data1});
  tail[chan].store(increment(current_tail[chan],push_len));

  // Return push_len
  return push_len;

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

  // We want to push n datagrams
  int push_num = n;
  
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
  // // Current size buffer pointer
  // current_head_size[chan] = make_int32_t(current_head[chan].size_0,
  // 					 current_head[chan].size_1);
  // // Current data buffer pointer
  // current_head_data[chan] = make_int32_t(current_head[chan].data_0,
  // 					 current_head[chan].data_1);  

  // Get number to read in buffer
  //  read_num_size[chan] = occupancy_size[chan].load();
  read_num[chan] = occupancy[chan].load();

  // // Get the available data in the buffer
  // pull_num = read_num_size[chan];

  // If nothing to read in the buffer
  if (read_num[chan] == 0){    
    // Return unsuccesful pull
    return read_num[chan];//{pull_num,pull_len};
  }
  // Get pull length and pull size
  pull_len = read_num[chan];
  pull_size = pull_len*sizeof(int16_t);

  // // Copy from size buffer
  // // If the increase wraps the buffer around
  // if(increment(current_head_size[chan],pull_num,RING_BUFFER_NUM) < current_head_size[chan]){ 

  //   // Get the distance to the end of the buffer     
  //   uint64_t end = RING_BUFFER_NUM - current_head_size[chan];
  
  //   // Memcpy from end of buffer
  //   memcpy(sizes,&size_buffer[chan][current_head_size[chan]],end*sizeof(uint64_t));
  //   // Memcpy from the start of the buffer
  //   memcpy(&sizes[end],&size_buffer[chan][0],(pull_num-end)*sizeof(uint64_t));
  
  // }
  // // Else if if it doesn't wrap around
  // else{
  //   // Memcpy all datagrams
  //   memcpy(sizes,&size_buffer[chan][current_head_size[chan]],pull_num*sizeof(uint64_t));
  // }

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
  //  occupancy_size[chan].fetch_sub(pull_num);

  // // Get new current size buffer pointer values
  // current_head_size[chan] = increment(current_head_size[chan],pull_num,RING_BUFFER_NUM);
  // int16_t size0 = current_head_size[chan] & 0xFFFF;
  // int16_t size1 = current_head_size[chan] >> 16;;

  // // Get new current data buffer pointer
  // current_head_data[chan] = increment(current_head_data[chan],pull_len,RING_BUFFER_LEN); 
  // int16_t data0 = current_head_data[chan] & 0xFFFF;
  // int16_t data1 = current_head_data[chan] >> 16;;

  // // Update head pointer
  // head_[chan].store({size0,size1,data0,data1});

  // Store the updated read pointer
  head[chan].store(increment(current_head[chan],pull_len));

  // Return successful pull
  //  return {pull_num,pull_len};
  return pull_len;
  
}

// Pull data from queue
uint64_t queue_buffer::pull(bool *timeout, int chan,  int16_t *&data){
  
  // Number of datagrams pulled  
  //  std::pair<uint64_t,uint64_t> pull_num = {0,0};
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
  
