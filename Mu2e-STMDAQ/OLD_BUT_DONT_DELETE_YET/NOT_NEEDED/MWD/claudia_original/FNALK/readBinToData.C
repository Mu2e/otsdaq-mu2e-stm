// Include ROOT headers/libraries                                                                                               
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TLegend.h"
#include "TProfile.h"
// Include C++ headers/libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>



// Include user-defined headers
#include "calcFuncs.h"


using namespace std;                                 

void readBinToData(std::string filename){
  int Draw_option=1;

  calcFuncs fcalcFuncs;
  std::vector<int16_t> DataADC;

  std::vector<unsigned> Channel_Num;
  std::vector<unsigned long int> DTCClock_Num;
  std::vector<unsigned long int> ADCClock_Num;
  std::vector<unsigned long int> Event_Num;

  std::vector<unsigned long int> Startdata_index;

  char* fname = const_cast<char*>(filename.c_str());
  std::cout<<"File read: "<<fname<<std::endl;

  DataADC = fcalcFuncs.readFile(fname);
  //Get  channel, DTCClock, ADCClock_Num, Event number and Startdata index per event vector
  Channel_Num = fcalcFuncs.returnChannel_Num();
  DTCClock_Num = fcalcFuncs.returnDTCClock_Num();
  ADCClock_Num = fcalcFuncs.returnADCClock_Num();
  Event_Num = fcalcFuncs.returnEvent_Num();
  Startdata_index = fcalcFuncs.returnStartdata_index();

  gROOT->SetStyle("ATLAS");

  auto c1= new TCanvas("c1"); 
  
  TGraph* gr_adc = new TGraph();
  unsigned long int limit = 1950000; 
  //unsigned long int limit = DataADC.size();

  int k=0;
  double ADC_digClock = 300;//MHz
  double DTC_clock = 40; //MHz
  double ADCtick_clock = 75; //MHz
  double T0=1.0/ADC_digClock; //us
  double* timevalue = new double[limit];


  /*  for(unsigned long int i = 0; i<Event_Num.size();i++){
    std::cout<<"Channel: "<<Channel_Num.at(i)<<std::endl;
    std::cout<<"DTCClock_Num: "<<DTCClock_Num.at(i)<<std::endl;
    std::cout<<"ADCClock_Num: "<<ADCClock_Num.at(i)<<std::endl;
    std::cout<<"Event num: "<<Event_Num.at(i)<<std::endl;
    std::cout<<" "<<std::endl;
  }

  for(unsigned long int i = 0; i<Startdata_index.size();i++){
    std::cout<<"Startdata_index: "<<Startdata_index.at(i)<<" time: "<<Startdata_index.at(i)*T0<<" us"<<std::endl;
    }*/

  //Print signal and fit the sine wave
  if(Draw_option ==1){

  double xmin = 0;//us
  double xmax = 100;//us
  double ymin = -7000;
  double ymax = 29000;

  double xx1[2]={xmin , xmax};
  double yy1[2]={ymin, ymax};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xmin, xmax);
  graph1->GetYaxis()->SetRangeUser(ymin,ymax);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  //graph1->Draw("ap");
  
  for(unsigned long int i=0;i<limit;i++){
    timevalue[k] = T0*i;
    gr_adc->SetPoint(k,timevalue[k],DataADC.at(i));
    k++;
  }

  gr_adc->SetMarkerColor(kBlack); 
  gr_adc->SetLineColor(kBlack);
  gr_adc->SetMarkerStyle(22);
  //gr_adc->Draw("same,p");


  //If we want to to fit the sine wave signal
  TF1* fitsin = new TF1("sinx", "[0]*sin([1]*x+[2])+[3]",xmin,xmax);
  fitsin->SetParameters(-26304.4, 1.88493, 10.1788, -7.56692);
  fitsin->SetLineColor(kRed);
  fitsin->SetLineStyle(2);
  gr_adc-> Fit(fitsin,"0","",xmin,xmax);
  //fitsin->Draw("same");
  
  TH1F* hdiff = new TH1F("hdiff","", 100, 0, 1000);
  double xminprof = 0;
  double xmaxprof = 1000000;
  int binsprofile = (xmaxprof-xminprof)/500; //10us bin
  TProfile *hprofx = new TProfile("","",binsprofile, xminprof, xmaxprof,"");

  //See the difference between fit function and data for sine wave
  /*  double Amplitude = fitsin->GetParameter(0);
  double frequency = fitsin->GetParameter(1);
  double phase = fitsin->GetParameter(2);
  double ydev = fitsin->GetParameter(3);*/

  double Amplitude = -26304.4;
  double frequency = 1.88493;
  double phase = 10.1788;
  double ydev = -7.56692;

 
  // Amplitude -26304.4 1.88493 10.1788 -7.56692
  for(unsigned long int i =0 ; i<DataADC.size(); i++){
    double t =T0*i;
    //double sinx_fit = (Amplitude*sin(frequency*t+phase) )+ ydev;
    double sinx_fit = fitsin->Eval(t);
    double diff = DataADC.at(i)-sinx_fit;      
    //Take absolute value of diff
    //if(diff<0){diff=(-1)*diff;}
    hprofx->Fill(t,diff);
    //std::cout<<"t: "<<t<<" data: "<<DataADC.at(i)<<" fit: "<<sinx_fit<<" Diff = "<<diff<<std::endl; 

    if(diff>1000){
      hdiff->Fill(t);
      //std::cout<<"t: "<<t<<" data: "<<DataADC.at(i)<<" fit: "<<sinx_fit<<" Diff = "<<diff<<std::endl;
    }
      
    }

  //HISTOGRAM
  //hdiff->GetXaxis()->SetTitle("Time for |Data-Fit|>1,000ADC values [#mus]");
  //hdiff->Draw("");

  //PROFILE
  hprofx->GetYaxis()->SetTitle("Data-Fit [ADC values]"); 
  hprofx->GetXaxis()->SetTitle("Time [#mus]");
  hprofx->SetMarkerStyle(21);
  hprofx->SetMarkerColor(kMagenta-2);
  hprofx->SetMarkerSize(1);
  hprofx->Draw("");
  }





  //Print ADC tick time
  if(Draw_option==2){

    double xmin = 0;   
    double xmax = 300;//events
    double ymin = ADCClock_Num.at(xmin)/ADCtick_clock;
    double ymax = ADCClock_Num.at(xmax)/ADCtick_clock;

    double xx1[2]={xmin , xmax};
    double yy1[2]={ymin, ymax};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xmin, xmax);
    graph1->GetYaxis()->SetRangeUser(ymin,ymax);
    graph1->GetXaxis()->SetTitle("Event Number");
    graph1->GetYaxis()->SetTitle("ADC Time [clock ticks/75 MHz = #mus]");
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");

    for(unsigned long int i=0;i<Event_Num.size();i++){
      unsigned long int ADCtimeus= ADCClock_Num.at(i)/ADCtick_clock;
      gr_adc->SetPoint(k,Event_Num.at(i),ADCtimeus);
      std::cout<<"Event: "<<Event_Num.at(i)<<" ADCClock: "<<ADCtimeus<<std::endl;
      k++;
    }

    gr_adc->SetMarkerColor(kCyan+2);
    gr_adc->SetMarkerStyle(8);
    gr_adc->Draw("same,p");  
  }

  //Print DTC tick time
  if(Draw_option==3){

    double xmin = 0;   
    double xmax = 300;//events
    double ymin = DTCClock_Num.at(xmin)/DTC_clock;
    double ymax = DTCClock_Num.at(xmax)/DTC_clock;

    double xx1[2]={xmin , xmax};
    double yy1[2]={ymin, ymax};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xmin, xmax);
    graph1->GetYaxis()->SetRangeUser(ymin,ymax);
    graph1->GetXaxis()->SetTitle("Event Number");
    graph1->GetYaxis()->SetTitle("DTC Time [clock ticks/40 MHz = #mus]");
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");

    for(unsigned long int i=0;i<Event_Num.size();i++){
      unsigned long int DTCtimeus= DTCClock_Num.at(i)/DTC_clock;
      gr_adc->SetPoint(k,Event_Num.at(i),DTCtimeus);
      std::cout<<"Event: "<<Event_Num.at(i)<<" DTCClock: "<<DTCtimeus<<std::endl;
      k++;
    }

    gr_adc->SetMarkerColor(kOrange);
    gr_adc->SetMarkerStyle(8);
    gr_adc->Draw("same,p");  


  }

  //Print 2 histograms with the difference in DTC ticks and ADC clock times
  if(Draw_option==4){

    TH1F*hADC = new TH1F("hADC","", 100, 90, 110);
    TH1F*hDTC = new TH1F("hDTC","", 100, 23000, 27000);
    hADC->GetXaxis()->SetTitle("#DeltaADCTime [clock ticks/75 MHz = #mus]");
    hDTC->GetXaxis()->SetTitle("#DeltaDTCTime [clock ticks/40 MHz = #mus]");

    for(unsigned long int i=0;i<(Event_Num.size()-1);i++){
      unsigned long int ADCtimeus_prev= ADCClock_Num.at(i)/ADCtick_clock;
      unsigned long int ADCtimeus_post= ADCClock_Num.at(i+1)/ADCtick_clock;
      unsigned long int delta_ADCtimeus= ADCtimeus_post- ADCtimeus_prev;
      hADC->Fill(delta_ADCtimeus);
      std::cout<<"Delta ADCClock: "<<delta_ADCtimeus<<" us"<<std::endl;
    }



    for(unsigned long int i=0;i<(Event_Num.size()-1);i++){
      unsigned long int DTCtimeus_prev= DTCClock_Num.at(i)/DTC_clock;
      unsigned long int DTCtimeus_post= DTCClock_Num.at(i+1)/DTC_clock;
      unsigned long int delta_DTCtimeus=DTCtimeus_post-DTCtimeus_prev;
      hDTC->Fill(delta_DTCtimeus);
      std::cout<<"Delta DTCClock: "<<delta_DTCtimeus<<" us"<<std::endl;
    }

    hADC->SetLineColor(kCyan+2);
    //hADC->Draw("");

    hDTC->SetLineColor(kOrange);
    hDTC->Draw("");


  }


  c1->Print("ProfileSine-ADCdata_500us.png","png");


}
