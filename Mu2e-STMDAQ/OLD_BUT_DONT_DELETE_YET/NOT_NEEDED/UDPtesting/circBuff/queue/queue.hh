#ifndef queue_h_
#define queue_h_

#include <stdint.h>
#include <pthread.h>

/** Metadata (header) for a message in the queue                        
 */
// struct message_t {
//   size_t  len;
//   //  size_t  seq;
// };

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
    
  // Sequence number of next consumable message
  //  size_t head_seq;
    
  // Sequence number of last written message
  //  size_t tail_seq;
    
  // Synchronization primitives
  pthread_cond_t readable;
  pthread_cond_t writeable;
  pthread_mutex_t lock;

};

//template <typename T, size_t buffer_size>
class queue {

public:

  // Initalise queue struct
  queue_t q;
  
  // Initialize a blocking queue *q* of size
  void init(size_t s);
  
  // Destroy the blocking queue *q*
  void destroy();
  
  // Insert into queue *q* a message of *size* bytes from *buffer*
  // Blocks until sufficient space is available in the queue.
  void put(int16_t *data, int16_t size);
  
  // Retrieves a message of at most *max* bytes from queue *q* and writes it to *buffer*.
  // Blocks until a message of no more than *max* bytes is available.
  // Returns the number of bytes in the written message.
  size_t get(int16_t *data, int16_t max);
  
private:

};

#endif
  
