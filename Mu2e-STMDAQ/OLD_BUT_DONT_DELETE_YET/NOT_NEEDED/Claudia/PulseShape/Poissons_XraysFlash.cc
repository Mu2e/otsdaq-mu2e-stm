#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <boost/chrono.hpp>

#include "TGraph.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"

#include "TApplication.h"
#include "TRootCanvas.h"

#include "STMDAQ-TestBeam/Claudia/PulseShape/ehDriftFunctions_setseed.h"


void Poisson_XraysFlash(int argc, char *argv[], std::default_random_engine gen){

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  gROOT->SetStyle("ATLAS");
  TCanvas* canvas = new TCanvas("c");


  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;


  double time_simulation_us = 10000;
  double rate_Xrays_Hz = 12;
  double rate_Flash_Hz[21] = {1,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100};

  double toMHz = 1000000;

  double rate_Xrays_MHz = rate_Xrays_Hz / toMHz;
  
  double t_Xrays = 1.0/rate_Xrays_MHz; //us

  //ADC Counts simulated data
  unsigned long int sampleNum = floor(timeSample/tadc);
  std::cout<<"Number of ADC values in the bin file: "<<sampleNum<<std::endl;
  std::cout<<"fADC: "<<fehDriftFunctions.fADC<<" MHz"<<std::endl;
  std::cout<<"Real time of the sample: "<<sampleNum*tadc<<" us"<<std::endl;


  //Rate
  double ratekHz = std::stod(rate_stringkHz);
  double rateHz = ratekHz*1000;
  std::cout<<"Rate: "<<rateHz<<" Hz"<<std::endl;
  //Rate in MHz
  double rate = rateHz * 1e-6;

  //Average number of pulses in the sample
  int pulseNum = rate*timeSample;
  std::cout<<"Number of pulses generated (real number of simulated pulses at the end of the file): "<<pulseNum<<std::endl;
  std::cout<<"Simulated Pulse Energy: "<<Energy<<" keV = "<<Energy/fehDriftFunctions.ADC_toE<<" ADC Counts"<<std::endl;


  t1 = boost::chrono::high_resolution_clock::now();

  
  // Randomly generate pulse times for a given rate within sample time
  std::poisson_distribution<int> pulseTime (1/rate); //mean of timings distribution in us, gives the time separation between pulses in us following a Poisson distribution

    for (int i = 0; i < pulseNum; i++){

        //Randomly generate pulse times in clock ticks
      timeIndex += int(pulseTime(gen)/tadc);
      //Need to add this


    }
  
  //canvas->Print(".png");
#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)canvas->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif

}//end Sim Function




int main(int argc, char *argv[]){

  unsigned int seedoption = std::stod(argv[1]);
  
  unsigned int seed;
  if(seedoption==0){seed = setseed_chrono();}
  else{seed = setseed_man(seedoption);}

  std::default_random_engine gen(seed);

  Poissons_XraysFlash(argc,argv,gen);

  return 0;
}
