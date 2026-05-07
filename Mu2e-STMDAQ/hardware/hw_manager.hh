#ifndef HARDWARE_MANAGER_hh
#define HARDWARE_MANAGER_hh

#include <memory>
#include <string>

// Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM Data Code
#include "Mu2e-STMDAQ/config/stm_data.hh"

// Hardward Manager class
class HardwareManager {

private:
  
  // Logger code
  const std::shared_ptr<AsyncLogger>& logger;
  
  // STM Data code
  const std::shared_ptr<STMdata>& stm;

  // Hardware/firmware directory
  const std::string hw_dir;
  
  // Pybind implementation
  struct Impl;
  std::unique_ptr<Impl> impl;

  
public:

  // Constructor
  HardwareManager(const std::shared_ptr<AsyncLogger>& logger_,
                  const std::shared_ptr<STMdata>& stm_);

  // Destructor
  ~HardwareManager();

  // Check whether FPGA firmware is loaded and return boolean   
  bool check_fpga_init();

  // Run script to load FPGA firmware 
  bool load_firmware();

  // Reset reaout
  void reset_readout();
  
  // Check whether ADC is initialised and return boolean 
  bool check_adc_init();

  // Run script to load initialise ADC
  void init_adc();

  // Check PS memory is ready
  bool check_ps_mem();

  // Run dtc simulation
  void run_dtc_sim();

  // Set dtc real
  bool set_real_dtc();

  // Check ADC temperature
  double read_adc_temp();

  
};

#endif
