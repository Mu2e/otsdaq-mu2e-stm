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
#include "TProfile.h"
#include "TProfile2D.h"
#include "TPaletteAxis.h"
#include "TColor.h"
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"

#include "TApplication.h"
#include "TRootCanvas.h"


#include "BeamVars.hh"

void AlHPGetimeSim(int argc, char *argv[], std::default_random_engine gen, double spillRatekHz /*kHz*/, int n_maininjector_cycles) {

  #if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv); 
  #endif


  gROOT->SetStyle("ATLAS");
  TCanvas* c = new TCanvas("c");

  /*****************************************/

  int print = 3; //print=0 print HPGe arrival time histogram, print=1 print Gap rate as a function of gap number, print=2 histogram gap rate, print=3 print Al energy spectrum
  bool savepeaks_toROOTfile=false; //run the simulation and store variables in a tree
  bool readpeaks_fromROOTfile=true; //don't run the simulation, read variables from tree

  /*****************************************/
  
  
  double spillRateHz = spillRatekHz*1000;
  double rate = spillRateHz * 1e-6; //MHz
  std::cout<<"Rate: "<<rate<<" MHz"<<std::endl;
  std::cout<<"Number of main injector cycles to simulate: "<<n_maininjector_cycles<<std::endl;

  //Number of peaks in first beam mode at this rate
  long int pulseNum = spillsize*spillRateHz;
  std::cout<<"Number of peaks per micropulse ("<<spillsize<<" s): "<<pulseNum<<std::endl;
  std::cout<<"Beam on and off (8 micropulses, 7 gaps) size in Spill Mode: "<<beam_on_period<<" s"<<std::endl;
  std::cout<<"Number of peaks in first Beam-on mode ("<<beam_on_size<<" s): "<<pulseNum*nspills<<std::endl;
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
  std::exponential_distribution<double> Xray844keV_time (log(2)/betadecay_time_us); // MHz (rate)
  std::exponential_distribution<double> Xray1809keV_time (log(2)/groundstateAl_lifetime_us); //MHz (rate)
  int xray, start_mustop_process;

  double arrivalHPGe_time;
  double xray_exptime;


  double arrivalAl_time, arrivalAl_time_init;
  std::vector<double> HPGe_timesv, XRay_energiesv;
  std::vector<double> gap_ratev, gap_numv;

  double max_time;
  unsigned long int max_gap;


  if(readpeaks_fromROOTfile==false){

  for(int k=0; k < n_maininjector_cycles; k++){
  
  arrivalAl_time_init= k*main_injectorcycle*1e6;
  std::cout<<"--------------------------Main injector Cyle num "<<k<<" starts in "<<arrivalAl_time_init<<" us--------------------------"<<std::endl; 

  //For one Beam On
  for(int j=0; j < nspills; j++){
  std::cout<<" "<<std::endl;std::cout<<" "<<std::endl;
  arrivalAl_time = arrivalAl_time_init+j*(spillsize+spillgaps)*1e6;
  std::cout<<"Start of micropulse "<<j<<": "<<arrivalAl_time<<" us"<<std::endl;
  
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
	HPGe_timesv.push_back(arrivalHPGe_time);
	XRay_energiesv.push_back(1809);
      }
      else if(xray==1){
	xray_exptime = Xray844keV_time(gen);
	arrivalHPGe_time=arrivalAl_time+muAl_lifetime_us+xray_exptime;/*beta decay*/
  	std::cout<<"844keV X-Rays: theoretical time beta decay "<<betadecay_time_us<<" us"<<std::endl;
	std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus beta decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	HPGe_timesv.push_back(arrivalHPGe_time);
	XRay_energiesv.push_back(844);
      }
      else{std::cout<<"No X-Rays produced"<<std::endl; continue;}
    }

    if(start_mustop_process==2){
      std::cout<<"**********************2p to 1s Process**********************"<<std::endl;
      start_mustop_process = distribution[2](gen);
      arrivalHPGe_time=arrivalAl_time+prompt_time_us;
      std::cout<<"347keV X-Rays: "<<prompt_time_us<<" us"<<std::endl;
      std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
      HPGe_timesv.push_back(arrivalHPGe_time);
      XRay_energiesv.push_back(347);
      if(start_mustop_process==0){std::cout<<"DIO"<<std::endl; continue; /*DIO*/}
      if(start_mustop_process==1){std::cout<<"Muon captured"<<std::endl; /*Muon captured*/
  	xray = distribution[3](gen);
	if(xray==0){
	  xray_exptime = Xray1809keV_time(gen);
  	  arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*excited Al*/
  	  std::cout<<"1809keV X-Rays: theoretical time excited Al decay "<<groundstateAl_lifetime_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus excited Al decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_timesv.push_back(arrivalHPGe_time);
	  XRay_energiesv.push_back(1809);
	}
	else if(xray==1){
	  xray_exptime = Xray844keV_time(gen);
	  arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*beta decay*/
  	  std::cout<<"844keV X-Rays: theoretical time beta decay "<<betadecay_time_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus beta decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_timesv.push_back(arrivalHPGe_time);
	  XRay_energiesv.push_back(844);
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
      HPGe_timesv.push_back(arrivalHPGe_time);
      XRay_energiesv.push_back(66);
      std::cout<<"347keV X-Rays: "<<prompt_time_us<<" us"<<std::endl;
      std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
      HPGe_timesv.push_back(arrivalHPGe_time);
      XRay_energiesv.push_back(347);
       start_mustop_process = distribution[1](gen);
        if(start_mustop_process==0){std::cout<<"DIO"<<std::endl; continue; /*DIO*/}
        if(start_mustop_process==1){std::cout<<"Muon captured"<<std::endl; /*Muon captured*/
  	xray = distribution[3](gen);
	if(xray==0){xray_exptime = Xray1809keV_time(gen);
          arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*excited Al*/
  	  std::cout<<"1809keV X-Rays: theoretical time excited Al decay "<<groundstateAl_lifetime_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus excited Al decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_timesv.push_back(arrivalHPGe_time);
	  XRay_energiesv.push_back(1809);
	}
	else if(xray==1){
	  xray_exptime = Xray844keV_time(gen);
	  arrivalHPGe_time=arrivalHPGe_time+muAl_lifetime_us+xray_exptime;/*beta decay*/
  	  std::cout<<"844keV X-Rays: theoretical time beta decay "<<betadecay_time_us<<" us"<<std::endl;
	  std::cout<<"Lifetime in Al: "<<muAl_lifetime_us<<" us plus beta decay exponential sampling time: "<<xray_exptime<<" us"<<std::endl;
	  std::cout<<"Arrival HPGe time: "<<arrivalHPGe_time<<" us"<<std::endl;
	  HPGe_timesv.push_back(arrivalHPGe_time);
	  XRay_energiesv.push_back(844);
	}
	else{std::cout<<"No X-Rays produced"<<std::endl; continue;}
      }
    }

  }//for pulseNum
 }//for nspills
}//for n_maininjector_cycles


  /*****************   GAP RATE   ******************/

  //Calculate integral of each Gap: 
  max_time = *max_element(HPGe_timesv.begin(), HPGe_timesv.end());
  max_gap = ceil(max_time/(main_injectorcycle*1e6));


  double startgap=beam_on_period*1e6;
  double endgap=startgap+gapsize*1e6;
  std::vector<unsigned long int> count_integral;


  for(unsigned long int k = 0; k < max_gap; k++){
  count_integral.push_back(0);
  gap_numv.push_back(k);
  for(unsigned long int i = 0; i < HPGe_timesv.size(); i++){
  if((HPGe_timesv.at(i)>startgap)&&(HPGe_timesv.at(i)<endgap)){count_integral.at(k)=count_integral.at(k)+1;}
}
  std::cout<<"Gap (main injector cycle) number: "<<k<<" starting at: "<<startgap<<" us and finishing: "<<endgap<<std::endl;
  std::cout<<"Number of X-Rays: "<<count_integral.at(k)<<std::endl;
  gap_ratev.push_back(count_integral.at(k)/gapsize);//Hz                                           

  std::cout<<"Gap rate: "<<gap_ratev.at(k)<<" Hz"<<std::endl;
  startgap=endgap+beam_on_period*1e6;
  endgap=startgap+gapsize*1e6;

}

  std::cout<<"Maximum arrival time at HPGe: "<<max_time<<"us"<<std::endl;
  std::cout<<"Maximum arrival time is in Gap: "<<max_gap<<std::endl;


}//if readpeaks_fromROOTfile=false 




  /*****************   ROOT FILE OPTION   ******************/

  std::string rootname;
  if((savepeaks_toROOTfile==true)||(readpeaks_fromROOTfile==true)){

  //rootname="/data1/cgarcia/DATA/Claudia/Al_GapRate/Spill_"+std::to_string(int(spillRatekHz))+"_Alsim_"+std::to_string(n_maininjector_cycles)+"maincycles.root";
  //rootname="hi.root";

  rootname = "1kHz_4000MIcycles.root";
  std::cout<<rootname<<std::endl;

  }




  if(readpeaks_fromROOTfile==true){
  TFile *input;
  TTree* treeread_HPGe;

  std::vector<double> *HPGe_timesp=0; 
  std::vector<double> *XRay_energiesp=0;
  std::vector<double> *gap_ratep=0;
  std::vector<double> *gap_nump=0;

  TBranch *HPGe_times =0;
  TBranch *XRay_energies =0;
  TBranch *gap_rate =0;
  TBranch *gap_num =0;

  input=new TFile(rootname.c_str());

  treeread_HPGe=(TTree*)input->Get("tree_HPGe");
  treeread_HPGe->SetBranchAddress("HPGe_times",&HPGe_timesp);
  treeread_HPGe->SetBranchAddress("XRay_energies",&XRay_energiesp);
  treeread_HPGe->SetBranchAddress("gap_rate",&gap_ratep);
  treeread_HPGe->SetBranchAddress("gap_num",&gap_nump);

  unsigned long int entries = treeread_HPGe->GetEntries();
  std::cout<<"Entries: "<<entries<<std::endl;

  treeread_HPGe->GetEntry(0);
  
  unsigned long int size1 = HPGe_timesp->size();
  std::cout<<"size branch 1: "<<size1<<std::endl;
  
  unsigned long int size2 = gap_ratep->size();
  std::cout<<"size branch 2: "<<size2<<std::endl;

  for(unsigned long int i = 0; i<size1; i++){
  HPGe_timesv.push_back(HPGe_timesp->at(i));
  XRay_energiesv.push_back(XRay_energiesp->at(i));
  //std::cout<<"HPGe arrival time: "<<HPGe_timesv.at(i)<<" XRay energy: "<<XRay_energiesv.at(i)<<std::endl;
}

  std::cout<<"SIMULATION RESULT-----------------------"<<std::endl;
  for(unsigned long int i = 0; i<size2; i++){
  gap_ratev.push_back(gap_ratep->at(i));
  gap_numv.push_back(gap_nump->at(i));
  std::cout<<"Gap rate: "<<gap_ratev.at(i)<<" Gap num: "<<gap_numv.at(i)<<std::endl;
}

  //treeread_HPGe->Draw("HPGe_times");



  /*****************   GAP RATE   ******************/

  max_time = *max_element(HPGe_timesv.begin(), HPGe_timesv.end());
  std::cout<<"Maximum arrival time at HPGe: "<<max_time<<"us"<<std::endl;
  max_gap = ceil(max_time/(main_injectorcycle*1e6));
  std::cout<<"Maximum arrival time is in Gap: "<<max_gap<<std::endl;

} //readpeaks_fromROOTfile


  std::cout<<"HPGe times size: "<<HPGe_timesv.size()<<", Gap rate size: "<<gap_ratev.size()<<", Number of gaps: "<<gap_numv.size()<<", Number of Al XRays: "<<XRay_energiesv.size()<<std::endl;


  
  if(savepeaks_toROOTfile==true){
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");

  TTree*tree_HPGe=new TTree("tree_HPGe","tree_HPGe");
  tree_HPGe->Branch("HPGe_times",&HPGe_timesv);
  tree_HPGe->Branch("XRay_energies",&XRay_energiesv);
  tree_HPGe->Branch("gap_rate",&gap_ratev);
  tree_HPGe->Branch("gap_num",&gap_numv);
  tree_HPGe->Fill();  

  rootfile->Write();
  rootfile->Close();
  
}
 




 


  /*****************   PRINT OPTION   ******************/
  /*****************HPGe time distribution******************/
  if(print==0){
  //Fill histogram with timings
  double xmin_t=0;
  double xmax_t= main_injectorcycle + beam_on_period;
  double ymin_t=0;
  double ymax_t=0.04;
  double xx1[2]={xmin_t , xmax_t};
  double yy1[2]={ymin_t , ymax_t};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xmin_t, xmax_t);
  graph1->GetYaxis()->SetRangeUser(ymin_t,ymax_t);
  graph1->GetXaxis()->SetTitle("Al-Xrays emission time [s]");
  graph1->GetYaxis()->SetTitle("Normalised Al-Xrays / (0.02s)");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  TH1F* HPGetimes = new TH1F("HPGe_times","", 100, xx1[0], xx1[1]);
  for(long unsigned int i = 0; i < HPGe_timesv.size(); i++){
    HPGetimes->Fill(HPGe_timesv.at(i)/1000000);
  }

  HPGetimes->SetLineColor(kGreen+2);

  HPGetimes->Scale(1./HPGetimes->Integral());
  HPGetimes->Draw("same, HIST");

  double bin = xmax_t/100;
  std::cout<<"Binning = "<<bin<<std::endl;
  
  //Line indicating Spill mode size in time 
  //TLine *line=new TLine(beam_on_period*1e6,yy1[0],beam_on_period*1e6,yy1[1]);
  //line->SetLineColor(kBlack);
  TLine *line=new TLine(main_injectorcycle,yy1[0],main_injectorcycle,yy1[1]);
  line->SetLineColor(kRed);
  line->SetLineStyle(2);
  line->SetLineWidth(6);
  line->Draw("same");

  std::string MIcycle = "#color[2]{#scale[1.5]{MI cycle}}";
  char* MIcyclechar = const_cast<char*>(MIcycle.c_str());
  TLatex latex1;
  latex1.SetTextSize(0.04);
  latex1.DrawLatex(0.2,0.036,MIcyclechar);
}


  TH1F* Integral_histotimes = new TH1F("Integral_histotimes","", 100, 0, max_time);
  for(unsigned long int i = 0; i < HPGe_timesv.size(); i++){
    Integral_histotimes->Fill(HPGe_timesv.at(i));
  }

  
  //double integral = Integral_histotimes->Integral(Integral_histotimes->FindFixBin(0.41096*1e6), Integral_histotimes->FindFixBin(1.4*1e6), "");
  //double integral = Integral_histotimes->Integral(0.41096*1e6, 1.4*1e6, "");
  //std::cout<<"Integral of first gap binning: "<<integral<<std::endl;
  
  /******************GAP RATE GRAPH******************/
  if(print==1){
  TGraph* gr_gaprate = new TGraph(max_gap,&gap_numv[0],&gap_ratev[0]);
  gr_gaprate->GetXaxis()->SetTitle("Gap number");
  gr_gaprate->GetYaxis()->SetTitle("Gap Rate [Hz]");
  gr_gaprate->SetMarkerColor(kBlack); 
  gr_gaprate->SetLineColor(kBlack);
  gr_gaprate->SetMarkerStyle(6);
  gr_gaprate->Draw("ap");
}


  /******************GAP RATE HISTO******************/
  if(print==2){
  //Calculate max gap rate
  double max_gaprate = *max_element(gap_ratev.begin(), gap_ratev.end());
  std::cout<<"Maximum Gap rate for this run: "<<max_gaprate<<std::endl;

  TH1F* GapRates = new TH1F("GapRates","", 100, 0, max_gaprate);
  for(long unsigned int i = 0; i < gap_ratev.size(); i++){
  GapRates->Fill(gap_ratev.at(i));
}

  GapRates->GetXaxis()->SetTitle("Gap Rate [Hz]");
  GapRates->Draw();
  GapRates->SetLineColor(kViolet+2);

  }
  /******************AL ENERGY SPECTRUM******************/
  if(print==3){
  //Fill histogram with energies 
  double xmin_e=0;
  double xmax_e=2000;
  TH1F* HPGeenergies = new TH1F("HPGe_energies","", 100, xmin_e, xmax_e);
  for(long unsigned int i = 0; i < XRay_energiesv.size(); i++){
    HPGeenergies->Fill(XRay_energiesv.at(i));
  }

  //HPGeenergies->Draw(""); 
  HPGeenergies->GetXaxis()->SetTitle("E [keV]"); 
  HPGeenergies->SetLineColor(kBlack);
  HPGeenergies->SetFillColor(kAzure+4);

  HPGeenergies->Scale(1./(spillRateHz*beam_on_size*n_maininjector_cycles));
  HPGeenergies->GetYaxis()->SetTitle("Al-Xrays / (T_{SPILL} #upoint #MI cycles #upoint R_{SPILL})");
  HPGeenergies->Draw("same,HIST");
  
  auto leg = new TLegend(0.6,0.75,0.8,0.85);
  leg->AddEntry(HPGeenergies, "{}^{27}Al","f");
  leg->Draw("same");
}


  /******************GAP RATE - GAP NUMBER TPROFILE******************/

  if(print==4){

    double xmin = 0;//us
    double xmax = 10000;//us
    double ymin = 0;
    double ymax = 0.02;
    double xx1[2]={xmin , xmax};
    double yy1[2]={ymin, ymax};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xmin, xmax);
    graph1->GetYaxis()->SetRangeUser(ymin,ymax);
    graph1->GetXaxis()->SetTitle("Gap number");
    graph1->GetYaxis()->SetTitle("R_{GAP}/R_{SPILL}");
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");
    
    TProfile* prof_gaprate = new TProfile("","",200,0,10000,"");
    for(long unsigned int i = 0; i < gap_ratev.size(); i++){
       prof_gaprate->Fill(gap_numv.at(i),gap_ratev.at(i)/spillRateHz);
     }
    prof_gaprate->GetXaxis()->SetTitle("Gap number");
    prof_gaprate->GetYaxis()->SetTitle("R_{GAP}/R_{SPILL}");
    prof_gaprate->SetMarkerColor(kBlack);
    prof_gaprate->SetMarkerStyle(21);
    prof_gaprate->Draw("p, same");

    std::cout<<"CALCULATION RESULT-----------------------"<<std::endl;
    double startgap, endgap;
    double decayrate_844_MHz = log(2)/betadecay_time_us;
    double decayrate_844_Hz = decayrate_844_MHz*1000000;
    std::cout<<"Decay rate: "<<decayrate_844_Hz<<std::endl;
    startgap = beam_on_period;
    endgap = main_injectorcycle;
    double gaprate;
    double N0;

    //N0 (generated 844keV peaks from total rate)
    N0=9.2*(60*(spillRateHz*beam_on_size)/100)/100;
    std::cout<<"N0: "<<N0<<std::endl;
    TGraph* gr_gaprate = new TGraph();
    //Calculate theoretical curve
    for(int i=0; i < n_maininjector_cycles; i++){
    //for(int i=0; i < 15000; i++){   
    std::cout<<""<<std::endl;
      std::cout<<"Gap: "<<i<<std::endl;
      std::cout<<"start Gap: "<<startgap<<" s"<<std::endl;
      std::cout<<"end Gap: "<<endgap<<" s"<<std::endl;
      double exp1=(-1)*decayrate_844_Hz*startgap;
      std::cout<<"exp1: "<<exp1<<std::endl;
      double exp2=(-1)*decayrate_844_Hz*endgap;
      std::cout<<"exp2: "<<exp2<<std::endl;
      double exp3=(decayrate_844_Hz*main_injectorcycle)*(i+1);
      std::cout<<"exp3: "<<exp3<<std::endl;
      double exp4=decayrate_844_Hz*main_injectorcycle;
      std::cout<<"exp4: "<<exp4<<std::endl;

      gaprate = N0*(exp(exp1)-exp(exp2))*((exp(exp3)-1)/(exp(exp4)-1))/gapsize;
      std::cout<<"Gap rate: "<<gaprate<<" Hz"<<std::endl;
      std::cout<<"Gap size: "<<gapsize<<" s"<<std::endl;      
      gr_gaprate->SetPoint(i, i, gaprate);
      startgap=startgap+main_injectorcycle;
      endgap=endgap+main_injectorcycle;
}
    gr_gaprate->SetLineColor(kRed);
    gr_gaprate->Draw("l, same");

    std::string MIcycle = "#scale[1.5]{4,000 MI cycles}";
    char* MIcyclechar = const_cast<char*>(MIcycle.c_str());
    TLatex latex1;
    latex1.SetTextSize(0.04);
    latex1.DrawLatex(1000,0.017,MIcyclechar);
    
  }


  /******************/
  std::string rate_kHz = "Spill Mode: "+std::to_string(int(spillRatekHz))+" kHz";
  char* ratechar = const_cast<char*>(rate_kHz.c_str());
  TLatex latex;
  latex.SetTextSize(0.04);
  //latex.DrawLatex(0,0,ratechar);
  

  c->Modified();
  c->Update();

  c->Print("AlXrays_4000maininjectorcycles_sim.png");

#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif

}



int main(int argc, char *argv[]){

  //argv[0]=program, argv[1]=seedoption, argv[2]=spillRatekHz, argv[2]=num of main injector cycles to simlate
  unsigned int seedoption =atoi(argv[1]);
  double spillRatekHz = atoi(argv[2]);
  int n_maininjector_cycles= atoi(argv[3]);

  unsigned int seed;
  if(seedoption==0){
    //construct a trivial random generator engine from a time-based seed 
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::cout<<"Using a time-based (epoch) seed = "<<seed<<std::endl;}
  else{
    unsigned int seed=seedoption;
    std::cout<<"Seed set manually = "<<seed<<std::endl; }

  std::default_random_engine gen(seed);




  AlHPGetimeSim(argc,argv,gen,spillRatekHz /*kHz*/,n_maininjector_cycles);


  return 0;
}
