// -*- mode: c++; -*-
#ifndef DQM_MANAGER_TPP
#define DQM_MANAGER_TPP

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <stdexcept>

template <typename T>
void DQM::registerBlock(DQMPageType type, const std::string& shm_name, size_t size) {
  if (blocks_.count(type)) {
    throw std::runtime_error("DQM block already registered.");
  }

  // Register in manager
  SHMmanager::Instance().registerBlock<T>(type, shm_name, size);

  // Log shm creation
  logger->log("Registered DQM shared memory block with SHMmanager: " + shm_name +
      " with size: " + std::to_string(size),1);
}

template <typename T>
T* DQM::get(DQMPageType type) {
  // Retrieve the pointer from the global registry
  T* ptr = SHMmanager::Instance().get<T>(type);
  if (!ptr) {
      throw std::runtime_error("Requested DQM block not found in SHMmanager.");
  }
  return ptr;
}

#endif // DQM_MANAGER_TPP
