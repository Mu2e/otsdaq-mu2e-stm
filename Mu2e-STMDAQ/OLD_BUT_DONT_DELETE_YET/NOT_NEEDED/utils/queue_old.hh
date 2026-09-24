#ifndef queue_h_
#define queue_h_

#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Data variables header
#include "STMDAQ-TestBeam/utils/dataVars.hh"
// UDP socket header
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Ring buffer size
#define RING_BUFFER_SIZE 65536  // power of 2 for efficient %

class queue_buffer{

public :

  // Constructor
  queue_buffer();

  // Try to push data to queue
  uint64_t try_push(int chan, int16_t *data, uint64_t n, uint64_t index);
  
  // Push data to queue
  void push(int chan, int16_t *data, uint64_t n);
  
  // Try to pull data from queue
  uint64_t try_pull(int chan, int16_t *&data);

  // Pull data from queue
  //  int pull(UDPsocket *udp, int chan, T *&data);
  uint64_t pull(bool *timeout, int chan, int16_t *&data);

  // Atomic write pointer
  std::atomic<int64_t> write[CHNUM];
  // Atomic read pointer
  std::atomic<int64_t> read[CHNUM];
  // Atomic occupancy counter
  std::atomic<int64_t> occupancy[CHNUM];

  // Current tail pointer
  int64_t current_tail[CHNUM];
  // Next tail pointer
  int64_t next_tail[CHNUM];
  // Current head pointer
  int64_t current_head[CHNUM];

  // Number written to the buffer
  int64_t write_num[CHNUM]; // in push
  int64_t read_num[CHNUM]; // in pull

  // The buffer size
  static const int64_t size = RING_BUFFER_SIZE;
  static const int64_t RING_BUFFER_NUM = RING_BUFFER_SIZE;
  static const int64_t RING_BUFFER_LEN = RING_BUFFER_SIZE*UDPsocket::MAX_UDP_LEN;

  // The maximum number of datagrams to pull
  int pull_max = 0;

  // The buffer
  int16_t* buffer[CHNUM];

  // Increment function
  int64_t increment(uint64_t n, uint64_t x){
    return (n + x) % size;
  }

private :


};

#endif
  
