#ifndef PULSE_HEIGHT_hh
#define PULSE_HEIGHT_hh

#include "Mu2e-STMDAQ/utils/async_logger.hh"
#include "Mu2e-STMDAQ/config/stm_data.hh"
#include "Mu2e-STMDAQ/processing/operations_base.hh"

#include <numeric>
#include <cmath>
#include <unordered_map>
#include <functional>
#include <vector>
#include <iostream>
#include <memory>
#include <cstdint>
#include <boost/lockfree/spsc_queue.hpp>

class PulseHeight : public OperationBase {

private:

  struct PulseCandidate {
    int pulseStart;
    float adaptiveThreshold;
  };
  
  Config& cfg;
  const std::shared_ptr<AsyncLogger>& logger;
  const std::shared_ptr<STMdata>& stm;

  
  size_t pulse_num = 0;
  int64_t global_sample_offset = 0;

  std::vector<int16_t> tailSamples;

  bool print_debug = false;
  bool hotpath = false;

  std::unordered_map<std::string, std::function<void(std::shared_ptr<DataStruct>&)>> functionMap;

  // queue for passing pulse indices between threads
  std::unique_ptr<boost::lockfree::spsc_queue<PulseCandidate>> indexQueue;

public:
  PulseHeight(Config& cfg_,
              const std::shared_ptr<AsyncLogger>& logger_,
              const std::shared_ptr<STMdata>& stm_);

  ~PulseHeight() {
    if (logger) {
      logger->log("PulseHeight: Found " + std::to_string(pulse_num) + " pulses.", 1);
      logger->log("PulseHeight: Tail sample buffer size = " +
                  std::to_string(tailSamples.size() * sizeof(int16_t) * 1e-9) + " GB (" +
                  std::to_string(tailSamples.size()) + " ADCs).", 1);
    }
    std::cout << "PulseHeight destructor called.\n";
  }

  std::pair<float, float> calculateBaseline(const std::vector<int16_t>& ADC, int pulseStart);

  void getSGAndParabola(const std::vector<int16_t>& data,
                        int center,
                        float adaptiveThreshold,
                        const std::array<float,7>& sg,
                        int& minima_index,
                        float& t_min_idx,
                        float& pulse_height);

  void detectPulseCandidates(std::shared_ptr<DataStruct>& buffer);
  void processPulseCandidates(std::shared_ptr<DataStruct>& buffer);

  void execute(const std::string& methodName,
               std::shared_ptr<DataStruct>& buffer)
  {
    auto it = functionMap.find(methodName);
    
    if (it != functionMap.end()) {
      it->second(buffer);
    }
    else {
      std::cerr << "Error: Invalid method name '"
                << methodName
                << "' in PulseHeight\n";
    }
  }

  void execute(const std::string& methodName,
               std::shared_ptr<DataStruct>& buffer1,
               std::shared_ptr<DataStruct>& buffer2)
  {
    (void)buffer2;
    
    execute(methodName, buffer1);
  }
  
  bool requires_two_buffers(const std::string& methodName) const {
    (void)methodName;
    return false;
  }
  
};

#endif
