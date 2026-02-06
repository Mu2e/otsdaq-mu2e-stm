#ifndef PRESCALE_CONFIG_HH_
#define PRESCALE_CONFIG_HH_

// Include config files
#include "Mu2e-STMDAQ/config/config.hh"

// Spill prescale struct
struct spill_t {
  const int prescale; // Prescale value 
  const int consecutive; // Consecutive events to store
  const int eff_prescale; // Effective prescale
  spill_t(int p, int c) :
    prescale(p), consecutive(c), eff_prescale(c*p) {} // Constructor
};

// Prescale struct
struct prescale_t {
  
  // Declare on-spill / off-spill structs
  const spill_t on_spill;
  const spill_t off_spill;
  
  // Prescale struct constructor
  prescale_t(int on_p, int on_c, int off_p, int off_c)
    : on_spill(on_p, on_c), off_spill(off_p, off_c) {}
};


// Prescale configurable variables
struct prescale_info{

  // Raw prescale struct
  const prescale_t raw;
  // ZS prescale struct
  const prescale_t zs;
  // Raw and prescale structs paired together
  const std::array<std::reference_wrapper<const prescale_t>, 2> prescales;

  // Constructor
  prescale_info(Config& cfg,
                const std::shared_ptr<AsyncLogger> logger):
    raw(cfg.getValue<int>("stm.prescale.raw.on_spill.prescale_num"),
        cfg.getValue<int>("stm.prescale.raw.on_spill.store_consecutive"),
        cfg.getValue<int>("stm.prescale.raw.off_spill.prescale_num"),
        cfg.getValue<int>("stm.prescale.raw.off_spill.store_consecutive")),
    zs(cfg.getValue<int>("stm.prescale.zs.on_spill.prescale_num"),
       cfg.getValue<int>("stm.prescale.zs.on_spill.store_consecutive"),
       cfg.getValue<int>("stm.prescale.zs.off_spill.prescale_num"),
       cfg.getValue<int>("stm.prescale.zs.off_spill.store_consecutive")),
    prescales({raw,zs})
  {
    
    // Notify user
    if (logger){
      logger->log("Config:prescale_info: Raw on-spill prescale =  " +
                  std::to_string(raw.on_spill.prescale) +
                  ". Storing " +
                  std::to_string(raw.on_spill.consecutive) +
                  " consecutive events. Effective prescale =  " +
                  std::to_string(raw.on_spill.eff_prescale) +                
                  ".",1);
      logger->log("Config:prescale_info: Raw off-spill prescale =  " +
                  std::to_string(raw.off_spill.prescale) +
                  ". Storing " +
                  std::to_string(raw.off_spill.consecutive) +
                  " consecutive events. Effective prescale =  " +
                  std::to_string(raw.off_spill.eff_prescale) +                
                  ".",1);
      logger->log("Config:prescale_info: ZS on-spill prescale =  " +
                  std::to_string(zs.on_spill.prescale) +
                  ". Storing " +
                  std::to_string(zs.on_spill.consecutive) +
                  " consecutive events. Effective prescale =  " +
                  std::to_string(zs.on_spill.eff_prescale) +                
                  ".",1);
      logger->log("Config:prescale_info: ZS off-spill prescale =  " +
                  std::to_string(zs.off_spill.prescale) +
                  ". Storing " +
                  std::to_string(zs.off_spill.consecutive) +
                  " consecutive events. Effective prescale =  " +
                  std::to_string(zs.off_spill.eff_prescale) +                
                  ".",1);    

    }

  }
  
};

#endif
