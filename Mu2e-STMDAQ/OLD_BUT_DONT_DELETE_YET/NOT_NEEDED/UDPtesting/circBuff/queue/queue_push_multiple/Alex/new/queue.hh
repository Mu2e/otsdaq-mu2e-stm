#ifndef queue_h_
#define queue_h_

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>
#include <cinttypes>

// Ring buffer size
//#define RING_BUFFER_SIZE 65536  // power of 2 for efficient %
#define RING_BUFFER_SIZE 4294967296  // power of 2 for efficient %

static const int CHNUM = 2;

template <typename T> class queue_buffer{

public :

  // Constructor
  queue_buffer();

  // Try to push data to queue
  int try_push(int chan, T *data, int n, int index);
  
  // Push data to queue
  void push(int chan, T *data, int n);
  
  // Try to pull data from queue
  int try_pull(int chan, T *&data);

  // Pull data from queue
  //  int pull(UDPsocket *udp, int chan, T *&data);
  int pull(bool *timeout, int chan, T *&data);

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

  // The maximum number of datagrams to pull
  int pull_max = 0;

  // Pull data pointer array
  T pdata[CHNUM];

  // The buffer
  T* buffer[CHNUM];

  // Increment function
  int64_t increment(int n, int x){
    return (n + x) % size;
  }

private :


};

#endif
  
