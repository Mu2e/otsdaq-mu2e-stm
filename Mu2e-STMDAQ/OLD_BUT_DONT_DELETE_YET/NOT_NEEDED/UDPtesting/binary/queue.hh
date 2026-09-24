#ifndef QUEUE_hh
#define QUEUE_hh

#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Ring buffer size
#define RING_BUFFER_SIZE 32768  // power of 2 for efficient %

// PACKET
// #define PACKET_SIZE 8198  
// #define PACKET_LEN 4199  
#define PACKET_SIZE 4
#define PACKET_LEN 2

class queue_buffer{

public :

  // Constructor
  queue_buffer(){

    // For each channel, set read and write pointers to zero
    for (int chan = 0; chan < channels; chan++){
      write[chan].store(0);
      read[chan].store(0);
    }
  }

  // Try to push data to queue_buffer
  bool try_push(int chan, int16_t* data){
    
    // Get the current write pointer
    const_cast<uint64_t&>(current_tail[chan]) = write[chan].load();

    // Increment the next write pointer
    const_cast<uint64_t&>(next_tail[chan]) = increment(current_tail[chan]);

    // If the next write pointer != the current read pointer
    if (next_tail[chan] != read[chan].load()){

      // Add data to queue_buffer buffer 
      memcpy(buffer[chan][current_tail[chan]],data,PACKET_SIZE);

      // Store the updated write pointer
      write[chan].store(next_tail[chan]);

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
  
  // Try to pull data from queue_buffer
  bool try_pull(int chan, int16_t* (&data)){

    // Get current read pointer
    current_head[chan] = read[chan].load();

    // If the current read pointer == write pointer
    if (current_head[chan] == write[chan].load()){

      // Return unsuccesful pull
      return false;
    }

    // Get data from queue_buffer
    data = buffer[chan][current_head[chan]];

    // Store the updated read pointer
    read[chan].store(increment(current_head[chan]));

    // Return succeesful pull
    return true;

  }

  // Pull data from queue_buffer
  int16_t* pull(int chan){

    // Wait until...
    while( ! try_pull(chan,pdata[chan]) ) {};

    // Return pulled data
    return pdata[chan];

  }

private :

  // Maximum number of queue_buffers 
  static const int channels = 2;

  // Atomic write pointer
  std::atomic<uint64_t> write[channels];
  // Atomic read pointer
  std::atomic<uint64_t> read[channels];

  // Current tail pointer
  uint64_t current_tail[channels];
  // Next tail pointer
  uint64_t next_tail[channels];
  // Current head pointer
  uint64_t current_head[channels];

  // The buffer size
  static const uint64_t size = RING_BUFFER_SIZE;

  // Pull data pointer array
  int16_t *pdata[channels];

  // The buffer
  int16_t buffer[channels][RING_BUFFER_SIZE][PACKET_LEN] = {{{}}};

  // Increment function
  uint64_t increment(int n){
    return (n + 1) % size;
  }

};

#endif
  
