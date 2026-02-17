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
void PlotAcceptance_E( double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, double branch){

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

  int N = 0;

  double E[6] = { 0.347, 1, 2, 3, 4, 5}; 
  double E_error[6];

  double acceptance_VD10_VD89[6],acceptance_VD10_VD89_error[6]; 
  double acceptance_VD10_VD90[6],acceptance_VD10_VD90_error[6];
  
  double nparticles_sim_costhetarange = 10000000;

  double detphot_VD10_VD89[6]={787, 1847, 2963, 3667, 4122, 4382};
  double detphot_VD10_VD90[6]={7, 122, 431, 725, 1066, 1349};
  
  
  for(int i = 0; i < 6; i++){

    E_error[i] = 0;

    acceptance_VD10_VD89[i]= detphot_VD10_VD89[i] / nparticles_sim_costhetarange;
    acceptance_VD10_VD89_error[i]= sqrt(acceptance_VD10_VD89[i]*(1-acceptance_VD10_VD89[i])/nparticles_sim_costhetarange);
    acceptance_VD10_VD90[i]= detphot_VD10_VD90[i] / nparticles_sim_costhetarange;
    acceptance_VD10_VD90_error[i]= sqrt(acceptance_VD10_VD90[i]*(1-acceptance_VD10_VD90[i])/nparticles_sim_costhetarange);
    
    N++;
  }
  
  if(branch == 0){
  
  TGraphErrors *gracc = new TGraphErrors( N, E, acceptance_VD10_VD89, E_error, acceptance_VD10_VD89_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kRed-9);
  gracc->SetLineColor(kRed-9);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  //TF1*Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  //Fit->SetParameters(0.00222, 237.5, 5.349e4, 5.656e4);

  TF1*Fit = new TF1("Fit", "[0] + [1]*x + [2]*x*x +[3]*x*x*x", xmin, xmax);
  Fit->SetParameters( 0, 0.00003, 0.00003, 0.00003);
  
  TFitResultPtr fitptr = gracc->Fit(Fit,"0S","", xmin, xmax);

  Fit->SetLineColor(kMagenta+3);
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
  }




if(branch == 1){

  TGraphErrors *gracc = new TGraphErrors( N, E, acceptance_VD10_VD90, E_error, acceptance_VD10_VD90_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kRed-9);
  gracc->SetLineColor(kRed-9);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  //TF1*Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  //Fit->SetParameters( 0.00102204, 99.9044, 45768.1, 10042.4 );

  TF1*Fit = new TF1("Fit", "[0] + [1]*x + [2]*x*x +[3]*x*x*x", xmin, xmax);
  Fit->SetParameters( 0, 0.00003, 0.00003, 0.00003);
  
  TFitResultPtr fitptr = gracc->Fit(Fit,"0S","", xmin, xmax);

  Fit->SetLineColor(kMagenta+3);
  Fit->SetLineStyle(2);
  Fit->SetLineWidth(3);
  
  graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetTitleOffset(1.9);
  gracc->Draw("same,p");
  Fit->Draw("same");

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.22);
  ps->SetY2NDC(.5);
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
  
 }

c1->Print("AcceptanceInSmallSolidAngleVD10_90_NewOffline_Grade3poly.png");
}


//================================================================

void Acceptance_NewGeom(){
  
  PlotAcceptance_E( 0, 6, 0, 0.0005, "E_{#gamma} [MeV]", "Acceptance (VD=10)/(VD=89) in cos(#theta)=[0.99999,1]", 0);

  //PlotAcceptance_E( 0, 6, 0, 0.00015, "E_{#gamma} [MeV]", "Acceptance (VD=10)/(VD=90) in cos(#theta)=[0.99999,1]", 1);
}
