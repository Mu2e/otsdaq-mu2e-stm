#include <thread> 

// Include CPU utils header
#include "Mu2e-STMDAQ/utils/cpu_utils.hh"

// Define static members for singleton management
std::shared_ptr<cpu_utils> cpu_utils::instance = nullptr;
std::once_flag cpu_utils::init_flag;

// Returns the singleton instance; initializes on first call
std::shared_ptr<cpu_utils> cpu_utils::getInstance(const Config& cfg) {
  std::call_once(init_flag, [&]() {
    instance = std::shared_ptr<cpu_utils>(new cpu_utils(cfg));
  });
  return instance;
}

// Constructor: sets up configuration, starting core, and detects hardware concurrency
cpu_utils::cpu_utils(const Config& cfg_)
  : cfg(cfg_), starting_core_id(cfg.getValue<int>("stm.starting_core_id")), next_core_id(0) {
  
  // Query total number of CPU cores
  max_cores = std::thread::hardware_concurrency();
  
  // Fallback if detection fails
  if (max_cores == 0) {
    std::cerr << "Warning: Unable to determine number of hardware cores. Defaulting to 1." << std::endl;
    max_cores = 1;
  }

  // Log initialization summary
  std::cout << "CPU Utils initialised: starting_core_id = "
	    << starting_core_id << ", max_cores = " << max_cores << std::endl;
  
}

// Main interface: assigns a core to the calling thread and returns the core ID
size_t cpu_utils::get_next_core(const std::string& name) {

  // Atomically increment counter and compute core ID using modulo wraparound
  size_t local_id = next_core_id.fetch_add(1);
  size_t core_id = (starting_core_id + local_id) % max_cores;
  
  {
    // Lock tracking data structure
    std::lock_guard<std::mutex> lock(tracking_mutex);
    
    // Warn if this core has already been used
    if (used_cores.find(core_id) != used_cores.end()) {
      std::cerr << "Warning: Core " << core_id << " has already been assigned to another thread!" << std::endl;
    }
    else {
      used_cores.insert(core_id);  // Mark core as used
    }
    
    // Warn if we've assigned more threads than available cores
    if (used_cores.size() > max_cores) {
      std::cerr << "Warning: Number of threads assigned exceeds available cores ("
		<< max_cores << ")!" << std::endl;
    }
  }

  // Actually pin the calling thread to the computed core
  pin_thread_to_core(core_id, name);

  // Return the assigned core ID
  return core_id;
}

// Low-level function to pin thread to specific CPU core
void cpu_utils::pin_thread_to_core(size_t core_id, const std::string& name) {
  
  // Create and configure a CPU set
  cpu_set_t cpuset;
  // Clear the set
  CPU_ZERO(&cpuset);
  // Add the specified core to the set
  CPU_SET(core_id, &cpuset);     

  // Attempt to apply CPU affinity
  int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    
  // Log result to user
  if (result != 0) {
    std::cerr << "Failed to pin " << name << " thread to core " << core_id
	      << " (error code: " << result << ")" << std::endl;
  }
  else {
    std::cout << "Pinned " << name << " thread to core " << core_id << std::endl;
  }
  
}


