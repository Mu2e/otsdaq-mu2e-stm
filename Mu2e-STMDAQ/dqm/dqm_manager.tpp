// -*- mode: c++; -*-
#ifndef DQM_MANAGER_TPP
#define DQM_MANAGER_TPP

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <stdexcept>

namespace bip = boost::interprocess;

// Constructor for creating and mapping shared memory
template <typename T>
DQMBlock<T>::DQMBlock(const std::string& shm_name)
  : shm_name_(shm_name), shm_size_(sizeof(T)) {
  bip::shared_memory_object::remove(shm_name_.c_str());  // Remove existing segment
  shm_ =
    std::make_unique<bip::shared_memory_object>(bip::open_or_create, shm_name.c_str(), bip::read_write);
  shm_->truncate(shm_size_);  // Resize memory
  region_ = std::make_unique<bip::mapped_region>(*shm_, bip::read_write);  // Map memory
  std::memset(region_->get_address(), 0, shm_size_);  // Zero-init
}

// Typed access to mapped region
template <typename T>
T* DQMBlock<T>::getTyped() {
  return static_cast<T*>(region_->get_address());
}

// Raw void* access
template <typename T>
void* DQMBlock<T>::get() {
  return region_->get_address();
}

// Cleanup shared memory segment
template <typename T>
DQMBlock<T>::~DQMBlock() {
  // Let region_ and shm_ be destroyed first
  region_.reset();
  shm_.reset();

  // Then remove the shared memory segment
  bip::shared_memory_object::remove(shm_name_.c_str());
}

// Register a new SHM block
template <typename T>
void DQM::registerBlock(DQMPageType type, const std::string& shm_name) {
  if (blocks_.count(type)) {
    throw std::runtime_error("DQM block already registered.");
  }
  blocks_[type] = std::make_unique<DQMBlock<T>>(shm_name);
  logger->log("Registered DQM shared memory block: " + shm_name,1);
}

// Get typed pointer to SHM block
template <typename T>
T* DQM::get(DQMPageType type) {
  auto it = blocks_.find(type);
  if (it == blocks_.end()) {
    throw std::runtime_error("DQM block not registered.");
  }
  return static_cast<DQMBlock<T>*>(it->second.get())->getTyped();
}

#endif // DQM_MANAGER_TPP
