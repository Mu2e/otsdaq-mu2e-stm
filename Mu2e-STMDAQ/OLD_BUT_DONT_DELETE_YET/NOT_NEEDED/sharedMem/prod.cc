#include "STMDAQ-TestBeam/utils/sharedMem.hh"

typedef SynchronizedQueue<gps_position> MySynchronisedQueue;

int main() {

  std::cout << "Entered producer..." << std::endl;

  bip::shared_memory_object::remove(SHARED_MEMORY_NAME);
    
  bip::managed_shared_memory mysegment(bip::create_only,SHARED_MEMORY_NAME, 65536);

  MySynchronisedQueue::allocator_type alloc(mysegment.get_segment_manager());
  MySynchronisedQueue *myQueue = mysegment.construct<MySynchronisedQueue>(SHARED_QUEUE_NAME)(alloc);

  for(int i = 0; i < 100; ++i){          
    std::cout << "Pushed " << i << std::endl;
    myQueue->push(gps_position(i, 2, 3));
  }

  // Wait until the queue is empty: has been processed by client(s)
  while(myQueue->sizeOfQueue() > 0) 
    continue;
}


