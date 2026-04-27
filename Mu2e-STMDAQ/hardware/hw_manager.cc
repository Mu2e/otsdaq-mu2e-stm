#include <pybind11/embed.h>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>
#include <chrono>
#include <sys/wait.h>  // WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG

// Hardware manager code
#include "Mu2e-STMDAQ/hardware/hw_manager.hh"

// Include pybind
namespace py = pybind11;

// Filesystem namespace
namespace fs = std::filesystem;

// Make pybind implementation hidden
#if defined(__GNUC__)
  #define HIDDEN __attribute__((visibility("hidden")))
#else
  #define HIDDEN
#endif

// pybind implementation
struct HIDDEN HardwareManager::Impl {

  // Pybind guard
  py::scoped_interpreter guard{};
  // Pybind adaptor
  py::module pb_adaptor;
  // Pybind object to run and get function
  py::object run_and_get;

  // STMDAQ root string
  std::string root;
  // Hardware code directory
  std::string hw_dir;

  // Hardware device to pass to python scripts 
  py::list device;

  // Pybind implementation constructor
  Impl(const std::shared_ptr<AsyncLogger>& logger_,
       const std::shared_ptr<STMdata>& stm_) :
    pb_adaptor(py::module::import("sys")) {

    // Get the hardware device name
    device.append(stm_->fw_config.device);

    // Import the python sys module
    py::module sys = py::module::import("sys");

    // Get the STMDAQ_ROOT environment variable
    const char* r = std::getenv("STMDAQ_ROOT");
    // Check for $STMDAQ_ROOT
    if (!r){
      logger_->log("HardwareManager::Impl: STMDAQ_ROOT not set!",0);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }
    // Get STMDAQ_ROOT dir as string
    root = r;
    // Get hardware dir as string
    hw_dir = root + stm_->fw_config.hw_dir;

    // Check hardware directory exists
    if (!fs::exists(hw_dir)) {
      logger_->log("HardwareManager::Impl: hardware directory (" +
                   hw_dir + ") does not exist!", 0);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }
    
    // Make sure it’s actually a directory
    if (!fs::is_directory(hw_dir)) {
      logger_->log("HardwareManager::Impl: hardware path (" +
                   hw_dir + ") exists but is not a directory!", 0);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }
    
    // Store as dir path: everything (pybind adaptor + fw scripts) lives here
    sys.attr("path").attr("append")(hw_dir);
    
    // Get pybind adaptor script name
    std::string pb = stm_->fw_config.python.pybind.first;
    // Check pybind adaptor script exists
    if (!fs::exists(hw_dir+"/"+pb+".py")) {
      logger_->log("HardwareManager::Impl: pybind adaptor script (" +
                   hw_dir+"/"+pb+".py" + ") does not exist!", 0);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }
    // Import the pybind adaptor script
    pb_adaptor  = py::module::import(pb.c_str()); // hardware/pybind_adaptor.py
    // Import run and get function from the pybind adaptor
    run_and_get = pb_adaptor.attr(stm_->fw_config.python.pybind.second.c_str()); // hardware/run_and_get.py
  }

  // Python object to call a script by pass arguments
  // and returning the value of a varaible
  py::object call_script_get(const std::string& script_filename,
                             const py::list& args,
                             const std::string& var = "result") {
    // Python script file path
    const std::string script_path = hw_dir + "/" + script_filename;
    // Run the script and return the var value
    return run_and_get(script_path, args, var);
    
  }
};

// Constructor
HardwareManager::HardwareManager(const std::shared_ptr<AsyncLogger>& logger_,
                                 const std::shared_ptr<STMdata>& stm_)
  : logger(logger_)
  , stm(stm_)
  , hw_dir(EnvVars::expand("${STMDAQ_ROOT}") + stm_->fw_config.hw_dir)
  , impl(std::make_unique<Impl>(logger_,stm_)) 
{

  // If we are using firmware for data input
  if (!stop::should_stop() && !stm->master_config.use_sw_sim){
    // Check if FPGA firmware is loaded
    bool fpga_init = check_fpga_init();
    // If firmware is not loaded, load firmware
    if(!fpga_init) fpga_init = load_firmware();
    if(!fpga_init){
      logger->log("HardwareManager: Error loading FPGA firmware. Exiting...",0);
      return;
    }
    // Check if ADC already is initialised
    bool init = check_adc_init();
    // If ADC is not initalised, initialise ADC
    if (!init){
      // Initalise ADC
      init_adc();
      // Check again that the adc has been initialised
      init = check_adc_init();
    }
    if (!init){      
      logger->log("HardwareManager::init_adc: ADC initialisation failed!. Exiting...",0);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }
    
    // If no stop signal has been issused
    if (!stop::should_stop()){
      // Check ADC temperature
      double temp = read_adc_temp();
    }    
  }
  
}


// Check whether FPGA firmware is loaded and return boolean
bool HardwareManager::check_fpga_init() {

  reset_readout();
  // Get python script name
  std::string script = stm->fw_config.python.check_fpga.first+".py";
  // Get python script return variable name
  std::string retvar = stm->fw_config.python.check_fpga.second;
  
  // Log to user
  logger->log("Checking for FPGA firmware...",1);
  // Declare the python object and run python script  
  py::object out = impl->call_script_get(script,// Script
                                         impl->device, // Device  
                                         retvar // Variable to return
                                         );
  // Get returned result
  bool success = out.cast<bool>();
  // Log outcome to user
  if (success){
    logger->log("Firmware already loaded.",1);
  }
  else{
    logger->log("FPGA firmware not loaded.",2);
  }
  // Return boolean
  return success;
}

// Run script to load FPGA firmware
bool HardwareManager::load_firmware() {

  // Get remote petalinux ssh address
  const std::string ssh_addr = stm->fw_config.load_fw.petalinux_user+"@"+stm->fw_config.load_fw.petalinux_ip;
  // Get remote petalinux ssh address
  const std::string bitfile = stm->fw_config.load_fw.bitfile;
  
  // Build a command to run under ksu using /bin/sh
  // Includes ssh-agent + ssh-add, then ssh to run fpgautil.
  // trap ensures ssh-agent is killed when the shell
  std::string cmd =
    "ksu -e /bin/sh -c '"
      "eval \"$(ssh-agent -s)\" >/dev/null; "
      "trap \"ssh-agent -k >/dev/null\" EXIT; "
      "ssh-add ~/.ssh/petalinux_rsa >/dev/null; "
      "ssh -x " + ssh_addr + " \"fpgautil -b " + bitfile + "\""
    "'";

  // Call system to run sequence
  int rc = std::system(cmd.c_str());

  // Log error codes
  if (rc == -1) {
    logger->log("HardwareManager::load_firmware: Failed to start command (system() error)", 2);
    return false;
  }

  // On POSIX, rc encodes signal/exit status. Use macros:
  if (WIFEXITED(rc)) {
    int code = WEXITSTATUS(rc);

    if (code == 0) {
      logger->log("HardwareManager::load_firmware: Success!", 1);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return true;
    } else {
      logger->log("HardwareManager::load_firmware: Command failed, exit code " + std::to_string(code), 2);
      return false;
    }
  }
  else if (WIFSIGNALED(rc)) {
    logger->log("HardwareManager::load_firmware: Command killed by signal " + std::to_string(WTERMSIG(rc)), 2);
    return false;
  }

  logger->log("HardwareManager::load_firmware: Command ended unexpectedly.", 2);
  return false;
}

// Reset readout
void HardwareManager::reset_readout(){

  py::gil_scoped_acquire gil;
  
  // Get python script name
  std::string script = stm->fw_config.python.reset_readout.first+".py";
  // Get python script return variable name
  std::string retvar = "finished";
  
  // Log to user
  logger->log("Running reset readout script: " + script + "...",1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // Declare the python object and run python script  
  py::object out;
  try {
    py::gil_scoped_acquire acquire;
    out = impl->call_script_get(script, impl->device, retvar);
  } catch (const py::error_already_set& e) {
    py::gil_scoped_acquire acquire;
    logger->log("PYTHON ERROR in " + script + ": " + std::string(e.what()), 3);
    throw;
  } catch (const std::exception& e) {
    logger->log("C++ Exception: " + std::string(e.what()), 3);
    throw;
  }

  //py::object out = impl->call_script_get(script,// Script
  //                                       impl->device, // Device  
  //                                       retvar // Variable to return
  //                                       );
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return;

}


// Check whether ADC is initialised and return boolean
bool HardwareManager::check_adc_init() {

  // Reset readout
  reset_readout();
  
  // Get python script name
  std::string script = stm->fw_config.python.check_adc.first+".py";
  // Get python script return variable name
  std::string retvar = stm->fw_config.python.check_adc.second;
  
  // Log to user
  logger->log("Checking if ADC is already initialised...",1);
  // Declare the python object and run python script  
  py::object out = impl->call_script_get(script,// Script
                                         impl->device, // Device  
                                         retvar // Variable to return
                                         );  
  // Get returned result
  bool success = out.cast<bool>();
  // Log outcome to user
  if (success){
    logger->log("ADC already initialised",1);
  }
  else{
    logger->log("ADC not initialised!",2);
  }
  // Return boolean
  return success;
}

// Run script to load initialise ADC
void HardwareManager::init_adc() {

  // Reset readout
  reset_readout();
  
  // Get python script name
  std::string script = stm->fw_config.python.init_adc.first+".py";
  // Get python script return variable name
  std::string retvar = stm->fw_config.python.init_adc.second;
  
  // Log to user
  logger->log("Initialising ADC...",1);
  // Declare the python object and run python script  
  py::object out = impl->call_script_get(script,// Script
                                         impl->device, // Device  
                                         retvar // Variable to return
                                         );
  
}

// Run dtc simulation
void HardwareManager::run_dtc_sim(){

  // Reset readout
  reset_readout();
  
  //py::gil_scoped_acquire gil;
  
  // Get python script name
  std::string script = stm->fw_config.python.dtc_sim.first+".py";
  // Get python script return variable name
  std::string retvar = "finished";
  
  // Log to user
  logger->log("Running DTC simulation script: " + script + "...",1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // Declare the python object and run python script  
  py::object out = impl->call_script_get(script,// Script
                                         impl->device, // Device  
                                         retvar // Variable to return
                                         );
  return;

}

// Set board to real DTC
bool HardwareManager::set_real_dtc(){

  //py::gil_scoped_acquire gil;

  // Get python script name
  std::string script = stm->fw_config.python.dtc_set_real.first+".py";
  // Get python script return variable name
  std::string retvar = "finished";

  // Log to user
  logger->log("Running script to set ROC to real DTC: " + script + "... ",1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // Declare the python object and run python script
  py::object out = impl->call_script_get(script,// Script
                                         impl->device, // Device
                                         retvar // Variable to return
                                         );
  // Get returned result
  bool success = out.cast<bool>();
  // Log outcome to user
  if (success){
    logger->log("Real DTC mode set",1);
  }
  else{
    logger->log("Error running real DTC script",2);
  }
  // Return boolean
  return success;

}


// Read and returc ADC temperature
double HardwareManager::read_adc_temp() {
  
  // Get python script name
  std::string script = stm->fw_config.python.adc_temp.first+".py";
  // Get python script return variable name
  std::string retvar = stm->fw_config.python.adc_temp.second;
  
  // Log to user
  logger->log("Checking ADC temperature...",1);
  // Declare the python object and run python script  
  py::object out = impl->call_script_get(script,// Script
                                         impl->device, // Device  
                                         retvar // Variable to return
                                         );
  // Get returned result
  double temp = out.cast<double>();
  // Log ADC temperature to user
  logger->log("ADC temp = " + std::to_string(temp) + " C",1);
  // Return ADC temperature
  return temp;
}

// Destructor
HardwareManager::~HardwareManager() = default;

