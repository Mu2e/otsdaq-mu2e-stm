#ifndef IPBUSMANAGER_HH
#define IPBUSMANAGER_HH

#include <iomanip>
#include <unistd.h>

// Environment variables
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// uhal
#include "uhal/uhal.hpp"
#include "uhal/ConnectionManager.hpp"

using namespace uhal;

class IPBusManager {

public:

  IPBusManager();
  IPBusManager(std::string conn_file, std::string dev_id);

  // Get xml connections file
  const std::string getConnectionFile(){
    return connection_file;
  }

  // Get device ID
  const std::string getDeviceID(){
    return device_id;
  }

  ConnectionManager*      getConnectionManager() const;
  HwInterface*            getHwInterface() const;

  // Function to get a std::vector std::string of all nodes in a device
  std::vector< std::string > getAllNodes();

  // Function to read single registry value of a given node
  uint64_t read(std::string node);
  // Function to write to single registry value of a given node
  void write(std::string node, uint64_t value);

  // Function to read entire registry block
  std::vector<uint32_t> readBlock(std::string node);
  
  // Function to set uhal logging level to maximum
  void maxUhalLogging();
  // Function to set uhal logging level to notice
  void noticeUhalLogging();
  // Function to set uhal logging level to minimum
  void minUhalLogging();

  // // --
  // // Functions to get private variables
  // // --

  // // Get ADC mode
  //  int getADCmode(){
  //   return adcMode;
  // }

  // // Get number burst
  //  int getNumberBurst(){
  //   return numberBurst;
  // }

  // // Get Nsamps
  //  int getNsamps(){
  //   return Nsamps;
  // }

  // // Get Nsize
  //  int getNsize(){
  //   return Nsize;
  // }

  // // Get burst length
  //  int getBurstLength(){
  //   return burstLength;
  // }

  // // Get burst length capture
  //  int getBurstLengthCapture(){
  //   return burstLength_capture;
  // }

private : 

  // xml connections file
  const std::string connection_file = EnvVars::expand("${STM_FW_XML}");
  // Device name
  const std::string device_id = EnvVars::expand("${STM_FPGA}");

  // IPBus connection manager variable                 
  std::unique_ptr<ConnectionManager> manager;

  // IPBus hardware interface variable
  std::unique_ptr<HwInterface> hw;

  // // Define variables for device
  // int adcMode = 1;
  // int numberBurst = 0;//1;
  // int Nsamps = 8192;                                                                                          
  // int Nsize = 8192;                                                                                           
  // int burstLength = 1536; // 512 * 2 * 12
  // int burstLength_capture = 1280; // 1256; // 628  

};

#endif
