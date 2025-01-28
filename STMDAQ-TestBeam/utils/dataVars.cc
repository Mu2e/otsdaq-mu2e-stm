/////////////////////////////////////////////////////////////////////////
/// 
/////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <bits/stdc++.h>

// Data variables header
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// Logger                                               
#include "STMDAQ-TestBeam/utils/Logger.hh"

// Standard constructor - shouldn't be used
dataVars::dataVars() {}

// // Define trigger header struct
// const fw_tHdr tHdr_vars;

const int16_t fw_tHdr::HdrStart_data[fw_tHdr::HdrStart_len] = {(int16_t)0xCAFE, // 0 = 0xCAFE
							   (int16_t)0xBBBB, // 1 = 0xBBBB
							   (int16_t)0xAAAA, // 2 = 0xAAAA
							   (int16_t)0xCAFE}; // 3 = 0xCAFE

// Set experimental configuration values from xml
expConfig dataVars::getXMLvalues(expConfig exp){

  // Open the XML file and instantiate the xml_file object
  std::string xml_path = EnvVars::expand("${STM_XML}");
  Xml* xml_file = new Xml(xml_path);

  // Accelerator clock frequency
  exp.trigger_time_clock = xml_file->
    double_value("stm.trigger_time_clock",13); // MHz
  // Macropulse width
  exp.macropulse_width = xml_file->
    double_value("stm.macropulse_width",1)*1e3; // us
  // Macropulse frequency
  exp.macropulse_freq = xml_file->
    double_value("stm.macropulse_freq",1); // Hz

  // ADC clock frequency 
  exp.adc_time_clock = xml_file->
    double_value("stm.adc_time_clock",320.0); // MHz
  // Sample period of ADC (microsec) 
  exp.adc_time_clock_period = 1/exp.adc_time_clock;; // us
  // ADC offset clock frequency
  exp.adc_offset_time_clock = xml_file->
    double_value("stm.adc_offset_time_clock",125.0);; // MHz

  // ADC prescale
  exp.prescale = xml_file->
    int_from_hex_value("stm.fmc144_prescale");

  // External mode ADC offset
  exp.ext_ADC_offset = xml_file->
    int_from_hex_value("stm.fmc144_ext_ADC_offset");
  // Slice length - external mode      
  exp.ext_slice_length = xml_file->
    int_from_hex_value("stm.fmc144_ext_slice_length"); // Number of int16_t values
  // Number of slices - external mode  
  exp.ext_slice_num = xml_file->
    int_from_hex_value("stm.fmc144_ext_slice_num");

  // The time T after a trigger to wait for a new trigger
  // before switching to external mode
  exp.ext_trig_timeout = xml_file->
    int_from_hex_value("stm.fmc144_ext_trig_timeout");

  // The offset in time to wait after external mode 
  // before sending the first internal trigger.
  exp.ext_int_delay = xml_file->
    int_from_hex_value("stm.fmc144_ext_int_delay"); // ticks

  // The number of internal triggers to generate.
  exp.int_trig_num = xml_file->
    int_from_hex_value("stm.fmc144_int_trig_num");
  // Internal mode ADC offset
  exp.int_ADC_offset = xml_file->
    int_from_hex_value("stm.fmc144_int_ADC_offset");
  // Slice length - internal mode      
  exp.int_slice_length = xml_file->
    int_from_hex_value("stm.fmc144_int_slice_length"); // Number of int16_t values
  // Number of slices - internal mode  
  exp.int_slice_num = xml_file->
    int_from_hex_value("stm.fmc144_int_slice_num");

  // ADC sampling time in external mode    
  exp.ext_sample_time = 
    exp.ext_slice_num*exp.ext_slice_length*exp.adc_time_clock_period; // us 
  // ADC sampling time in internal mode 
  exp.int_sample_time =
    exp.int_slice_num*exp.int_slice_length*exp.adc_time_clock_period; // us 

  return exp;

}

int dataVars::checkExpConfig(expConfig exp){

  // Check that the number of slices are a power of 2
  // External
  bool extPowerof2 = (ceil(log2(exp.ext_slice_num)) == floor(log2(exp.ext_slice_num)));
  if (!extPowerof2) {  
    cout << "ERROR: External slice number must be power of 2!" << endl;
    cout << "\text_slice_num = " << exp.ext_slice_num << endl;
    return 0;
  }
  // Internal
  bool intPowerof2 = (ceil(log2(exp.int_slice_num)) == floor(log2(exp.int_slice_num)));
  if (!intPowerof2) {  
    cout << "ERROR: Internal slice number must be power of 2!" << endl;
    cout << "\tint_slice_num = " << exp.int_slice_num << endl;
    return 0;
  }

  // Check that slice lengths are a multiple of 16
  // External
  double extMultiple16 = exp.ext_slice_length % 16;
  if (extMultiple16 != 0) {  
    cout << "ERROR: External slice length must be multiple of 16!" << endl;
    cout << "\text_slice_length = " << exp.ext_slice_length << endl;
    return 0;
  }
  // Internal
  double intMultiple16 = exp.int_slice_length % 16;
  if (intMultiple16 != 0) {  
    cout << "ERROR: Internal slice length must be multiple of 16!" << endl;
    cout << "\tint_slice_length = " << exp.int_slice_length << endl;
    return 0;
  }

  // Check that the external/internal delay is at least as 
  // long as the total external sampling period
  double extIntDelay = exp.ext_int_delay/exp.adc_offset_time_clock; // us
  // if (extIntDelay < exp.ext_sample_time){
  //   cout << "ERROR: External/internal delay must be at least as long as the total external sampling period!" << endl;
  //   cout << "\tExternal sampling period = " << exp.ext_sample_time << " us" << endl;
  //   cout << "\tExternal/internal delay = " << HEX(exp.ext_int_delay,2) << " (in " << 
  //     exp.adc_offset_time_clock << " MHz ticks)" << endl;
  //   cout << "                                = " << extIntDelay << " us" << endl;
  //   cout << "\tMinimum = " << 
  //     HEX(exp.ext_sample_time*exp.adc_offset_time_clock,2) << " (in " <<
  //     exp.adc_offset_time_clock << " MHz ticks)" << endl;
  //   return 0;
  // }

  // Check if the extenral adc offet + external sampling time
  // is longer than the external macropulse width / bunch period
  double extADCoffset = exp.ext_ADC_offset/exp.adc_offset_time_clock; // us 
  if ((extADCoffset + exp.ext_sample_time) > exp.macropulse_width){
    cout << "Error: Total external samping period is longer than macropulse width / bunch period!" << endl;
    cout << "\tExternal ADC offset + sampling period = " << 
      extADCoffset + exp.ext_sample_time << " us" << endl;
    cout << "\tExternal macropulse width = " << exp.macropulse_width << " us" << endl;
    return 0;
  }

   // Check that the number of internal triggers * internal sampling period does
   // not exceed the total remaining internal period
   // Get accelerator cylce period
   double cyclePeriod = 1e6/exp.macropulse_freq; // us
   double extTrigTimeout = exp.ext_trig_timeout/exp.adc_offset_time_clock; // us 
   double intADCoffset = exp.int_ADC_offset/exp.adc_offset_time_clock; // us   
   // Get remaining time for internal sampling
   double intPeriod = cyclePeriod // cycle period
     - exp.macropulse_width // minus the macropulse width
     - extTrigTimeout // minus external trig timeout [us]
     - extIntDelay // minus external/internal delay [us]
     - (intADCoffset + exp.int_sample_time); // minus a single internal trigger period (effeictively int_ext_delay)
  
  // Make sure remaining internal period is greater than zero
  if (intPeriod <= 0){
    cout << "Error: Remaining time for internal triggers is <= 0!" << endl;
    cout << "\tRemaining internal period = " << intPeriod << endl;
  }
  
  // Check if the internal adc offet + internal sampling time
  // is longer than the remaining internal period
  if ((intADCoffset + exp.int_sample_time) > intPeriod){
    cout << "Error: Total internal samping time is longer than remianing internal period!" << endl;
    cout << "\tInternal ADC offset + sampling period = " << 
      intADCoffset + exp.int_sample_time << " us" << endl;
    cout << "\tRemaining internal period = " << intPeriod << " us" << endl;
    return 0;
  }
  
  // If number of internal triggers * internal sampling period is                       
  // greater than the remaining internal period...
  if (exp.int_trig_num*(intADCoffset + exp.int_sample_time) > intPeriod){
    cout << "Error: Number of internal triggers exceeds remaining internal period!" << endl;
    cout << "\tSet number of internal triggers = " << exp.int_trig_num << endl;
    cout << "\tMaximum number of internal triggers allowed in reamining time = " << 
      floor(intPeriod/(exp.int_trig_num*(intADCoffset + exp.int_sample_time))) << endl;
    return 0;
  }

  return 1;

}
 
// Form uin32_t out of 2 x int16_t 
uint32_t dataVars::make_uint32_t(int16_t p0, int16_t p1){
 
  uint64_t value = static_cast<uint16_t>(p1);
  value <<= 16;
  value |= static_cast<uint16_t>(p0);

  return value;
    
}
  
// Form uin64_t out of 4 x int16_t 
uint64_t dataVars::make_uint64_t(int16_t p0, int16_t p1, int16_t p2, int16_t p3){
 
  uint64_t value = static_cast<uint16_t>(p3);
  value <<= 16;
  value |= static_cast<uint16_t>(p2);
  value <<= 16;
  value |= static_cast<uint16_t>(p1);
  value <<= 16;
  value |= static_cast<uint16_t>(p0);

  return value;
    
}

// Split uint32_t into 2 x int16_t 
int16_t* dataVars::split_uint32_t(uint32_t value){
 
  int16_t* split_value = new int16_t[2];
  split_value[0] = value & 0xFFFF;
  split_value[1] = (value >> 16) & 0xFFFF;

  return split_value;
    
}

// Split uint64_t into 4 x int16_t 
int16_t* dataVars::split_uint64_t(uint64_t value){
 
  int16_t* split_value = new int16_t[4];
  split_value[0] = value & 0xFFFF;
  split_value[1] = (value >> 16) & 0xFFFF;
  split_value[2] = (value >> 32) & 0xFFFF;
  split_value[3] = (value >> 48) & 0xFFFF;

  return split_value;
    
}

// Get just the event number from the event header
uint64_t dataVars::get_event_number(int16_t *data, uint64_t hdr_start_loc){

  // Get the trigger/event number
  uint64_t event_number = make_uint64_t(data[hdr_start_loc+tHdr_vars.EvNum_0],
					data[hdr_start_loc+tHdr_vars.EvNum_1],
					data[hdr_start_loc+tHdr_vars.EvNum_2],
					0);
  return event_number;

}

// Get just the EWT from the event header
uint64_t dataVars::get_EWT(int16_t *data, uint64_t hdr_start_loc){

  // Get the trigger/event number
  uint64_t EWT = make_uint64_t(data[hdr_start_loc+tHdr_vars.EWT_0],
			       data[hdr_start_loc+tHdr_vars.EWT_1],
			       data[hdr_start_loc+tHdr_vars.EWT_2],
			       0);
  
  return EWT;

}


// Check the end of a packet for repeated 0xDEADBEEF
uint16_t dataVars::check_dead_beef(int16_t* data, uint64_t packet_start,
				   uint16_t leftInPacket){

  // Get the packet number
  uint32_t pnum = (uint16_t)data[packet_start+1] << 16 
    | (uint16_t)data[packet_start];

  // Check remainder of packet is filled with 0xDEADBEEF
  while (leftInPacket != 0){
    // Find location to check
    int loc = packet_start + MAX_PACKET_LEN - leftInPacket;
    // If end of packet isn't filled with 0xDEADBEE, throw critical error
    if ((uint16_t)(data[loc] & 0xFFFF) != BEEF	
	and (uint16_t)(data[loc+1] & 0xFFFF) != DEAD){
      Logger::Instance()->write(0,"ERROR! End of packet "
				+ std::to_string(pnum)
				+ " not filled with 0xDEADBEEF!!");
    }
    // Subtract from leftInPacket
    leftInPacket -= 2;
  }

  return leftInPacket;

}
