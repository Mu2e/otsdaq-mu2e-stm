#ifndef CPU_UTILS_HH
#define CPU_UTILS_HH

#include <pthread.h>
#include <sched.h>
#include <iostream>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <string>

// Config code
#include "Mu2e-STMDAQ/config/config.hh"

// Singleton utility class for managing CPU thread pinning
class cpu_utils {

private:
  
  // Private constructor; only accessible by getInstance()
  explicit cpu_utils(const Config& cfg_);

  // Low-level function to pin thread to a specified core
  void pin_thread_to_core(size_t core_id, const std::string& name);

  // Store reference to the Config instance
  const Config& cfg;

  // First core ID to assign from (loaded from config)
  const size_t starting_core_id;

  // Total number of available hardware cores
  size_t max_cores;

  // Atomic counter for round-robin assignment of cores
  std::atomic<size_t> next_core_id;

  // Mutex to protect access to used_cores set
  std::mutex tracking_mutex;

  // Set of all cores that have been assigned so far
  std::unordered_set<size_t> used_cores;

  // Singleton instance pointer
  static std::shared_ptr<cpu_utils> instance;

  // Flag to ensure singleton is initialized only once
  static std::once_flag init_flag;

public:
  
  // Delete copy and move constructors/operators to enforce singleton pattern
  cpu_utils(const cpu_utils&) = delete;
  cpu_utils& operator=(const cpu_utils&) = delete;
  cpu_utils(cpu_utils&&) = delete;
  cpu_utils& operator=(cpu_utils&&) = delete;

  // Static method to access the singleton instance
  static std::shared_ptr<cpu_utils> getInstance(const Config& cfg);

  // Assigns the calling thread to the next available CPU core and returns the core ID
  size_t get_next_core(const std::string& name);

  // Destructor
  ~cpu_utils(){
    std::cout << "cpu_utils shutting down...\nFinal core index assigned: "
	      << next_core_id.load() << ". Unique cores used: " << used_cores.size() << std::endl;    
    std::cout << "cpu_utils destructor called.\n";
  }

};

#endif // CPU_UTILS_H
