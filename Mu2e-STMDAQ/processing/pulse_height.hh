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
    int64_t pulseStartGlobal;
    int searchEnd;
    float adaptiveThreshold;
    float baselineMean;
    float baselineStd;
    bool stitchedCandidate = false;
  };

  struct FinderState {
    bool firstPulse = true;
    int64_t lastConfirmedPulseEnd = -200;
    int carryoverDeadSamples = 0;
    float baselineMean = 0.0f;
    float baselineStd = 30.0f;
    float baselineMod = 0.0f;
    int64_t globalProcessedSamples = 0;
  };

  FinderState state;

  Config& cfg;

  const std::shared_ptr<AsyncLogger>& logger;
  const std::shared_ptr<STMdata>& stm;

  size_t pulse_num = 0;
  int64_t global_sample_offset = 0;
  bool print_debug = false;
  bool hotpath = false;
  
  std::unordered_map<std::string,std::function<void(std::shared_ptr<DataStruct>&,std::shared_ptr<DataStruct>&)>> functionMap;

  // queue for passing pulse indices between threads
  std::unique_ptr<boost::lockfree::spsc_queue<PulseCandidate>> indexQueue;
  // vector for reuse
  std::vector<int16_t> stitched;

public:

  PulseHeight(Config& cfg_,
              const std::shared_ptr<AsyncLogger>& logger_,
              const std::shared_ptr<STMdata>& stm_);

  ~PulseHeight(){
    if (logger) {

      logger->log("PulseHeight: Found " +
                  std::to_string(pulse_num) +
                  " pulses.",
                  1);
    }

    std::cout << "PulseHeight destructor called.\n";
  }

  std::pair<float, float>calculateBaseline(const std::vector<int16_t>& ADC,int pulseStart);

  void getSGAndParabola(const std::vector<int16_t>& data,
                        int center,
                        float adaptiveThreshold,
			float baselineMean,
                        const std::array<float,7>& sg,
                        int& minima_index,
                        float& t_min_idx,
                        float & fitted_min,
                        float& pulse_height,
			int16_t& raw_min_val);

  void detectPulseCandidates(std::shared_ptr<DataStruct>& currentBuffer,
                             std::shared_ptr<DataStruct>& previousBuffer);

  void processPulseCandidates(std::shared_ptr<DataStruct>& currentBuffer,
                              std::shared_ptr<DataStruct>& previousBuffer);

  void execute(const std::string& methodName,
               std::shared_ptr<DataStruct>& currentBuffer) override
  {
    auto it = functionMap.find(methodName);

    if (it != functionMap.end()) {

      std::shared_ptr<DataStruct> nullBuffer = nullptr;

      it->second(currentBuffer, nullBuffer);
    }
    else {
      std::cerr
        << "Error: Invalid method name '"
        << methodName
        << "'\n";
    }
  }
  
  void execute(const std::string& methodName,
               std::shared_ptr<DataStruct>& currentBuffer,
               std::shared_ptr<DataStruct>& previousBuffer) override {
    auto it = functionMap.find(methodName);

    if (it != functionMap.end()) {
      it->second(currentBuffer, previousBuffer);
    }
    else {
      std::cerr << "Error: Invalid method name '" << methodName << "'\n";
    }
  }

  bool requires_two_buffers(const std::string& methodName) const override {
    return methodName == "detectPulseCandidates" ||
      methodName == "processPulseCandidates";
  }
};

#endif
