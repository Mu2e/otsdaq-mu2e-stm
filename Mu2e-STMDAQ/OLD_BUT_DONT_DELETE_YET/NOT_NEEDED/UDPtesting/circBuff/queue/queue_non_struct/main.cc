#include <time.h>
#include <chrono>
#include <math.h>

// Queue buffer header
#include "queue.hh"

queue_buffer queue;

static const uint64_t packet_num = 2*queue_buffer::RING_BUFFER_NUM + 10;

static const int push_max = 10000;

static const int chnum = 1;

// Push to queue thread function
void push(int chan, int16_t (*data)[packet_len]){
  
  std::cout << "In push thread" << std::endl;

  // Start timing clock
  auto start = std::chrono::steady_clock::now();
  
  // The return value
  int retval = 0;    

  // Data buffer
  int16_t *data_buffer = new int16_t [push_max*packet_len] ();
  // Datagram size buffer (size, accumulated size, start location)
  uint64_t **size_buffer;
  size_buffer = new uint64_t *[2];
  size_buffer[queue_buffer::SIZE_INDEX] 
    = new uint64_t [push_max] (); // size
  size_buffer[queue_buffer::ACC_SIZE_INDEX] 
    = new uint64_t [push_max] (); // accumulated size
  
  // Store size, accumulated size, and start location of datagrams to buffer
  for (int i = 0; i < push_max; i++){
    size_buffer[queue_buffer::SIZE_INDEX][i] 
      = packet_len*sizeof(int16_t); // size
    size_buffer[queue_buffer::ACC_SIZE_INDEX][i] 
      = (i+1)*packet_len*sizeof(int16_t); // accumulated size
  }
  
  // While retval < than the number of packets
  while(retval < (packet_num)){                    
    // Get a random number of packets to push
    int push_num = 1 + (rand() % push_max);         
    // If the retval + the number to push is greater than
    // the maximum number of packets, only push remainder
    if ((retval + push_num) > packet_num) push_num = packet_num - retval;
    // Get the size to push
    uint push_size = push_num*packet_size;
    uint push_len = push_size/2;
    // Memcpy the data to the buffer
    memcpy(data_buffer,&data[retval],push_size);
    // Push the data to the buffer
    queue.push(chan,size_buffer,data_buffer,push_num);  
    // Update the retval
    retval += push_num;        
  }      

  // End timing clock
  auto end = std::chrono::steady_clock::now();

  // Find time taken to push packets
  auto diff = end - start;
  // Get time in nanseconds
  double timeNano = std::chrono::duration <double, std::nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = packet_num*packet_size*1e-9;
  // Calculate speed as Gbits/s
  double speed = data_size*8/(timeNano*1e-9);  

  std::cout << "Pushed packets at " << speed << " Gbit/s" << std::endl;

}

// Pull from queue thread function
void pull(int chan, uint64_t *sizes, int16_t *data){

  bool b = false;
  bool* timeout = &b;
  
  std::cout << "In pull thread" << std::endl;

  // Start timing clock
  auto start = std::chrono::steady_clock::now();
  
  // The return value
  int retval = 0;    

  // The pull count
  uint64_t pull_count = 0;
  
  // Datagram size buffer 
  uint64_t *size_buffer = new uint64_t [queue_buffer::RING_BUFFER_NUM] ();
  // Push buffer
  int16_t *data_buffer = new int16_t [queue_buffer::RING_BUFFER_LEN] ();

  // While still data to pull
  while(pull_count < (uint64_t)(packet_num)*(uint64_t)packet_len){
    retval = queue.pull(timeout,chan,size_buffer,data_buffer);
    memcpy(&data[pull_count],data_buffer,retval*sizeof(int16_t));
    memcpy(&sizes[pull_count/packet_len],
	   size_buffer,
	   retval/packet_len*sizeof(uint64_t));
    pull_count += retval;
   }

  // End timing clock
  auto end = std::chrono::steady_clock::now();

  // Find time taken to push packets
  auto diff = end - start;
  // Get time in nanseconds
  double timeNano = std::chrono::duration <double, std::nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = packet_num*packet_size*1e-9;
  // Calculate speed as Gbits/s
  double speed = data_size*8/(timeNano*1e-9);  
  
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "Pulled packets at " << speed << " Gbit/s" << std::endl;

}

// Pointer/memcpy test function
int test(){

  static const int num = 4;
  static const int buffnum = 2;
  static const int len = 2;

  int16_t (*push_arr)[len] = new int16_t [num][len]();

  push_arr[0][0] = 1;
  push_arr[0][1] = 2;
  push_arr[1][0] = 3;
  push_arr[1][1] = 4;
  push_arr[2][0] = 5;
  push_arr[2][1] = 6;
  push_arr[3][0] = 7;
  push_arr[3][1] = 8;

  int16_t *pull_arr = new int16_t [num*len] ();

  int count = 0;
  for (int i = 0; i < num; i++){
    for (int j = 0; j < len; j++){
    std::cout << push_arr[i][j] << "   " <<  pull_arr[count] << std::endl;
    count++;
    }
  }
 
  memcpy(pull_arr,push_arr,num*len*sizeof(int16_t));

  for (int i = 0; i < num*len; i++) std::cout << pull_arr[i] << std::endl;
  std::cout << "-----------" << std::endl;
  
  int16_t (*buff_arr)[len] = new int16_t [buffnum][len]();
  for (int i = 0; i < num/buffnum; i++){
    memcpy(buff_arr,&push_arr[i*buffnum],buffnum*len*sizeof(int16_t));
    for (int j = 0; j < buffnum; j++){
      for (int k = 0; k < len; k++){
  	std::cout << buff_arr[j][k] << std::endl;
      }
    }
  }

  std::cout << "-----------" << std::endl;

  int16_t *dpt[num];
  for (int i = 0; i < num; i++){
    dpt[i] = new int16_t [len] ();
    for (int j = 0; j < len; j++){
      dpt[i][j] = push_arr[i][j];
      std::cout << dpt[i][j] << std::endl;
    }
  }

  std::cout << "-----------" << std::endl;
  memcpy(push_arr,&dpt,num*len*sizeof(int16_t));
 
  //  pull_arr = &dpt[0][0]x;

  count = 0;
  for (int i = 0; i < num; i++){
    for (int j = 0; j < len; j++){
      std::cout << dpt[i][j] << "   " <<  push_arr[i][j] << std::endl;
      count++;
    }
  }


  return 0;
  
}

// Main function
int main(){

  // Packets to push
  int16_t (*push_packets)[packet_len] = new int16_t [packet_num][packet_len]();

  // Generate random data
  std::cout << "Generating random data..." << std::endl;
  // Loop over number of packets
  for(int i = 0; i < packet_num; i++){
    // Loop over packet length
    for(int j = 0; j < packet_len; j++){
      // Generate random number
      int16_t random = 1 + (rand() % packet_len);
      // Add to packet
      push_packets[i][j] = random;
    }
  }
  std::cout << "Data generation complete!" << std::endl;
  
  // Get pull buffers
  uint64_t * size_data = new uint64_t[packet_num] ();
  int16_t * pull_data = new int16_t[packet_num*packet_len] ();

  // Start push thread
  std::thread *push_thread = new std::thread(push,0,push_packets);

  // Start pull thread
  std::thread *pull_thread = new std::thread(pull,0,size_data,pull_data);

  // Join threads
  push_thread->join();
  pull_thread->join();

  // Compare input and output sizes
  bool success = true;
  for (int i = 0; i < packet_num; i++){
    if (size_data[i] != packet_size){
      //      std::cout << i << ": " << size_data[i] << std::endl;
      success = false;
    }
  }
  if (success) std::cout << "SIZE SUCCESS: you are not a tit!" << std::endl;
  if (!success) std::cout << "SIZE FAILURE: you are a tit!" << std::endl;

  // Compare input and output data
  success = true;
  int count = 0;
  for (int i = 0; i < packet_num; i++){
    for (int j = 0; j < packet_len; j++){
      if (push_packets[i][j] != pull_data[count]) success = false;
      count++;
    }
  }
  if (success) std::cout << "DATA SUCCESS: you are not a tit!" << std::endl;
  if (!success) std::cout << "DATA FAILURE: you are a tit!" << std::endl;

  
  return 1;

}
