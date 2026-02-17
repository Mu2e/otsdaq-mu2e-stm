#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <unistd.h>

// Queue buffer header
#include "queue.hh"

struct tester {
  uint16_t x;
  uint16_t y;
  
    // default + parameterized constructor
    tester(int x=0, int y=0) 
        : x(x), y(y)
    {
    }
  
};

// assignment operator modifies object, therefore non-const
tester& operator+=(tester& a,const tester& b)
{
  a.x += b.x;
  a.y += b.y;
  return a;
  
}

// Constructor 
queue_buffer::queue_buffer() {
  
  // Check that the data buffer length isn't larger than an int32_t max
  if (RING_BUFFER_LEN > INT32_T_MAX){
    std::cout << "CRITICAL ERROR. Queue buffer length cannot be more than " << INT32_T_MAX << std::endl;
    exit(0);
  }

  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    // Initialise tail pointers
    tail_[chan].store({0,0,0,0});
    // Initialise head pointers
    head_[chan].store({0,0,0,0});
    // Initialise occupancy pointers
    occupancy_size[chan].store(0);
    occupancy_data[chan].store(0);
    // Initialise buffers
    size_buffer[chan] = new uint64_t [RING_BUFFER_NUM];
    //   header_buffer[chan] = new packet_header [RING_BUFFER_NUM];
    data_buffer[chan] = new int16_t [RING_BUFFER_LEN];

  }

}

// Try to push data to queue
int queue_buffer::try_push(int chan, 
			   uint64_t** sizes, int16_t *data, 
			   int n, int index){  
  
  // Initialise the push values
  uint64_t push_num = 0;
  uint64_t push_size = 0;
  uint64_t push_len = 0;
  uint64_t start_index = 0;
  uint64_t end_index = 0;

  // Load the current tail pointers
  current_tail[chan] = tail_[chan].load();
  // Current size buffer pointer
  current_tail_size[chan] = make_int32_t(current_tail[chan].size_0,
					 current_tail[chan].size_1);
  // Current data buffer pointer
  current_tail_data[chan] = make_int32_t(current_tail[chan].data_0,
					 current_tail[chan].data_1);  

  // Load the amount in buffer
  write_num_size[chan] = occupancy_size[chan].load();

  // Get the available space in the buffer
  uint64_t space = RING_BUFFER_NUM - write_num_size[chan];

  // If space is zero
  if(space == 0){
    // Return zero
    return push_num;

  }
  // Else if n < space
  else if(n < space){
    // Push all n
    push_num = n;
  }
  // Else if n > space
  else if(n > space){
    // Push only avaliable space
    push_num = space;
  }

  // Get data index, push size and push length
  start_index = sizes[ACC_SIZE_INDEX][index] 
    - sizes[SIZE_INDEX][index];
  push_size = sizes[ACC_SIZE_INDEX][index+push_num-1] 
    - sizes[ACC_SIZE_INDEX][index]
    + sizes[SIZE_INDEX][index];  
  push_len = push_size/2;

  // Copy to size buffer
  // If the increase wraps the size buffer around
  if(increment(current_tail_size[chan],push_num,RING_BUFFER_NUM) < current_tail_size[chan]){
    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_NUM - current_tail_size[chan];
    // Memcpy to the end of buffer
    memcpy(&size_buffer[chan][current_tail_size[chan]],
  	   &sizes[SIZE_INDEX][index],
  	   end*sizeof(uint64_t));
     // Memcpy the rest to the start of the buffer
    memcpy(&size_buffer[chan][0],
  	   &sizes[SIZE_INDEX][index+end],
  	   (push_num-end)*sizeof(uint64_t));
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy all datagrams
    memcpy(&size_buffer[chan][current_tail_size[chan]],
  	   &sizes[SIZE_INDEX][index],push_num*sizeof(uint64_t));
  }

  // Copy to data buffer
  // If the increase wraps the data buffer around
  if(increment(current_tail_data[chan],push_len,RING_BUFFER_LEN) < current_tail_data[chan]){
    // Get the distance to the end of the buffer
    uint64_t end = RING_BUFFER_LEN - current_tail_data[chan];
    // Memcpy to the end of buffer
    memcpy(&data_buffer[chan][current_tail_data[chan]],
 	   &data[start_index],
	   end*sizeof(int16_t));
     // Memcpy the rest to the start of the buffer
    memcpy(&data_buffer[chan][0],
	   &data[start_index+end],
	   (push_len-end)*sizeof(int16_t));
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy all datagrams
    memcpy(&data_buffer[chan][current_tail_data[chan]],
 	   &data[start_index],push_size);
  }
  
  // Store the updated occupancy numbers
  occupancy_data[chan].fetch_add(push_len);
  occupancy_size[chan].fetch_add(push_num);

  // Get new current size buffer pointer values
  current_tail_size[chan] = increment(current_tail_size[chan],push_num,RING_BUFFER_NUM);
  int16_t size0 = current_tail_size[chan] & 0xFFFF;
  int16_t size1 = current_tail_size[chan] >> 16;;

  // Get new current data buffer pointer
  current_tail_data[chan] = increment(current_tail_data[chan],push_len,RING_BUFFER_LEN); 
  int16_t data0 = current_tail_data[chan] & 0xFFFF;
  int16_t data1 = current_tail_data[chan] >> 16;;
  
  // Update tail pointer
  tail_[chan].store({size0,size1,data0,data1});

  // Return push_num
  return push_num;

}
  
// Push data to queue
void queue_buffer::push(int chan, uint64_t** sizes, int16_t *data, int n){
  
  // We want to push n datagrams
  int push_num = n;
  
  // While datagrams to push > 0
  while(push_num > 0) {
    // Find the index in the array to push from
    int index = n - push_num;    
    // Push, return the amount pushed and reclculate 
    // how many left to push
    push_num -= try_push(chan,sizes,data,push_num,index);
 };
    
}
  
// Try to pull data from queue
int queue_buffer::try_pull(int chan, uint64_t*& sizes, int16_t *&data){

  // Initialise the pull values
  uint64_t pull_num = 0;
  uint64_t pull_size = 0;
  uint64_t pull_len = 0;

  // Load the current head pointers
  current_head[chan] = head_[chan].load();
  // Current size buffer pointer
  current_head_size[chan] = make_int32_t(current_head[chan].size_0,
					 current_head[chan].size_1);
  // Current data buffer pointer
  current_head_data[chan] = make_int32_t(current_head[chan].data_0,
					 current_head[chan].data_1);  

  // Get number to read in buffer
  read_num_size[chan] = occupancy_size[chan].load();
  read_num_data[chan] = occupancy_data[chan].load();

  // Get the available data in the buffer
  pull_num = read_num_size[chan];

  // If nothing to read in the buffer
  if (pull_num == 0){
    // Return unsuccesful pull
    return pull_num;
  }
  // Get pull length and pull size
  pull_len = read_num_data[chan];
  pull_size = pull_len*sizeof(int16_t);

  // Copy from size buffer
  // If the increase wraps the buffer around
  if(increment(current_head_size[chan],pull_num,RING_BUFFER_NUM) < current_head_size[chan]){ 

    // Get the distance to the end of the buffer     
    uint64_t end = RING_BUFFER_NUM - current_head_size[chan];
  
    // Memcpy from end of buffer
    memcpy(sizes,&size_buffer[chan][current_head_size[chan]],end*sizeof(uint64_t));
    // Memcpy from the start of the buffer
    memcpy(&sizes[end],&size_buffer[chan][0],(pull_num-end)*sizeof(uint64_t));
  
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy all datagrams
    memcpy(sizes,&size_buffer[chan][current_head_size[chan]],pull_num*sizeof(uint64_t));
  }

  // Copy from data buffer  
  // If the increase wraps the buffer around
  if(increment(current_head_data[chan],pull_len,RING_BUFFER_LEN) < current_head_data[chan]){ 

    // Get the distance to the end of the buffer     
    uint64_t end = RING_BUFFER_LEN - current_head_data[chan];
    
    // Memcpy from end of buffer
    memcpy(data,&data_buffer[chan][current_head_data[chan]],end*sizeof(int16_t));
    // Memcpy from the start of the buffer
    memcpy(&data[end],&data_buffer[chan][0],(pull_len-end)*sizeof(int16_t));
    
  }
  // Else if if it doesn't wrap around
  else{
    // Memcpy all datagrams
    memcpy(data,&data_buffer[chan][current_head_data[chan]],pull_size);
  }
  
  // Store the updated number in buffer
  occupancy_data[chan].fetch_sub(pull_len);
  occupancy_size[chan].fetch_sub(pull_num);

  // Get new current size buffer pointer values
  current_head_size[chan] = increment(current_head_size[chan],pull_num,RING_BUFFER_NUM);
  int16_t size0 = current_head_size[chan] & 0xFFFF;
  int16_t size1 = current_head_size[chan] >> 16;;

  // Get new current data buffer pointer
  current_head_data[chan] = increment(current_head_data[chan],pull_len,RING_BUFFER_LEN); 
  int16_t data0 = current_head_data[chan] & 0xFFFF;
  int16_t data1 = current_head_data[chan] >> 16;;

  // Update head pointer
  head_[chan].store({size0,size1,data0,data1});

  // Return successful pull
  return pull_len;
  
}

// Pull data from queue
int queue_buffer::pull(bool *timeout, int chan, uint64_t*& sizes, int16_t *&data){
  
  int retval = 0;
  
  // Wait until...
  while( (retval = try_pull(chan,sizes,data)) == 0 && !*timeout ) {};
  
  // if (*timeout and (write[chan].load() - current_head[chan]) != 0){
  //   std::cout << "ERROR: Still data in queue!!!!" << std::endl;
  //   exit(0);
  // }
  
  // Return pulled data
  return retval;
  
}
  
