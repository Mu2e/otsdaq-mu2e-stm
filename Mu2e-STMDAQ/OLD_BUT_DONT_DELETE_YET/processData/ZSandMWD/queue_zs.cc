#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Zero-suppression queue header
#include "queue_zs.hh"

// Constructor 
template <typename T> queue_zs<T>::queue_zs(int event_num, int buffer_size) {

  size = buffer_size;
  data_buffer.header_data = new int16_t [event_num * fw_tHdr_Len] ();
  data_buffer.adc_data = new T [event_num * off_spill_len] ();

  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    write[chan].store(0);
    read[chan].store(0);
    buffer[chan] = new data_struct<T> [size];      
    for (int i = 0; i < size; i++){
      buffer[chan][i].header_data = new int16_t [event_num * fw_tHdr_Len] ();
      buffer[chan][i].adc_data = new T [event_num * off_spill_len] ();      
    }
  }

}

// Try to push data to queue
template <typename T> bool queue_zs<T>::try_push(int chan, data_struct<T> data){  
  
  // Initialise the return value
  uint64_t retval = 0;

  // Load the current tail
  current_tail[chan] = write[chan].load();

  // Load the current tail
  next_tail[chan] = increment(write[chan].load());

  // If the next write pointer != the current read pointer
  if (next_tail[chan] != read[chan].load()){

    // Add data to queue buffer 
    //    memcpy(&buffer[chan][current_tail[chan]],&data,sizeof(data));

    buffer[chan][current_tail[chan]] = data;

    // Store the updated write pointer
    write[chan].store(next_tail[chan]);

    // Return succesful push
    return true;

  }

  // Return unsuccesful push
  return false; 

}
  
// Push data to queue
template <typename T> void queue_zs<T>::push(int chan, data_struct<T> data){

  // Wait until pushed..
  while( ! try_push(chan,data) ){}; 

}
  
// Try to pull data from queue
template <typename T> bool queue_zs<T>::try_pull(int chan, data_struct<T> &data){

  // Initialise retval
  uint64_t retval = 0;
  
  // Get current read pointer
  current_head[chan] = read[chan].load();

  // If nothing to read in the buffer
  if ((write[chan].load() - current_head[chan]) == 0){
    // Return unsuccesful pull
    return false;
  }
  
  // Memcpy retval datagrams
  //memcpy(data,&buffer[chan][current_head[chan]],sizeof(data));
  
  data = buffer[chan][current_head[chan]];

  // Store the updated read pointer
  read[chan].store(increment(current_head[chan]));

  // Return successful pull
  return true;
  
}

// Pull data from queue
template <typename T> int queue_zs<T>::pull(bool *timeout, int chan, data_struct<T> &data){
  
  int retval = 0;
  
  // Wait until...
  while( !try_pull(chan,data) && !*timeout ) {};
  
  if (*timeout and (write[chan].load() - current_head[chan]) != 0){
    std::cout << "ERROR: Still data in queue!!!!" << std::endl;
    exit(0);
  }

  if (*timeout) return 0;
  
  // Return pulled data
  return data.data_len;
  
}
  
template class queue_zs<int16_t>;
template class queue_zs<double>;
