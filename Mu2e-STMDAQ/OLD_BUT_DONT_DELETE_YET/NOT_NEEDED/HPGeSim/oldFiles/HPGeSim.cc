#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include <list>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <time.h>
#include <memory.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// HPGeSim Header
#include "STMDAQ-TestBeam/HPGeSim/HPGeSim.hh"

// Extract data
#include "STMDAQ-TestBeam/processData/extractData.hh"

// Extract data class  
extractData exData; 

bool ADCemulate = true;

using namespace std;

// Standard constructor - shouldn't be used
HPGeSim::HPGeSim() {}

// Function to generate pulse data
int16_t* HPGeSim::genPulseData(struct expConfig exp, double rateHz, 
		 double sampleTime, unsigned long int sampleLen){

  // Rate in us
  double rate = rateHz * 1e-6; // 1kHz in us
  // Average number of pulses in the sample
  int pulseNum = rate*sampleTime;

  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleLen] ();
  
  // Randomly generate pulse times for given rate within sample time
  random_device rd;
  exponential_distribution<double> pulseTime ( rate ) ;
  mt19937 rnd_gen(rd()) ;
  
  // Now, calculate and add pulses to ADC array
  // Loop over average number of pulses
  int timeIndex = 0;
  for (int i = 0; i < pulseNum; i++){
    // Randonly generate pulse times in clock ticks
    timeIndex += int(pulseTime(rnd_gen)/exp.adc_time_clock_period);
    // Get the index value of the last element in the pulse
    uint max = timeIndex + pulseLength/exp.adc_time_clock_period;
    // If the pulse exceeds the sample length, max out at the sample length
    if (timeIndex + pulseLength/exp.adc_time_clock_period > sampleLen) max = sampleLen;
    // Print the pulse time
    // Loop over the number of ADC values for a pulse, starting from the randolmy generated pulse time
    for (uint j = timeIndex; j < max; j++){
      // Find the time starting from zero
      double time = (j-timeIndex)*exp.adc_time_clock_period;
      // Generate pulse data with noise and add to ADC array
      ADC[j] += pulseCalc(time);
    }
  }

  return ADC;

}

// Function to get a trigger's worth of data
void HPGeSim::getData(struct expConfig exp, struct headerInfo trigInfo) {

  // Get external/internal mode
  uint8_t mode = trigInfo.mode;

  // Get sampling time and average rate in detector 
  // depending on whether we're in external or internal mode
  double rateHz = 0;
  double sampleTime = 0;
  uint sliceLen = 0;
  uint sliceNum = 0;

  // External mode
  if (mode == 0){
    sampleTime = exp.ext_sample_time;
    rateHz = getOnSpillRate();
    sliceLen = exp.ext_slice_length;
    sliceNum = exp.ext_slice_num;
  }
  // Internal mode
  else if (mode == 1) {
    sampleTime = exp.int_sample_time;
    rateHz = getOffSpillRate();
    sliceLen = exp.int_slice_length;
    sliceNum = exp.int_slice_num;
  }
  else{
    cout << "Error: Mode is not set to external [0] or internal [1]..." << endl;
    cout << "Exiting...\n" << endl;
    exit(0);
  }
  
  // cout << "\nSample time = " << sampleTime << " us" << endl;
  // cout << "Slice Len = " << sliceLen << endl;
  // cout << "Slice Num = " << sliceNum << endl;

  // Size in bytes of each slice
  uint sliceSize = 2*sliceLen;
  // Length of each slice in time
  double sliceTime = sliceLen*exp.adc_time_clock_period;
  // cout << "Slices per packet = " << slicesPerPacket << endl;
  // cout << "Slice size = " << sliceSize << " bytes" << endl;
  // cout << "Slice time = " << sliceTime << " us" << endl;

  // Total number of ADC values  
  unsigned long int sampleLen = sliceLen*sliceNum;
  // Total sample size in bytes
  unsigned long int sampleSize = 2*sampleLen;

  // cout << "Sample time = " << sampleTime << " us" << endl;
  // cout << "Sample length = " << sampleLen << " int16_t entries" << endl;
  // cout << "Sample size = " << sampleSize << " bytes" << endl;
  
  // Find number of data packets
  int packetNum = sliceNum/slicesPerPacket;
  if (packetNum < 1) packetNum = 1;
  // Calculate array length of each packet
  unsigned int packetLen = fw_pHdr_Len+fw_tHdr_Len+slicesPerPacket*(fw_sHdr_Len+sliceLen);
  // Calcuate actual data size of each packet in bytes
  uint packetSize = 2*packetLen;

  // cout << "Number of packets = " << packetNum << endl;
  // cout << "Packet length including headers = " << packetLen << " int16_t entries" << endl;     
  // cout << "Packet size = " << packetSize << " bytes" << endl;
  
  // Calculate total length of data array including headers
  long unsigned int dataLen = sampleLen
    +sliceNum*fw_sHdr_Len
    +packetNum*(fw_pHdr_Len+fw_tHdr_Len);
  // Calcuate full data size in bytes
  long unsigned int dataSize = 2*dataLen;

  // cout << "Total data length including headers = " << dataLen << " int16_t entries" << endl;
  // cout << "Total data size = " << dataSize << " bytes" << endl;

  // Call function to generate pulse data
  int16_t* ADC = genPulseData(exp,rateHz,sampleTime,sampleLen);

  // Intialise data array for each packet
  int16_t data[packetLen];
  
  // Initialise struct for each packet
  struct packet pack;

  // Initialise data array for write to binary file
  int16_t* dataForBin = new int16_t[sampleLen] ();

  // Check writing the ADC data does not exceed the remaining size
  exData._bf->incoming_raw_data_size(sampleLen);

  // Count the number of adc values
  unsigned long int adcCount = 0;
  // Count the number of slices
  int sliceCount = 0;

  // Max int16_t value counter
  uint int16MAXcount = 0;

  // Get trigger header info
  uint8_t channel = trigInfo.channel;
  uint64_t macroTime = trigInfo.macroTime;
  uint32_t ADCoffset = trigInfo.ADCoffset;
  uint32_t macroCount = trigInfo.macroCount;
  uint16_t microCount = trigInfo.microCount;
  uint32_t sliceTm = sliceTime/exp.adc_time_clock_period;

  // Loop over total number of packets
  for (int p = 0; p < packetNum; p++){
    // Count the total number of data entries per packet
    unsigned long int count = 0;  
    // Insert packet header	
    uint16_t *packetHeader = sim_fw_pHdr(uint32_t(packetCount));
    memcpy(data+count, packetHeader, fw_pHdr_Len * sizeof *packetHeader);
    count += fw_pHdr_Len; // Increment data counter
    // Insert trigger header
    uint16_t *triggerHeader = sim_fw_tHdr(channel,
				      mode,
				      macroTime,
				      ADCoffset,
 				      macroCount,
				      microCount,
				      sliceNum,
				      sliceTm);
    memcpy(data+count, triggerHeader, fw_tHdr_Len * sizeof *triggerHeader);
    count += fw_tHdr_Len; // Increment data counter
    // Loop over number of slices per packet
    for (uint s = 0; s < slicesPerPacket; s++){
      // Insert slice header
      uint64_t sliceNo = sliceCount;
      uint16_t *sliceHeader = sim_fw_sHdr(sliceNo);
      memcpy(data+count, sliceHeader, fw_sHdr_Len * sizeof *sliceHeader);
      count += fw_sHdr_Len; // Increment data counter     
       // Loop over slice length and fill with ADC data
      for (uint j = 0; j < sliceLen; j++) {
	int16_t ADCdata = 0;
	// Insert incremeting data like ADC emulation
	if (ADCemulate){
	  ADCdata = adcCount - int16MAXcount*INT16_T_MAX;
	  if (adcCount == INT16_T_MAX*(int16MAXcount+1)) int16MAXcount += 1;
	}
	else{
	  // Insert ADC data with random noise
	  double time = adcCount*exp.adc_time_clock_period;
	  double rd = (double)rand() / RAND_MAX;
	  double noise = -noiseSD + rd*(2*noiseSD); // Random noise
	  ADCdata = ADC[adcCount]+noise;
	}
	data[count] = ADCdata; // Add ADC data to packet
	dataForBin[adcCount] = ADCdata; // Add ADC data to binary file array
	count += 1; // Increment data counter
	adcCount += 1; // Increment ADC counter
      }
      // Increment slice counte
      sliceCount += 1;
    } // End loop over number of slices per packet
    // Send each 10G packet struct to socket
    pack.data = data;
    pack.size = packetSize;
    // UDP send
    _UDPout->sendPacket(pack,socket);
    // Check if packet counter is at max and zero if so
    if (packetCount == UINT32_T_MAX) {
      packetCount = 0;
    }
    else{
      // Increment packet counter
      packetCount += 1;
    }
  } // End loop over number of packets

  // Write ADC data to binary file
  exData._bf->write_raw_data(dataForBin,adcCount);
}

// Function that simulates the accelerator super cycle 
void HPGeSim::simSuperCycle(expConfig exp,bool output){

  // Time at start of macropulse
  auto macroStart = std::chrono::high_resolution_clock::now();
  
  // ************
  // EXTERNAL MODE
  // ************
  
  // Set mode to external
  mode = 0;
  
  // Get macropulse trigger time (SHOULD CONVERT TO CLOCK TICKS)
  uint64_t macroTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); // ms 
  // Fill struct with firmware trigger header info
  struct headerInfo extMode = {trigCount, // Trigger number
			       macroTime, // Macropulse trigger time
			       mode, // Trigger mode: external = 0
			       microCount, // Micropulse counter
			       channel, // HPGe/LaBr
			       exp.ext_ADC_offset}; // ADC offset
  
  // Wait for ADC offset
  usleep(extADCoffset); // us
  
  // Receive external beam trigger - sample data
  getData(exp,extMode);
  
  // Increment the trigger counter
  trigCount++;
  
  // Set the micropulse number and time to zero if not in the first macropulse
  if (macroCount != 0) microStartTime = 0;
  
  // Now, find the time in the accelerator cycle after concluding external mode
  double timeInMacropulse = microStartTime // The time of the first micropulse trigger
    + extADCoffset // The external ADC offset
    + exp.ext_sample_time // The external sampling time
    + extTrigTimeout // The external trigger timeout
    + extIntDelay; // The external/internal delay
  
  // The time after external mode data
  auto extEnd = std::chrono::high_resolution_clock::now(); 
  
  // Time taken in us to simulate external data
  double extTime = std::chrono::duration<double, std::milli>(extEnd-macroStart).count()*1e3; 
  
  // Now, find the time to wait before switching to internal mode
  double extWait = 0;
  // Subtract the time to simulate data from the wait time... 
  if (extTime < timeInMacropulse) extWait = timeInMacropulse-extTime;
  // Wait
  usleep(extWait);
  
  // Increase micropulse counter by number of micropulses in that macropulse
  if (macroCount == 0){
    microCount += microPerMacro-microStart;
  }
  else{
    microCount += microPerMacro;
  }
  
  // ************
  // INTERNAL MODE
  // ************
  
  // Set mode to internal
  mode = 1;
  
  // Loop over number of internal triggers
  for (int t = 0; t < exp.int_trig_num; t++){
    
    // Fill struct with firmware trigger header info
    struct headerInfo intMode = {trigCount, // Trigger number
				 macroTime, // Macropulse trigger time
				 mode, // Trigger mode: external = 0
				 microCount, // Micropulse counter
				 channel, // HPGe/LaBr
				 exp.int_ADC_offset}; // ADC offset
    
    
    // Wait for internal ADC offset
    usleep(intADCoffset);
    
    // Receive internal trigger - sample data
    getData(exp,intMode);
    
    // Increment trigger counter
    trigCount++;
    
  } // End loop over internal triggers
  
    // The time after internal mode data
  auto intEnd = std::chrono::high_resolution_clock::now(); 
  
  // Time taken in us to simulate internal data
  double intTime = std::chrono::duration<double, std::milli>(intEnd-macroStart).count()*1e3; 
  
  // Now, find the time to wait to conclude the cycle before switching to external mode
  double intWait = 0;
  // Subtract the time to simulate data from the wait time... 
  if (intTime <= cyclePeriod){
    intWait = cyclePeriod-intTime;
  }
  else{
    cout << "Error! Simulation takes longer than the given accelerator cycle period!" << endl;
    cout << "Accelerator cycle period = " << cyclePeriod << " us" << endl;
    cout << "Simulation cycle period = " << intTime << " us"<< endl;
  }
  // Wait
  usleep(intWait);
  
  auto macroEnd = std::chrono::high_resolution_clock::now();
  double elapsed_time_ms = std::chrono::duration<double, std::milli>(macroEnd-macroStart).count();
  
  // If outputting information to screen...
  if (output){
    cout << "Macropulse " << macroCount << " (" << exp.macropulse_freq << " Hz) simulation period =  " << elapsed_time_ms << " ms" << endl;
  }
  
  // Increment macropulse counter
  macroCount += 1;
  
  // if (macroCount == 2) {
  //   exData._bf->close_output_file();
  //   exit(0);
  // }
  
}

void HPGeSim::setupSim(Xml* xml_file, expConfig exp, bool output, int run_number){

  // Create the instance of the UDP socket to write data
  UDPsocket* UDPout = new UDPsocket();
  setUDPout(UDPout);

  // Create UDP socket to be sent to READ IP of frontend
  socket = UDPout->createClient(READ);

  // Create the instance to write the binary files
  BinaryFile* bf = new BinaryFile();
  exData.setBF(bf);

  // Get max subrun size from XML
  int max_binary_file_size = xml_file->int_value("stm.max_binary_file_size",100);

 // Calculate max binary size in MB
  int Mbytes = 1048576;
  unsigned int max_binary_size = (unsigned int) max_binary_file_size * (unsigned int) Mbytes;
  std::string binary_file = xml_file->value("stm.sim_binary_filename");

  // Write warning in case of max binary size being too large
  if (max_binary_size >= INT32_MAX) {
    std::cout << "Max binary file size has to be less than 2 Gb, else int32 issues, EXITING" << std::endl;
    exit(-1);
  }

  // Set the maximum subrun file size
  exData._bf->set_subrun_filesize(max_binary_size);

  // Open the binary file to write data to
  exData._bf->open_raw_output_file(binary_file, run_number, 0);

  // Micropulse counter
  microCount = 0;
  // Macropulse counter
  macroCount = 0;
  // Trigger counter
  trigCount = 0;
  // External (beam) [0] or internal (source) mode [1]
  mode = 0;
  // Channel (HPGe [0] or LaBr [1])
  channel = 0;
  
  // Get the accelerator cycle period in us
  cyclePeriod = 1e6/exp.macropulse_freq; // us

  // Set micropulse frequency
  micropulse_frequency = microFrequency(exp.trigger_time_clock*1e6);

  // Calculate micropulses per macropulse
  microPerMacro = (exp.macropulse_width*1e-6)*micropulse_frequency;  

  // Find random micropulse in first macropulse to start from
  std::random_device rd;
  std::default_random_engine eng(rd());
  std::uniform_int_distribution<int> distr(0, microPerMacro);
  microStart = distr(eng);

  // If outputting information to screen...
  if (output){
    cout << "Accepting micropulse trigger " << microStart 
	 << " out of " << microPerMacro 
	 << " micropulses in first macropulse..." << endl;    
  }

  // Get the elapsed time of the accepted micropulse trigger 
  microStartTime = microStart/(exp.trigger_time_clock); // us

  // Get external ADC offset in us
  extADCoffset = exp.ext_ADC_offset/exp.adc_offset_time_clock;

  // Get external trigger timeout in us
  extTrigTimeout = exp.ext_trig_timeout/exp.adc_offset_time_clock; // us

  // Get external/internal delay in us
  extIntDelay = exp.ext_int_delay/exp.adc_offset_time_clock; // us

  // Get internal ADC offset in us
  intADCoffset = exp.int_ADC_offset/exp.adc_offset_time_clock; // us


}

// Main function to run if not calling simulation from frontend
int main(){

  // Define instance of HPGeSim
  HPGeSim sim;

  // Set boolean to ouput messages to screen to true
  bool output = true;

  // Open the XML file and instantiate the xml_file object
  string xml_path = EnvVars::expand("${STM_XML}");
  Xml* xml_file = new Xml(xml_path);

  // Set struct experimental configuration variables from xml file
  struct expConfig exp;
  exp = getXMLvalues(exp);

  // Check experimental configuration variables
  if (checkExpConfig(exp) == 0){
    cout << "Exiting due to bad value in " << xml_path << endl;
    exit(0);
  }

  cout << "Parameters in " << xml_path << " are okay!" << endl;

  // Get random run number
  int run_number = rand();

  // Setup simulation
  sim.setupSim(xml_file,exp,output,run_number);

  // Call simulation in infinite loop
  //  while(1){
    sim.simSuperCycle(exp,output);
    //  }
    
  exData._bf->close_output_file();

  return 1;
  

}
