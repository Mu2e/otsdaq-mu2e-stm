#ifndef THREAD_FUNCS_hh_
#define THREAD_FUNCS_hh_

#include <iomanip>
#include <cstring>
#include <functional> 

// Buffer pool code
#include "buffer_pool.hh" 
// Dummy operation header
#include "operation.hh"

// Standard thread functions
class ThreadFuncs {

private:

  // Number of threads
  const size_t threadNum = 7;
  // Signal when starter thread is finished
  std::atomic<bool> dataDone{false}; 
  // Flags to signal when each thread has finished
  std::vector<std::atomic<bool>> threadDone;

public:

  // Constructor to initialize threads
  ThreadFuncs();

  // Destructor for logging
  ~ThreadFuncs() { 
    std::cout << "ThreadFuncs destructor called.\n";
  }

  // Pin thread to specific CPU core
  void pin_thread_to_core(size_t core_id);
  
  // Starter thread to fill first buffer
  void starterThread(RingBuffer<DataStruct>& outputQueue,
		     BufferPool& pool,
		     size_t core_id);
  
  // General worker function for processing stages
  void workerThread(RingBuffer<DataStruct>& inputQueue,
		    RingBuffer<DataStruct>& outputQueue,
		    BufferPool& pool,
		    const char* stage,
		    std::atomic<uint64_t>& totalBytesProcessed,
		    size_t core_id,
		    std::function<void(std::shared_ptr<DataStruct>&)> operation,
		    size_t threadIndex);
  
  // Ending thread to ensure the final buffer is emptied
  void endingThread(RingBuffer<DataStruct>& inputQueue,
		      BufferPool& pool,
		      size_t core_id,
		      size_t threadIndex);
  
};

#endif 
