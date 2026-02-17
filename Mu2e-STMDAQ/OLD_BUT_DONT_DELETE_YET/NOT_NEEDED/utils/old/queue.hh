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

// Ring buffer size
//#define RING_BUFFER_SIZE 65536  // power of 2 for efficient %
//#define RING_BUFFER_SIZE 4294967296  // power of 2 for efficient %
#define RING_BUFFER_SIZE 536870912

class queue_buffer{

public :

  // Constructor
  queue_buffer();

  // Try to push data to queue
  int try_push(int chan, int16_t *data, int n);
  
  // Push data to queue
  void push(int chan, int16_t *data, int n);
  
  // Try to pull data from queue
  int try_pull(int chan, int16_t *&data);

  // Pull data from queue
  //  int pull(UDPsocket *udp, int chan, T *&data);
  int pull(bool *timeout, int chan, int16_t *&data);

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

  // The buffer
  int16_t* buffer[CHNUM];

  // Increment function
  int64_t increment(int n, int x){
    return (n + x) % size;
  }

private :


};

#endif
  
