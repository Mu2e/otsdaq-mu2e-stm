// Shared memory manager header
#include "Mu2e-STMDAQ/utils/shm_manager.hh" 

// Constructor
SharedMemoryManager::SharedMemoryManager() {
  
  shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  
  if (ftruncate(shm_fd, SHARED_MEMORY_SIZE) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }
  
  shared_memory = mmap(0, SHARED_MEMORY_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  
  std::memset(shared_memory, 0, SHARED_MEMORY_SIZE);
  
}

// Write data to shared memeory segment
void SharedMemoryManager::write_data(const void* data, size_t offset, size_t size) {
  std::memcpy(static_cast<char*>(shared_memory) + offset, data, size);
}
