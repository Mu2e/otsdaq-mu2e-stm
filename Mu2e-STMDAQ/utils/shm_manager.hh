#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

// Shared Memory Manager
class SharedMemoryManager {

private:
  
  int shm_fd;
  void* shared_memory;
  // Global Constants
  const char* SHARED_MEMORY_NAME = "/cpp_shared_memory";
  const size_t SHARED_MEMORY_SIZE = 4096; // 4 KB
  const size_t BUFFER_DATA_SIZE = 1024;   // 256 values per thread

public:

  // Constructor
  SharedMemoryManager();

  // Destructor
  ~SharedMemoryManager() {
    munmap(shared_memory, SHARED_MEMORY_SIZE);
    close(shm_fd);
    shm_unlink(SHARED_MEMORY_NAME);
    std::cout << "SharedMemoryManager destructor called.\n";
  }

  // Write data to shared memory segment
  void write_data(const void* data, size_t offset, size_t size); 
  
};

