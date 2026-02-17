/////////////////////////////////////////////////////////////////////////\
/// This module creates a circular buffer for data readout (header).            
/////////////////////////////////////////////////////////////////////////\

#ifndef CIRCULAR_BUFFER_hh
#define CIRCULAR_BUFFER_hh


#include<stdlib.h>
#include<iostream>
#include<fstream>

// Define a template for a cicular buffer 
// of type "T" and size "buff_size"
template <typename T, size_t buffer_size>
class circBuffer {

public:

  // The buffer
  T buffer[buffer_size];
  // Head pointer
  size_t head;
  // Tail pointer
  size_t tail;

  // Put data into circular buffer
  void put(const T* data, const size_t size) {
    
    // Add data to buffer
    for (int i = 0 ; i < size; ++i) {
      this->buffer[(this->tail + i) % this->buffer_size] = data[i];
    }

    // Update the tail pointer (wrap around using modulo operator)
    this->tail = (this->tail + size) % this->buffer_size;

  }

  // Get data from circular buffer
  bool get(queue_t *q, uint8_t *data, size_t size) {

    // Add data to buffer
    for (int i = 0 ; i < size; ++i) {
      this->buffer[(this->tail + i) % this->buffer_size] = data[i];
    }
    
    
    // Get data from buffer
    for(size_t i = 0; i < size; i++){
      data[i] = q->buffer[(q->head + i) % q->buffer_size];
    }
    
    // Update the head pointer (wrap around using modulo operator)    
    q->head = (q->head + size) % q->buffer_size;
    
    // Update the reminaing available size
    q->bytes_avail -= size;
    return true;

  }
  
  
  // // Circular buffer queue struct
  // struct queue_t {
    
  //   // The buffer
  //   uint8_t *buffer;
  //   // The buffer size
  //   size_t   buffer_size;
  //   size_t   head;
  //   size_t   tail;
  //   // Bytes avilable in buffer
  //   size_t   bytes_avail;
  // };

  // // Put data into circular buffer
  // bool put(queue_t *q, uint8_t *data, size_t size) {

  //   // Check for sufficient space before attempting to write
  //   if(q->buffer_size - q->bytes_avail < size){
  //     // Return false if not enough space in buffer
  //     return false;
  //   }

  //   // Add data to buffer
  //   for(size_t i = 0; i < size; i++){
  //     q->buffer[(q->tail + i) % q->buffer_size] = data[i];
  //   }

  //   // Update the tail pointer (wrap around using modulo operator)
  //   q->tail = (q->tail + size) % q->buffer_size;
    
  //   // Update the remaining available size
  //   q->bytes_avail += size;
  //   return true;
  // }

  // // Get data from circular buffer
  // bool get(queue_t *q, uint8_t *data, size_t size) {

  //   // If the remaining available size in the buffer is
  //   // less than the size of the data to get
  //   if(q->bytes_avail < size){
  //     // Return false
  //     return false;
  //   }

  //   // Get data from buffer
  //   for(size_t i = 0; i < size; i++){
  //     data[i] = q->buffer[(q->head + i) % q->buffer_size];
  //   }

  //   // Update the head pointer (wrap around using modulo operator)    
  //   q->head = (q->head + size) % q->buffer_size;

  //   // Update the reminaing available size
  //   q->bytes_avail -= size;
  //   return true;
  // }

private:

};

#endif
