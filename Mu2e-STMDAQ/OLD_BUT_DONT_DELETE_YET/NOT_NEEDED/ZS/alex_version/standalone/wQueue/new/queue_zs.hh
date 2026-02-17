#ifndef queue_zs_h_
#define queue_zs_h_

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <vector>

#include "dataVars.hh"
#include "queue.hh"

// Ring buffer size
#define ZS_BUFFER_SIZE 100

// The global data struct
struct data_struct {
  int count = 0;
  std::vector<uint64_t> start_index; // Events
  std::vector<uint64_t> pulse_index; // Pulses
  int16_t* header_data;// headers
  uint64_t data_len; // ADC data length
  int16_t* adc_data;// ADC data
};

class queue_zs{

public :

  // Constructor
  queue_zs();

  // Try to push data to queue
  bool try_push(int chan, data_struct data);
  
  // Push data to queue
  void push(int chan, data_struct data);
  
  // Try to pull data from queue
  bool try_pull(int chan, data_struct &data);

  // Pull data from queue
  int pull(bool *timeout, int chan, data_struct &data);

  // Atomic write pointer
  std::atomic<int64_t> write[CHNUM];
  // Atomic read pointer
  std::atomic<int64_t> read[CHNUM];

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
  static const int64_t size = ZS_BUFFER_SIZE;

  // The buffer
  data_struct* buffer[CHNUM];

  // Increment function
  int64_t increment(int n){
    return (n + 1) % size;
  }

private :


};

#endif
  
