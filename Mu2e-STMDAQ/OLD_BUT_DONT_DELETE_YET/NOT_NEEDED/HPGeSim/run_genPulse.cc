#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <boost/chrono.hpp>

#include "STMDAQ-TestBeam/HPGeSim/HPGe_genPulse.hh"

// Data variables class
dataVars inVars;

// Timers 
std::chrono::high_resolution_clock::time_point t1 ;
std::chrono::high_resolution_clock::time_point t2 ;

// Main function with arguments
int main(int argc, char *argv[]){

  // Photon rate
  std::string rate_stringkHz = std::string(argv[1]);
  // Photon energy (keV)
  double Energy = std::stod(argv[2]);
  // Noise standard deviation
  double noiseSD = std::stod(argv[3]);
  // The simulationn sample time
  double timeSample = std::stod(argv[4]);
  // The random seed
  unsigned int seed_value = std::stod(argv[5]);
  // Path to file
  std::string output_path = std::string(argv[6]);

  // Initialise the Logger (set threshold for messages to be 
  // DEBUG (other threshold are INFO, WARNING, ERROR)
  Logger::Instance(Logger::DEBUG);
  Logger::Instance()->setStylePlain();

  // Open the XML file and instantiate the xml_file object
  string xml_path = EnvVars::expand("${STM_XML}");
  Xml* xml_file = new Xml(xml_path);

  // Set struct experimental configuration variables from xml file
  struct expConfig exp;
  exp = inVars.getXMLvalues(exp);

  // Check experimental configuration variables
  if (inVars.checkExpConfig(exp) == 0){
    cout << "Exiting due to bad value in " << xml_path << endl;
    exit(0);
  }

  // Print success to user
  cout << "Parameters in " << xml_path << " are okay!" << endl;
  
  // Create instance of HPGe_genPulse
  HPGe_genPulse HPGe_pulse;

  // If the provided seed is zero...
  if(seed_value == 0){
    // Initliase with random seed
    Random::Init();
  }
  // Else set the seed that is provided
  else{
    Random::Init(seed_value);
  }

  // Split up large time sample into smaller chunks of data
  int loops = 0;
  int final_time_sample = 0;
  if (timeSample > 1e5){
    loops = floor(timeSample/1e5);
    final_time_sample = timeSample - (loops*1e5);
  }
  else{
    final_time_sample = timeSample;
  }

  // Total processing time
  double t_total = 0;

  // Loops
  for (int i = 0; i <= loops; i++){

    // Print guestimate remaining time for user
    if (i > 0){
      double t_avg = t_total/(i+1);
      double t_rem_ms = t_avg*((double)loops + final_time_sample/1e5);
      double t_rem_hours = t_rem_ms/3.6e6;
      double t_rem_minutes = t_rem_hours*60;
      if (t_rem_hours >= 1){
	std::cout << "Roughly " << t_rem_hours << " hours remaining..." << std::endl;
      }
      else{
	std::cout << "Roughly " << t_rem_minutes << " minutes remaining..." << std::endl;
      }
    }

    // Set time to maximum allowed time
    double time = 1e5;
    // If final call, set time to final time sample
    if (i == loops) time = final_time_sample;

    // Call simulation initalisation
    HPGe_pulse.init_sim(exp,rate_stringkHz,Energy,time);
    
    // Get the time at the start as t1
    t1 = std::chrono::high_resolution_clock::now();
    
    // Generate gaussian noise
    HPGe_pulse.gen_noise(noiseSD);
    
    // // Poisson generate pulse times for a given rate within sample time
    // std::poisson_distribution<int> pulseTime (1/HPGe_pulse.rate);
    // unsigned long int timeIndex = 0;
    // for (int i = 0; i < 1; i++){
    //   int dummy = int(pulseTime(HPGe_pulse.gen)/HPGe_pulse.tadc);
    //   timeIndex = 0;
    //   unsigned long int max = timeIndex + HPGe_pulse.pulseLength/HPGe_pulse.tadc;
    //   int16_t* pulse = HPGe_pulse.gen_pulse(Energy,timeIndex,max);
    //   for (int j = 0; j < 10; j++) std::cout << pulse[j] << std::endl;
    // }
    
    // Call the simulation
    HPGe_pulse.HPGe_ehModelSim(rate_stringkHz,Energy,noiseSD,time);
    
    // Get time at end as t2
    t2 = std::chrono::high_resolution_clock::now();
    double t_ms = std::chrono::duration<double, std::milli>(t2-t1).count();

    // Print simulation time
    std::cout << "HPGe Simulation computing time = " << t_ms << " millseconds" << std::endl;
    
    // Update total time
    t_total += t_ms;
    
    // Write data to binary file
    HPGe_pulse.write_to_file(output_path,i);

    delete[] HPGe_pulse.ADC;
    
  } // End loops

  // Print total number of pulses for user
  std::cout << "Genereated a total of " << HPGe_pulse.total_pulses << " pulses" << std::endl;

  return 0;

}
