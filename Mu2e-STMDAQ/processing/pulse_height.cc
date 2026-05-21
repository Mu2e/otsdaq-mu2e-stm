#include "Mu2e-STMDAQ/processing/pulse_height.hh"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
#include <limits>

// -------------------- Constructor --------------------
PulseHeight::PulseHeight(Config& cfg_,
                         const std::shared_ptr<AsyncLogger>& logger_,
                         const std::shared_ptr<STMdata>& stm_)
  : cfg(cfg_), logger(logger_), stm(stm_)
{
  pulse_num = 0;
  global_sample_offset = 0;

  // allocate SPSC queue with size from config
  size_t queue_len = stm->pulseheight_config.queue_len;
  indexQueue = std::make_unique<boost::lockfree::spsc_queue<PulseCandidate>>(queue_len);

  functionMap["detectPulseCandidates"]  = [this](std::shared_ptr<DataStruct>& buf){ detectPulseCandidates(buf); };
  functionMap["processPulseCandidates"] = [this](std::shared_ptr<DataStruct>& buf){ processPulseCandidates(buf); };
}

// -------------------- Baseline calculation --------------------
std::pair<float, float> PulseHeight::calculateBaseline(const std::vector<int16_t>& ADC, int pulseStart)
{
  const auto& cfg = stm->pulseheight_config;

  int baselineEnd = std::max(0, pulseStart - static_cast<int>(cfg.baselineLookback));

  int baselineStart = std::max(0, baselineEnd - cfg.baselineWindow);

  int baselineN = baselineEnd - baselineStart;

  if (baselineN < 4) {
    baselineStart = 0;
    baselineEnd = std::min(static_cast<int>(ADC.size()), 16);
    baselineN = baselineEnd - baselineStart;
  }

  float sum = 0.0f;

  for (int i = baselineStart; i < baselineEnd; ++i)
    sum += static_cast<float>(ADC[i]);

  const float mean = sum / baselineN;

  float sq_sum = 0.0f;

  for (int i = baselineStart; i < baselineEnd; ++i) {
    const float d = static_cast<float>(ADC[i]) - mean;
    sq_sum += d * d;
  }

  const float stddev =
    std::sqrt(sq_sum / baselineN);

  return {mean, stddev};
}

// -------------------- SG + Parabola --------------------
void PulseHeight::getSGAndParabola(const std::vector<int16_t>& data,
                                   int center,
                                   float adaptiveThreshold,
                                   const std::array<float,7>& sg,
                                   int& minima_index,
                                   float& t_min_idx,
                                   float& pulse_height)
{
  const int size = static_cast<int>(data.size());

  if (center < 4 || center >= size - 4) {
    minima_index = center;
    t_min_idx = static_cast<float>(center);
    pulse_height = 0.0f;
    return;
  }

  const auto& cfg = stm->pulseheight_config;
  const int16_t* d = data.data();
  // Search region for raw min
  int search_end = std::min(center + cfg.slopeWindow, size - 4);

  // adaptive window
  while (search_end + 1 < size - 4 &&
         d[search_end + 1] <= d[search_end] + cfg.noiseTolerance &&
         d[search_end + 1] < -adaptiveThreshold &&
         (search_end - center) < cfg.localWindowMax) {
    ++search_end;
  }

  minima_index = center;

  int16_t min_val = d[center];

  for (int i = center + 1; i <= search_end; ++i) {
    if (d[i] < min_val) {
      min_val = d[i];
      minima_index = i;
    }
  }

  // Edge protection for SG filter
  if (minima_index < 4 || minima_index >= size - 4) {
    t_min_idx = static_cast<float>(minima_index);
    pulse_height = -static_cast<float>(min_val);
    return;
  }

  // Get 3 SG-smoothed points needed for parabolic interpolation
  // unroll for speed (ugly but compiler should handle better)
  const int i0 = minima_index - 1;
  const int i1 = minima_index;
  const int i2 = minima_index + 1;

  const float y0 =
    sg[0] * d[i0 - 3] +
    sg[1] * d[i0 - 2] +
    sg[2] * d[i0 - 1] +
    sg[3] * d[i0]     +
    sg[4] * d[i0 + 1] +
    sg[5] * d[i0 + 2] +
    sg[6] * d[i0 + 3];

  const float y1 =
    sg[0] * d[i1 - 3] +
    sg[1] * d[i1 - 2] +
    sg[2] * d[i1 - 1] +
    sg[3] * d[i1]     +
    sg[4] * d[i1 + 1] +
    sg[5] * d[i1 + 2] +
    sg[6] * d[i1 + 3];

  const float y2 =
    sg[0] * d[i2 - 3] +
    sg[1] * d[i2 - 2] +
    sg[2] * d[i2 - 1] +
    sg[3] * d[i2]     +
    sg[4] * d[i2 + 1] +
    sg[5] * d[i2 + 2] +
    sg[6] * d[i2 + 3];

  const float denom = y0 - 2.0f * y1 + y2;

  constexpr float epsilon = 1e-6f;

  // Parabolic fit
  if (std::abs(denom) > epsilon) {
    float delta = 0.5f * (y0 - y2) / denom;
    // Protect against bad fits
    delta = std::clamp(delta, -1.0f, 1.0f);
    t_min_idx = static_cast<float>(minima_index) + delta;
    pulse_height = -(y1 - 0.25f * (y0 - y2) * delta);
  }
  else {
    // Fallback to discrete minimum
    t_min_idx = static_cast<float>(minima_index);
    pulse_height = -y1;
  }
}

// -------------------- Detect pulse start indices --------------------
void PulseHeight::detectPulseCandidates(std::shared_ptr<DataStruct>& buffer)
{
  if (hotpath) return;

  const std::vector<int16_t>& ADC = buffer->raw;
  const int size = static_cast<int>(ADC.size());
  const auto& cfg = stm->pulseheight_config;

  int pos = 0;
  float baselineMod = 0.0f;
  bool firstPulse = true;
  int lastConfirmedPulseEnd = -200;

  while (pos + cfg.slopeWindow < size) {

    const int current = ADC[pos];
    const int window_val = ADC[pos + cfg.slopeWindow];
    const float trigThreshold = cfg.threshold + baselineMod;

    if (window_val < current - trigThreshold || window_val < -trigThreshold) {
      const float grad = static_cast<float>(current - window_val) / cfg.slopeWindow;

      bool isSharpDrop = true;

      for (int j = 1; j <= cfg.Nsharp && pos + j < size; ++j) {
        if (ADC[pos + j] >= ADC[pos + j - 1]) {
          isSharpDrop = false;
          break;
        }
      }

      if (grad > cfg.slopeThreshold && window_val < -trigThreshold && isSharpDrop) {
        const int pulseStart = pos;

        // baseline recalculation
        if (firstPulse || pulseStart - lastConfirmedPulseEnd > cfg.minBaselineCheck) {
          auto baselineStats = calculateBaseline(ADC, pulseStart);

          baselineMod = 2.0f * std::min(20.0f, baselineStats.second);

          firstPulse = false;
        }

        const float updatedThreshold = cfg.threshold + baselineMod;
        
        lastConfirmedPulseEnd = pulseStart;

        PulseCandidate candidate;

        candidate.pulseStart = pulseStart;

        candidate.adaptiveThreshold = updatedThreshold;
        
        // Push candidate to SPSC queue
        while (!indexQueue->push(candidate)) {
          std::this_thread::yield();
        }

        pos += cfg.localWindowMax;
        continue;
      }
    }
    ++pos;
  }
}

// -------------------- Process pulse candidates --------------------
void PulseHeight::processPulseCandidates(std::shared_ptr<DataStruct>& buffer)
{
  const std::vector<int16_t>& ADC = buffer->raw;
  const auto& cfg = stm->pulseheight_config;

  if (buffer->EWTs.empty()) return;

  size_t EWT_count = 0;

  EWT_info* this_EWT = &buffer->EWTs[EWT_count];
  int64_t EWT_start = this_EWT->raw.start;
  int64_t EWT_len = this_EWT->raw.len;
  int64_t new_EWT_loc = EWT_start + EWT_len;

  PulseCandidate candidate;

  while (indexQueue->pop(candidate)) {

    int minima_index;

    float t_min_idx;
    float amp;

    getSGAndParabola(ADC,
                     candidate.pulseStart,
                     candidate.adaptiveThreshold,
                     cfg.sg7_coeffs,
                     minima_index,
                     t_min_idx,
                     amp);

    int64_t abs_in_original = static_cast<int64_t>(t_min_idx);

    while (abs_in_original >= new_EWT_loc) {

      ++EWT_count;

      if (EWT_count >= buffer->EWTs.size()) {
        this_EWT = nullptr;
        break;
      }

      this_EWT = &buffer->EWTs[EWT_count];
      EWT_start = this_EWT->raw.start;
      EWT_len = this_EWT->raw.len;
      new_EWT_loc = EWT_start + EWT_len;
    }

    if (!this_EWT) break;

    int64_t peak_time_from_ewt = abs_in_original - EWT_start;

    if (peak_time_from_ewt < 0 ||
	peak_time_from_ewt >= EWT_len) {
      continue;
    }

    const int16_t time16 = static_cast<int16_t>(peak_time_from_ewt);
    const int16_t amp16 = static_cast<int16_t>(amp + 0.5f);

    buffer->ph.push_back(time16);
    buffer->ph.push_back(amp16);

    this_EWT->ph.emplace_back(time16, amp16);

    ++pulse_num;
  }
}

