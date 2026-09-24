
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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

class Functions {
 
 public:
  Functions(); 
  int16_t pulseCalc_Joe(double x, double twiceA);
  std::vector<int16_t> data_peak(std::string filename);
  void print(std::vector<double> const &input);
  int Number_compt_phot(double twiceA);
  std::vector<double> Times_compt_phot(int n_compt_phot);
  std::vector<double> Distribute_amplitudes(double twiceA, int n_compt_phot);


  double fADC;
  double twiceA;
  double noiseSD;
  double xshift;
  double lfall;

 private:
  double invtaufall;
  double invtaudecay;

};





//Default constructor
Functions::Functions(){
  //fADC = 320.0520833313;
  fADC = 370;
  //The amplitude of the pulse is the half of this value
  twiceA=2370;
  //twiceA=5370;  
  //The x point in where we have the pulse in us
  xshift=10;
  //fall from baseline time
  invtaufall=15;
  //decaytime(rise to baseline)
  invtaudecay=0.028;
  //Standard deviation of gaussian noise 
  noiseSD = 0.32;
  //Lfall us
  lfall=0.25;//us= 250ns   

}


//Return simulated peaks based on Joe's function

int16_t Functions::pulseCalc_Joe(double x, double twiceA){
 
  //int16_t pulse = -(twiceA / (1 + exp(-(x - xshift)*invtaufall)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x - xshift)*invtaudecay))));

    int16_t pulse;
    if(x<xshift){pulse=-(twiceA / (1 + exp(-(x - xshift)*invtaufall)) );}
    if(x>=xshift){pulse=-twiceA* ( 1.0  - ( 1.0/ (1 + exp(-(x - xshift)*invtaudecay))));}
  

  return pulse;
}


//Return a vector with the simulated peak

std::vector<int16_t> Functions::data_peak(std::string filename){

  std::ifstream readfile;
  readfile.open(filename,ios::in);
  std::vector<int16_t> peak;
  int16_t peakADC;
    
  while( readfile.read( reinterpret_cast<char*>( &peakADC ), sizeof(peakADC) )){
    peak.push_back(peakADC-920);//so that baseline is at 0
    //std::cout<<peakADC<<std::endl;
    }

  readfile.close();
  //vector with the peak from data
  return peak;
}


//print vectors
void Functions::print(std::vector<double> const &input) {
  for (int i = 0; i < input.size(); i++) {
    std::cout << input.at(i) << ' ';
  }
}






//Return a random value for the number of comptons and photoelectric effects contributing to the 
//peak according to results from geant4

int Functions::Number_compt_phot(double twiceA){
  
  //energy of the simulated peak
  double energy = (twiceA/2)*0.57;

  //construct a trivial random generator engine from a time-based seed:
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator2 (seed); //with this generator get a different distribution every time we run it   

  std::discrete_distribution<int> distribution[10];
  //wheights (probability of having 0 to 11 comptons of photoelectric effects at different energies)
  distribution[0]= {0,762,196,37,4,1,0,0,0,0,0,0}; //100keV X-rays
  distribution[1]= {0,381,310,185,88,23,9,3,1,0,0,0}; //200keV X-rays
  distribution[2]= {5,205,244,228,165,96,39,12,2,3,1,0}; //300keV X-rays
  distribution[3]= {13,156,184,244,176,128,54,31,11,2,1,0}; //400keV X-rays
  distribution[4]= {17,124,186,232,210,124,63,29,11,3,1,0}; //500keV X-rays
  distribution[5]= {24,119,161,211,196,147,68,48,17,7,2,0}; //600keV X-rays
  distribution[6]= {28,112,154,221,203,137,84,39,12,6,3,0}; //700keV X-rays
  distribution[7]= {40,110,160,171,227,154,74,38,14,11,1,0}; //800keV X-rays
  distribution[8]= {32,124,167,187,178,157,81,49,11,10,4,0}; //900keV X-rays
  distribution[9]= {45,125,153,186,165,146,101,43,25,6,2,3}; //1000keV X-rays

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

  int n_compt_phot = distribution[index](generator2);
  return n_compt_phot;
}





//Return timings in microseconds for each compton or photoelectric effect taking place (gaussian with mean in Lfall)
std::vector<double> Functions::Times_compt_phot(int n_compt_phot){

  std::vector<double> times; 
  double sigma=0.04;

  //WAY 1: construct a trivial random generator engine from a time-based seed:
  //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  //std::default_random_engine generator2 (seed); //with this generator get a different distribution every time we run it

  //WAY 2: std::default_random_engine generator2;
  //First peak starts at 0us
  
  //WAY 3:
  random_device rd;
  mt19937 rnd_gen(rd()) ;

  times.push_back(0);

  //gaussian to generate timings
  std::normal_distribution<double> distribution(lfall,sigma);
  double time_event;
 

  for(int i=0; i< (n_compt_phot-1);i++){
    time_event=distribution(rnd_gen);
    while(time_event<=0){time_event=distribution(rnd_gen);}
    
    //Fill vector with timings
    times.push_back(time_event);
  }

  //Organise the timings from minimum to maximum
  sort(times.begin(), times.end());

  std::cout<<"Number of compts+phot: "<<n_compt_phot<<" Timings: ";
  print(times);
  std::cout<<" us"<<std::endl;

  return times;

}

//Function to distribute the total amplitude (twiceA/2) for each event
//Return a vector with the amplitudes (energies) in ADC counts. The vector returned will be of the size of number of processes (n_compt_phot) in the event

std::vector<double> Functions::Distribute_amplitudes(double twiceA, int n_compt_phot){
  
  //Amplitude
  double A = twiceA/2; 
  std::vector<double> amplitudes;
  //return vector with twice amplitudes
  std::vector<double> twiceamplitudes;

  //Distribute amplitudes
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  
  //max value to generate the amplitudes
  double max=A/2;
  double leftA=A;

  //Loop to generate amplitudes for each process
  for(int i=0;i< n_compt_phot;i++){
   
    //if last process fill with all the remaining energy
    if(i==(n_compt_phot-1)){amplitudes.push_back(leftA); break;}

    std::uniform_int_distribution<> distr(1, max); // define the range
    double randA = distr(gen);
    amplitudes.push_back(randA);
    
    //update max value to generate the amplitudes
    max=max-(randA/2);
 
    leftA = leftA-randA;
  }


  //twice the amplitudes
  for(unsigned long int j=0;j<amplitudes.size();j++){twiceamplitudes.push_back(2*amplitudes.at(j));}

  std::cout<<"Total simulated ampliude: "<<A<<" ADC Counts"<<std::endl;
  std::cout<<"Amplitudes for each process: ";
  print(amplitudes);
  std::cout<<" ADC counts"<<std::endl;
  std::cout<<"Twice the amplitude for each process: ";
  print(twiceamplitudes);
  std::cout<<" ADC counts"<<std::endl;


  return twiceamplitudes;
}






#endif
