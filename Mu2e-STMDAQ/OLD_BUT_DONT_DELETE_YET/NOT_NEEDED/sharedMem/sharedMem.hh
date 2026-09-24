#ifndef SHAREDMEM_hh
#define SHAREDMEM_hh

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <sstream>

#include <boost/interprocess/shared_memory_object.hpp>
#include <iostream>

#include <thread>

namespace bip = boost::interprocess;

template <class T> class SynchronizedQueue {

public:

  typedef bip::allocator<T, bip::managed_shared_memory::segment_manager> allocator_type;
  
private:
  
  bip::deque<T, allocator_type> sQueue;
  mutable bip::interprocess_mutex io_mutex_;
  mutable bip::interprocess_condition waitCondition;
public:

  SynchronizedQueue(allocator_type alloc) : sQueue(alloc) {} 

  void push(T element);
  // {
  //   boost::lock_guard<bip::interprocess_mutex> lock(io_mutex_);
  //   sQueue.push_back(element);
  //   waitCondition.notify_one();
  // }
  
  bool empty() const {
    boost::lock_guard<bip::interprocess_mutex> lock(io_mutex_);
    return sQueue.empty();
  }
  
  bool pop(T &element) {
    boost::lock_guard<bip::interprocess_mutex> lock(io_mutex_);

    if (sQueue.empty()) {
      return false;
    }

    element = sQueue.front();
    sQueue.pop_front();

    return true;
  }

  unsigned int sizeOfQueue() const {
    // try to lock the mutex
    boost::lock_guard<bip::interprocess_mutex> lock(io_mutex_);
    return sQueue.size();
  }

  void waitAndPop(T &element) {
    boost::lock_guard<bip::interprocess_mutex> lock(io_mutex_);

    while (sQueue.empty()) {
      waitCondition.wait(lock);
    }

    element = sQueue.front();
    sQueue.pop();
  }

  std::string toString() const {
    bip::deque<T> copy;
    // make a copy of the class queue, to reduce time locked
    {
      boost::lock_guard<bip::interprocess_mutex> lock(io_mutex_);
      copy.insert(copy.end(), sQueue.begin(), sQueue.end());
    }

    if (copy.empty()) {
      return "Queue is empty";
    } else {
      std::stringstream os;
      int counter = 0;

      os << "Elements in the Synchronized queue are as follows:" << std::endl;
      os << "**************************************************" << std::endl;

      while (!copy.empty()) {
	T object = copy.front();
	copy.pop_front();
	os << "Element at position " << counter << " is: [" << typeid(object).name()  << "]\n";
      }
      return os.str();
    }
  }
};

struct gps_position {
  int degrees;
  int minutes;
  float seconds;

  gps_position(int d=0, int m=0, float s=0) : degrees(d), minutes(m), seconds(s) {}
};

static char const *SHARED_MEMORY_NAME = "MySharedMemory";
static char const *SHARED_QUEUE_NAME  =  "MyQueue";
//typedef SynchronizedQueue<gps_position> MySynchronisedQueue;

#endif
