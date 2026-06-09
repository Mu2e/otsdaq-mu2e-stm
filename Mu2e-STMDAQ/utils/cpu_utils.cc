#include <thread>

// Include CPU utils header
#include "Mu2e-STMDAQ/utils/cpu_utils.hh"

// Define static members for singleton management
std::shared_ptr<cpu_utils> cpu_utils::instance = nullptr;
std::once_flag cpu_utils::init_flag;

// Allowed cores for NUMA socket 0
const std::vector<int> cpu_utils::socket0_cores =
{
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23
};

// Allowed cores for NUMA socket 1
const std::vector<int> cpu_utils::socket1_cores =
{
  24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47
};

// Returns the singleton instance; initializes on first call
std::shared_ptr<cpu_utils> cpu_utils::getInstance(const Config& cfg, CpuRole role) {
  
  std::call_once(init_flag, [&]() {
    instance = std::shared_ptr<cpu_utils>(new cpu_utils(cfg, role));
  });

  return instance;
}


// Constructor: sets up configuration, starting core, and detects hardware concurrency
cpu_utils::cpu_utils(const Config& cfg_, CpuRole role_)
  : cfg(cfg_), role(role_), next_core_id(0) {

  // Query total number of CPU cores
  max_cores = std::thread::hardware_concurrency();

  // Fallback if detection fails
  if(max_cores == 0){
    max_cores = 1;
  }

  // Set to NUMA node socket
  if(role == CpuRole::Standalone){
    starting_core_id = cfg.getValue<int>("stm.stmdaq_starting_core");
    socket_id = static_cast<Socket>(cfg.getValue<int>("stm.stmdaq_cpu_socket"));
  }
  else {
    starting_core_id = cfg.getValue<int>("stm.artdaq_starting_core");
    socket_id = static_cast<Socket>(cfg.getValue<int>("stm.artdaq_cpu_socket"));
  }

  allowed_cores = get_socket_cores(socket_id);

  // Log initialization summary
  std::cout
    << "CPU Utils initialised\n"
    << "  role           : "
    << (role == CpuRole::Standalone ? "Standalone" : "ArtDAQ")
    << "\n  socket         : "
    << (socket_id == Socket::Socket0 ? 0 : 1)
    << "\n  starting core  : "
    << starting_core_id
    << "\n  managed cores  : "
    << allowed_cores.size()
    << std::endl;
}

// Main interface: assigns a core to the calling thread and returns the core ID
size_t cpu_utils::get_next_core(const std::string& name) {

  // Atomically increment counter and compute core ID using modulo wraparound
  const size_t local_id = next_core_id.fetch_add(1);
  const size_t logical_core = (starting_core_id + local_id) % allowed_cores.size();
  const size_t core_id = allowed_cores[logical_core];

  {
    // Lock tracking data structure
    std::lock_guard<std::mutex> lock(tracking_mutex);

    // Warn if this core has already been used
    if(used_cores.find(core_id) != used_cores.end()){
      std::cerr
        << "Warning: Core "
        << core_id
        << " has already been assigned."
        << std::endl;
    }
    else {
      used_cores.insert(core_id);
    }

    // Warn if we've assigned more threads than available cores
    if(used_cores.size() > allowed_cores.size()){
      std::cerr 
      << "Warning: Thread count exceeds "
      << "available cores on socket."
      << std::endl;
    }
  }

  // Actually pin the calling thread to the computed core
  pin_thread_to_core(core_id, name);

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
  const int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

  // Log result to user
  if(result != 0){
    std::cerr
      << "Failed to pin "
      << name
      << " thread to core "
      << core_id
      << " (error code "
      << result
      << ")"
      << std::endl;
  }
  else {
    std::cout
      << "Pinned "
      << name
      << " thread to core "
      << core_id
      << std::endl;
  }
}
