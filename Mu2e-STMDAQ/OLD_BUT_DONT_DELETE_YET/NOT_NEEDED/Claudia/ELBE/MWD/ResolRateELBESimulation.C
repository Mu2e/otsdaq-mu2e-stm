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

void ResolRateELBESimulation() {
  gROOT->SetStyle("ATLAS");

  double xx1[2]={0,90};
  double yy1[2]={0,6};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("#sigma_{algorithm} [keV]");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

 
  //Simulation
  const Int_t n=4;
  Double_t rate[4]={4,40,60,80};
  Double_t rate_error[4]={0,0,0,0};
  
  
  ////////////////////ELBE simulated data using ELBE Calibration with M=400, L=200
  Double_t resolution_elbeM400L200[4]={3.39,3.50,3.575,3.648};
  //Error from fit
  Double_t resolution_errorelbeM400L200[4]={0.04,0.012,0.010,0.009};

  TGraphErrors *greselbe = new TGraphErrors(n,&rate[0],&resolution_elbeM400L200[0],&rate_error[0],&resolution_errorelbeM400L200[0]);
  greselbe->SetMarkerColor(kRed-9);
  greselbe->SetMarkerStyle(21);
  greselbe->SetMarkerSize(1.1);
  greselbe->Draw("same,p");
  ////////////////////


  TLatex latex;
  latex.SetTextSize(0.05);
  latex.DrawLatex(13,5,"M=400,L=200");
 

  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(greselbe,"#splitline{ELBE Data Calibration}{(M=400,L=200)}","p");
  legend->Draw("same");

 
}
