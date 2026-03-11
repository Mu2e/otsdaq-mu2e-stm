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
  }

  // Readable anywhere downstream (no object needed)
  static const std::string& run_start_timestamp() {
    static const std::string ts = []{
      std::time_t t = std::chrono::system_clock::to_time_t(run_start_time());
      std::tm tm = *std::localtime(&t);
      std::ostringstream oss;
      oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
      return oss.str();
    }();
    return ts;
  }

  static const std::string& run_start_utc_string() {
    static const std::string s = []{
      std::time_t t = std::chrono::system_clock::to_time_t(run_start_time());
      return std::string(std::ctime(&t)); // includes newline
    }();
    return s;
  }

private:
  static const std::chrono::system_clock::time_point& run_start_time() {
    static const auto t0 = std::chrono::system_clock::now();
    return t0;
  }

  const std::chrono::steady_clock::time_point start_steady;
  const std::chrono::system_clock::time_point start_sys;
};



#endif
