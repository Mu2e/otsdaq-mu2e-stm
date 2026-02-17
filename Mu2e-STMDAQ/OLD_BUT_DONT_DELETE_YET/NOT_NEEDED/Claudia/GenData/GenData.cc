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

#include "GenData.h"

using namespace std;


int16_t pulseCalc(double x, double twiceA){

  //Uncomment to plot Joe's function
  //int16_t pulse = -(twiceA / (1 + exp(-(x - xshift)*invtaufall)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x - xshift)*invtaudecay))));
  //Uncomment to plot Joe's function by parts
  int16_t pulse;
  if(x<xshift){pulse=-(twiceA / (1 + exp(-(x - xshift)*invtaufall)) );}
  if(x>=xshift){pulse=-twiceA* ( 1.0  - ( 1.0/ (1 + exp(-(x - xshift)*invtaudecay))));}
  return pulse;
}



//Rate
//01kHz, 05kHz, ... 200kHz 
//std::string rate_stringkHz =  "200";

void Gendata(int argc, char *argv[],std::string rate_stringkHz,double twiceA, double noiseSD, std::string stringoutputpath){


  //1 seg
  //double timeSample =  1*1e6;//us
  double timeSample =  50000;//us
  // Total number of ADC values                                                                                  
  unsigned long int sampleNum = floor(timeSample/tadc);
  std::cout<<"Number of ADC values in the bin file: "<<sampleNum<<std::endl;
  std::cout<<"Real time of the sample: "<<sampleNum*tadc<<std::endl;


  double ratekHz = std::stold(rate_stringkHz);
  double rateHz = ratekHz*1000;
  std::cout<<"Rate: "<<rateHz<<" Hz"<<std::endl;
  std::cout<<"fADC: "<<fADC<<" MHz"<<std::endl; 
  // Rate in us                   
  double rate = rateHz * 1e-6; // 1kHz in us

  // Average number of pulses in the sample
  int pulseNum = rate*timeSample;
  std::cout<<"Number of pulses generated (real number of simulated pulses at the end of the file): "<<pulseNum<<std::endl;
  std::cout<<"Simulated Pulse Height: "<<twiceA/2<<" ADC Counts = "<<(twiceA/2)*0.57<<" keV"<<std::endl;
  
  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleNum];
  // Randomly generate pulse times for given rate within sample time
  random_device rd;
  //exponential_distribution<double> pulseTime ( rate ) ;
  std::poisson_distribution<int> pulseTime (1/rate) ; //mean of timings distribution in us, gives the time separation between pulses in us following a Poisson distribution
  mt19937 rnd_gen(rd()) ;


  /*//Generate Noise with random numbers, if noiseSD=10, then we generate integer numbers between -9 and 9 as the noise
    std::cout<<"NoiseSD: "<<noiseSD<<std::endl;
    for(unsigned long int i=0; i<sampleNum;i++){
    //Noise
    double rd = (double)rand() / RAND_MAX;
    double noise = -noiseSD + rd*(2*noiseSD); // Random noise
    ADC[i]=noise;
    }*/

  //Generate noise with random numbers following a gaussian, the input parameter is noiseSD=0.17 mV which is the standard deviation of the gaussian in mV
  double sigma_noise_ADC=noiseSD*38.5;
  std::cout<<"NoiseSD: "<<noiseSD<<" mV = "<<sigma_noise_ADC<<" ADC Counts, xshift= "<<xshift<<std::endl;
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0,sigma_noise_ADC);
  //Generate the noise in ADC Counts
  for(unsigned long int i=0; i<sampleNum;i++){ 
    double noise=distribution(generator);
    ADC[i]=noise;
  }



  // Now, calculate and add pulses to ADC array
  // Loop over average number of pulses
  unsigned long int timeIndex = 0;
  int realpulses_binary =0;
  for (int i = 0; i < pulseNum; i++){

    // Randomly generate pulse times in clock ticks        
    timeIndex += int(pulseTime(rnd_gen)/tadc);
    
    //Need to add this
    if((timeIndex+xshift/tadc)>=sampleNum){continue;}

    // Get the index value of the last element in the pulse
    unsigned long int max = timeIndex + pulseLength/tadc;
    // If the pulse exceeds the sample length, max out at the sample length
    if (timeIndex + (pulseLength/tadc) > sampleNum) {max = sampleNum;}
    std::cout<<"Peak "<<i<<" at "<<timeIndex*tadc<<" us"<<std::endl;
    
    // Print the pulse time
    // Loop over the number of ADC values for a pulse, starting from the randomly generated pulse time
    for (unsigned long int j = timeIndex; j < max; j++){
      // Find the time starting from zero
      double time = (j-timeIndex)*tadc;
      // Generate pulse data with noise and add to ADC array                    
      ADC[j] += pulseCalc(time,twiceA);
      //std::cout<<"peak: "<<i<<" time: "<<time<<"us ADC value: "<<ADC[j]<<std::endl;
      // Having into account clipping
      // Baseline of simulated data is always around 0
      if(ADC[j]>1000){
	ADC[j]=-32768;
      }
     
     
    }
    realpulses_binary++;
  }

  std::cout<<"Real number of pulses in binary file: "<<realpulses_binary<<std::endl;
  std::cout<<"Rate: "<<(realpulses_binary/timeSample)*1000<<" kHz"<<std::endl;
  //for(int i=0;i<sampleNum;i++){std::cout<<ADC[i]<<std::endl;}

  //Create the binary file
  //std::string stringfile = stringoutputpath+"/GendataNoise_"+rate_stringkHz+"kHz_"+std::to_string(int(twiceA))+"ADC_0.32noisebyparts.bin";
  std::string stringfile = "hi.bin";
  char file_name[stringfile.size()+1];//as 1 char space for null is also required
  strcpy(file_name, stringfile.c_str());
  
  FILE * fp = fopen(file_name, "wb");
  fwrite(&ADC[0], sizeof(int16_t), sampleNum, fp );
  fclose(fp);
  
}


int main(int argc, char *argv[]){


  //argv[0]=program, argv[1]=rate (kHz), argv[2]=twiceA, argv[3]=noiseSD, argv[4]= outputfilepath
  std::string rate_stringkHz = std::string(argv[1]) ;
  double twiceA = stod(argv[2]);
  double noiseSD = stod(argv[3]);
  std::string stringoutputpath = std::string(argv[4]);

 
  Gendata(argc,argv,rate_stringkHz,twiceA,noiseSD,stringoutputpath);


  return 0;
}
