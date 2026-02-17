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
#include <vector>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// XML interface
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Binary file code
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

// HPGeSim Header
#include "STMDAQ-TestBeam/HPGeSim/HPGeSim.hh"

#include "STMDAQ-TestBeam/utils/Logger.hh"
#include "STMDAQ-TestBeam/utils/Random.hh"

#include "STMDAQ-TestBeam/utils/ziggurat.hh"

using namespace std;

// Data variables class
dataVars inVars;

// Main function to run if not calling simulation from frontend
int main(){

  // Define instance of HPGeSim
  HPGeSim * sim = new HPGeSim();

  // Initialise the Logger (set threshold for messages to be
  // DEBUG (other threshold are INFO, WARNING, ERROR)
  Logger::Instance(Logger::DEBUG);
  Logger::Instance()->setStylePlain();

  // Initliase instance of Random() with random seed
  Random::Init();

  // Set boolean to ouput messages to screen to true
  bool output = true;
 
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

  // Get random run number
  int run_number = Random::Instance()->IntegerValue(2,10,false);

  // Setup simulation
  sim->setupSim(xml_file,exp,output,run_number);

  // Run simulation
  sim->runSim(exp,output);  
    
  return 1;
  

}
