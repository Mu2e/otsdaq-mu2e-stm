#ifndef FW_CONFIG_HH_
#define FW_CONFIG_HH_

// Include config file
#include "Mu2e-STMDAQ/config/config.hh"

// Parameters to load firmware 
struct load_fw_{
  const std::string script; // Bash script to call to load firmware
  const std::string petalinux_user; // Petalinux OS user
  const std::string petalinux_ip; // Petalinux OS interface IP
  const std::string bitfile; // Bitfile to load

  // Constructor 
  load_fw_(std::string script_,
          std::string petalinux_user_,
          std::string petalinux_ip_,
           std::string bitfile_)
          : script(script_),
          petalinux_user(petalinux_user_),
          petalinux_ip(petalinux_ip_),
          bitfile(bitfile_) {}
};


// Python scripts to call by hw_manager
struct python_scripts{
  const std::pair<std::string,std::string> pybind; // Pybind adaptor & execute function
  const std::pair<std::string,std::string> check_fpga; // Check FPGA script & return variable
  const std::pair<std::string,std::string> check_adc; // Check ADC script & return variable
  const std::pair<std::string,std::string> init_adc; // Initialise ADC script & return variable
  const std::pair<std::string,std::string> adc_temp; // Check adc temperature script & return variable
  const std::pair<std::string,std::string> reset_readout; // Reset readout script
  const std::pair<std::string,std::string> dtc_sim; // DTC simulation script
  const std::pair<std::string,std::string> dtc_set_real; // DTC set real script
  // Constructor 
  python_scripts(std::string pybind_,
                 std::string pybind_func_,
                 std::string check_fpga_,
                 std::string check_fpga_var_,
                 std::string check_adc_,
                 std::string check_adc_var_,
                 std::string init_adc_,
                 std::string init_adc_var_,
                 std::string adc_temp_,
                 std::string adc_temp_var_,
                 std::string reset_readout_,
                 std::string dtc_sim_,
		 std::string dtc_set_real_)
    : pybind(pybind_,pybind_func_)
    , check_fpga(check_fpga_,check_fpga_var_)
    , check_adc(check_adc_,check_adc_var_)
    , init_adc(init_adc_,init_adc_var_)
    , adc_temp(adc_temp_,adc_temp_var_)
    , reset_readout(reset_readout_,"")
    , dtc_sim(dtc_sim_,"")
    , dtc_set_real(dtc_set_real_,"") {}
  // Get all python scripts as tuple
  auto as_tuple() const {
    return std::tie(
                    pybind,
                    check_fpga,
                    check_adc,
                    init_adc,
                    adc_temp,
                    reset_readout,
                    dtc_sim,
		    dtc_set_real
                    );
  }
};
  

// Firmware configurable variables
struct fw_info{
  
  // The control server
  const std::string ctrl_srvr;
  // Hardware dirrectory
  const std::string hw_dir;
  // Hardware device
  const std::string device;
  // Python scripts
  struct python_scripts python;
  // Parameters to remotely load firmware
  struct load_fw_ load_fw;
  // The ADC sampling frequency
  const double fADC;
  // The ADC sampling period
  const double tADC;
  // Use DTC simulation?
  const int use_dtc_sim;
  
  // Constructor
  fw_info(Config& cfg, const std::shared_ptr<AsyncLogger> logger) :
    ctrl_srvr(cfg.getValue<std::string>("stm.fw.ctrl_srvr")), // Control server
    hw_dir(cfg.getValue<std::string>("stm.fw.dir")), // Hardware directory
    device(cfg.getValue<std::string>("stm.fw.device")), // Hardware device 
    python(cfg.getValue<std::string>("stm.fw.python.pybind"),
           cfg.getValue<std::string>("stm.fw.python.pybind.exec_func"),
           cfg.getValue<std::string>("stm.fw.python.check_fpga"),
           cfg.getValue<std::string>("stm.fw.python.check_fpga.retvar"),
           cfg.getValue<std::string>("stm.fw.python.check_adc"),
           cfg.getValue<std::string>("stm.fw.python.check_adc.retvar"),
           cfg.getValue<std::string>("stm.fw.python.init_adc"),
           cfg.getValue<std::string>("stm.fw.python.init_adc.retvar"),
           cfg.getValue<std::string>("stm.fw.python.adc_temp"),
           cfg.getValue<std::string>("stm.fw.python.adc_temp.retvar"),
           cfg.getValue<std::string>("stm.fw.python.reset_readout"),
           cfg.getValue<std::string>("stm.fw.python.dtc_sim"),
           cfg.getValue<std::string>("stm.fw.python.dtc_set_real")           
           ),
    load_fw(cfg.getValue<std::string>("stm.fw.load_fw.script"),
           cfg.getValue<std::string>("stm.fw.load_fw.petalinux_user"),
           cfg.getValue<std::string>("stm.fw.load_fw.petalinux_ip"),
           cfg.getValue<std::string>("stm.fw.load_fw.bitfile")
           ),
    fADC(cfg.getValue<double>("stm.fw.fADC")), // fADC in MHz
    tADC(1.0/fADC), // tADC in µs per value
    use_dtc_sim(cfg.getValue<int>("stm.fw.use_dtc_sim")) // Use DTC simulation
  {
    
    // Notify user
    if(logger){
      logger->log("Config:fw_info: Control server = " +
                  ctrl_srvr + ".",1);            
      logger->log("Config:fw_info: Hardware directory = " +
                  hw_dir + ".",1);      
      logger->log("Config:fw_info: Hardware device = " +
                  device + ".",1);      
      logger->log("Config:fw_info: ADC sampling frequency = " +
                  std::to_string(fADC) +
                  " MHz (" + std::to_string(tADC*1e3) +
                  " ns per ADC value).",1);
      std::apply([&](const auto&... elems) {
        ((logger->log(
                      std::string("Config:fw_info: Using python script: ") +
                      hw_dir + "/" + elems.first + ".py: " + elems.second,1)
                      ), ...);
      }, python.as_tuple()); 
      logger->log("Config:fw_info: Script to remotely load firmware = " +
                  load_fw.script + ".",1);
      logger->log("Config:fw_info: Petalinux OS user = " +
                  load_fw.petalinux_user + ", interface IP = " +
                  load_fw.petalinux_ip + ".",1);
      logger->log("Config:fw_info: Firmware bitfile to load = " +
                  load_fw.bitfile + ".",1);
      std::string ON = "ON";
      std::string OFF = "OFF";
      logger->log("Config:fw_info: Data source: DTC Simulation = " + (use_dtc_sim ? ON : OFF) + ".",1);

    }
  }
  
};

#endif
