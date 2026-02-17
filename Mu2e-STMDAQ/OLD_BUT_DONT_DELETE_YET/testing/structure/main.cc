// Buffer pool header
#include "buffer_pool.hh"
// Thread functions header
#include "thread_funcs.hh" 
// Dummy operation header
#include "operation.hh" 
// Zero suppression header
#include "zero_suppress.hh" 
// MWD header
#include "mwd.hh"

int main() {

  const size_t max_queue_size = 20; // Increased maximum size for each ring buffer
  const size_t buffer_pool_size = 30; // Increased pre-allocated buffer pool size
  const size_t buffer_size = 60 * 1024 * 1024 / sizeof(int16_t); // Buffer size in elements

  // Create buffer pool
  BufferPool pool(buffer_pool_size, buffer_size);

  // Define ring buffers for each step
  RingBuffer<DataStruct> startBufferQueue(max_queue_size);
  RingBuffer<DataStruct> checkDataQueue(max_queue_size);
  RingBuffer<DataStruct> formEventsQueue(max_queue_size);
  RingBuffer<DataStruct> rawBufferQueue(max_queue_size);
  RingBuffer<DataStruct> zsBufferQueue(max_queue_size);
  RingBuffer<DataStruct> mwdBufferQueue(max_queue_size);

  // Track total bytes processed for each thread
  std::atomic<uint64_t> totalBytes{0};
  std::atomic<uint64_t> totalBytesCheck{0};
  std::atomic<uint64_t> totalBytesForm{0};
  std::atomic<uint64_t> totalBytesRaw{0};
  std::atomic<uint64_t> totalBytesZS{0};
  std::atomic<uint64_t> totalBytesMWD{0};
  
  // Instance of thread functions
  std::shared_ptr<ThreadFuncs> tf = std::make_shared<ThreadFuncs>();

    // Set up dummy data operation
  std::shared_ptr<DummyOperation> op = std::make_shared<DummyOperation>();
  // Using a lambda to bind the member function to the instance
  auto operationExample = [op](std::shared_ptr<DataStruct>& buffer) {
    op->operation(buffer);
  };

  // Set up zero suppression operation
  std::shared_ptr<ZeroSuppress> zs = std::make_shared<ZeroSuppress>();
  // Using a lambda to bind the member function to the instance
  auto zs_ = [zs](std::shared_ptr<DataStruct>& buffer) {
    zs->operation(buffer);
  };

  // Set up MWD operation
  std::shared_ptr<MWD> mwd = std::make_shared<MWD>();
  // Using a lambda to bind the member function to the instance
  auto mwd_ = [mwd](std::shared_ptr<DataStruct>& buffer) {
    mwd->operation(buffer);
  };
  
  // Launch worker threads for each stage
  std::thread starter(&ThreadFuncs::starterThread, tf, std::ref(startBufferQueue), std::ref(pool), 0);
  std::thread checkData(&ThreadFuncs::workerThread, tf, std::ref(startBufferQueue), std::ref(checkDataQueue), std::ref(pool), " | Checked", std::ref(totalBytesCheck), 1, operationExample, 1);
  std::thread formEvents(&ThreadFuncs::workerThread, tf, std::ref(checkDataQueue), std::ref(formEventsQueue), std::ref(pool), " | Events Formed", std::ref(totalBytesForm), 2, operationExample, 2);
  std::thread rawProcess(&ThreadFuncs::workerThread, tf, std::ref(formEventsQueue), std::ref(rawBufferQueue), std::ref(pool), " | Algorithm Processed", std::ref(totalBytesRaw), 3, operationExample, 3);
  std::thread zsProcess(&ThreadFuncs::workerThread, tf, std::ref(rawBufferQueue), std::ref(zsBufferQueue), std::ref(pool), " | ZS Processed", std::ref(totalBytesZS), 4, zs_, 4);
  std::thread mwdProcess(&ThreadFuncs::workerThread, tf, std::ref(zsBufferQueue), std::ref(mwdBufferQueue), std::ref(pool), " | MWD Processed", std::ref(totalBytesMWD), 5, mwd_, 5);
  std::thread finalConsumer(&ThreadFuncs::endingThread, tf, std::ref(mwdBufferQueue), std::ref(pool), 6, 6);

  // Wait for threads to complete
  starter.join();
  checkData.join();
  formEvents.join();
  rawProcess.join();
  zsProcess.join();
  mwdProcess.join();
  finalConsumer.join();

  // Shutdown all queues
  startBufferQueue.shutdown();
  checkDataQueue.shutdown();
  formEventsQueue.shutdown();
  rawBufferQueue.shutdown();
  zsBufferQueue.shutdown();
  mwdBufferQueue.shutdown();

  return 0;
}
