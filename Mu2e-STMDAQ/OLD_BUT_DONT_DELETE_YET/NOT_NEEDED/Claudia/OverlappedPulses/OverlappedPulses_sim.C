#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <bits/stdc++.h> 

#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TRandom3.h"
#include "TStopwatch.h"

//================================================================
void OverlappedPulses_sim(){

  TStopwatch t;
  t.Start();
  
  std::random_device rd;
  std::mt19937 rnd_gen(rd()) ;
  
  double TimeSim = 100000; //s

  std::cout<<"Sim time: "<<TimeSim<<" s"<<std::endl;
  
  //Rate of flash photons
  double RatekHz = 8;
  double RateHz = RatekHz*1000;
  double RateMHz = RateHz * 1e-6; //MHz

  std::cout<<"Rate Flash gammas: "<<RateMHz<<" MHz"<<std::endl;
  std::poisson_distribution<int> pulseTime_flash ((1.0/RateMHz)*1000); //ns (mean in ns)
  
  //Assume these Mu2e parameters
  double prot_per_maininjectorcycle = 4.87e12;
  double maininjectortime = 0.4;
  double STM_accept = 8.03e-10;
  double stoppedmuons_proton = 0.0016;
  double percentage_347keV = 0.8;
  double rate_prot_s = prot_per_maininjectorcycle/maininjectortime;
  double POT = rate_prot_s*TimeSim;
  double rate_stoppedmuons_s = rate_prot_s*stoppedmuons_proton;
  double rate_347keV_s = rate_stoppedmuons_s*percentage_347keV;

  //Rate Hz from X-rays at the STM
  double rate_347keV_s_STM = rate_347keV_s*STM_accept;

  double rate_347keV_STM_MHz = rate_347keV_s_STM/1000000;

  std::cout<<"Rate Xray 347keV: "<<rate_347keV_STM_MHz<<" MHz"<<std::endl;
   
  std::poisson_distribution<int> pulseTime_347 ((1.0/rate_347keV_STM_MHz)*1000); //ns (mean in ns)

  //Number of flash photons in the sample
  unsigned long int pulseNumFlash = RateHz*TimeSim;

  //Number of 347keV XRays
  unsigned long int pulseNumXrays = rate_347keV_s_STM*TimeSim;

  std::cout<<"Number of flash gammas arriving to STM: "<<pulseNumFlash<<" number of 347keV X-rays: "<<pulseNumXrays<<std::endl;
  
  unsigned long int N = pulseNumFlash+pulseNumXrays;

  std::vector<double> VarrivalSTM_time_ns, vectortag;
  double arrivalSTM_time_ns_flash = 0;
  double arrivalSTM_time_ns_347keV = 0;

  double overlapped_pulses = 0;
  
  for (int i = 0; i < pulseNumFlash; i++){
    //Randomly generate pulse times in ns
    double timens = pulseTime_flash(rnd_gen);
    arrivalSTM_time_ns_flash = arrivalSTM_time_ns_flash+timens;
    VarrivalSTM_time_ns.push_back(arrivalSTM_time_ns_flash);
    vectortag.push_back(0);
    //std::cout<<"Pulse Num Flash: "<<i<<" time: "<<setprecision(12)<<arrivalSTM_time_ns_flash<<" ns"<<std::endl;
  }

  for (int i = 0; i < pulseNumXrays; i++){
    //Randomly generate pulse times in ns
    double timens = pulseTime_347(rnd_gen);
    arrivalSTM_time_ns_347keV = arrivalSTM_time_ns_347keV+timens;
    VarrivalSTM_time_ns.push_back(arrivalSTM_time_ns_347keV);
    vectortag.push_back(1);
    //std::cout<<"Pulse Num 347 X-ray: "<<i<<" time: "<<std::setprecision(12)<<arrivalSTM_time_ns_347keV<<" ns"<<std::endl;
  }

 
  //Falling edge time 250ns
  sort(VarrivalSTM_time_ns.begin(), VarrivalSTM_time_ns.end()); 

  /* for(int i = 0; i < VarrivalSTM_time_ns.size(); i++){
    std::cout<<"check: "<<i<<" "<<VarrivalSTM_time_ns.at(i)<<std::endl;
  }
  */
  
  for(int i = 0; i < (VarrivalSTM_time_ns.size()-1); i++){

    double diff_ns = (VarrivalSTM_time_ns.at(i+1)-VarrivalSTM_time_ns.at(i));
    
    if(diff_ns < 250){ //ns
      std::cout<<"Overlapped pulses: "<<std::setprecision(12)<<VarrivalSTM_time_ns.at(i)<<" "<<VarrivalSTM_time_ns.at(i+1)<<std::endl;
      std::cout<<"Tag (0 flash photon and 1 for 347keV peak): "<<vectortag.at(i)<<" "<<vectortag.at(i+1)<<std::endl;
      overlapped_pulses++;
    }//if diff

  }

  
  std::cout<<"Missed pulses: "<<overlapped_pulses<<std::endl;
  t.Stop();
  t.Print();
}



