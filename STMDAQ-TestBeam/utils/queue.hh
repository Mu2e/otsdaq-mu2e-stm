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
  queue_buffer(uint64_t buffer_len);

  // Try to push data to queue
  uint64_t try_push(int chan, int16_t *data, uint64_t n);
  
  // Push data to queue
  void push(int chan, int16_t *data, uint64_t n);
  
  // Try to pull data from queue
  uint64_t try_pull(int chan, int16_t *&data);

  // Pull data from queue
  uint64_t pull(bool *timeout, int chan, int16_t *&data);

  // Atomic push tail pointer
  std::atomic<int64_t> tail[CHNUM];
  // Atomic pull head pointer
  std::atomic<int64_t> head[CHNUM];
  // Atomic buffer occupancy pointer
  std::atomic<int64_t> occupancy[CHNUM];

  // Current pointers
  int64_t current_tail[CHNUM];
  int64_t current_head[CHNUM];

  // Number written to the buffer
  int64_t write_num[CHNUM];
  int64_t read_num[CHNUM];

  // The max number of datagrams in a buffer
  //  static const int64_t RING_BUFFER_NUM = pow(2,13);

  // The buffer length
  ///  static const int64_t RING_BUFFER_LEN = RING_BUFFER_NUM*UDPsocket::MAX_UDP_LEN;
  int64_t RING_BUFFER_LEN = 0;
  

  // The buffer
  int16_t* process_buffer[CHNUM];
  int16_t* data_buffer[CHNUM];

  // Increment function
  int64_t increment(uint64_t n, uint64_t x){
    return (n + x) % RING_BUFFER_LEN;
  }

private :


};

#endif
  
