#include "STMDAQ-TestBeam/sharedMem/sharedMem.hh"

template<class T> void SynchronizedQueue<T>::push(T element){  

  boost::lock_guard<bip::interprocess_mutex> lock(io_mutex_);
  sQueue.push_back(element);
  waitCondition.notify_one();
}

//typedef SynchronizedQueue<gps_position> MySynchronisedQueue;
