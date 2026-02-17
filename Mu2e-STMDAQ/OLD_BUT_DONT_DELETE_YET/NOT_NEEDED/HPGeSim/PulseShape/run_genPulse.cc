#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <boost/chrono.hpp>

#include "STMDAQ-TestBeam/HPGeSim/PulseShape/ehDriftFunctions_setseed.hh"

// Data variables class
dataVars inVars;

// Timers                                                                                        
boost::chrono::high_resolution_clock::time_point t1 ;
boost::chrono::high_resolution_clock::time_point t2 ;

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
  
  // Create instance of ehDriftFunctions
  ehDriftFunctions ehFunc;

  // If the provided seed is zero...
  if(seed_value == 0){
    // Initliase with random seed
    Random::Init();
  }
  // Else set the seed that is provided
  else{
    Random::Init(seed_value);
  }

  // Call simulation initalisation
  ehFunc.init_sim(exp,rate_stringkHz,Energy,timeSample);

  // Get the time at the start as t1
  t1 = boost::chrono::high_resolution_clock::now();

  // Generate gaussian noise
  ehFunc.gen_noise(noiseSD);

  // // Poisson generate pulse times for a given rate within sample time
  // std::poisson_distribution<int> pulseTime (1/ehFunc.rate);
  // unsigned long int timeIndex = 0;
  // for (int i = 0; i < 1; i++){
  //   int dummy = int(pulseTime(ehFunc.gen)/ehFunc.tadc);
  //   timeIndex = 0;
  //   unsigned long int max = timeIndex + ehFunc.pulseLength/ehFunc.tadc;
  //   int16_t* pulse = ehFunc.gen_pulse(Energy,timeIndex,max);
  //   for (int j = 0; j < 10; j++) std::cout << pulse[j] << std::endl;
  // }

  // Call the simulation
  ehFunc.HPGe_ehModelSim(rate_stringkHz,Energy,noiseSD,timeSample);

  // Get time at end as t2
  t2 = boost::chrono::high_resolution_clock::now();
  // Print simulation time
  std::cout << "HPGe Simulation computing time = " << 
    boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  // Write data to binary file
  ehFunc.write_to_file(output_path);

  return 0;

}
