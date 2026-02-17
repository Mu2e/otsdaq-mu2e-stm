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



//Rate
//01kHz, 05kHz, ... 200kHz 
//std::string rate_stringkHz =  "200";

void Gendata_PulseShapes(int argc, char *argv[], std::string rate_stringkHz, double noiseSD, std::string stringoutputpath){

  //Read 10 pulses shapes
  fstream readfile;
  readfile.open("/work/cgarcia/DATA/Claudia/PulseShape_Analysis/Shapes/pulses.txt",ios::in);
  std::string name;
  std::vector<string> file_name;
  file_name.clear();
  
  while(1){
  
    if(readfile.eof())break;
    readfile >> name;
    //std::cout << name <<std::endl;
    file_name.push_back(name);
    
  }
  

  std::vector<int16_t> pulse[10]; 

  for(int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<path<<std::endl;

    //Read the files and store the peaks in vectors
    std::ifstream myFilepeak;
    myFilepeak.open(path.c_str(), std::ios::in | std::ios::binary);
    
    int16_t peakADC;
    
    while( myFilepeak.read( reinterpret_cast<char*>( &peakADC ), sizeof(peakADC) )){
      pulse[file].push_back(peakADC-920);//so that baseline is at 0
      //std::cout<<peakADC<<std::endl;
    }

  }//for int file

  //Peaks stored in vectors pulse[0]...,pulse[9]

  //Sample time 1 seg
  double timeSample =  1*1e6;//us
  //double timeSample =  500;//us
  //Total number of ADC values                                                                                  
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
 
  
  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleNum];
  // Intialise data array with noise
  int16_t* track_noise = new int16_t[sampleNum];
  // Randomly generate pulse times for given rate within sample time
  random_device rd;
  //exponential_distribution<double> pulseTime ( rate ) ;
  std::poisson_distribution<int> pulseTime (1/rate) ; //mean of timings distribution in us, gives the time separation between pulses in us following a Poisson distribution
  mt19937 rnd_gen(rd()) ;


 
  //Generate noise with random numbers following a gaussian, the input parameter is noiseSD=mV which is the standard deviation of the gaussian in mV
  double sigma_noise_ADC=noiseSD*38.5;
  std::cout<<"NoiseSD: "<<noiseSD<<" mV = "<<sigma_noise_ADC<<" ADC Counts"<<std::endl;
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0,sigma_noise_ADC);
  




//Generate the noise in ADC Counts
  for(unsigned long int i=0; i<sampleNum;i++){ 
    double noise=distribution(generator);
    ADC[i]=noise;
    track_noise[i]=noise;
  }



  // Now, calculate and add pulses to ADC array
  // Loop over average number of pulses
  unsigned long int timeIndex = 0;
  int realpulses_binary =0;

  //seed
  unsigned long int lasttime_index = 0;
  
  for (int i = 0; i < pulseNum; i++){

    // Randomly generate pulse times in clock ticks        
    timeIndex += int(pulseTime(rnd_gen)/tadc);

    //Generate a random number between 0 and 9 to choose the pulse shape
    int iRand = rand() % 10;    
   
    //Get the size of the pulse in ADC counts
    unsigned long int pulseLength = pulse[iRand].size();

    // Get the index value of the last element in the pulse
    unsigned long int max = timeIndex + pulseLength;

    //Need to add this
    if(timeIndex>=sampleNum){continue;}    
    std::cout<<"Time index: "<<timeIndex<<std::endl;

    // If the pulse exceeds the sample length, max out at the sample length
    if (timeIndex + pulseLength > sampleNum) {max = sampleNum;}
    std::cout<<"Peak "<<i<<" at "<<timeIndex*tadc<<" us, pulse type: "<<iRand<<std::endl;
   


    // Loop over the number of ADC values for a pulse, starting from the randomly generated pulse time
    for (unsigned long int j = timeIndex; j < max; j++){
      
      // Overlaped data (the noise has already been subtracted)
      if(j<lasttime_index){ADC[j] += pulse[iRand].at(j-timeIndex);}
      // Subtract noise (the signal from data already has 0.32 mV noise) and add pulse
      else{ADC[j] += pulse[iRand].at(j-timeIndex)-track_noise[j];}


      // Having into account clipping
      // Baseline of simulated data is always around 0
      if(ADC[j]>1000){
	ADC[j]=-32768;
      }
     
    }

    lasttime_index=max;

    realpulses_binary++;
  }




  std::cout<<"Real number of pulses in binary file: "<<realpulses_binary<<std::endl;
  std::cout<<"Rate: "<<(realpulses_binary/timeSample)*1000<<" kHz"<<std::endl;
  //for(int i=0;i<sampleNum;i++){std::cout<<ADC[i]<<std::endl;}

  //Create the binary file
  //std::string stringfile = stringoutputpath+"/GendataNoise_"+rate_stringkHz+"kHz_"+std::to_string(int(twiceA))+"ADC_0.32noise.bin";
  std::string stringfile = "hi.bin";
  char output_name[stringfile.size()+1];//as 1 char space for null is also required
  strcpy(output_name, stringfile.c_str());
  
  FILE * fp = fopen(output_name, "wb");
  fwrite(&ADC[0], sizeof(int16_t), sampleNum, fp );
  fclose(fp);
 
}


int main(int argc, char *argv[]){


  //argv[0]=program, argv[1]=rate (kHz), argv[2]=noiseSD, argv[3]= outputfilepath
  std::string rate_stringkHz = std::string(argv[1]) ;
  double noiseSD = stod(argv[2]);
  std::string stringoutputpath = std::string(argv[3]);

 
  Gendata_PulseShapes(argc,argv,rate_stringkHz,noiseSD,stringoutputpath);
  

  return 0;
 
}
