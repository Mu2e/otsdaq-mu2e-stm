#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cstdio>
#include<stdio.h>
#include<stdlib.h>


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

//================================================================
//This program reads readvd/ntvd or readvd/ntvext trees: Virtual detectors analyser
//Reads the root files from a txt and analyse the trees
//================================================================  


double gaus1 (double *x, double *par) {
  return par[0]*TMath::Gaus(x[0],par[1],par[2]);
}

double gaus2 (double *x, double *par) {
  return par[0]*TMath::Gaus(x[0],par[1],par[2]);
}

double gausfit (double *x, double *par) {
  return gaus1(x,par)+gaus2(x,&par[3]);
}




void Readroot2Gaus(std::string rootinput, int nbins, double xmin, double xmax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0010);

  TCanvas *c1 = new TCanvas("");
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  std::cout<<"Reading code from: "<<rootinput<<std::endl;

  TFile *input=new TFile(rootinput.c_str());
  TTree* PXtree=(TTree*)input->Get("PXtree");

  float px_photVD15;
  PXtree->SetBranchAddress("px_photVD15",&px_photVD15);


  TH1F*h1 = new TH1F("TH1","", nbins, xmin, xmax);

  unsigned long entries=PXtree->GetEntries();
  std::cout<<"entries: "<<entries<<std::endl;

  if(branch==0){
    for(unsigned long i=0;i<entries;i++){
      PXtree->GetEntry(i);
      h1->Fill(px_photVD15);
    }
  }


  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  h1->SetFillStyle(3001);
  h1->SetFillColor(kGreen+2);
  
  double rangex1=-0.3;
  double rangex2=0.3;
  
  /*TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", rangex1, rangex2); 
  Fit->SetParameters(80000,0,0.02);
  //h1->Fit(Fit,"0","",rangex1,rangex2);
  Fit->SetLineColor(kRed);
  Fit->SetLineStyle(2);  
  TF1*Fitw = new TF1("Fitw", "[0]*TMath::Gaus(x,[1],[2])", rangex1, rangex2);
  Fitw->SetParameters(10000,0,0.07);
  //h1->Fit(Fitw,"0","",rangex1,rangex2);
  Fitw->SetLineColor(kBlue);
  Fitw->SetLineStyle(2);*/

  TF1*Fit2 = new TF1("Fit2", gausfit, rangex1, rangex2, 6);
  Fit2->SetParameters(80000,0,0.02,10000,0,0.07);
  h1->Fit(Fit2,"0","",rangex1,rangex2); 
  Fit2->SetLineColor(kMagenta+2);
  Fit2->SetLineStyle(1);
  Fit2->SetNpx(500);


  TF1*Fitt = new TF1("Fitt", gaus1, rangex1, rangex2, 3);
  Fitt->SetLineColor(kRed);
  Fitt->SetLineStyle(2);
  Fitt->SetLineWidth(3);

  TF1*Fitw = new TF1("Fitw", gaus2, rangex1, rangex2, 3);
  Fitw->SetLineColor(kBlue);
  Fitw->SetLineStyle(2);
  Fitw->SetLineWidth(3);

  double par[6];
  Fit2->GetParameters(par);
  Fitt->SetParameters(par);
  Fitw->SetParameters(&par[3]);

  h1->Draw("");
  Fitt->Draw("same");
  Fitw->Draw("same");
  Fit2->Draw("same");

  c1->Update();
  TPaveStats *ps = (TPaveStats*)h1->FindObject("stats");
  ps->SetX1NDC(0.71); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.55); //new y start position
  ps->SetY2NDC(0.9); //new y end position                                                                                                                          
  ps->SetTextSize(0.032);
  ps->SetName("Fit2stats");

  // the following line is needed to avoid that the automatic redrawing of stats 
  h1->SetStats(0);
  ps->GetLineWith("p0")->SetTextColor(kRed);
  ps->GetLineWith("p1")->SetTextColor(kRed);
  ps->GetLineWith("p2")->SetTextColor(kRed);
  ps->GetLineWith("p3")->SetTextColor(kBlue);
  ps->GetLineWith("p4")->SetTextColor(kBlue);
  ps->GetLineWith("p5")->SetTextColor(kBlue);  

  c1->Modified();

  c1->Print("pxVD15_100Mphotonsdetected347keVFIT.pdf");
  c1->Print("pxVD15_100Mphotonsdetected347keVFIT.png");

  /*double Chi2errors=h1->GetFunction("Fit")->GetChisquare();
  std::cout<<"Chi2: "<<Chi2errors<<std::endl;

  double Integral = h1->Integral(h1->FindFixBin(0), h1->FindFixBin(2), "");
  std::cout<<"Integral: "<<Integral<<std::endl;
 
  TH1F*hnorm1 = (TH1F*)(h1->Clone("TH1"));
  hnorm1->Scale(1./hnorm1->Integral());
  hnorm1->Draw("HIST");
 
  TF1*FitnormBrems = new TF1("FitnormBrems", "[0]*exp([1]*x)+[2]", rangex1, rangex2);
  FitnormBrems->SetParameters(3.04314e+04,-6.84989,5.94630e+02);
  hnorm1->Fit(FitnormBrems,"0","",rangex1,rangex2);
  FitnormBrems->SetLineColor(kRed);
  FitnormBrems->SetLineStyle(2);
  FitnormBrems->Draw("same");
  */
  //input->Close(); doesn't work for plotting histogram
}


void Readroot1Gaus(std::string rootinput, int nbins, double xmin, double xmax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(0);
  //gStyle->SetOptFit(0010);
  gStyle->SetOptFit(0);

  TCanvas *c1 = new TCanvas("");
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  std::cout<<"Reading code from: "<<rootinput<<std::endl;

  TFile *input=new TFile(rootinput.c_str());
  TTree* PXtree=(TTree*)input->Get("PXtree");

  float px_photVD101;
  PXtree->SetBranchAddress("px_photVD101",&px_photVD101);


  TH1F*h1 = new TH1F("TH1","", nbins, xmin, xmax);

  unsigned long entries=PXtree->GetEntries();
  std::cout<<"entries: "<<entries<<std::endl;

  if(branch==0){
    for(unsigned long i=0;i<entries;i++){
      PXtree->GetEntry(i);
      h1->Fill(px_photVD101);
    }
  }


  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  h1->SetFillStyle(3001);
  h1->SetFillColor(kGreen+2);
  
  double rangex1=-0.02;
  double rangex2=0.02;

  TF1*Fit1 = new TF1("Fit1", gaus1, rangex1, rangex2, 3);
  Fit1->SetParameters(5.56785e+04,-3.28024e-06,-1.62298e-03);
  h1->Fit(Fit1,"0","",rangex1,rangex2); 
  Fit1->SetLineColor(kRed);
  Fit1->SetLineStyle(2);
  Fit1->SetLineWidth(3);
  Fit1->SetNpx(500);

  //TF1*F1 = new TF1("F1", gaus1, rangex1, rangex2, 3);
  //F1->SetParameters(4.66785e+04,-3.28024e-06,-2.42298e-03);

  h1->Draw("");
  Fit1->Draw("same");
  //F1->Draw("same");

  c1->Update();
  TPaveStats *ps = (TPaveStats*)h1->FindObject("stats");
  ps->SetX1NDC(0.71); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.76); //new y start position
  ps->SetY2NDC(0.9); //new y end position                                                                                                                          
  ps->SetTextSize(0.032);
  ps->SetName("Fit1stats");
 
  // the following line is needed to avoid that the automatic redrawing of stats 
  h1->SetStats(0);
  ps->GetLineWith("p0")->SetTextColor(kRed);
  ps->GetLineWith("p1")->SetTextColor(kRed);
  ps->GetLineWith("p2")->SetTextColor(kRed);
 
  c1->Modified();

  //c1->Print("pxVD101_100Mphotonsdetected347keVfunction.pdf");
  //c1->Print("pxVD101_100Mphotonsdetected347keVfunction.png");

}



//================================================================
void ReadVD_READFITrootfile(std::string rootfile_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 
  
  //To print generated root file 
  //branch 0: Photons that come from bremsstrahlung
  //Readroot2Gaus(rootfile_location, 100, -0.3, 0.3, "p_{x, #gamma, VD=15} [MeV]", "Counts", 0);
  //Readroot1Gaus(rootfile_location, 100, -0.1, 0.1, "p_{x, #gamma, VD=81} [MeV]", "Counts", 0); 
  Readroot1Gaus(rootfile_location, 100, -0.02, 0.02, "p_{x, #gamma, VD=101} [MeV]", "Counts", 0);
}

//================================================================
