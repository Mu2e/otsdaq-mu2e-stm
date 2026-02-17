#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/memfd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "queue.hh"

// The get index from the queue;
int16_t getIndex = 0;
// The data from the queue
packet get_data;

size_t maxPacketSize = 8198;


// Convenience wrappers for erroring out
static inline void queue_error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "queue error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  abort();
}
static inline void queue_error_errno(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "queue error: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, " (errno %d)\n", errno);
  va_end(args);
  abort();
}

// Initialise the queue
void queue::init(size_t s) {

  // Initialise data array with maximum packet size
  //  get_data.data = new int16_t [9000];

  // We're going to use a trick where we mmap two adjacent pages (in virtual memory) that point to the
  // same physical memory. This lets us optimize memory access, by virtue of the fact that we don't need
  // to even worry about wrapping our pointers around until we go through the entire buffer. 

  //  get_data.data = new int16_t [maxPacketSize/2] ();

  // Check that the requested size is a multiple of a page. If it isn't, we're in trouble.
  if(s % getpagesize() != 0) {
    queue_error("Requested size (%lu) is not a multiple of the page size (%d)", s, getpagesize());
  }
    
  // Create an anonymous file backed by memory
  if((q.fd = fileno(tmpfile ())) == -1){
    queue_error_errno("Could not obtain anonymous file");
  }

  // Set buffer size
  if(ftruncate(q.fd, s) != 0){
    queue_error_errno("Could not set size of anonymous file");
  }
    
  // Ask mmap for a good address
  if((q.buffer = (uint8_t*)mmap(NULL, 2 * s, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){
    queue_error_errno("Could not allocate virtual memory");
  }
    
  // Mmap first region
  if(mmap(q.buffer, s, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, q.fd, 0) == MAP_FAILED){
    queue_error_errno("Could not map buffer into virtual memory");
  }
    
  // Mmap second region, with exact address
  if(mmap(q.buffer + s, s, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, q.fd, 0) == MAP_FAILED){
    queue_error_errno("Could not map buffer into virtual memory");
  }
    
  // Initialize remaining members
  q.size = s;
  q.head = 0;
  q.tail = 0;

}

// Destroy / unmap the queue
void queue::destroy() {

  if(munmap(q.buffer + q.size, q.size) != 0){
    queue_error_errno("Could not unmap buffer");
  }
    
  if(munmap(q.buffer, q.size) != 0){
    queue_error_errno("Could not unmap buffer");
  }
    
  if(close(q.fd) != 0){
    queue_error_errno("Could not close anonymous file");
  }

}

// Put data in the queue
void queue::put(packet data) {

  // Wait if the tail has wrapped around into a new buffer and is about to overlap the head...
  while (((q.put_buf_count > q.get_buf_count) or (q.put_buf_count <= 0 && q.get_buf_count > 0)) 
	 && (q.tail + data.size) >= q.head){}

  // Write data packet to queue
  memcpy(q.buffer + q.tail, &data, sizeof(data.size) + data.size);

  // Update tail in bytes
  q.tail += sizeof(data.size) + data.size;

  // When read buffer moves into 2nd memory region, we can reset to the 1st region
  if(q.tail >= q.size) {
    q.tail -= q.size;
    // Increment the put buffer counter
    q.put_buf_count++;
    // // If the buffer counter has reached INT16_T MAX, reset to zero
    // if (q.put_buf_count >= INT16_MAX) q.put_buf_count -= INT16_MAX;
  }

}

// Get data from the queue
packet queue::get() {

  // Wait if head is about to catch up with tail...
  while(q.put_buf_count == q.get_buf_count && (q.tail - q.head) == 0){}

  // Get data size
  memcpy(&get_data.size, q.buffer + q.head, sizeof(get_data.size));
 
  // Get data
  memcpy(get_data.data, q.buffer + q.head + sizeof(get_data.size), get_data.size);

  // Update head in bytes
  q.head += sizeof(get_data.size) + get_data.size;
    
  // When read buffer moves into 2nd memory region, we can reset to the 1st region
  if(q.head >= q.size) {
    q.head -= q.size;
    // Increment the get buffer counter
    q.get_buf_count++;
    // // If the buffer counter has reached INT16_T MAX, reset to zero
    // if (q.get_buf_count >= INT16_MAX) q.get_buf_count -= INT16_MAX;
  }
       
  return get_data;
}
