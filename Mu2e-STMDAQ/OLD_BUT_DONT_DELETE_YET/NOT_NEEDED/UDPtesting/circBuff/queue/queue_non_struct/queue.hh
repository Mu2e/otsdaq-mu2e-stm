#ifndef queue_h_
#define queue_h_

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>
#include <cinttypes>

// Ring buffer size
static const int CHNUM = 2;
static const int packet_size = 8198;
static const int packet_len = packet_size/2;

class queue_buffer{

public :

  // Constructor
  queue_buffer();

  // The header for each packet in the buffer
  struct packet_header{
    int16_t header[packet_len] = {};
  };

  // Try to push data to queue
  int try_push(int chan, uint64_t** sizes, int16_t *data, int n, int index);
  
  // Push data to queue
  void push(int chan, uint64_t** sizes, int16_t *data, int n);
  
  // Try to pull data from queue
  int try_pull(int chan, uint64_t*& sizes, int16_t *&data);

  // Pull data from queue
  int pull(bool *timeout, int chan, uint64_t*& sizes, int16_t *&data);

  // Make int32_t from two int16_ts
  int32_t make_int32_t(int16_t p0, int16_t p1){
    return (p1 & 0xFFFF) << 16 | p0 & 0xFFFF;

  }

  // Atomic occupancy counters
  std::atomic<uint32_t> occupancy_size[CHNUM];
  std::atomic<uint32_t> occupancy_data[CHNUM];
  
  // Push tail pointers
  struct tail {
    // Size buffer pointer (int32_t)
    int16_t size_0; // Lower 16 bits
    int16_t size_1; // Upper 16 bits
    // Data buffer pointer (int32_t)
    int16_t data_0; // Lower 16 bits
    int16_t data_1; // Upper 16 bits
  };

  // Pull head pointers
  struct head {
    // Size buffer pointer (int32_t)
    int16_t size_0; // Lower 16 bits
    int16_t size_1; // Upper 16 bits
    // Data buffer pointer (int32_t)
    int16_t data_0; // Lower 16 bits
    int16_t data_1; // Upper 16 bits
  };

  // Atomic push tail pointer
  std::atomic<tail> tail_[CHNUM];
  // Atomic pull head pointer
  std::atomic<head> head_[CHNUM];

  // Current tail pointers
  tail current_tail[CHNUM];
  int32_t current_tail_size[CHNUM];
  int32_t current_tail_data[CHNUM];
  // Current head pointers
  head current_head[CHNUM];
  int32_t current_head_size[CHNUM];
  int32_t current_head_data[CHNUM];

  // Number written to the buffer
  int32_t write_num_size[CHNUM]; // in push
  int32_t write_num_data[CHNUM]; // in push
  int32_t read_num_size[CHNUM]; // in push
  int32_t read_num_data[CHNUM]; // in push

  // Size buffer indeces
  static const uint SIZE_INDEX = 0; // Individual datagram size
  static const uint ACC_SIZE_INDEX = 1; // Accumulated datagram size

  // Maximum size of an int32_t
  static const int32_t INT32_T_MAX = 2147483647;

  // The max number of datagrams in a buffer
  static const uint64_t RING_BUFFER_NUM = 65536;

  // The buffer length
  static const uint32_t RING_BUFFER_LEN = RING_BUFFER_NUM*packet_len;

  // The buffer
  //  data_pointer* pointer_buffer[CHNUM];
  uint64_t* size_buffer[CHNUM];
  packet_header* header_buffer[CHNUM];
  int16_t* data_buffer[CHNUM];

  // Increment function
  int64_t increment(int n, int x, uint32_t buffer_size){
    return (n + x) % buffer_size;
  }

private :


};

#endif
  
