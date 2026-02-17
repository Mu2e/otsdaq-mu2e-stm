#ifndef EHDRIFTFUNCTIONS_H
#define EHDRIFTFUNCTIONS_H

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

#define PI 3.14159265358979323846  /* pi */
//energy in keV, pos in cm, phi in rads, velocity in cm/us, time in us

class ehDriftFunctions {
 
 public:
  ehDriftFunctions();
 
  unsigned int setseed_man(unsigned int seedoption);
  unsigned int setseed_chrono();

  int Number_compt_phot(double energy, std::default_random_engine gen);
  std::vector<double> Distribute_energies(double energy, int n_compt_phot, std::default_random_engine gen);
  std::vector<double> PosActive(std::default_random_engine gen);
  int Neh(double energy);
  double PosCompPhot(std::vector<double> cylpos_init,double energy, std::default_random_engine gen);
  double Q_tNtype(double t, double r0);
  double Q_tPtype(double t, double r0);
  double HighPassFilter(double t, double tshape, double Neh);
  int16_t preamptoADC(double Voltage);

  void print(std::vector<double> const &input);
  //int16_t ADCpulse(double x);


  double fADC;
  //double noiseSD;
  double q0;
  double R1;
  double R2;
  double Lcyl;
  double vh;
  double ve;
  double Eeh_Ge;
  double taushape;
  double Capacity;
  double pulseLength;
  double ADC_toE;
  //private:
  };





//Default constructor
ehDriftFunctions::ehDriftFunctions(){
  //fADC = 320.0520833313;
  fADC = 370;
  //Standard deviation of gaussian noise 
  //noiseSD = 0.32;
  //inner radii cylinder (cm)
  R1 = 0.44;
  //outer radii cylinder (cm)
  R2 = 3.5;
  //Length of cylinder (cm)
  Lcyl = 8.38;
  //holes saturated drift velocity (cm/us)
  vh = 10;
  //electrons saturated drift velocity (cm/us) 
  ve = 10;
  //positron charge in coulomb 
  q0 = 1.602176487e-19;
  //Energy in keV required to create an eh pair in Ge at 77 K
  Eeh_Ge = 0.00296;
  //Pulse length (us)
  pulseLength=300;
  //1ADC=0.57keV
  ADC_toE = 0.57;


  //preamp circuit constants
  taushape = 50; //us
  Capacity = 10000000000; //F
}





unsigned int setseed_man(unsigned int seedoption){
  //set seed manually
  unsigned int seed=seedoption; 
  std::cout<<"Seed set manually = "<<seed<<std::endl; 
  return seed;
}

unsigned int setseed_chrono(){
  //construct a trivial random generator engine from a time-based seed
  unsigned int seed= std::chrono::system_clock::now().time_since_epoch().count(); 
  std::cout<<"Using a time-based (epoch) seed = "<<seed<<std::endl; 
  return seed;
   
}



//Generate a radial position in the active volume for the first process in cm
std::vector<double> ehDriftFunctions::PosActive(std::default_random_engine gen){
  
  //ranges
  std::uniform_real_distribution<> distr_rho(R1, R2); //cm
  double rho_init = distr_rho(gen);
  std::uniform_real_distribution<> distr_phi(0, 2*PI); //rad
  double phi_init= distr_phi(gen);
  std::uniform_real_distribution<> distr_z(0, Lcyl); //cm
  double z_init= distr_z(gen);

  std::vector<double> cylpos_init;
  cylpos_init.push_back(rho_init);
  cylpos_init.push_back(phi_init);
  cylpos_init.push_back(z_init);
  
  return cylpos_init;
}



//Generate radial position in the detector for the following Comptons/Photoelectric effects based on RMS gaussian distributions
//from Geant4 histograms, energy in keV
double ehDriftFunctions::PosCompPhot(std::vector<double> cylpos_init,double energy, std::default_random_engine gen){

  double pos_processx,pos_processy,pos_processz;
  double rad_pos;
  //Convert to cartesian coordinates
  double xinit=cylpos_init.at(0)*cos(cylpos_init.at(1));
  double yinit=cylpos_init.at(0)*sin(cylpos_init.at(1));
  double zinit=cylpos_init.at(2);
  /* std::cout<<"rho init: "<<cylpos_init.at(0)<<std::endl;
  std::cout<<"phi init: "<<cylpos_init.at(1)<<std::endl;
  std::cout<<"z init: "<<cylpos_init.at(2)<<std::endl;

  std::cout<<"x init: "<<xinit<<std::endl;
  std::cout<<"y init: "<<yinit<<std::endl;
  std::cout<<"z init: "<<zinit<<std::endl;
  std::cout<<"Sampling RMS..."<<std::endl;
  std::cout<<""<<std::endl;*/
  //gaussian to generate positions
  if(energy<600){
    std::normal_distribution<double> distribution1(0,0.57);
    pos_processx=xinit+distribution1(gen);
    pos_processy=yinit+distribution1(gen);
    pos_processz=zinit+distribution1(gen);
    /*std::cout<<"New x:"<< pos_processx<<std::endl;
    std::cout<< "New y:"<< pos_processy<<std::endl;
    std::cout<< "New z:"<< pos_processz<<std::endl;*/
  }
  if((energy>=600)&&(energy<=1000)){
    std::normal_distribution<double> distribution2(0,0.83);
    pos_processx=xinit+distribution2(gen);
    pos_processy=yinit+distribution2(gen);
    pos_processz=zinit+distribution2(gen);
    /*std::cout<<"New x:"<< pos_processx<<std::endl;
    std::cout<< "New y:"<< pos_processy<<std::endl;
    std::cout<< "New z:"<< pos_processz<<std::endl;*/
  }
  if(energy>1000){
    std::normal_distribution<double> distribution3(0,0.87);
    pos_processx=xinit+distribution3(gen);
    pos_processy=yinit+distribution3(gen);
    pos_processz=zinit+distribution3(gen);
    /*std::cout<<"New x:"<< pos_processx<<std::endl;
    std::cout<< "New y:"<< pos_processy<<std::endl;
    std::cout<< "New z:"<< pos_processz<<std::endl;*/
  }

  //In reality we don't care about the movement in z direction we just care about the distance between the position and 
  //the electrodes which is in the xy plane
  //Return radial position of the following comptons
  rad_pos=sqrt((pos_processx*pos_processx) + (pos_processy*pos_processy));
  //std::cout<<"Radial position of new compton: "<<rad_pos<<std::endl;

  //check if the position is in the active volume and return radial value
  if((rad_pos>R1)&&(rad_pos<R2)){return rad_pos;}
  else{std::cout<<"POINT GENERATED OUTSIDE THE ACTIVE VOLUME, r0= "<<rad_pos<<" cm, call function again..."<<std::endl;
    return this->PosCompPhot(cylpos_init,energy,gen);}
}




//Return the number of electron-holes pair created by each process (compton or photoelectric effect)
int ehDriftFunctions::Neh(double energy){return energy/Eeh_Ge; //True #Neh need to account for the statistical fluctuations (Fano Factor) to get reco #Neh
}






//Return Q(t)
double ehDriftFunctions:: Q_tNtype(double t, double r0){
  
  //Caculate collection time for holes
  double th=(R2-r0)/vh; 
  //Calculate collection time for electrons
  double te=(r0-R1)/ve;
  //std::cout<<"Collection time for electrons: "<<te<<" us, collection time for holes: "<<th<<std::endl;
  double Qpulse;
  
  //Both e and h drifting
  if((t<th)&&(t<te)){
    Qpulse=(q0/log(R2/R1))*(log(1+(vh*t/r0))-log(1-(ve*t/r0)));
    //std::cout<<"both drifting"<<std::endl;
  }
  //e have been collected but h are still drifting
  else if((t<th)&&(t>=te)){
    Qpulse=(q0/log(R2/R1))*(log(1+(vh*t/r0))-log(R1/r0));
    //std::cout<<"e collected"<<std::endl;
  }
  //h have been collected but e are still drifting
  else if((t<te)&&(t>=th)){
    Qpulse=(q0/log(R2/R1))*(log(R2/r0)-log(1-(ve*t/r0)));
    //std::cout<<"h collected"<<std::endl;
  }
//Both e and h have been collected
 else if ((t>th)&&(t>te)){
   Qpulse=q0;
   //Do the shaping just in this part
   //Qpulse=(-1)*(q0/Capacity)*(1-exp((-1)*(t+1000)/taushape));
   //std::cout<<"both collected"<<std::endl;
 }

  //inverted pulse
  return (-1)*Qpulse;
}



double ehDriftFunctions:: Q_tPtype(double t, double r0){

  //Caculate collection time for holes
  double th=(r0-R1)/vh;
  //Calculate collection time for electrons
  double te=(R2-r0)/ve;
  //std::cout<<"Collection time for electrons: "<<te<<" us, collection time for holes: "<<th<<std::endl;
  double Qpulse;

  //Both e and h drifting
  if((t<th)&&(t<te)){
    Qpulse=(q0/log(R2/R1))*(log(1+(ve*t/r0))-log(1-(vh*t/r0)));
    std::cout<<"both drifting"<<std::endl;
  }
  //e have been collected but h are still drifting
  else if((t<th)&&(t>=te)){
    Qpulse=(q0/log(R2/R1))*(log(R2/r0)-log(1-(vh*t/r0)));
    std::cout<<"e collected"<<std::endl;
  }
  //h have been collected but e are still drifting
  else if((t<te)&&(t>=th)){
    Qpulse=(q0/log(R2/R1))*(log(1+(ve*t/r0))-log(R1/r0));
    std::cout<<"h collected"<<std::endl;
  }
  //Both e and h have been collected
  else if ((t>th)&&(t>te)){
    Qpulse=q0;
    std::cout<<"both collected"<<std::endl;
  }
  //inverted pulse 
  return (-1)*Qpulse;
}






//Return a random value for the number of comptons and photoelectric effects contributing to the 
//peak according to results from geant4. Input: energy of the simulated peak

int ehDriftFunctions::Number_compt_phot(double energy,  std::default_random_engine gen){
  
  std::discrete_distribution<int> distribution[10];
  //wheights (probability of having 0 to 11 comptons of photoelectric effects at different energies)
  //Choose the probability of having 0 comptons to 0, although it is not
  distribution[0]= {0,762,196,37,4,1,0,0,0,0,0,0}; //100keV X-rays
  distribution[1]= {0,381,310,185,88,23,9,3,1,0,0,0}; //200keV X-rays
  distribution[2]= {0,205,244,228,165,96,39,12,2,3,1,0}; //300keV X-rays
  distribution[3]= {0,156,184,244,176,128,54,31,11,2,1,0}; //400keV X-rays
  distribution[4]= {0,124,186,232,210,124,63,29,11,3,1,0}; //500keV X-rays 
  distribution[5]= {0,119,161,211,196,147,68,48,17,7,2,0}; //600keV X-rays
  distribution[6]= {0,112,154,221,203,137,84,39,12,6,3,0}; //700keV X-rays 
  distribution[7]= {0,110,160,171,227,154,74,38,14,11,1,0}; //800keV X-rays
  distribution[8]= {0,124,167,187,178,157,81,49,11,10,4,0}; //900keV X-rays
  distribution[9]= {0,125,153,186,165,146,101,43,25,6,2,3}; //1000keV X-rays


  /*distribution[2]= {5,205,244,228,165,96,39,12,2,3,1,0}; //300keV X-rays
  distribution[3]= {13,156,184,244,176,128,54,31,11,2,1,0}; //400keV X-rays
  distribution[4]= {17,124,186,232,210,124,63,29,11,3,1,0}; //500keV X-rays
  distribution[5]= {24,119,161,211,196,147,68,48,17,7,2,0}; //600keV X-rays
  distribution[6]= {28,112,154,221,203,137,84,39,12,6,3,0}; //700keV X-rays
  distribution[7]= {40,110,160,171,227,154,74,38,14,11,1,0}; //800keV X-rays
  distribution[8]= {32,124,167,187,178,157,81,49,11,10,4,0}; //900keV X-rays
  distribution[9]= {45,125,153,186,165,146,101,43,25,6,2,3}; //1000keV X-rays
  */
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

  int n_compt_phot = distribution[index](gen);
  return n_compt_phot;
}





//Function to distribute the total amplitude (twiceA/2) for each event
//Return a vector with the amplitudes (energies) in ADC counts. The vector returned will be of the size of number of processes (n_compt_phot) in the event

std::vector<double> ehDriftFunctions::Distribute_energies(double energy, int n_compt_phot, std::default_random_engine gen){

  //Distribute amplitudes
  //max value to generate the amplitudes
  double max=energy/2;
  double leftE=energy;
  std::vector<double> energies;

  //Loop to generate energies for each process
  for(int i=0;i< n_compt_phot;i++){
   
    //if last process fill with all the remaining energy
    if(i==(n_compt_phot-1)){energies.push_back(leftE); break;}

    std::uniform_real_distribution<> distr(1, max); // define the range
    double randE = distr(gen);
    energies.push_back(randE);
    
    //update max value to generate the energies
    max=max-(randE/2);
 
    leftE = leftE-randE;
  }


  std::cout<<"Total simulated energy: "<<energy<<" keV"<<std::endl;
  std::cout<<"Energy for each process: ";
  print(energies);
  std::cout<<" keV"<<std::endl;
 
  return energies;
}



//Functions that simulates the preamp shaping of the decay tail 
double ehDriftFunctions:: HighPassFilter(double t, double tshape, double Neh){
  
  double Voltage;
  Voltage=q0*Neh*(1.0/Capacity+(1-1.0/Capacity)*exp((tshape-t)/taushape)); 
  return (-1)*Voltage;

}




//Covert Voltage from preamp (which is proportional to Neh which is equivalent to energy) to ADC Counts
int16_t ehDriftFunctions:: preamptoADC(double Voltage){
  double Energy = Voltage*Eeh_Ge;
  double ADCdoub = Energy/ADC_toE; 
  int16_t ADC= int16_t(ADCdoub); //ADC Counts

  return ADC;
}




//print vectors
void ehDriftFunctions::print(std::vector<double> const &input) {
  for (long unsigned int i = 0; i < input.size(); i++) {
    std::cout << input.at(i) << ' ';
  }
}


#endif
