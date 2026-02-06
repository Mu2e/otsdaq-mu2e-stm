#pragma once
#include <atomic>

namespace stop {

  // Global flags 
  inline std::atomic<bool> user_stop{false};
  inline std::atomic<bool> critical_stop{false};

  // Check any stop
  inline bool should_stop() noexcept {
    return user_stop.load(std::memory_order_acquire) ||
      critical_stop.load(std::memory_order_acquire);
  }

  // Check critical stop only
  inline bool should_critical_stop() noexcept {
    return critical_stop.load(std::memory_order_acquire);
  }

  // Trigger flags
  inline void trigger_user_stop() noexcept {
    user_stop.store(true, std::memory_order_release);
  }
  inline void trigger_critical_stop() noexcept {
    critical_stop.store(true, std::memory_order_release);
  }

  // Reset for reuse 
  inline void reset_stops() noexcept {
    user_stop.store(false, std::memory_order_relaxed);
    critical_stop.store(false, std::memory_order_relaxed);
  }

} // namespace stop
