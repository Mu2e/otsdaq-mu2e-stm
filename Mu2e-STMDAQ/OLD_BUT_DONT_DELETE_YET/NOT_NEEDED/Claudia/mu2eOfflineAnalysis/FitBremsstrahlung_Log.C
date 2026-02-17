#include<iostream>
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
#include "TF1.h"
#include "TPaveStats.h"
#include "TLatex.h"

#include "Math/DistFunc.h"
#include "TFitResult.h"
#include "TMatrixD.h"

//================================================================
//This program reads readvd/ntvd or readvd/ntvext trees: Virtual detectors analyser
//Reads the root files from a txt and analyse the trees
//================================================================  

void Readroot(std::string rootinput, int nbins, double xmin, double xmax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);
  //No print prob
  //gStyle->SetOptFit(01111);
  //Print prob
  gStyle->SetOptFit(11111);

  TCanvas *c1 = new TCanvas("");
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  std::cout<<"Reading code from: "<<rootinput<<std::endl;

  TFile *input=new TFile(rootinput.c_str());
  TTree* treeBrems=(TTree*)input->Get("Bremstree");
  TTree* treeAnnihil=(TTree*)input->Get("Annihiltree");
  TTree* treePhot=(TTree*)input->Get("Phottree");
  TTree* treeCompt=(TTree*)input->Get("Compttree");

  float bremsMom, annihilMom, photMom, comptMom;
  treeBrems->SetBranchAddress("bremsMom",&bremsMom);
  treeAnnihil->SetBranchAddress("annihilMom",&annihilMom);
  treePhot->SetBranchAddress("photMom",&photMom);
  treeCompt->SetBranchAddress("comptMom",&comptMom);

  TH1F*h1 = new TH1F("TH1","", nbins, xmin, xmax);

  double binning = (xmax - xmin) / nbins;

  unsigned long entriesBrems=treeBrems->GetEntries();
  std::cout<<"entriesBrems: "<<entriesBrems<<std::endl;

  if(branch==0){
    for(unsigned long i=0;i<entriesBrems;i++){
      treeBrems->GetEntry(i);
      h1->Fill(bremsMom);
    }
  }


  double logbincontent[nbins], logbinerror[nbins], bincontent[nbins], bincenter[nbins], binerror[nbins];

  double ndof = 0;
  for(int i = 0 ; i < nbins ; i++){
    bincenter[i] = h1->GetBinCenter(i);
    bincontent[i] = h1->GetBinContent(i);
    binerror[i] = h1->GetBinError(i);

    if(bincontent[i]!=0){
      logbincontent[i] = log(bincontent[i]);
      logbinerror[i] = binerror[i]/bincontent[i];
    }
    else{logbincontent[i] = bincontent[i]; logbinerror[i] = binerror[i];}
    std::cout<<"bincenter: "<<bincenter[i]<<" bincontent: "<<bincontent[i]<<" log(bincontent): "<<logbincontent[i]<<std::endl;
    std::cout<<"binerror: "<<binerror[i]<<" log(binerror): "<<logbinerror[i]<<std::endl;
  }

  double Integral_bef = h1->Integral(h1->FindFixBin(xmin), h1->FindFixBin(xmax), "");
  std::cout<<"Integral histo before log entries, between "<<xmin<<" and "<<xmax<<": "<<Integral_bef<<std::endl;
  std::cout<<"Entries before: "<<h1->GetEntries()<<std::endl;

  h1->Reset();

  double check_integral = 0;
  for(int i = 0 ; i < nbins ; i++){
    //content
    h1->SetBinContent(i,logbincontent[i]);
    h1->SetBinError(i,logbinerror[i]);

    double _bincenter = h1->GetBinCenter(i);
    double _bincontent = h1->GetBinContent(i);
    double _binerror = h1->GetBinError(i);

    if(i>=(h1->FindFixBin(xmin))&&(i<=h1->FindFixBin(xmax))){
      std::cout<<"bin: "<<i<<" content: "<<_bincontent<<" center: "<<_bincenter<<std::endl;
      check_integral = check_integral+_bincontent;
    }
    else{std::cout<<"bin out of integral: "<<i<<" content: "<<_bincontent<<" center: "<<_bincenter<<std::endl;}
  }


  double Integral_notnorm = h1->Integral(h1->FindFixBin(xmin), h1->FindFixBin(xmax), "");
  std::cout<<"Integral histo log entries bot norm, between "<<xmin<<" and "<<xmax<<": "<<Integral_notnorm<<" check integral: "<<check_integral<<std::endl;
  std::cout<<"Entries: "<<h1->GetEntries()<<std::endl;

  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);

  if(branch==0){h1->SetFillColor(kOrange-3);h1->SetLineColor(kOrange-3);}
  if(branch==1){h1->SetFillColor(kOrange-2);}
  if(branch==2){h1->SetFillColor(kOrange-1);}
  if(branch==3){h1->SetFillColor(kOrange-4);}
  //h1->Draw("");

  //Define signal region
  double signal_region1 = 0.297;
  double signal_region2 = 0.397;
  
  //DO FIT  
  //fit for log bremsstrahlung
  double rangex1 = 0.004;
  double rangex2 = 1.809;

  //double rangex1 = signal_region1;
  //double rangex2 = signal_region2;
  
  double Integral = h1->Integral(h1->FindFixBin(xmin), h1->FindFixBin(xmax), "");
  std::cout<<"Integral histo between "<<xmin<<" and "<<xmax<<": "<<Integral<<std::endl;

  //NORMALISED FITTING
  TH1F*hnorm1 = (TH1F*)(h1->Clone("TH1"));
  hnorm1->Scale(1./Integral);
  std::cout<<"Normalising to integral: "<<Integral<<std::endl;
  hnorm1->GetYaxis()->SetRangeUser(0., 0.0025);
  hnorm1->Draw("HIST");
 
  //TF1*FitnormBrems = new TF1("FitnormBrems", "log(([0]/(exp([1]*x)+[2]))+[3])", rangex1, rangex2);
  //int npar = 4;
  //FitnormBrems->SetParameters(5.72266e-06, 6.05393e-03, -9.96100e-01, 1.00042e+00); 

  TF1*FitnormBrems = new TF1("FitnormBrems", "[0]+[1]*x+[2]*x*x+[3]*x*x*x+[4]*x*x*x*x+[5]*x*x*x*x*x", rangex1, rangex2);
  int npar = 6;
  FitnormBrems->SetParameters(1, 1, 1, 1, 1, 1);

  TFitResultPtr r = hnorm1->Fit(FitnormBrems,"ESR0","",rangex1,rangex2);
  TMatrixD cov = r->GetCovarianceMatrix();
  cov.Print();

  FitnormBrems->SetLineColor(kRed);
  FitnormBrems->SetLineStyle(2);
  FitnormBrems->Draw("same");


  for(int i = 0 ; i < nbins ; i++){

    double bincenter = hnorm1->GetBinCenter(i);
    double bincontent = hnorm1->GetBinContent(i);
    double binerror = hnorm1->GetBinError(i);

    //std::cout<<sqrt(bincontent)<<" "<<binerror<<std::endl;
    if((bincenter > rangex1)&&(bincenter < rangex2)&&(bincontent!=0)&&(binerror!=0)){
      ndof++;
    }

  }

  ndof = ndof - npar ; //number of fit parameters
  std::cout<<"ndof: "<<ndof<<std::endl;

  double Chi2errors = hnorm1->GetFunction("FitnormBrems")->GetChisquare();
  std::cout<<"Chi2: "<<Chi2errors<<std::endl;

  double prob = TMath::Prob(Chi2errors,ndof);
  std::cout<<"Prob: "<<prob<<std::endl;
  
  double Integralnorm1 = hnorm1->Integral(hnorm1->FindFixBin(0), hnorm1->FindFixBin(2), "");
  std::cout<<"Integral norm histo 0,2: "<<Integralnorm1<<std::endl;
  double Integralnorm2 = hnorm1->Integral(hnorm1->FindFixBin(xmin), hnorm1->FindFixBin(xmax), "");
  std::cout<<"Integral norm histo -0.5,2: "<<Integralnorm2<<std::endl;
  double Integralnorm3 = hnorm1->Integral();
  std::cout<<"Integral norm histo -0.5,2: "<<Integralnorm3<<std::endl;
  double Integralnorm4 = hnorm1->Integral(hnorm1->FindFixBin(0.04), hnorm1->FindFixBin(2), "WIDTH");
  std::cout<<"Integral norm histo 0.04,2 (width): "<<Integralnorm4<<std::endl;
  double Integralnorm5 = hnorm1->Integral(hnorm1->FindFixBin(0.04), hnorm1->FindFixBin(2), "");
  std::cout<<"Integral norm histo 0.04,2 (no width): "<<Integralnorm5<<std::endl;
  double Integralnorm6 = hnorm1->Integral(hnorm1->FindFixBin(signal_region1), hnorm1->FindFixBin(signal_region2), "WIDTH");
  std::cout<<"Integral norm histo signal region "<<signal_region1<<", "<<signal_region2<<" (width): "<<Integralnorm6<<std::endl;
  std::cout<<""<<std::endl;
  std::cout<<""<<std::endl;
  

  double Integralfunc1 = FitnormBrems->Integral(0.04,2);
  std::cout<<"Integral func histo 0.04,2: "<<Integralfunc1<<std::endl;
  double Integralfunc_signal = FitnormBrems->Integral(signal_region1,signal_region2);
  std::cout<<"Integral func signal region "<<signal_region1<<", "<<signal_region2<<": "<<Integralfunc_signal<<std::endl;
  
  double ratio = Integralnorm4/Integralfunc1;
  double ratio_signalregion = Integralnorm6/Integralfunc_signal;
  
  std::cout<<"----Ratio histogram/function (width)---: "<<ratio<<std::endl;
  std::cout<<"----Ratio histogram/function (no width)---: "<<Integralnorm5/Integralfunc1<<std::endl;
  std::cout<<"----Ratio histogram/function in signal region (width)---: "<<ratio_signalregion<<std::endl;
  std::cout<<"Nbins: "<<nbins<<" binning: "<<binning<<std::endl;

  //Calculate the chi2 in signal region
  double chi2_signalregion =0;
  double ndof_signalregion =0;
  double nfit_params = 6;
  double diff = hnorm1->FindFixBin(signal_region2)-hnorm1->FindFixBin(signal_region1);
  double bin1 = hnorm1->FindFixBin(signal_region1); std::cout<<"Bin1: "<<bin1<<" bin center: "<<hnorm1->GetBinCenter(bin1)<<std::endl;
  double bin2 = hnorm1->FindFixBin(signal_region2); std::cout<<"Bin2: "<<bin2<<" bin center: "<<hnorm1->GetBinCenter(bin2)<<std::endl;
  for(int i=bin1; i<=bin2;i++){
    double y = (hnorm1->GetBinContent(i)-FitnormBrems->Eval(hnorm1->GetBinCenter(i)))/hnorm1->GetBinError(i);
    chi2_signalregion = chi2_signalregion + y*y;
    ndof_signalregion++;
    
  }
  
  std::cout<<"Chi2 signal region:"<<chi2_signalregion<<std::endl;
  std::cout<<"ndof signal region:"<<ndof_signalregion<<std::endl;
  



  //DRAW
  std::stringstream streamratio;
  streamratio << std::fixed << setprecision(3) << ratio;
  std::string str_latex = "Histogram/Fit Integral = "+streamratio.str();
  char* char_latex = const_cast<char*>(str_latex.c_str());
  TLatex latex;
  latex.DrawLatexNDC(.35,.85,char_latex);



  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.55); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.32); //new y start position
  ps->SetY2NDC(0.8); //new y end position

  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats 
  h1->SetStats(0);
  //ps->GetLineWith("Entries")->SetTextColor(kBlue);

  /*  
  ps->GetLineWith("Entries")->SetTextSize(0.027);
  ps->GetLineWith("Mean")->SetTextSize(0.027);
  ps->GetLineWith("Std Dev")->SetTextSize(0.027);
  */

  c1->Modified();


  //c1->Print("ComptonSpectrumPhotVD15_new.png");
  //c1->Print("ComptonSpectrumPhotVD15_new.pdf");
}

//================================================================
void FitBremsstrahlung_Log() {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log");
  
  //To print generated root file 
  //branch 0: Photons that come from bremsstrahlung
  Readroot("/work/mu2e/data1/cgarcia/EleBeamGenPOTVDana/VDData/VD15BremsstrahlungPhotMomentum.root", 1500, 0.04, 2, "p_{bremsstrahlung #gamma, VD=15} [MeV]", "Normalised log(bin Content)", 0);

}

//================================================================
