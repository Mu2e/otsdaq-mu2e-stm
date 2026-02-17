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
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"

#include "TApplication.h"
#include "TRootCanvas.h"


#include "BeamVars.hh"


void RSpillRGap(){

  gROOT->SetStyle("ATLAS");

  TCanvas* c1 = new TCanvas("");
  
  std::vector<double> v_rates, v_gaps;
  
  double parA, parB, parC;
  parA = (tot_captured_muons/100) * (Mgbetadecay_muons/100) * (beam_on_size/gapsize);
  std::cout<<"parA: "<<parA<<std::endl;

  std::cout<<"beam_on_period = "<<beam_on_period<<" main_injectorcycle: "<<main_injectorcycle<<std::endl;
  unsigned long int ngaps = 10000;
  double spillrate_Hz = 10000;
  for(unsigned long int i = 0 ; i < ngaps; i++){

    double t1 = beam_on_period + (i * main_injectorcycle);
    double t2 = (i+1) * main_injectorcycle;
    parB = exp((-1)*betadecay_rate_Hz*t1)-exp((-1)*betadecay_rate_Hz*t2);
    
    parC = (exp(betadecay_rate_Hz*main_injectorcycle*(i+1)) - 1) / (exp(betadecay_rate_Hz*main_injectorcycle) - 1);

    std::cout<<"gap= "<<i<<std::endl;
    std::cout<<"lambda: "<<betadecay_rate_Hz<<"Hz, t1: "<<t1<<" t2: "<<t2<<std::endl;
    std::cout<<"parB: "<<parB<<std::endl;
    std::cout<<"parC: "<<parC<<std::endl;
      
    double rateGapsSpills = parA * parB * parC;

    std::cout<<"gap rate/ spill rate = "<<rateGapsSpills<<std::endl;
    std::cout<<"At spill rate "<<spillrate_Hz<<" Hz, the gap rate =  "<<spillrate_Hz*rateGapsSpills<<" Hz"<<std::endl;
    v_rates.push_back(rateGapsSpills);
    v_gaps.push_back(i);
  }

  double xx1[2]={0,double(ngaps)};
  double yy1[2]={0,0.015};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetYaxis()->SetTitle("R_{GAP}/R_{SPILL}");
  graph1->GetXaxis()->SetTitle("Number of previous beam-ON periods");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  

  TGraph *gr = new TGraph(ngaps,&v_gaps[0],&v_rates[0]);
  gr->SetMarkerColor(kRed);
  gr->SetLineColor(kRed);
  gr->SetMarkerStyle(20);
  gr->SetMarkerSize(1.3);
  gr->SetLineWidth(2);
  gr->Draw("same,l");

  //gr->Print();
  c1->Print("RgapRspill.png");
}

