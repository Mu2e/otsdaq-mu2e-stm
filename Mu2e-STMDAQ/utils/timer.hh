#ifndef TIMER_hh_
#define TIMER_hh_

#include <chrono>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

class Timer {
public:
  Timer()
    : start_steady(std::chrono::steady_clock::now()),
      start_sys(std::chrono::system_clock::now()){

    // Force initialization so it prints/exists when Timer is constructed
    (void)run_start_utc_string();
    (void)run_start_timestamp();
    std::cout << "DAQ start time: " << run_start_utc_string();
  }

  ~Timer() {
    auto end = std::chrono::steady_clock::now();
    auto duration = end - start_steady;
    double ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "DAQ run time: " << ms*1e-3 << " seconds." << std::endl;
    
    reset();
  }

  static void reset() {
    run_start_time(true);
    // Force re-initialization of the strings
    run_start_timestamp(true);
    run_start_utc_string(true);
  }

  // Readable anywhere downstream (no object needed)
  static std::string& run_start_timestamp(bool reset = false) {
    static std::string ts;
    if (reset || ts.empty()) {
      std::time_t t = std::chrono::system_clock::to_time_t(run_start_time());
      std::tm tm = *std::localtime(&t);
      std::ostringstream oss;
      oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
      ts = oss.str();
    }
    return ts;
  }

  static std::string& run_start_utc_string(bool reset = false) {
    static std::string s;
    if (reset || s.empty()) {
      std::time_t t = std::chrono::system_clock::to_time_t(run_start_time());
      s = std::string(std::ctime(&t)); // includes newline
    }
    return s;
  }

private:
  static std::chrono::system_clock::time_point& run_start_time(bool reset = false) {
    static std::chrono::system_clock::time_point t0;
    if (reset || t0 == std::chrono::system_clock::time_point{}) {
      t0 = std::chrono::system_clock::now();
    }
    return t0;
  }

  const std::chrono::steady_clock::time_point start_steady;
  const std::chrono::system_clock::time_point start_sys;
};



#endif
