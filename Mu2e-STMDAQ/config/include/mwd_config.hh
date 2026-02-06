#ifndef MWD_CONFIG_HH_
#define MWD_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/config/include/fw_config.hh"

// Zero suppression configurable variables
struct mwd_info{

  // Baseline subtraction
  const bool use_manual_baseline; // Use manual value?
  const double manual_mean, manual_sigma; // Manual (user config) mean and sigma
  const bool use_window_baseline; // If not manual, use dynamic sliding window baseline?

  // Deconvolution
  const double tau, tau_norm; // HPGe decay time constant (us)                                   
  
  /// Differentiation
  const int M; // M value (must be power of 2)
  const size_t m_mask; // M-1 

  // Averaging
  const int L; // L value (must be power of 2)
  const double inv_L; // 1/L
  const size_t l_mask; // L-1

  // Peak finding
  const double peak_min_init; // The initial minimum value for the peak
  const bool use_fixed_cut; // Use fixed peak finding threshold cut?
  const double fixed_cut_value; // Fixed cut value
  const double nsigma_cut; // Dynamic cut: number of sigma below baseline
  
  // Constructor
  mwd_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger,
           fw_info fw_config,
           baseline_info baseline_config) :
    // Baseline subtraction
    use_manual_baseline(cfg.getValue<int>("stm.mwd.baseline.use_manual_value")), 
    manual_mean(cfg.getValue<double>("stm.mwd.baseline.manual_value.mean")),
    manual_sigma(cfg.getValue<double>("stm.mwd.baseline.manual_value.sigma")),
    use_window_baseline(cfg.getValue<int>("stm.mwd.baseline.use_window_value")),
    // Deconvolution
    tau(cfg.getValue<double>("stm.mwd.deconv.hpge_tau")), // Decay constant (us)
    tau_norm(1.0-fw_config.tADC/tau),
    // Differentiation
    M(cfg.getValue<int>("stm.mwd.diff.M")), // M value
    m_mask(M-1), // M-1
    // Averaging
    L(cfg.getValue<int>("stm.mwd.avg.L")),  // L value
    inv_L(1.0/static_cast<double>(L)), // Reciprocal of L  
    l_mask(L-1), // L-1
    // Peak finding
    peak_min_init(cfg.getValue<double>("stm.mwd.peak_finding.peak_min_init")), // minimum peak value
    use_fixed_cut(cfg.getValue<int>("stm.mwd.peak_finding.use_fixed_cut")), // Fixed  cut?
    fixed_cut_value(cfg.getValue<double>("stm.mwd.peak_finding.fixed_cut_value")), // Fixed value
    nsigma_cut(cfg.getValue<double>("stm.mwd.peak_finding.nsigma_cut")) // Dynamic sigma cut
  {
    if (logger){
      // Check that M value is a power of 2 
      if ((M & (M - 1)) != 0) {
        logger->log("Config:mwd_info: Error! M must be a power of 2! M = "
                    + std::to_string(M) + " in " + cfg.getXMLpath() + ".",0);
      }
      
      // Check that L value is a power of 2 
      if ((L & (L - 1)) != 0) {
        logger->log("Config:mwd_info: Error! L must be a power of 2! L = "
                    + std::to_string(L) + " in " + cfg.getXMLpath() + ".",0);
      }

      // Check whether baseline operations are on
      const bool ops_baseline_class =
        cfg.getValue<int>("stm.operations.Baseline");
      const bool ops_baseline_func =
        cfg.getValue<int>("stm.operations.Baseline.calc_baseline");

      // Check any of the conditions requiring fallback to manual baseline
      if (!ops_baseline_class || !ops_baseline_func){        
        std::string msg =
          "Config:mwd_info: Baseline operations disabled in config. Forcing use of manual (user) baseline.";
        logger->log(msg, 2);        
        // Force manual baseline
        const_cast<bool&>(use_manual_baseline) = true;
      }
      
      // Notify user      
      if (use_manual_baseline){ // If using manual baseline value
        logger->log("Config:mwd_info: Using manual (user) baseline value of " +
                    std::to_string(manual_mean) +
                    " ± " + std::to_string(manual_sigma) +
                    " ADCs for baseline subtraction and pulse height calculation.",1);
      } 
      // If if not manual baseline
      else{
        if (use_window_baseline){ // If using sliding window
          logger->log("Config:mwd_info: Using " +
                      std::to_string(baseline_config.hist_window_period) +
                      " second sliding window baseline for baseline subtraction "
                      "and pulse height calculation.", 1);
        }
        else{ // Else use baseline value calculated since start of run
          logger->log("Config:mwd_info: Using baseline calculated continuosly since "
                      "start of run for baseline subtraction "
                      "and pulse height calculation.", 1);
        }
      }
      
      logger->log("Config:mwd_info HPGe decay time constant [tau] = "
                  + std::to_string(tau)
                  + " us: 1 - tADC/tau = "
                  + std::to_string(tau_norm) + ".",1);      
      logger->log("Config:mwd_info M/L values = " + std::to_string(M)
                  + "/" + std::to_string(L) + ".",1);
      logger->log("Config:mwd_info Initial minimum value for peak = "
                  + std::to_string(peak_min_init) + ".",1);
      std::string fixed_cut = (use_fixed_cut) ? "TRUE" : "FALSE";
      logger->log("Config:mwd_info Using fixed peak finding threshold cut = " + fixed_cut + ".",1);    
      if (use_fixed_cut){
        logger->log("Config:mwd_info Fixed peak finding threshold cut value = "
                    + std::to_string(fixed_cut_value) + ".",1);
      }
      else{
        logger->log("Config:mwd_info Calculating peak finding cut value as "
                    + std::to_string(nsigma_cut) + " sigmas away from baseline mean.",1);
      }
    }       
  }
  
};

#endif
