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
#include "TFitResult.h"
#include "TMatrixD.h"

#define mm_to_cm 10
#define PI 3.14159265359

//================================================================
void PlotAcceptance_E(double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest){

  gROOT->SetStyle("ATLAS");

  gStyle->SetOptStat(1110);
  gStyle->SetOptFit(01111);

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  //gPad->SetLogx();

  int N = 14;

  double y_1step[14], y_moresteps[14];
  double y_1step_e[14], y_moresteps_e[14];
  
  double E[14] = {0.05, 0.07, 0.09, 0.2, 0.5, 0.7, 0.9, 1, 1.5, 2, 2.5, 3, 4, 5};
  double E_error[14];

  double ratio_VD10_STM[14], ratio_VD10_STM_error[14];
  
  y_1step[0]=4.00647e-09;
  y_1step_e[0]=3.13473e-10;
  y_1step[1]=6.34899e-09;
  y_1step_e[1]=4.60716e-10;
  y_1step[2]=8.68222e-09;
  y_1step_e[2]=6.06032e-10;
  y_1step[3]=1.97163e-08;
  y_1step_e[3]=1.28882e-09;
  y_1step[4]=4.38294e-08;
  y_1step_e[4]=2.77675e-09; 
  y_1step[5]=5.58581e-08;
  y_1step_e[5]=3.51861e-09;
  y_1step[6]=6.88257e-08;
  y_1step_e[6]=4.3183e-09;
  y_1step[7]=7.30924e-08;
  y_1step_e[7]=4.58142e-09;
  y_1step[8]=9.65735e-08;
  y_1step_e[8]=6.02933e-09;
  y_1step[9]=1.13222e-07;
  y_1step_e[9]=7.05591e-09;
  y_1step[10]=1.29731e-07;
  y_1step_e[10]=8.07387e-09;
  y_1step[11]=1.42494e-07;
  y_1step_e[11]=8.86083e-09;
  y_1step[12]=1.64246e-07;
  y_1step_e[12]=1.0202e-08;
  y_1step[13]=1.74332e-07;
  y_1step_e[13]=1.08239e-08;

  

  y_moresteps[0]=6.3809e-09;
  y_moresteps_e[0]=4.41074e-10;
  y_moresteps[1]=1.18367e-08;
  y_moresteps_e[1]=8.18199e-10;
  y_moresteps[2]=1.59375e-08;
  y_moresteps_e[2]=1.10166e-09;
  y_moresteps[3]=3.1986e-08;
  y_moresteps_e[3]=2.211e-09;
  y_moresteps[4]=6.42258e-08;
  y_moresteps_e[4]=4.43955e-09;
  y_moresteps[5]=8.21265e-08;
  y_moresteps_e[5]=5.67692e-09;
  y_moresteps[6]=9.81133e-08;
  y_moresteps_e[6]=6.78199e-09;
  y_moresteps[7]=1.05512e-07;
  y_moresteps_e[7]=7.29343e-09;
  y_moresteps[8]=1.37833e-07;
  y_moresteps_e[8]=9.52759e-09;
  y_moresteps[9]=1.6428e-07;
  y_moresteps_e[9]=1.13557e-08;
  y_moresteps[10]=1.8655e-07;
  y_moresteps_e[10]=1.28951e-08;
  y_moresteps[11]=2.05701e-07;
  y_moresteps_e[11]=1.42189e-08;
  y_moresteps[12]=2.37258e-07;
  y_moresteps_e[12]=1.64003e-08;
  y_moresteps[13]=2.62487e-07;
  y_moresteps_e[13]=1.81442e-08;

  for(int i=0; i<N ; i++){

    E_error[i] = 0;
    ratio_VD10_STM[i] = y_1step[i] / y_moresteps[i];
    double e_1step = y_1step_e[i] / y_1step[i];
    double e_moresteps = y_moresteps_e[i] / y_moresteps[i];
    ratio_VD10_STM_error[i] =  ratio_VD10_STM[i]*sqrt( e_1step*e_1step + e_moresteps*e_moresteps);
  }

  

  TGraphErrors *gracc = new TGraphErrors( N, E, ratio_VD10_STM, E_error, ratio_VD10_STM_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kOrange+7);
  gracc->SetLineColor(kOrange+7);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  TF1*Fit = new TF1("Fit", "[0]", xmin, xmax);
  Fit->SetParameter(0,0.6);

  TFitResultPtr fitptr = gracc->Fit(Fit,"0S","", xmin, xmax);

  Fit->SetLineColor(kOrange-6);
  Fit->SetLineStyle(2);
  Fit->SetLineWidth(3);
  
  graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetTitleOffset(1.9);
  gracc->Draw("same,p");
  Fit->Draw("same");

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.22);
  ps->SetY2NDC(.55);
  ps->SetX1NDC(0.52);
  ps->SetX2NDC(0.9);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);

  c1->Modified();
  c1->Update();

  graph1->Draw("ap");
  gracc->Draw("same,p");
  Fit->Draw("same");
  
  
  c1->Print("Ratio_Fits.png");
  c1->Print("Ratio_Fits.pdf");
}


//================================================================

void RatioMethods_Fit(){
  
  PlotAcceptance_E(0, 6, 0, 1, "E_{#gamma} [MeV]", "Flash Acceptance Ratio");
}
