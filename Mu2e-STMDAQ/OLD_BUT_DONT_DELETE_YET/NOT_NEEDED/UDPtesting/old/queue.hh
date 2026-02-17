#ifndef queue_h_
#define queue_h_

#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>

#include "UDPsocket.hh"

//  Blocking queue data structure
struct queue_t {
  
  // Backing buffer and size
  uint8_t *buffer;
  size_t size;
    
  // Backing buffer's memfd descriptor
  int fd;
    
  // Read / write indices
  size_t head;
  size_t tail; 

  // The buffer wrap around count for the put/get threads
  int16_t put_buf_count = 0;
  int16_t get_buf_count = 0; 

};

//template <typename T, size_t buffer_size>
class queue {

public:

  // Initalise queue struct
  queue_t q;
  
  // Initialize a circular buffer queue of size s
  void init(size_t s);
  
  // Destroy the queue
  void destroy();
  
  // Put data packet into queue
  void put(packet data);
  
  // Get data only from queue
  packet get();
  
private:

};

#endif
  
