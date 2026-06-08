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

  // Reserve space in stitched vector
  size_t stitched_len = stm->pulseheight_config.localWindowMax * 2;
  stitched.reserve(stitched_len);

  functionMap["detectPulseCandidates"] =
    [this](std::shared_ptr<DataStruct>& current,std::shared_ptr<DataStruct>& previous) {
      detectPulseCandidates(current, previous);
    };

  functionMap["processPulseCandidates"] =
    [this](std::shared_ptr<DataStruct>& current,std::shared_ptr<DataStruct>& previous) {
      processPulseCandidates(current, previous);
    };
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

  const float stddev = std::sqrt(sq_sum / baselineN);

  return {mean, stddev};
}

// -------------------- SG + Parabola --------------------
void PulseHeight::getSGAndParabola(const std::vector<int16_t>& data,
                                   int center,
                                   float adaptiveThreshold,
				   float baselineMean,
                                   const std::array<float,7>& sg,
                                   int& minima_index,
                                   float& t_min_idx,
                                   float& fitted_min,
                                   float& pulse_height,
				   int16_t& raw_min_val)
{
  const int size = static_cast<int>(data.size());

  if (center < 4 || center >= size - 4) {
    minima_index = center;
    raw_min_val = data[center];
    t_min_idx = static_cast<float>(center);
    pulse_height = baselineMean - static_cast<float>(raw_min_val);
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

  raw_min_val = min_val;

  // Require pulse persistence below threshold
  int persistence_count = 0;
  
  const int persistence_end = std::min(minima_index + cfg.persistenceWindow, size);
  
  for (int i = minima_index; i < persistence_end; ++i) {
    if (d[i] < -adaptiveThreshold * cfg.persistenceFraction)
      ++persistence_count;
  }

  if (persistence_count < cfg.persistenceCount) {
    t_min_idx = static_cast<float>(minima_index);
    pulse_height = 0.0f;
    return;
  }
  
  // Edge protection for SG filter
  if (minima_index < 4 || minima_index >= size - 4) {
    t_min_idx = static_cast<float>(minima_index);
    pulse_height = baselineMean - static_cast<float>(min_val);
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
  const float curvature = std::abs(denom);
  const float curvature_threshold = cfg.curvature_threshold;
  fitted_min = 0.0f;
  
  // Parabolic fit
  if (curvature > curvature_threshold) {
    float delta = 0.5f * (y0 - y2) / denom;
    delta = std::clamp(delta, -1.0f, 1.0f); 
    t_min_idx = static_cast<float>(minima_index) + delta;
    fitted_min = y1 - 0.25f * (y0 - y2) * delta;
  }
  else {
    t_min_idx = static_cast<float>(minima_index);
    fitted_min = y1;
  }

  pulse_height = baselineMean - fitted_min;
  
}

// -------------------- Detect pulse start indices --------------------
void PulseHeight::detectPulseCandidates(std::shared_ptr<DataStruct>& currentBuffer,
					std::shared_ptr<DataStruct>& previousBuffer)
{
  if (hotpath) return;

  const std::vector<int16_t>& ADC = currentBuffer->raw;
  const int size = static_cast<int>(ADC.size());

  if (size <= 0) return;

  const auto& cfg = stm->pulseheight_config;
  auto& firstPulse = state.firstPulse;
  auto& lastConfirmedPulseEnd = state.lastConfirmedPulseEnd;
  auto& baselineMean = state.baselineMean;
  auto& baselineStd = state.baselineStd;
  auto& baselineMod = state.baselineMod;
  int pos = state.carryoverDeadSamples;

  // Boundary continuation check
  if (previousBuffer && !previousBuffer->raw.empty()) {

    const auto& prevADC = previousBuffer->raw;
    const int prevSize = static_cast<int>(prevADC.size());

    if (prevSize >= 2 && size >= 2) {
      const int prevLast2 = prevADC[prevSize - 2];
      const int prevLast1 = prevADC[prevSize - 1];
      const int curr0 = ADC[0];
      const int curr1 = ADC[1];
      const bool monotonicFall = prevLast1 < prevLast2 && curr0 < prevLast1 && curr1 < curr0;
      const bool thresholdCross = prevLast1 < -cfg.threshold || curr0 < -cfg.threshold;
      const float grad = static_cast<float>(prevLast2 - curr1) / 3.0f;
      if (monotonicFall && thresholdCross && grad > cfg.slopeThreshold) {

	int localWindowDynamic = cfg.slopeWindow;

	while ((localWindowDynamic + 1) < size &&
	       ADC[localWindowDynamic] <= (ADC[localWindowDynamic - 1] + cfg.noiseTolerance) &&
	       localWindowDynamic < cfg.localWindowMax) {

	  ++localWindowDynamic;
	}

	const int searchEnd = localWindowDynamic;

	PulseCandidate candidate;
	candidate.pulseStart = 0; 
	candidate.pulseStartGlobal = state.globalProcessedSamples;
	candidate.searchEnd = searchEnd;
	candidate.adaptiveThreshold = cfg.threshold;
	candidate.baselineMean = baselineMean;
	candidate.baselineStd = baselineStd;
	candidate.stitchedCandidate = true;

	if (!indexQueue->push(candidate)) {
	  std::this_thread::yield();
	}

	lastConfirmedPulseEnd = state.globalProcessedSamples + searchEnd;
	pos = searchEnd + 1;
      }
    }
  }

  // Main streaming detector
  while (pos + cfg.slopeWindow < size) {
    
    const int current = ADC[pos];
    const int future = ADC[pos + cfg.slopeWindow];
    const float trigThreshold = cfg.threshold + baselineMod;

    // Fast coarse rejection
    if (!(future < -trigThreshold)) {
      ++pos;
      continue;
    }

    // Edge gradient
    const float grad = static_cast<float>(current - future) / cfg.slopeWindow;

    // Monotonic sharp-drop validation
    bool isSharpDrop = true;

    for (int j = 1; j <= cfg.Nsharp && pos + j < size; ++j) {

      if (ADC[pos + j] >= ADC[pos + j - 1]) {
	isSharpDrop = false;

	break;
      }
    }

    // Final trigger decision
    if (grad <= cfg.slopeThreshold || !isSharpDrop) {
      ++pos;
      continue;
    }

    const int pulseStart = pos;
    const int64_t pulseStartGlobal = state.globalProcessedSamples + pulseStart;

    // Recalculate baseline only when needed
    if (firstPulse || pulseStartGlobal - lastConfirmedPulseEnd > cfg.minBaselineCheck) {
      auto baselineStats = calculateBaseline(ADC, pulseStart);
      baselineMean = baselineStats.first;
      baselineStd = std::max(1.0f, baselineStats.second);
      baselineMod = 2.0f * std::min(20.0f, baselineStd);
      firstPulse = false;
    }

    // Adaptive local pulse growth
    int localWindowDynamic = cfg.slopeWindow;

    while ((pulseStart + localWindowDynamic + 1) < size &&
	   ADC[pulseStart + localWindowDynamic] <= (ADC[pulseStart + localWindowDynamic - 1] + cfg.noiseTolerance) && 
	   localWindowDynamic < cfg.localWindowMax) {
      ++localWindowDynamic;
    }

    const int searchEnd = pulseStart + localWindowDynamic;

    // Store detector state
    lastConfirmedPulseEnd = state.globalProcessedSamples + searchEnd;

    // Build candidate
    PulseCandidate candidate;
    candidate.pulseStart = pulseStart;
    candidate.pulseStartGlobal = pulseStartGlobal;
    candidate.searchEnd = searchEnd;
    candidate.adaptiveThreshold = cfg.threshold + baselineMod;
    candidate.baselineMean = baselineMean;
    candidate.baselineStd = baselineStd;
    candidate.stitchedCandidate = false;

    // Push candidate
    if (!indexQueue->push(candidate)) {
      std::this_thread::yield();
    }

    // streaming advancement
    pos = searchEnd + 1;
  }

  // Preserve deadtime continuity
  state.carryoverDeadSamples = static_cast<int>(std::max<int64_t>(0, lastConfirmedPulseEnd - (state.globalProcessedSamples + static_cast<int64_t>(size))));

  // Global continuity
  state.globalProcessedSamples += size;
}

// -------------------- Process pulse candidates --------------------
void PulseHeight::processPulseCandidates(std::shared_ptr<DataStruct>& currentBuffer,
                                         std::shared_ptr<DataStruct>& previousBuffer)
{

  const std::vector<int16_t>& ADC = currentBuffer->raw;
  const auto& cfg = stm->pulseheight_config;

  if (currentBuffer->EWTs.empty()) return;

  // Reset counters
  currentBuffer->peak_count = 0;
  currentBuffer->ph_len = 0;

  size_t peak_count = 0; // Local count of peaks
  size_t EWT_count = 0; // Count of EWTs

  // Get EWT info for current buffer
  EWT_info* this_EWT = &currentBuffer->EWTs[EWT_count];
  int64_t EWT_start = this_EWT->raw.start;
  int64_t EWT_len = this_EWT->raw.len;
  int64_t new_EWT_loc = EWT_start + EWT_len;

  PulseCandidate candidate;

  while (indexQueue->pop(candidate)) {

    int minima_index;
    float t_min_idx;
    float amp;
    int16_t raw_min_val;
    float fitted_min = 0.0f;
    int overlap = 0;

    // Check whether we need to stitch buffers
    bool stitchedPulse = candidate.stitchedCandidate && previousBuffer && !previousBuffer->raw.empty();

    if (stitchedPulse) {
      const std::vector<int16_t>& prevADC = previousBuffer->raw;
      overlap = std::min(static_cast<int>(prevADC.size()),cfg.localWindowMax);

      // Refresh vector
      stitched.clear();

      // Stitch end of last buffer
      stitched.insert(stitched.end(),prevADC.end() - overlap,prevADC.end());

      // Stitch start of current buffer
      stitched.insert(stitched.end(),ADC.begin(),ADC.end());

      getSGAndParabola(stitched,
                       overlap,
                       candidate.adaptiveThreshold,
		       candidate.baselineMean,
                       cfg.sg7_coeffs,
                       minima_index,
                       t_min_idx,
                       fitted_min,
                       amp,
		       raw_min_val);

      // Pulse belongs to PREVIOUS EWT
      if (!previousBuffer->EWTs.empty()) {
        this_EWT = &previousBuffer->EWTs.back();
        EWT_start = this_EWT->raw.start;
        EWT_len = this_EWT->raw.len;
      }
    }

    // Normal path
    else {

      getSGAndParabola(ADC,
                       candidate.pulseStart,
                       candidate.adaptiveThreshold,
		       candidate.baselineMean,
                       cfg.sg7_coeffs,
                       minima_index,
                       t_min_idx,
                       fitted_min,
                       amp,
		       raw_min_val);

      int64_t abs_pos = candidate.pulseStartGlobal - candidate.pulseStart + static_cast<int64_t>(t_min_idx);

      while (abs_pos >= new_EWT_loc) {

        ++EWT_count;

        if (EWT_count >= currentBuffer->EWTs.size()) {
          this_EWT = nullptr;
          break;
        }

        this_EWT = &currentBuffer->EWTs[EWT_count];
        EWT_start = this_EWT->raw.start;
        EWT_len = this_EWT->raw.len;
        new_EWT_loc = EWT_start + EWT_len;
      }

      if (!this_EWT) break;
    }

    const bool contaminatedBaseline = candidate.baselineStd > stm->pulseheight_config.contaminatedBaselineStd;

    // If pileup then baseline is contaminated - take the pulse shoulder as baseline
    if (contaminatedBaseline) {

      const std::vector<int16_t>& data = stitchedPulse ? stitched : ADC;
      const int minIdx = static_cast<int>(std::round(t_min_idx));
      const int triggerIdx = stitchedPulse ? overlap : candidate.pulseStart;
      const int searchStart = std::max(0, triggerIdx - static_cast<int>(cfg.localWindowMax));
      float shoulder = static_cast<float>(data[searchStart]);

      for (int i = searchStart; i < minIdx; ++i) {
	shoulder = std::max(shoulder, static_cast<float>(data[i]));
      }

      amp = shoulder - fitted_min;
    }

    int64_t abs_in_original = candidate.pulseStartGlobal - candidate.pulseStart + static_cast<int64_t>(t_min_idx);
    if (stitchedPulse) {
      abs_in_original -= overlap;
    }

    int64_t peak_time_from_ewt = abs_in_original - EWT_start;

    if (peak_time_from_ewt < 0 || peak_time_from_ewt >= EWT_len) {
      continue;
    }

    const float dynamicMinPulseHeight = contaminatedBaseline ? 30.0f : std::max(30.0f, 3.0f * candidate.baselineStd);

    // Reject weak reconstructed amplitudes
    if ((raw_min_val > -dynamicMinPulseHeight) || (amp <= 0.0f)) {      
      continue;
    }

    // Truncate to int16
    const int16_t time16 = static_cast<int16_t>(peak_time_from_ewt);
    const int16_t amp16 = static_cast<int16_t>(amp + 0.5f);

    // Store pulse in correct buffer
    if (stitchedPulse) {
      double* data_ptr = previousBuffer->ph.data();      
      const size_t idx = previousBuffer->peak_count;
      
      data_ptr[2 * idx] = static_cast<double>(peak_time_from_ewt);
      data_ptr[2 * idx + 1] = static_cast<double>(amp);
      
      ++previousBuffer->peak_count;
      previousBuffer->ph_len = 2 * previousBuffer->peak_count;
    }
    else {
      double* data_ptr = currentBuffer->ph.data();      

      logger->log("PulseHeight ACCEPT: "
		  "peak_time_from_ewt=" +
		  std::to_string(peak_time_from_ewt) +
		  " amp=" +
		  std::to_string(amp) +
		  " peak_count=" +
		  std::to_string(peak_count) +
		  " current_ph_len=" +
		  std::to_string(currentBuffer->ph_len) +
		  " current_peak_count=" +
		  std::to_string(currentBuffer->peak_count) +
		  " EWT_start=" +
		  std::to_string(EWT_start) +
		  " EWT_len=" +
		  std::to_string(EWT_len) +
		  " minima_index=" +
		  std::to_string(minima_index) +
		  " t_min_idx=" +
		  std::to_string(t_min_idx),
		  1);
      
      data_ptr[2 * peak_count] = static_cast<double>(peak_time_from_ewt);
      data_ptr[2 * peak_count + 1] = static_cast<double>(amp);

      logger->log("PulseHeight WRITE: "
		  "stored_time=" +
		  std::to_string(data_ptr[2 * peak_count]) +
		  " stored_amp=" +
		  std::to_string(data_ptr[2 * peak_count + 1]) +
		  " updated_peak_count=" +
		  std::to_string(peak_count + 1),
		  1);
      
      ++peak_count;
    }

    
    this_EWT->ph.emplace_back(time16, amp16);

    ++pulse_num;

  }

  // Increment counters
  currentBuffer->peak_count = peak_count;
  currentBuffer->ph_len = 2 * peak_count;
  
}
