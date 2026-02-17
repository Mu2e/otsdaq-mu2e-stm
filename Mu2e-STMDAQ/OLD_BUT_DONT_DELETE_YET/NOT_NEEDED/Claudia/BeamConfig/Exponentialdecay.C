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

void Exponentialdecay(){

  gROOT->SetStyle("ATLAS");

  //double spillRatekHz=10;
  //double spillRateHz = spillRatekHz*1000;
  //double rate = spillRateHz * 1e-6; //MHz

  //std::poisson_distribution<int> pulseTime (1.0/rate); //us (mean in us)

  //Decay time distribution for 844keV and 1809keV
  std::exponential_distribution<double> Xray844keV_time (1.0/betadecay_time_us); // MHz (rate)
  std::exponential_distribution<double> Xray1809keV_time (1.0/groundstateAl_lifetime_us); //MHz (rate) 

  int nplot = 100000;

  unsigned int seed=3;
  std::default_random_engine gen(seed);
  double xray_exptime;
  std::vector<double> XRay_energies1809, XRay_energies844;

  for(int k=0; k < nplot; k++){
  
    xray_exptime = Xray1809keV_time(gen);
    XRay_energies1809.push_back(xray_exptime);
    //std::cout<<xray_exptime<<std::endl; 
    xray_exptime = Xray844keV_time(gen);
    //std::cout<<xray_exptime<<std::endl;
    XRay_energies844.push_back(xray_exptime);
  }



  TH1F* expdecay = new TH1F("Exp decay [us]","", 100, 0, 4.98883e-06);

  for(int k=0; k < nplot; k++){
    expdecay->Fill(XRay_energies1809.at(k));
    }
  expdecay->GetXaxis()->SetTitle("HPGe arrival times [#mus]");
  //expdecay->Draw();

  TLatex latex;
  latex.SetTextSize(0.04);
  //latex.DrawLatex(0,0,"1809 keV time distribution");



  auto fb = new TF1("fb"," exp((-1)*([0]*x+[1]))",0,50);

  fb->SetParameters(1,1);
  fb->Draw();



}
