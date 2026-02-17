#ifndef queue_h_
#define queue_h_

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>
#include <cinttypes>

// Data variables header
#include "STMDAQ-TestBeam/utils/dataVars.hh"
// UDP socket header
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

class queue_buffer{

public :

  // Constructor
  queue_buffer();

  // // The header for each packet in the buffer
  // struct packet_header{
  //   int16_t header[MAX_PACKET_LEN] = {};
  // };

  // Try to push data to queue
  int try_push(int chan, int16_t *data, int n);
  
  // Push data to queue
  void push(int chan, int16_t *data, int n);
  
  // Try to pull data from queue
  uint64_t try_pull(int chan, int16_t *&data);

  // Pull data from queue
  uint64_t pull(bool *timeout, int chan, int16_t *&data);

  // Make int32_t from two int16_ts
  int32_t make_int32_t(int16_t p0, int16_t p1){
    return (p1 & 0xFFFF) << 16 | p0 & 0xFFFF;

  }

  // // Atomic occupancy counters
  // std::atomic<uint32_t> occupancy_size[CHNUM];
  // std::atomic<uint32_t> occupancy_data[CHNUM];
  
  // // Push tail pointers
  // struct tail {
  //   // Size buffer pointer (int32_t)
  //   int16_t size_0; // Lower 16 bits
  //   int16_t size_1; // Upper 16 bits
  //   // Data buffer pointer (int32_t)
  //   int16_t data_0; // Lower 16 bits
  //   int16_t data_1; // Upper 16 bits
  // };

  // // Pull head pointers
  // struct head {
  //   // Size buffer pointer (int32_t)
  //   int16_t size_0; // Lower 16 bits
  //   int16_t size_1; // Upper 16 bits
  //   // Data buffer pointer (int32_t)
  //   int16_t data_0; // Lower 16 bits
  //   int16_t data_1; // Upper 16 bits
  // };

  // Atomic push tail pointer
  std::atomic<int64_t> tail[CHNUM];
  // Atomic pull head pointer
  std::atomic<int64_t> head[CHNUM];
  // Atomic buffer occupancy pointer
  std::atomic<int64_t> occupancy[CHNUM];

  // // Current tail pointers
  // tail current_tail[CHNUM];
  // int32_t current_tail_size[CHNUM];
  // int32_t current_tail_data[CHNUM];
  // // Current head pointers
  // head current_head[CHNUM];
  // int32_t current_head_size[CHNUM];
  // int32_t current_head_data[CHNUM];

  // Current pointers
  int64_t current_tail[CHNUM];
  int64_t current_head[CHNUM];

  // Number written to the buffer
  int64_t write_num[CHNUM];
  int64_t read_num[CHNUM];
  // int32_t write_num_size[CHNUM]; // in push
  // int32_t write_num_data[CHNUM]; // in push
  // int32_t read_num_size[CHNUM]; // in push
  // int32_t read_num_data[CHNUM]; // in push

  // // Size buffer indeces
  // static const uint SIZE_INDEX = 0; // Individual datagram size
  // static const uint ACC_SIZE_INDEX = 1; // Accumulated datagram size

  // Maximum size of an int32_t
  static const int32_t INT32_T_MAX = 2147483647;

  // The max number of datagrams in a buffer
  //  static const uint64_t RING_BUFFER_NUM = 65536;
  static const int64_t RING_BUFFER_NUM = pow(2,17);

  // The buffer length
  static const int64_t RING_BUFFER_LEN = RING_BUFFER_NUM*UDPsocket::MAX_UDP_LEN;

  // The buffer
  //  uint64_t* size_buffer[CHNUM];
  //  packet_header* header_buffer[CHNUM];
  int16_t* data_buffer[CHNUM];

  // Increment function
  int64_t increment(int n, int x){
    return (n + x) % RING_BUFFER_LEN;
  }

private :


};

#endif
  
