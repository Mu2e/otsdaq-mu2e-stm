#ifndef queue_zs_h_
#define queue_zs_h_

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <vector>

#include "dataVars.hh"

// Ring buffer size
//#define ZS_BUFFER_SIZE 100

// The global data struct
template <typename T>
struct data_struct {
  int count = 0;
  std::vector<uint64_t> start_index; // Events
  std::vector<uint64_t> pulse_index; // Pulses
  //  int16_t* header_data = new int16_t [1000*fw_tHdr_Len] ();// headers
  int16_t* header_data;// headers
  uint64_t data_len = 0; // ADC data length
  //  T* adc_data = new T [1000*30000] ();// ADC data
  T* adc_data;// ADC data
  double baseline_mean = 0;
  double baseline_rms = 0;
};

template <typename T> class queue_zs{

public :

  // Constructor
  queue_zs(int event_num, int buffer_size);

  data_struct<T> data_buffer;

  // Try to push data to queue
  bool try_push(int chan, data_struct<T> data);
  
  // Push data to queue
  void push(int chan, data_struct<T> data);
  
  // Try to pull data from queue
  bool try_pull(int chan, data_struct<T> &data);

  // Pull data from queue
  int pull(bool *timeout, int chan, data_struct<T> &data);

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
  //  static const int64_t size = 100;
  int64_t size;

  // The buffer
  data_struct<T>* buffer[CHNUM];

  // Increment function
  int64_t increment(int n){
    return (n + 1) % size;
  }

private :


};

#endif
  
