#ifndef testQueueMAIN_h_
#define testQueueMAIN_h_

#include <iostream>                                                                                   
#include <thread>                                                                                      
#include <atomic>  
#include <cinttypes>
#include <cstring>
#include <thread>
#include <mutex>

static const int CHNUM = 2;
//static const int buffer_size = 65537; //65536;  // power of 2 for efficient %
static const int buffer_size = 131072; //65536;  // power of 2 for efficient %
static const int m_num = 100*buffer_size;
static const int push_max = 65536;
static const int packet_size = 8198;
static const int packet_len = packet_size/2;
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
  int pull(int chan, T *&data);
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
  // number in push buffer
  int64_t writeNum[CHNUM];
  // number in pull buffer
  int64_t readNum[CHNUM];
  // Atomic fetch_add write pointer
  int64_t writeNumInBuffer[CHNUM];
  // Atomic fetch_sub read pointer
  int64_t readNumInBuffer[CHNUM];
  // The buffer size
  static const int64_t size = buffer_size;
  // The maximum number of datagrams to pull
  int pull_max = m_num;
  // Pull data pointer array
  T pdata[CHNUM];
  // The buffer
  T* buffer[CHNUM];
  // Increment function
  int64_t increment(int n, int x){
    return (n + x) % (size);
  }
  int64_t memPercent(int n, int x){
    return n*(n / x);
  }
  bool startOfRun[CHNUM] = {true, true};
private :
};

#endif
