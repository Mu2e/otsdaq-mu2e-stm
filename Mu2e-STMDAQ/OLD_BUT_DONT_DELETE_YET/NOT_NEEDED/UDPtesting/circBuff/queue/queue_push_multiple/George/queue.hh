#ifndef queue_h_
#define queue_h_

#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <cstring>

struct packet{

  int size = 1;
  int data[size] = 9;

};

template <typename T> class queue_buffer{

public :

  // Constructor
  queue_buffer();

  // The buffer size
  static const int64_t size = 16  // power of 2 for efficient %

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
  // Atomic memory checker                                                  
  std::atomic<int64_t> numInBuffer[CHNUM];

  // Current tail pointer
  int64_t current_tail[CHNUM];
  // Next tail pointer
  int64_t next_tail[CHNUM];
  // Current head pointer
  int64_t current_head[CHNUM];

  // The maximum number of datagrams to pull
  int pull_max = 0;

  // Pull data pointer array
  T pdata[CHNUM];

  // The buffer
  T* buffer[CHNUM];

  // Increment function
  int64_t increment(int chan, int n, int x){
    return (n + x) % size;
  }

private :


};

#endif
  
