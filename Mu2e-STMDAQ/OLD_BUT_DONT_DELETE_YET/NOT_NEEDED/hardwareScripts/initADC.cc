//////////////////////////////////////////////
/// This module initialises the ADC (main).   
//////////////////////////////////////////////

#include<iostream>
#include<fstream>

// General ADC scripts
#include "STMDAQ-TestBeam/hardwareScripts/initADC.hh"
#include "STMDAQ-TestBeam/hardwareScripts/utils.hh"
#include "STMDAQ-TestBeam/hardwareScripts/repeat.hh"

// FMC120
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_spi.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_freqcnt.hh"

// FMC144
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/capture.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_monitor.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_freqcnt.hh"

// General ADC classes
class utils u;
class repeat r;

// FMC20 classes
class FMC120 f_120;
class FMC120_freqcnt fc_120;

// FMC144 classes
class FMC144 f_144;
class FMC144_monitor m_144;
class FMC144_freqcnt t_144;
class capture c_144;

// Standard constructor - shouldn't be used
initADC::initADC() {}

// Initialise ADC 
void initADC::init(IPBusManager* hw, std::string adc){

  // Notify user
  printf("Initalising ADC...\n");  
  std::cout << "Info" << std::endl;

  // Set UHAL logging level to notice  
  hw->noticeUhalLogging();

  // Initialise FMC120
  if (adc == "FMC120"){
    init_FMC120(hw);  
  }
  // Initialise FMC144
  else if (adc == "FMC144"){
    init_FMC144(hw);
  }
  // Throw error
  else{
    std::cout << "ERROR! Do not recognise ADC name: " << adc << std::endl;
    exit(0);
  }
  
}

// Initialise FMC120
int initADC::init_FMC120(IPBusManager* hw){

  // Notify user
  printf("Initalising FMC120...\n");

  double result = 0;

  // Configure I2C switch to HPC connector on the ZCU102
  // Set one byte per cycle
  hw->write("i2c_master.i2c.byte",0x0);
  spi_120.i2c_write(hw,0x7500,0x01); // For HPC0?

  sleep(0.2);
  std::cout << "1st step" << std::endl;
  uint64_t temp = hw->read("CID_registers.constellation_id");
  std::cout << std::hex << temp << std::endl;
  temp = hw->read("CID_registers.sw_build");
  std::cout << std::hex << temp << std::endl;
  std::cout << "Get the diagnostics from the FMC120 daughter board" << std::endl;
  hw->write("i2c_master.i2c.byte",0x1);  
  spi_120.i2c_write(hw, 0x2F00, 0x2200);
  sleep(0.2);
  std::cout << "1st step" << std::endl;
  spi_120.i2c_write(hw, 0x2F00, 0xA000);
  sleep(0.2);
  std::cout << "2nd step" << std::endl;
  temp = spi_120.i2c_read(hw, 0x2F02);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 8) {
    result = (temp & 0xFFF) / 4.0;
    if (result > -40.0 && result < 85.0) {
      std::cout << "Temp (AD7291) : OK " 
		<< std::fixed << std::setprecision(2) 
		<< result << std::endl;
    } 
    else {
      std::cout << "Temp (AD7291) : ERROR " 
		<< std::fixed << std::setprecision(2) 
		<< result << std::endl;
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2080);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 0) {
    result = (temp & 0xFFF) * 2.5 / 4096;
    if (result > (0.9 * 0.9) && result < (0.9 * 1.1)) {
      std::cout << "Vin0 (0.9Va) : OK " 
		<< std::fixed << std::setprecision(2) 
		<< result << "V" << std::endl;
    } 
    else {
      std::cout << "Vin0 (0.9Va) : ERROR " 
		<< std::fixed << std::setprecision(2) 
		<< result << "V" << std::endl;
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2040);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 1) {
    result = (temp & 0xFFF) * 2.5 / 4096;
    if (result > (1.15 * 0.9) && result < (1.15 * 1.1)) {
      printf("Vin1 (1.15Va) : OK %.2fV\n", result);
    } 
    else {
      printf("Vin1 (1.15Va) : ERROR %.2fV\n", result);
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2020);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 2) {
    result = (temp & 0xFFF) * 2.5 / 4096;
    if (result > (1.8 * 0.9) && result < (1.8 * 1.1)) {
      printf("Vin2 (1.8Va) : OK %.2fV\n", result);
    } else {
      printf("Vin2 (1.8Va) : ERROR %.2fV\n", result);
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2010);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 3) {
    result = (temp & 0xFFF) * 2.5 / 4096;
    if (result > (1.9 * 0.9) && result < (1.9 * 1.1)) {
      printf("Vin3 (1.9Va) : OK %.2fV\n", result);
    } 
    else {
      printf("Vin3 (1.9Va) : ERROR %.2fV\n", result);
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2008);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 4) {
    result = (temp & 0xFFF) * 5.0 / 4096;
    if (result > (2.6 * 0.9) && result < (2.6 * 1.1)) {
      printf("Vin4 (2.6Va) : OK %.2fV\n", result);
    } 
    else {
      printf("Vin4 (2.6Va) : ERROR %.2fV\n", result);
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2004);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 5) {
    result = (temp & 0xFFF) * 5.0 / 4096;
    if (result > (3.0 * 0.9) && result < (3.0 * 1.1)) {
      std::cout << "Vin5 (3.0Va) : OK " 
		<< std::fixed << std::setprecision(2) 
		<< result << "V" << std::endl;
    } 
    else {
      std::cout << "Vin5 (3.0Va) : ERROR " 
		<< std::fixed << std::setprecision(2) 
		<< result << "V" << std::endl;
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2002);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 6) {
    result = (temp & 0xFFF) * 5.0 / 4096;
    if (result > (3.3 * 0.9) && result < (3.3 * 1.1)) {
      std::cout << "Vin6 (3.3Va) : OK " << std::fixed << std::setprecision(2) << result << "V" << std::endl;
    } 
    else {
      std::cout << "Vin6 (3.3Va) : ERROR " << std::fixed << std::setprecision(2) << result << "V" << std::endl;
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  spi_120.i2c_write(hw, 0x2F00, 0x2001);
  sleep(0.2);
  temp = spi_120.i2c_read(hw, 0x2F01);
  std::cout << temp << std::endl;
  if (((temp & 0xF000) >> 12) == 7) {
    result = -3.3 + ((temp & 0xFFF) * 5.0 / 4096);
    if (result < (-2.6 * 0.9) && result > (-2.6 * 1.1)) {
      std::cout << "Vin7 (-2.6Va) : OK " << std::fixed << std::setprecision(2) << result << "V" << std::endl;
    } 
    else {
      std::cout << "Vin7 (-2.6Va) : ERROR " << std::fixed << std::setprecision(2) << result << "V" << std::endl;
    }
  } 
  else {
    std::cout << "Error reading from monitoring device. Wrong channel number returned" << std::endl;
  }

  f_120.init(hw, CLOCKTREE_CLKSRC_INTERNAL);

  std::cout << "Measuring on-board frequencies ---" << std::endl;
  for (int i = 0; i < 5; i++) {
    fc_120.getfrequency(hw, i);
  }

  return 1;

}

// Initialise FMC144
int initADC::init_FMC144(IPBusManager* hw){

  // ****************************************************************** // 
  // ************************ Erdem's code **************************** // 
  // ****************************************************************** // 

  // Set variables for device (private variables defined in IPBusManager.hh)
  int adc_mode = getADCmode();
  int numberburst = getNumberBurst();
  //  int N_SAMPS = getNsamps();
  //  int N_SIZE = getNsize();
  int burstlength = getBurstLength();
  int burstlength_capture = getBurstLengthCapture();
  
  // Configure and readout monitoring device on the FMC144.  
  m_144.getdiags(hw);

  // Generate sine wave form
  int16_t* buf = u.GenerateWaveform16(burstlength, 128, pow(2.0,15.0),u.genSineWave());
  // for (int i = 0; i < burstlength; i++){
  //   std::cout << i << "   " << buf[i] << std::endl;
  // }

  // Initialize both ADC chips on the FMC144.
  f_144.init(hw,0,adc_mode);

  // Obtain frequencies from the clock tree.  
  std::cout << "Measuring on-board frequencies ---" << std::endl;
  for (int i = 0; i < 5; i++){
    t_144.getfrequency(hw,i,adc_mode);
  }

  // Initialise router control output for each node
  hw->write("Router_ctrl.Output_1_select",0x0);
  hw->write("Router_ctrl.Output_2_select",0x0);
  hw->write("Router_ctrl.Output_3_select",0x0);
  hw->write("Router_ctrl.Output_4_select",0x0);

  // Set the burst size and prepare wave form memory of each node
  r.setburstlength(hw,0,burstlength);
  r.prepare_wfm_upload(hw,0);
  r.setburstlength(hw,1,burstlength);
  r.prepare_wfm_upload(hw,1);
  r.setburstlength(hw,2,burstlength);
  r.prepare_wfm_upload(hw,2);
  r.setburstlength(hw,3,burstlength);
  r.prepare_wfm_upload(hw,3);

  // Write generated waveform to FPGA buffers
  int x = 0;
  for (int item = 0; item < burstlength/2; item++){
    std::string tmp1 = u.int2bin(buf[x],16);
    // std::cout << buf[x] << std::endl;
    // std::cout << tmp1 << std::endl;
    std::string tmp2 = u.int2bin(buf[x+1],16);
    // std::cout << buf[x+1] << std::endl;
    // std::cout << tmp2 << std::endl;
    // std::cout << tmp2+tmp1 << std::endl;
    uint64_t word = std::stoll(tmp2+tmp1, nullptr, 2);
    // std::cout << word << std::endl;
    x += 2;
    hw->write("Buffers.Host2FPGA_buf_reg",word);
  }

  // Check status, read back and arm each repeat node
  for (int i = 0; i < 4; i++){
    // Check status 
    uint64_t value = r.check_status(hw,i);
    // Print value
    printf("value_%d = %lX\n",i,value);
    // Arm node
    r.arm(hw,i);
  }

  // Reset FIFO and set burst length of each capture node
  for (int i = 0; i < 4; i++){    
    // Reset FIFO
    c_144.fifo_rst(hw,i);
    // Set FIFO
    c_144.setburstlength(hw,i,numberburst,burstlength_capture);
    // Enable upload
    // c_144.enable_upload(hw,i);
  }

  // Wait
  usleep(10000.0); // 0.1 seconds
  // Enable external trigger and arm each capture node
  for (int i = 0; i < 4; i++){
    // Enable external trigger
    c_144.external_trigger_enable(hw,i);
    // Arm node
    c_144.arm(hw,i);
    // Enable software trigger
    // c_144.sw_trigger(hw,i);
  }

  // Enable upload for hardware trigger of each capture node
  for (int i = 0; i < 4; i++){
    c_144.enable_upload_hw_trig(hw,i);
    printf("ADC_%d dump end\n",i);
  }

  return 1;

}

