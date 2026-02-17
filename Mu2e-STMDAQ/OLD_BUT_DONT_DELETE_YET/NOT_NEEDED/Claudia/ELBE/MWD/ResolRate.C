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

void ResolRate() {
  gROOT->SetStyle("ATLAS");

  TCanvas* c1 = new TCanvas();
  
  double xx1[2]={0,90};
  double yy1[2]={0,6};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("#sigma_{TOT} [keV]");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

 
  //Run 16, 19, 21, 28_00 and 28_01 (both)
  const Int_t n=4;
  Double_t rate[4]={40,60,80,4};
  Double_t rate_error[4]={0,0,0,0};
  
  
  ////////////////////ELBE data using ELBE Calibration with M=400, L=200
  Double_t resolution_elbeM400L200[4]={2.84,4,5.15,2};
  //Not the fit error but the error calculated sigma/sqrt(2N)
  Double_t resolution_errorelbeM400L200[4]={0.2,0.2,0.15,0.08};

  TGraphErrors *greselbe = new TGraphErrors(n,&rate[0],&resolution_elbeM400L200[0],&rate_error[0],&resolution_errorelbeM400L200[0]);
  greselbe->SetMarkerColor(kTeal-5);
  greselbe->SetMarkerStyle(21);
  greselbe->SetMarkerSize(1.2);
  greselbe->Draw("same,p");
  ////////////////////

  ////////////////////ELBE data using Liverpool Calibration with M=8000, L=1000
  Double_t resolution_livM8000L1000[4]={2.85,4,5.4,2.05};
  //Not the fit error but the error calculated sigma/sqrt(2N)
  Double_t resolution_errorlivM8000L1000[4]={0.2,0.19,0.15,0.08};
  TGraphErrors *gresliv = new TGraphErrors(n,&rate[0],&resolution_livM8000L1000[0],&rate_error[0],&resolution_errorlivM8000L1000[0]);
  gresliv->SetMarkerColor(kOrange+1);
  gresliv->SetMarkerSize(1.2);
  //gresliv->Draw("same,p");
  ////////////////////

  TLatex latex;
  latex.SetTextSize(0.05);
  latex.DrawLatex(13,5,"M=400,L=200");
 

  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(gresliv,"#splitline{Liverpool Data Calibration}{(M=8000,L=1000)}","p");
  legend->AddEntry(greselbe,"#splitline{ELBE Data Calibration}{(M=400,L=200)}","p");
  //legend->Draw("same");

  c1->Print("ResolRateELBE_M400L200ELBECalibration.png");
}
