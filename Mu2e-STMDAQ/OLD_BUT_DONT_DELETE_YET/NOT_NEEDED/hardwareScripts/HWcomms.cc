///////////////////////////////////////////////////////////////////////////////////
/// This module is the main function in sending commands to the firmware/hardware (main).   
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

#include<iostream>
#include<fstream>

// XML interface
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// HW comms
#include "STMDAQ-TestBeam/hardwareScripts/HWcomms.hh"

/*-- Hardware Init -------------------------------------------------*/

// Standard constructor - shouldn't be used
HWcomms::HWcomms() {}

//**********************************************                             
// BELOW ARE USED FOR ELBE                                                   
//********************************************** 

// Ping firmware interfaces on initialisation
int HWcomms::pingInterfaces(){

  printf("Pinging firmware interfaces...\n");

  // Ping gigabit Ethernet for ipbus interface
  std::string ip1 = EnvVars::expand("${STM_IPBUS_IP}");
  std::string ping1 = "ping -c 1 -w2 " + ip1;
  const char *p1 = ping1.c_str();
  system(p1);
  // Ping 10gb Ethernet interface
  std::string ip2 = EnvVars::expand("${STM_10G_IP}");
  std::string ping2 = "ping -c 1 -w2 " + ip2;
  const char *p2 = ping2.c_str();
  system(p2);

  return 1;

}


// Set firmware registers from xml config file
int HWcomms::setRegisters(IPBusManager* hw){

  // Define the XML reader instance
  Xml *xml_file;
  // Open XML config file and instantiate the xml_file object
  std::string xml_path = EnvVars::expand("${STM_XML}");
  //  std::cout << "XML_PATH = " << xml_path << std::endl;
  xml_file = new Xml(xml_path);
      
  // Set prescale value
  uint prescale = xml_file->
    int_from_hex_value("stm.fmc144_prescale");
  hw->write("Buffers.Test_beam.prescale",prescale);
  //  cout << "prescale = " << HEX(prescale,2) << endl;

  // Set external mode ADC offset
  uint ext_ADCoffset = xml_file->
    int_from_hex_value("stm.fmc144_ext_ADC_offset");
  hw->write("Buffers.Test_beam.ext_ADC_offset",ext_ADCoffset);
  //  cout << "ext_ADCoffset = " << HEX(ext_ADCoffset,2) << endl;

  // Set pause in between packets
  uint interpacket_pause = xml_file->
    int_from_hex_value("stm.fmc144_teng_interpacket_pause");
  hw->write("Buffers.Test_beam.teng_interpacket_pause",interpacket_pause);
  //  cout << "interpacket_pause = " << HEX(interpacket_pause,2) << endl;  

  // Set external mode slice length
  uint ext_sliceLength = xml_file->
    int_from_hex_value("stm.fmc144_ext_slice_length");
  hw->write("Buffers.Test_beam.ext_slice_length",ext_sliceLength);
  //  cout << "ext_sliceLength = " << HEX(ext_sliceLength,2) << endl;  

  // Set number of slices to record in external mode
  uint ext_sliceNum = xml_file->
    int_from_hex_value("stm.fmc144_ext_slice_num");
  hw->write("Buffers.Test_beam.ext_slice_num",ext_sliceNum);
  //  cout << "ext_sliceNum = " << HEX(ext_sliceNum,2) << endl;  
  
  // Set external/internal transition delay
  uint extIntDelay = xml_file->
    int_from_hex_value("stm.fmc144_ext_int_delay");
  hw->write("Buffers.Test_beam.ext_int_delay",extIntDelay);
  //  cout << "extIntDelay = " << HEX(extIntDelay,2) << endl;  

  // Set number of internal triggers
  uint int_trigNum = xml_file->
    int_from_hex_value("stm.fmc144_int_trig_num");
  hw->write("Buffers.Test_beam.int_trig_num",int_trigNum);
  //  cout << "int_trigNum = " << HEX(int_trigNum,2) << endl;  
  
  // Set internal mode ADC offset
  uint int_ADCoffset = xml_file->
    int_from_hex_value("stm.fmc144_int_ADC_offset");
  hw->write("Buffers.Test_beam.int_ADC_offset",int_ADCoffset);
  //  cout << "int_ADCoffset = " << HEX(int_ADCoffset,2) << endl;  

  // Set internal mode slice length
  uint int_sliceLength = xml_file->
    int_from_hex_value("stm.fmc144_int_slice_length");
  hw->write("Buffers.Test_beam.int_slice_length",int_sliceLength);
  //  cout << "int_sliceLength = " << HEX(int_sliceLength,2) << endl;  

  // Set number of slices to record in internal mode  
  uint int_sliceNum = xml_file->
    int_from_hex_value("stm.fmc144_int_slice_num");
  hw->write("Buffers.Test_beam.int_slice_num",int_sliceNum);
  //  cout << "int_sliceNum = " << HEX(int_sliceNum,2) << endl;  
  
  // Set the external trigger timeout to signal transition to internal mode
  uint ext_trigTimeout = xml_file->
    int_from_hex_value("stm.fmc144_ext_trig_timeout");
  hw->write("Buffers.Test_beam.ext_trig_timeout",ext_trigTimeout);

  return 1;

}

// Start and reset the external clock counter 
int HWcomms::startResetClock(IPBusManager* hw){
  
  hw->write("Buffers.Test_beam.Start_reset_stop_clock",0x1);
  
  return 1;
  
}

// Reset trigger FIFOs
int HWcomms::resetTriggerFIFOs(IPBusManager* hw){
  
  hw->write("Buffers.Controls_misc.reset_trig_fifos",0x1);
  
  return 1;
  
}

// Set the 10G readout mode
int HWcomms::start10Greadout(IPBusManager* hw){
  
  hw->write("Buffers.Test_beam.Test_beam_10g_readout",0x1);
  
  return 1;
  
}

// Strop the 10G readout mode
int HWcomms::stop10Greadout(IPBusManager* hw){
  
  hw->write("Buffers.Test_beam.Test_beam_10g_readout",0x0);
  
  return 1;
  
}


// // Reset and initliaise DDR4 read
// int HWcomms::reset_DDR4(IPBusManager* hw){
  
//   // Reset write/read address to zero
//   hw->write("Buffers.Debug_controls_pulse_4.DDR_wr_addr_reset",0x1);
//   hw->write("Buffers.Debug_controls_pulse_4.DDR_read_addr_reset",0x1);
 
//   // Reset FIFO
//   for (int i = 0; i < 4; i++) c.fifo_rst(hw,i);

//   // Enable external trigger
//   for (int i = 0; i < 4; i++) {
//     c.external_trigger_enable(hw,i);
//     // Arm node
//     c.arm(hw,i);
//   }

//   // Enable upload for hardware trigger of each capture node
//   for (int i = 0; i < 4; i++) c.enable_upload_hw_trig(hw,i);

//   return 1;

// }

// Start ADC emulation
int HWcomms::emulateADC(IPBusManager* hw){
  
  hw->write("Buffers.Controls_misc.adc_emulation_enable",0x1);

  return 1;

}

// Stop ADC emulation
int HWcomms::stopEmulateADC(IPBusManager* hw){
  
  hw->write("Buffers.Controls_misc.adc_emulation_enable",0x0);

  return 1;

}


// Enable triggered data taking
int HWcomms::enable_trigData(IPBusManager* hw){

  hw->write("Buffers.FPGA2Host_buf_stat.trigs_enable",0x1);

  return 1;

}

// Disable triggered data taking
int HWcomms::disable_trigData(IPBusManager* hw){

  hw->write("Buffers.FPGA2Host_buf_stat.trigs_enable",0x0);

  return 1;

}

// Start continous 10g packet sending                                    
int HWcomms::start10Gpackets(IPBusManager* hw){

  hw->write("Buffers.10g_readout.cont_readout_8K_chunks",0x1);

  return 1;

}

// Stop continous 10g packet sending                                    
int HWcomms::stop10Gpackets(IPBusManager* hw){

  hw->write("Buffers.10g_readout.cont_readout_8K_chunks",0x0);

  return 1;

}

// Checks whether the DDR4 memory rolls over 
// and checks where the read address is. 
// Bit 31 tells you whether the memory rollover happened (bit 31 = 1). 
// If rollover, it returns the read address in the least 30 bits. 
uint32_t HWcomms::checkMemoryRollover(IPBusManager* hw){

  uint32_t temp = hw->read("Buffers.Test_beam.Mem_rollover_stat");

  temp = temp >> 31;

  return temp;

}

// Check DDR4 read/write address
uint32_t HWcomms::getDDR4addr(IPBusManager* hw){
  
  // Get snapshot of DDR4 write/read address  
  hw->write("Buffers.Debug_RAM_Status.Status_trig_and_wr_address_return",0x1);

  uint32_t temp = hw->
    read("Buffers.Debug_RAM_Status.Status_trig_and_wr_address_return");
  
  return temp;

}

//**********************************************
//********************************************** 
//********************************************** 

// NOT USED IN ELBE

int HWcomms::enable_contData(IPBusManager* hw){

  // Turn on continuous data taking
  hw->write("Buffers.Debug_controls_pulse_2.cont_data_on_debug",0x1);

  return 1;

}

int HWcomms::disable_contData(IPBusManager* hw){

  // Turn on continuous data taking
  hw->write("Buffers.Debug_controls_pulse_2.cont_data_on_debug",0x0);

  return 1;

}

int HWcomms::allow_DDR4overwrite(IPBusManager* hw){

  // Turn on continuous data taking
  hw->write("Buffers.stop_overwrite",0x0);

  return 1;

}

int HWcomms::stop_DDR4overwrite(IPBusManager* hw){

  // Turn on continuous data taking
  hw->write("Buffers.stop_overwrite",0x1);

  return 1;

}

uint32_t HWcomms::checkReadAdrr(IPBusManager* hw){

  // Get DDR4 read address  
  hw->write("Buffers.Debug_RAM_Status_10g.Status_trig_and_wr_address_return",0x1);

  uint32_t temp = hw->read("Buffers.Debug_RAM_Status_10g.Read_address_return");
  
  return temp;

}


uint32_t HWcomms::checkBuffer(IPBusManager* hw){

  uint32_t temp = hw->read("Buffers.FPGA2Host_buf_stat.Empty");
  
  return temp;

}

std::vector<uint32_t> HWcomms::readDDR4(IPBusManager* hw){

  hw->write("Buffers.Debug_controls_pulse_3.1K_read_start",0x1);
  
  uint32_t temp = 0;
  while(temp == 0){
    temp = hw->read("Buffers.FPGA2Host_buf_stat.Empty");
    //temp = checkBuffer();
  }
  
  //  Initiate a 1KByte block  read from the DDR4 RAM and writes the data to a FIFO.
  std::vector<uint32_t> mem = hw->readBlock("Buffers.FPGA2Host_buf_reg");
  
  return mem;
  
}


