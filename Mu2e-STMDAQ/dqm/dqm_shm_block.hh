#ifndef DQM_SHM_BLOCK_HH
#define DQM_SHM_BLOCK_HH

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <string>
#include <memory>
#include <cstring>

namespace bip = boost::interprocess;

// Enum representing unique pages of the DQM
enum class DQMPageType {
  BASELINE,
  RAW,
  PEAKS,
  SPEEDS,
  ALARMS
};

// Base class
class DQMBlockBase {
public:
    virtual ~DQMBlockBase() = default;
    virtual void* get() = 0;
};

template <typename T>
class DQMBlock : DQMBlockBase {
public:
  DQMBlock(const std::string& shm_name, size_t shm_size, bool persist = false)
    : shm_name_(shm_name), shm_size_(shm_size), persist_(persist) {
      
      // Remove existing and recreate (Creator mode)
      bip::shared_memory_object::remove(shm_name_.c_str());
      shm_ = std::make_unique<bip::shared_memory_object>(
	    bip::open_or_create, shm_name_.c_str(), bip::read_write);

      shm_->truncate(shm_size_);
      region_ = std::make_unique<bip::mapped_region>(*shm_, bip::read_write);
      std::memset(region_->get_address(), 0, shm_size_);
  }

  ~DQMBlock() {
      region_.reset();
      shm_.reset();
      // Clean up if not set to persist
      if (!persist_) {
	bip::shared_memory_object::remove(shm_name_.c_str());
      }
  }

  // Return typed pointer to shared memory
  T* getTyped() { return static_cast<T*>(region_->get_address()); }
  void* get() { return region_->get_address(); }

private:
  // Shared memory name
  std::string shm_name_;
  // Size of the shared memory region
  size_t shm_size_;
  // Bool to say whether the SHM should remain after DAQ (for alarms)
  bool persist_;
  // Boost shm object
  std::unique_ptr<boost::interprocess::shared_memory_object> shm_;
  // Boost shm region
  std::unique_ptr<boost::interprocess::mapped_region> region_;
};

#endif
