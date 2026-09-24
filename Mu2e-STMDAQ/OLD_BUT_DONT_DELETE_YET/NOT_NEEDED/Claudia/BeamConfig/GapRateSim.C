#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <sys/stat.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TGraph2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h" 
#include "TStyle.h"
#include "TAxis.h"
#include "TLatex.h"
#include "TProfile2D.h"
#include "TPaletteAxis.h"
#include "TColor.h"

#include "BeamVars.hh"

void GapRateSim(unsigned int seed_option, double spillRatekHz /*kHz*/) {

  gROOT->SetStyle("ATLAS");

  unsigned int seedoption=seed_option;
  unsigned int seed;

  if(seedoption==0){
    //construct a trivial random generator engine from a time-based seed
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::cout<<"Using a time-based (epoch) seed = "<<seed<<std::endl;}
  else{
    unsigned int seed=seedoption; 
    std::cout<<"Seed set manually = "<<seed<<std::endl; }
  
  std::default_random_engine gen(seed);

  double spillRateHz = spillRatekHz*1000;
  double rate = spillRateHz * 1e-6; //MHz
  std::cout<<"Rate: "<<rate<<" MHz"<<std::endl;

  //Number of peaks in first beam mode at this rate
  long int pulseNum = beam_on_size*spillRateHz;
  std::cout<<"Number of peaks in first Beam-on mode ("<<beam_on_size<<" s): "<<pulseNum<<std::endl;
  std::cout<<"THIS IS ASSUMING ALL MUONS ARE STOPPED IN AL"<<std::endl;

  //Probability of DIO, muons captured, 2p1s and 3d2p
  std::discrete_distribution<int> distribution[4];
  distribution[0] = {DIO_muons,captured_muons,ps_muons,dp_muons};
  std::cout<<"Pulses arriving at Al can: DIO with a prob of "<<DIO_muons<<"%, be captured "<<captured_muons<<"%, 3d->2p transition "<<dp_muons<<"% or 2p->1s "<<ps_muons<<"%"<<std::endl;
  //start 3d->2p
  double DIO_3d = tot_DIO_muons*dp_muons/100;
  double capture_3d = tot_captured_muons*dp_muons/100;
  distribution[1] = {DIO_3d,capture_3d};
  std::cout<<" If muons are stopped in 3d orbital all of them decay to 2p and then to 1s, once in 1s, they can: DIO with a prob of "<<DIO_3d<<"% or be captured "<<capture_3d<<"%"<<std::endl;
  //start 2p->1s
  double DIO_2p = tot_DIO_muons*ps_muons/100;
  double capture_2p = tot_captured_muons*ps_muons/100;
  distribution[2] = {DIO_2p,capture_2p};
  std::cout<<" If muons are stopped in 2p, they decay to 1s,once in 1s, they can: DIO with a prob of "<<DIO_2p<<"% or be captured "<<capture_2p<<"%"<<std::endl;
  //Xray Probs
  distribution[3] = {excitedAl_muons,Mgbetadecay_muons,noxrays};
  std::cout<<"Once muons are captured, they can: Emit 1809 keV X-Rays (excited Al): "<<excitedAl_muons<<"%, emit 844 keV X-Rays (beta decay): "<<Mgbetadecay_muons<<"% or nothing: "<<noxrays<<"%"<<std::endl;
  //Generate pulse arrival time to Al following a Poisson distribution
  std::poisson_distribution<int> pulseTime (1.0/rate); //us (mean in us)

  //Decay time distribution for 844keV and 1809keV
  exponential_distribution<double> Xray844keV_time (1.0/betadecay_time_us); // MHz (rate)
  exponential_distribution<double> Xray1809keV_time (1.0/groundstateAl_lifetime_us); //MHz (rate)
  int xray, start_mustop_process;

  double arrivalHPGe_time;
  double xray_exptime;
  double arrivalAl_time = 0;

  std::vector<double> HPGe_times, XRay_energies;

  for (int i = 0; i < pulseNum; i++){

    std::cout<<" "<<std::endl;
    std::cout<<"PULSE NUM: "<<i<<std::endl;
    //Randomly generate pulse times in us
    arrivalAl_time += pulseTime(gen);
    start_mustop_process = distribution[0](gen);
    std::cout<<"Stopped process: "<<start_mustop_process<<", arriving to Al target: "<<arrivalAl_time<<" us"<<std::endl;
    
   
    if(start_mustop_process==0){
      std::cout<<"**********************DIO Process**********************"<<std::endl;
      continue;
    }
    
    if(start_mustop_process==1){
      std::cout<<"**********************Muon captured Process**********************"<<std::endl;
      xray = distribution[3](gen);
      if(xray==0){
	xray_exptime = Xray1809keV_time(gen);
	arrivalHPGe_time=arrivalAl_time+muAl_lifetime_us+xray_exptime;/*excited Al*/
	std::cout<<"1809keV X-Rays: theoretical time excited Al decay "<<groundstateAl_lifetime_us<<" us"<<std::endl;
	std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus excited Al decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	HPGe_times.push_back(arrivalHPGe_time);
	XRay_energies.push_back(1809);
      }
      if(xray==1){
	xray_exptime = Xray844keV_time(gen);
	arrivalHPGe_time=arrivalAl_time+muAl_lifetime_us+xray_exptime;/*beta decay*/
	std::cout<<"844keV X-Rays: theoretical time beta decay "<<betadecay_time_us<<" us"<<std::endl;
	std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus beta decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	HPGe_times.push_back(arrivalHPGe_time);
	XRay_energies.push_back(844);
      }
      else{std::cout<<"No X-Rays produced"<<std::endl; continue;}
    }

    if(start_mustop_process==2){
      std::cout<<"**********************2p to 1s Process**********************"<<std::endl;
      start_mustop_process = distribution[2](gen);
      arrivalHPGe_time=arrivalAl_time+prompt_time_us;
      std::cout<<"347keV X-Rays: "<<prompt_time_us<<" us"<<std::endl;
      std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
      HPGe_times.push_back(arrivalHPGe_time);
      XRay_energies.push_back(347);
      if(start_mustop_process==0){std::cout<<"DIO"<<std::endl; continue; /*DIO*/}
      if(start_mustop_process==1){std::cout<<"Muon captured"<<std::endl; /*Muon captured*/
	xray = distribution[3](gen);
	if(xray==0){
	  xray_exptime = Xray1809keV_time(gen);
	  arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*excited Al*/
	  std::cout<<"1809keV X-Rays: theoretical time excited Al decay "<<groundstateAl_lifetime_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus excited Al decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_times.push_back(arrivalHPGe_time);
	  XRay_energies.push_back(1809);
	}
	if(xray==1){
	  xray_exptime = Xray844keV_time(gen);
	  arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*beta decay*/
	  std::cout<<"844keV X-Rays: theoretical time beta decay "<<betadecay_time_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus beta decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_times.push_back(arrivalHPGe_time);
	  XRay_energies.push_back(844);
	}
	else{std::cout<<"No X-Rays produced"<<std::endl; continue;}
      }
    }

    if(start_mustop_process==3){
      std::cout<<"**********************3d to 2p process**********************"<<std::endl;
      std::cout<<"followed by 2p to 1s muon"<<std::endl;
      arrivalHPGe_time=arrivalAl_time+prompt_time_us;
      std::cout<<"66keV X-Rays: "<<prompt_time_us<<" us"<<std::endl;
      std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
      HPGe_times.push_back(arrivalHPGe_time);
      XRay_energies.push_back(66);
      std::cout<<"347keV X-Rays: "<<prompt_time_us<<" us"<<std::endl;
      std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
      HPGe_times.push_back(arrivalHPGe_time);
      XRay_energies.push_back(347);
      start_mustop_process = distribution[1](gen);
      if(start_mustop_process==0){std::cout<<"DIO"<<std::endl; continue; /*DIO*/}
      if(start_mustop_process==1){std::cout<<"Muon captured"<<std::endl; /*Muon captured*/
	xray = distribution[3](gen);
	if(xray==0){xray_exptime = Xray1809keV_time(gen);
          arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*excited Al*/
	  std::cout<<"1809keV X-Rays: theoretical time excited Al decay "<<groundstateAl_lifetime_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus excited Al decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_times.push_back(arrivalHPGe_time);
	  XRay_energies.push_back(1809);
	}
	if(xray==1){
	  xray_exptime = Xray844keV_time(gen);
	  arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*beta decay*/
	  std::cout<<"844keV X-Rays: theoretical time beta decay "<<betadecay_time_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus beta decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_times.push_back(arrivalHPGe_time);
	  XRay_energies.push_back(844);
	}
	else{std::cout<<"No X-Rays produced"<<std::endl; continue;}
      }
    }

  }//for



  /*****************HPGe time distribution******************/

  //Fill histogram with timings
  //double xmin_t=0;
  //double xmax_t=beam_on_size*1e6+100000; 
  double xmin_t=beam_on_size*1e6+100000;
  double xmax_t=100*60*1e6;
  double ymin_t=0;
  double ymax_t=20;
  double xx1[2]={xmin_t , xmax_t};
  double yy1[2]={ymin_t, ymax_t};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xmin_t, xmax_t);
  graph1->GetYaxis()->SetRangeUser(ymin_t,ymax_t);
  graph1->GetXaxis()->SetTitle("HPGe arrival times [#mus]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  TH1F* HPGetimes = new TH1F("HPGe_times","", 100, xx1[0], xx1[1]);
  for(int i = 0; i < HPGe_times.size(); i++){
    HPGetimes->Fill(HPGe_times.at(i));
  }

  HPGetimes->Draw("same");
  HPGetimes->SetLineColor(kGreen+2);

  //Line indicating Spill mode size in time 
  TLine *line=new TLine(beam_on_size*1e6,yy1[0],beam_on_size*1e6,yy1[1]);
  line->SetLineColor(kBlack);
  line->SetLineStyle(2);
  line->SetLineWidth(2);
  //line->Draw("same");

  /******************AL ENERGY SPECTRUM******************/

  //Fill histogram with energies 
  double xmin_e=0;
  double xmax_e=2000;
  TH1F* HPGeenergies = new TH1F("HPGe_energies","", 100, xmin_e, xmax_e);
  for(int i = 0; i < XRay_energies.size(); i++){
    HPGeenergies->Fill(XRay_energies.at(i));
  }

  //HPGeenergies->Draw(""); 
  HPGeenergies->GetXaxis()->SetTitle("HPGe energies [keV]"); 
  HPGeenergies->SetLineColor(kBlack);
  HPGeenergies->SetFillColor(kAzure+4);

  auto leg = new TLegend(0.1,0.7,0.48,0.9);
  leg->AddEntry(HPGeenergies, "{}^{27}Al","f");
  //leg->Draw("same");

  /******************/
  std::string rate_kHz = "Spill Mode: "+std::to_string(int(spillRatekHz))+" kHz";
  char* ratechar = const_cast<char*>(rate_kHz.c_str());
  TLatex latex;
  latex.SetTextSize(0.04);
  latex.DrawLatex(0,0,ratechar);

}
