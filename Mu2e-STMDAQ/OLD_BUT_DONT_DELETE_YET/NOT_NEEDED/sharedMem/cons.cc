#include "STMDAQ-TestBeam/utils/sharedMem.hh"

typedef SynchronizedQueue<gps_position> MySynchronisedQueue;

int main(){

  std::cout << "Entered consumer..." <<std::endl;

  bip::managed_shared_memory openedSegment(bip::open_only, SHARED_MEMORY_NAME);
    
  MySynchronisedQueue *openedQueue = openedSegment.find<MySynchronisedQueue>(SHARED_QUEUE_NAME).first;
  gps_position position;
  
  
  while (openedQueue->pop(position)) {
    std::cout << "Degrees= " << position.degrees << " Minutes= " << position.minutes << " Seconds= " << position.seconds;
    std::cout << "\n";
  }

}

