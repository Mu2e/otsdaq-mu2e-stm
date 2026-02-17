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
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TRandom.h"

//================================================================
void PlotConv_Factor(double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest){

  gROOT->SetStyle("ATLAS");

  gStyle->SetOptStat(1110);
  gStyle->SetOptFit(01111);
  gStyle->SetOptFit(0);
 
  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  //graph1->SetTitle("VD10 r cut < 75.0224 mm, p_{z}>0");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetTitleOffset(1.85);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  //gPad->SetLogy();

  int N = 7;

  double EMeV[7] = { 0.05, 0.2, 0.35, 0.45, 0.75, 3, 10}; //Take a mean energy in the interval
  double EMeV_error[7] = { 0,0,0,0,0,0,0 };
  
  double part[7] = { 480178, 258533, 66424, 48253, 272177, 211057, 78004};
  double part_smallangle[8] = { 64, 42, 15, 11, 77, 42, 12};
  
  double conv_factor[7];
  double conv_factor_error[7];

  
  for(int i = 0; i < N; i++){

    conv_factor[i] = part_smallangle[i]/part[i];

    conv_factor_error[i] = sqrt( conv_factor[i]*(1-conv_factor[i])/ part[i]);
    
    //std::cout<<" "<<std::endl;
  }


  graph1->Draw("ap");
  
  TGraphErrors *gracc;

  gracc = new TGraphErrors( N, EMeV, conv_factor, EMeV_error, conv_factor_error );

  gracc->Print("");
  gracc->SetMarkerStyle(21);

  gracc->SetMarkerColor(kAzure-9);


  TF1* Fit = new TF1("Fit", "[0]", xmin, xmax);
  Fit->SetParameter(1,0.00018);

  gracc->Fit(Fit,"0","", xmin, xmax);
  Fit->SetLineColor(kBlue);
  Fit->SetLineStyle(2);
  gracc->Draw("same,p");
  Fit->Draw("same");

  double Chi2errors, p0;
  Chi2errors= gracc->GetFunction("Fit")->GetChisquare();
  p0 = gracc->GetFunction("Fit")->GetParameter(0);

  std::cout<<Chi2errors<<" "<<p0<<std::endl;
  
  std::string str_latex1 = "#splitline{#bf{#chi^{2}/ndf    19.84/6}}{#bf{p0          (1.72 #pm 0.11) #times 10^{-4}}}";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.5,.82,char_latex1);
  
  /*
  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.62);
  ps->SetY2NDC(.9);
  ps->SetX1NDC(0.5);
  ps->SetX2NDC(0.9);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);
  */
  
  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);

  c1->Modified();
  c1->Update();
 
  
  gracc->Draw("same,p"); 
  Fit->Draw("same");
  
  c1->Print("Conversionfactor.png");
  //c1->Print("");
}


//================================================================

void ConversionFactorPhotons(){

  PlotConv_Factor( 0, 15, 0, 0.0004, "E_{#gamma} (VD10 r cut < 7.502 cm, p_{z}>0) [MeV]", "(# #gamma cos(#theta)=[0.99999,1])/(# #gamma) at VD10 MDC2020");
}
