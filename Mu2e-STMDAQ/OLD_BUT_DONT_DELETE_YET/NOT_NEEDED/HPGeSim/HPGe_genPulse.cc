#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <boost/chrono.hpp>

#include "STMDAQ-TestBeam/HPGeSim/HPGe_genPulse.hh"

// Standard constructor - shouldn't be used
HPGe_genPulse::HPGe_genPulse() {}

// Function to write simulation to binary file
void HPGe_genPulse::write_to_file(std::string  output_path, int loop_num){

  // Store in a binary file
  // Get output path
  std::string stringfile = output_path;
  // Get file name
  char file_name[stringfile.size()+1];//as 1 char space for null is also required
  // Convert filename to string
  strcpy(file_name, stringfile.c_str());

  // Open binary file
  FILE *fp = 0;
  if (loop_num == 0){
    fp = fopen(file_name, "w+b");
  }
  else{
    fp = fopen(file_name, "a+b");
  }

  // Write to file
  fwrite(&ADC[0], sizeof(int16_t), sampleNum, fp );
  // Close file
  fclose(fp);
  if (output) std::cout << "Binary file: " << stringfile << " created." << std::endl;


  return;

}

// Generate pulse shape
int16_t* HPGe_genPulse::gen_pulse(double Energy, unsigned long int pulseStart,
	       unsigned long int pulseEnd){

  // Generate the number of comptons and photoelectric effects 
  // according to geant4 simulation distributions
  int n_compt_phot = Number_compt_phot(Energy);
  if (output) std::cout << "***********************NEW EVENT: " << realpulses_binary << 
		" with " <<  n_compt_phot << " comptons***********************" << std::endl;
    
  // Total number of eh pairs 
  double Nehevent=0;
    
  // Get the distribution of the energy for each compton
  std::vector<double> energiescomptons = Distribute_energies(Energy,n_compt_phot);
  // Generate initial position
  std::vector<double> InitPos = PosActive();

  // Positions of compton events
  double r0;

  // Initialise induced charged (Qpulse) array
  double* Qpulse = new double[pulseEnd-pulseStart] {};
  // Initialise voltage in preamp array
  double* Voltage = new double[pulseEnd-pulseStart] {};
  // Pulse array
  int16_t* pulse = new int16_t[pulseEnd-pulseStart] {};

  // Loop over the number of Comptons in the event
  for(int c = 0; c < n_compt_phot; c++){
    
    // Get the energy and the number of eh pairs created in the Compton
    double Ei = energiescomptons.at(c); // Energy
    int Nehi = Neh(Ei); // Number
    if (output) std::cout << "Nehi: " << Nehi << std::endl;

    // Add to total number of eh pairs
    Nehevent += Nehi;
      
    // Calculate the positions for following comptons              
    if (c == 0){ 
      // Initial compton position
      r0 = InitPos.at(0);
    }
    else{
      // Positions of subsequent comptons
      r0 = PosCompPhot(InitPos,Ei);
    }
    // Print new properties of new compton event
    if (output) std::cout << "---------New compton at: " << r0 << " cm, energy: " 
			  << Ei << " keV, Neh: " << Nehi << " ---------" << std::endl;

    // Loop over ADC values in pulse, starting from the randomly generated pulse time
    for (unsigned long int j = pulseStart; j < pulseEnd; j++){
	
      // Find start time in tadc (not us)
      double time = (j+1-pulseStart)*tadc;
	
      // Add to charge array
      Qpulse[j-pulseStart] += Nehi*Q_tNtype(time,r0);
	
    } // For timeIndex
      
  } // For n_compt_phot   

    // Print e-h pair event number
  if (output) std::cout << "Neh event: " << Nehevent << std::endl;

  // *******************
  // Preamplifier effect
  // *******************

  // Initialise shaping time (the first time where the induced charge is Nehxq0)
  double ts = 0; // us
    
  // Need to add the preamp shaping to charge induced in the detector (Qpulse) 
  // Loop over ADC values in pulse
  for (unsigned long int j = pulseStart; j < pulseEnd; j++){
      
    // Find start time in tadc (not us)
    double timeV = (j+1-pulseStart)*tadc;

    // When Qpulse/q0 is equal to  Neh, then shape it
    int var1 = Qpulse[j-pulseStart]/q0;
    int var2 = (-1)*Nehevent;
    //if (output) std::cout << "var1: " << var1 << " var2: " << var2 << std::endl;
      
    // If Qpulse/q0 == Neh, then shape it
    // Need to add the +1 or -1 because when converting to int 
    // we can get a difference of +-1 between var1 and var2
    if (var1==var2 || (var1+1)==var2 || (var1-1)==var2){ 
	
      // If shaping time is zero...
      if(ts==0){
	  
	// Set the shaping time as the pulse start time
	ts = timeV;	 
	// Print shaping time to screen
	if (output) std::cout << "------Shaping time: " << ts << " us" << std::endl;	  
	// Get preamp voltage
	Voltage[j-pulseStart] += HighPassFilter(timeV,ts,Nehevent);
      }
      // Else if shaping time is not zero...
      else{
	// Get preamp voltage
	Voltage[j-pulseStart] += HighPassFilter(timeV,ts,Nehevent);
	/*if (output) std::cout << "hi1" << std::endl;*/
      }
    } // If var1==var2
      // Else if Qpulse/q0 /= Neh (+/- 1), don't shape it
    else{
      Voltage[j-pulseStart] += Qpulse[j-pulseStart]; /*if (output) std::cout << "hi2" << std::endl;*/
    }

    // Convert voltage output from preamp to ADC counts 
    pulse[j-pulseStart] = preamptoADC(Voltage[j-pulseStart]/q0);

  } // For timeIndex 

  delete[] Qpulse;
  delete[] Voltage;
  
  return pulse;

}

// Main pulse simulation code
void HPGe_genPulse::HPGe_ehModelSim(std::string rate_stringkHz, 
		     double Energy, double noiseSD, double timeSample){

  // Time index
  unsigned long int timeIndex = 0;

  // Loop over number of pulses
  for (int i = 0; i < pulseNum; i++){

    // Poisson generate pulse times for a given rate within sample time
    // Randomly generate pulse time in clock ticks
    timeIndex += int(Random::Instance()->PoissValue(rate)/tadc);
    
    // Need to add this
    if (timeIndex >= sampleNum){
      if (output) std::cout << "" << std::endl; 
      if (output) std::cout << "-----Peak generated at: " << timeIndex*tadc << " us " 
		<< " outside sample lenght of: " << timeSample << " us -----END LOOP-----" 
		<< std::endl; 
      continue;
    }

    // Get the index value of the last element in the pulse 
    unsigned long int max = timeIndex + pulseLength/tadc;

    // If the pulse exceeds the sample length, max out at the sample length
    if (timeIndex + (pulseLength/tadc) > sampleNum){
      max = sampleNum;
    }

    // Generate pulse shape
    int16_t* pulse = gen_pulse(Energy,timeIndex,max);
    
    // Add pulse to noise
    for(long unsigned int i = timeIndex; i < max; i++){
      ADC[i] += pulse[i-timeIndex];
    }    

    delete[] pulse;

    // Print peak time
    if (output) std::cout << "Peak " << i << " at " << double(timeIndex*tadc) << " us" << std::endl;

    // Increment number of simulated pulses
    realpulses_binary++;    

  } // for pulseNum

  // Print total number of pulses
  if (output) std::cout << "Real number of pulses stored in binary file: " << realpulses_binary << std::endl;


} // End Sim Function

// Generate gaussian noise
void HPGe_genPulse::gen_noise(double noiseSD){

  // Intialise data array of ADC values with Gaussian noise
  // The input parameter is noiseSD mV which is the standard deviation of the gaussian in mV
  double sigma_noise_ADC = noiseSD*38.5;
  if (output) std::cout << "NoiseSD: " << noiseSD << " mV = " 
			<< sigma_noise_ADC << " ADC Counts" << std::endl;


  // Loop over all samples
  for(unsigned long int i = 0; i < sampleNum; i++){
    // Add noise from Gaussian
    double noise = Random::Instance()->GaussValue(0,sigma_noise_ADC,false);
    ADC[i] = noise;
  }

  return;

}

void HPGe_genPulse::init_sim(expConfig exp, std::string rate_stringkHz, double Energy, double timeSample){

  // get the ADC sampling frequency
  fADC = exp.adc_time_clock;
  tadc = exp.adc_time_clock_period;

  // Get the size of the full ADC sample
  sampleNum = floor(timeSample/tadc);

  // Initialise ADC array
  ADC = new int16_t[sampleNum] {};
  
  // Print to screen
  if (output) std::cout << "Number of ADC values in the bin file: " << sampleNum << std::endl;
  if (output) std::cout << "fADC: " << fADC << " MHz" << std::endl;
  if (output) std::cout << "Real time of the sample: " << sampleNum*tadc<<" us" << std::endl;

  // Get the rate as a double in MHz, kHz and Hz
  ratekHz = std::stod(rate_stringkHz); // kHz
  rateHz = ratekHz*1000; // Hz
  rate = rateHz * 1e-6; // MHz
  if (output) std::cout<<"Rate: "<<rateHz<<" Hz"<<std::endl;

  // Get the average number of pulses in the sample
  pulseNum = rate*timeSample;
  total_pulses += pulseNum;
  std::cout << "Number of pulses generated (real number of simulated pulses at the end of the file): " << pulseNum << std::endl;
  if (output) std::cout << "Simulated Pulse Energy: " << Energy << " keV = " << 
		Energy/ADC_toE << " ADC Counts" << std::endl;

  return;

}

