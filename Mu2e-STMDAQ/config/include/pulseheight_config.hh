#ifndef PULSEHEIGHT_CONFIG_HH_
#define PULSEHEIGHT_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/fw_config.hh"

// Pulse height configurable variables
struct pulseheight_info {

  // Length of queue
  const size_t queue_len;

  // Baseline
  const int baselineWindow;
  const int baselineLookback;

  // Peak detection / pulse finding
  const double threshold;
  const int slopeWindow;
  const double slopeThreshold;
  const double noiseTolerance;
  const int minBaselineCheck;
  const int Nsharp;
  const int localWindowMax;
  const int persistenceWindow;
  const float persistenceFraction;
  const int persistenceCount;
  const float curvature_threshold;
  const float contaminatedBaselineStd;
  
  // Smoothing / Savitzky-Golay
  const std::array<float,7> sg7_coeffs;

  // Constructor
  pulseheight_info(Config& cfg,
                   const std::shared_ptr<AsyncLogger> logger,
                   fw_info fw_config,
                   baseline_info baseline_config) :

    // Queue length
    queue_len(static_cast<size_t>(cfg.getValue<int>("stm.ph.queue_len"))),

    // Baseline
    baselineWindow(cfg.getValue<int>("stm.ph.baselineWindow")),
    baselineLookback(cfg.getValue<int>("stm.ph.baselineLookback")),

    // Peak detection / pulse finding
    threshold(cfg.getValue<double>("stm.ph.threshold")),
    slopeWindow(cfg.getValue<int>("stm.ph.slopeWindow")),
    slopeThreshold(cfg.getValue<double>("stm.ph.slopeThreshold")),
    noiseTolerance(cfg.getValue<double>("stm.ph.noiseTolerance")),
    minBaselineCheck(cfg.getValue<int>("stm.ph.minBaselineCheck")),
    Nsharp(cfg.getValue<int>("stm.ph.Nsharp")),
    localWindowMax(cfg.getValue<int>("stm.ph.localWindowMax")),
    persistenceWindow(cfg.getValue<int>("stm.ph.persistenceWindow")),
    persistenceFraction(cfg.getValue<float>("stm.ph.persistenceFraction")),
    persistenceCount(cfg.getValue<int>("stm.ph.persistenceCount")),
    curvature_threshold(cfg.getValue<float>("stm.ph.curvature_threshold")),
    contaminatedBaselineStd(cfg.getValue<float>("stm.ph.contaminatedBaselineStd")),

    // SG coefficients
    sg7_coeffs({
      cfg.getValue<float>("stm.ph.sg7.c0"),
      cfg.getValue<float>("stm.ph.sg7.c1"),
      cfg.getValue<float>("stm.ph.sg7.c2"),
      cfg.getValue<float>("stm.ph.sg7.c3"),
      cfg.getValue<float>("stm.ph.sg7.c4"),
      cfg.getValue<float>("stm.ph.sg7.c5"),
      cfg.getValue<float>("stm.ph.sg7.c6")
    })
  {
    if (logger) {

      logger->log("Config:pulseheight_info: Queue len = " +
                  std::to_string(queue_len) + ".", 1);

      logger->log("Config:pulseheight_info: baselineWindow = " +
                  std::to_string(baselineWindow) +
                  "; baselineLookback = " +
                  std::to_string(baselineLookback) + ".", 1);

      logger->log("Config:pulseheight_info: threshold = " +
                  std::to_string(threshold) +
                  "; slopeWindow = " +
                  std::to_string(slopeWindow) +
                  "; slopeThreshold = " +
                  std::to_string(slopeThreshold) + ".", 1);

      logger->log("Config:pulseheight_info: noiseTolerance = " +
                  std::to_string(noiseTolerance) +
                  "; minBaselineCheck = " +
                  std::to_string(minBaselineCheck) + ".", 1);

      logger->log("Config:pulseheight_info: Nsharp = " +
                  std::to_string(Nsharp) +
                  "; localWindowMax = " +
                  std::to_string(localWindowMax) + ".", 1);

      logger->log("Config:pulseheight_info: persistence window = " +
                  std::to_string(persistenceWindow) +
                  "; persistence fraction = " +
                  std::to_string(persistenceFraction) +
                  "; persistence count = " +
                  std::to_string(persistenceCount) + ".", 1);

      logger->log("Config:pulseheight_info: curvature_threshold = " +
                  std::to_string(curvature_threshold) + ".", 1);

      logger->log("Config:pulseheight_info: contaminatedBaselineStd = " +
                  std::to_string(contaminatedBaselineStd) + ".", 1);

      // Validate sg7_coeffs length
      if (sg7_coeffs.size() != 7) {

        logger->log("Config:pulseheight_info: Error! sg7_coeffs must contain 7 values.", 0);

      } else {

        std::string coeffs_str;

        for (size_t i = 0; i < sg7_coeffs.size(); ++i) {

          coeffs_str += std::to_string(sg7_coeffs[i]);

          if (i + 1 < sg7_coeffs.size())
            coeffs_str += ", ";
        }

        logger->log("Config:pulseheight_info: sg7_coeffs = [" +
                    coeffs_str + "].", 1);
      }
    }
  }
};

#endif
