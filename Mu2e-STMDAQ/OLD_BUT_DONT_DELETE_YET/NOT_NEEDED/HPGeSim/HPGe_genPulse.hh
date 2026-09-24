#ifndef HPGE_GENPULSE_HH
#define HPGE_GENPULSE_HH

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
#include <algorithm>    // std::sort

// XML interface
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Binary file code
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

#include "STMDAQ-TestBeam/utils/Logger.hh"
#include "STMDAQ-TestBeam/utils/Random.hh"

#include "STMDAQ-TestBeam/utils/dataVars.hh"

#define PI 3.14159265358979323846  /* pi */
//energy in keV, pos in cm, phi in rads, velocity in cm/us, time in us

// Clock                                                                                         
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

// Define HPGe_genPulse class
class HPGe_genPulse {
 
public:

  // Default constructor - shouldn't be used
  HPGe_genPulse();

  // ****************************************
  // Simulation variables
  // ****************************************

  // Clock frequency
  // double fADC = 320.0520833313;
  //  double fADC = 370;  
  double fADC = 0; // Get from config xml
  
  // Standard deviation of gaussian noise 
  // double noiseSD = 0.32;

  // Positron charge in coulomb 
  double q0 = 1.602176487e-19;

  // Inner radii cylinder (cm)
  double R1 = 0.44;
  // Outer radii cylinder (cm)
  double R2 = 3.5;
  // Length of cylinder (cm)
  double Lcyl = 8.38;

  // Holes saturated drift velocity (cm/us)
  double vh = 10;
  // Electrons saturated drift velocity (cm/us) 
  double ve = 10;

  // Energy in keV required to create an eh pair in Ge at 77 K
  double Eeh_Ge = 0.00296;

  // Pulse length (us)
  double pulseLength = 300;
  // 1ADC=0.57keV
  double ADC_toE = 0.57;

  // Preamp circuit constants
  double taushape = 50; // us
  double Capacity = 10000000000; // F

  // Weight for probability of  0 --> 11 comptons of photoelectric effects at different energies
  // Choose the probability of having 0 comptons to 0, although it is not
  // Max number of comptons
  int maxcompt=12;
  // Disbribution array
  int* distribution = new int[maxcompt]();
  int disc_prob[10][12] = {{0,762,196,37,4,1,0,0,0,0,0,0}, //100keV X-rays
			{0,381,310,185,88,23,9,3,1,0,0,0}, //200keV X-rays
			{0,205,244,228,165,96,39,12,2,3,1,0}, //300keV X-rays
			{0,156,184,244,176,128,54,31,11,2,1,0}, //400keV X-rays
			{0,124,186,232,210,124,63,29,11,3,1,0}, //500keV X-rays 
			{0,119,161,211,196,147,68,48,17,7,2,0}, //600keV X-rays
			{0,112,154,221,203,137,84,39,12,6,3,0}, //700keV X-rays 
			{0,110,160,171,227,154,74,38,14,11,1,0}, //800keV X-rays
			{0,124,167,187,178,157,81,49,11,10,4,0}, //900keV X-rays
			{0,125,153,186,165,146,101,43,25,6,2,3}}; //1000keV X-rays

  // Boolean to print to screen
  bool output = false;

  // Set the random seed
  unsigned int seed;   

  // Adc clock time
  double tadc = 0 ; // Get from config xml

  // Size of the full ADC sample
  unsigned long int sampleNum = 0;

  // ADC counnt array
  int16_t* ADC;

  // Rate in MHz, kHz and Hz
  double ratekHz = 0;
  double rateHz = 0;
  double rate = 0;

  // Average number of pulses in the sample
  int pulseNum = 0;

  // The total generated pulses
  uint32_t total_pulses = 0;

  // Counter for real number of generated pulses
  int realpulses_binary = 0;

  // ****************************************
  // Main cc functions
  // ****************************************
   
  // Simulation initalisation
  void init_sim(expConfig exp, std::string rate_stringkHz, double Energy, double timeSample);

  // Generate gaussian noise
  void gen_noise(double noiseSD);

  // Main pulse simulation code 
  void HPGe_ehModelSim(std::string rate_stringkHz,
		       double Energy, double noiseSD, double timeSample);

  // Generate pulse shape
  int16_t* gen_pulse(double Energy, unsigned long int pulseStart, unsigned long int pulseEnd);

  // Function to write simulation to binary file
  void write_to_file(std::string  output_path, int loop_num);

  // ****************************************
  // Auxillary functions
  // ****************************************

  // Generate a radial position in the active volume for the first process in cm
  std::vector<double> PosActive(){
  
    double rho_init = Random::Instance()->RealValue(R1,R2,false); //cm
    double phi_init = Random::Instance()->RealValue(0,2*PI,false); //rad
    double z_init = Random::Instance()->RealValue(0,Lcyl,false); //cm 
    
    std::vector<double> cylpos_init;
    cylpos_init.push_back(rho_init);
    cylpos_init.push_back(phi_init);
    cylpos_init.push_back(z_init);
    
    return cylpos_init;
    
  }

  // Generate radial position in the detector for the following 
  // Comptons/Photoelectric effects based on RMS gaussian distributions
  // from Geant4 histograms, energy in keV
  double PosCompPhot(std::vector<double> cylpos_init,double energy){

    double pos_processx = 0, pos_processy = 0, pos_processz = 0;
    double rad_pos = 0;
    
    // Convert to cartesian coordinates
    double xinit=cylpos_init.at(0)*cos(cylpos_init.at(1));
    double yinit=cylpos_init.at(0)*sin(cylpos_init.at(1));
    double zinit=cylpos_init.at(2);

    // Gaussian to generate positions
    double mean=0;
    double sigma, x_gauss, y_gauss, z_gauss;
    if(energy < 600) sigma = 0.57;
    if(energy >= 600 && energy <= 1000) sigma = 0.83;
    if(energy > 1000) sigma = 0.87;
    
    x_gauss = Random::Instance()->GaussValue(mean,sigma,false);
    y_gauss = Random::Instance()->GaussValue(mean,sigma,false);
    z_gauss = Random::Instance()->GaussValue(mean,sigma,false);
    
    pos_processx=xinit+x_gauss;
    pos_processy=yinit+y_gauss;
    pos_processz=zinit+z_gauss;

    // In reality we don't care about the movement in z direction we just care about the distance between the position and 
    // The electrodes which is in the xy plane
    // Return radial position of the following comptons
    rad_pos = sqrt((pos_processx*pos_processx) + (pos_processy*pos_processy));
    
    // Check if the position is in the active volume and return radial value
    if ((rad_pos > R1) && (rad_pos < R2)){
      return rad_pos;
    }
    else{
      if (output) std::cout << "POINT GENERATED OUTSIDE THE ACTIVE VOLUME, r0 = " 
			    << rad_pos << " cm, call function again..." << std::endl;
      return this->PosCompPhot(cylpos_init,energy);
    }
    
  }
  
  // Return the number of electron-holes pair created 
  // by each process (compton or photoelectric effect)
  int Neh(double energy){
  
    //True #Neh need to account for the statistical fluctuations (Fano Factor) to get reco #Neh
    return energy/Eeh_Ge; 

  }

  // Return Q(t) [N type]
  double Q_tNtype(double t, double r0){
  
    //Caculate collection time for holes
    double th=(R2-r0)/vh; 

    //Calculate collection time for electrons
    double te=(r0-R1)/ve;

    double Qpulse = 0;
  
    //Both e and h drifting
    if((t < th) && (t < te)){
      Qpulse = (q0/log(R2/R1))*(log(1+(vh*t/r0))-log(1-(ve*t/r0)));
    }
    // e have been collected but h are still drifting
    else if((t < th) && (t >= te)){
      Qpulse=(q0/log(R2/R1))*(log(1+(vh*t/r0))-log(R1/r0));
    }
    //h have been collected but e are still drifting
    else if((t < te) && (t >= th)){
      Qpulse=(q0/log(R2/R1))*(log(R2/r0)-log(1-(ve*t/r0)));
    }
    //Both e and h have been collected
    else if ((t > th) && (t > te)){
      Qpulse=q0;
      //Do the shaping just in this part
      //Qpulse=(-1)*(q0/Capacity)*(1-exp((-1)*(t+1000)/taushape));
    }

    //inverted pulse
    return (-1)*Qpulse;

  }

  // Return Q(t) [P type]
  double Q_tPtype(double t, double r0){

    //Caculate collection time for holes
    double th = (r0-R1)/vh;
    //Calculate collection time for electrons
    double te = (R2-r0)/ve;

    double Qpulse = 0; 

    //Both e and h drifting
    if((t<th)&&(t<te)){
      Qpulse=(q0/log(R2/R1))*(log(1+(ve*t/r0))-log(1-(vh*t/r0)));
      if (output) std::cout << "both drifting" << std::endl;
    }
    //e have been collected but h are still drifting
    else if((t<th)&&(t>=te)){
      Qpulse=(q0/log(R2/R1))*(log(R2/r0)-log(1-(vh*t/r0)));
      if (output) std::cout << "e collected" << std::endl;
    }
    //h have been collected but e are still drifting
    else if((t<te)&&(t>=th)){
      Qpulse=(q0/log(R2/R1))*(log(1+(ve*t/r0))-log(R1/r0));
      if (output) std::cout << "h collected" << std::endl;
    }
    //Both e and h have been collected
    else if ((t>th)&&(t>te)){
      Qpulse=q0;
      if (output) std::cout << "both collected" << std::endl;
    }

    //inverted pulse 
    return (-1)*Qpulse;

  }
  
  // Return a random value for the number of comptons and
  // photoelectric effects contributing to the peak according 
  // to results from geant4. Input: energy of the simulated peak
  int Number_compt_phot(double energy){
  
    // Distribution index 
    int index;
  
    //Choose the distribution according to the energy of peaks simulated
    if(energy<=150){index=0;}
    if((energy>150)&&(energy<=250)){index=1;}
    if((energy>250)&&(energy<=350)){index=2;}
    if((energy>350)&&(energy<=450)){index=3;}
    if((energy>450)&&(energy<=550)){index=4;}
    if((energy>550)&&(energy<=650)){index=5;}
    if((energy>650)&&(energy<=750)){index=6;}
    if((energy>750)&&(energy<=850)){index=7;}
    if((energy>850)&&(energy<=950)){index=8;}
    if(energy>950){index=9;}

    // Get the number of Compton photons from the distribution
    int n_compt_phot = Random::Instance()->DiscreteValue(disc_prob[index], maxcompt);
    
    // Return the number of compton photons
    return n_compt_phot;
    
  }
  
  // Function to distribute the total amplitude (twiceA/2) for each event
  // Return a vector with the amplitudes (energies) in ADC counts. The vector 
  // returned will be of the size of number of processes (n_compt_phot) in the event  
  std::vector<double> Distribute_energies(double energy, int n_compt_phot){

    //Distribute amplitudes
    //max value to generate the amplitudes
    double max=energy/2;
    double leftE=energy;
    std::vector<double> energies;

    //Loop to generate energies for each process
    for(int i=0;i< n_compt_phot;i++){
   
      //if last process fill with all the remaining energy
      if(i==(n_compt_phot-1)){energies.push_back(leftE); break;}
 
      double randE = Random::Instance()->RealValue(1.0,max,false); // define the range 
      energies.push_back(randE);
    
      //update max value to generate the energies
      max=max-(randE/2);
 
      leftE = leftE-randE;
    }

    if (output) std::cout << "Total simulated energy: " << energy << " keV" << std::endl;
    if (output) std::cout << "Energy for each process: ";
    if (output) print(energies);
    if (output) std::cout << " keV" << std::endl;
 
    return energies;

  }

  // Functions that simulates the preamp shaping of the decay tail 
  double HighPassFilter(double t, double tshape, double Neh){
  
    double Voltage;
    Voltage=q0*Neh*(1.0/Capacity+(1-1.0/Capacity)*exp((tshape-t)/taushape)); 
    return (-1)*Voltage;
    
  }

  // Covert Voltage from preamp (which is proportional to 
  // Neh which is equivalent to energy) to ADC Counts
  int16_t preamptoADC(double Voltage){

    double Energy = Voltage*Eeh_Ge;
    double ADCdoub = Energy/ADC_toE; 
    int16_t ADC= int16_t(ADCdoub); //ADC Counts
    
    return ADC;

  }

  // Print vectors
  void print(std::vector<double> const &input){ 
    for (long unsigned int i = 0; i < input.size(); i++) {
      if (output) std::cout  <<  input.at(i)  <<  ' ';
    }
  }

private:

};

#endif
