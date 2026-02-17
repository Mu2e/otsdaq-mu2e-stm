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

void resolutionML() {

  gROOT->SetStyle("ATLAS");
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1");
  c1->SetGrid();


  double xx1[2]={0,21000};
  double yy1[2]={1, 4.1};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,21000);
  graph1->GetYaxis()->SetRangeUser(1, 4.1);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("L");
  graph1->GetYaxis()->SetTitle("#sigma_{TOT} [keV]");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  const Int_t n=5;
  double M8000L[n]={100,500,1000,5000,8000};
  double M20000L[n]={1000,5000,10000,15000,20000};
  double M15000L[n]={500,1000,5000,10000,15000};
  //double M5000L[n]={100,500,1000,2000,5000};
  
  double M8000Le[n]={0,0,0,0,0};
  double M20000Le[n]={0,0,0,0,0};
  double M15000Le[n]={0,0,0,0,0};
  //double M5000Le[n]={0,0,0,0,0};

  double resolM8000L[n]={2.61,2.039,1.923,1.75,1.70};
  double resolM20000L[n]={1.561,1.19,1.24,1.25,1.31};
  double resolM15000L[n]={2.14,1.931,1.79,1.67,1.481};
  //double resolM5000L[n]={2.4,1.844,1.752,1.65,1.768};

  double resolM8000Le[n]={0.03,0.024,0.022,0.02,0.02};
  double resolM20000Le[n]={0.023,0.03,0.04,0.04,0.03};
  double resolM15000Le[n]={0.03,0.023,0.03,0.03,0.023};
  //double resolM5000Le[n]={0.03,0.022,0.021,0.02,0.022};

  auto g1= new TGraphErrors(n,M8000L,resolM8000L,M8000Le,resolM8000Le);
  g1->SetMarkerStyle(20);
  g1->SetMarkerColor(kBlue);
  //g1->SetLineColor(kBlue);
  g1->Draw("pl");

  auto g2= new TGraphErrors(n,M20000L,resolM20000L,M20000Le,resolM20000Le);
  g2->SetMarkerColor(kMagenta);
  //g2->SetLineColor(kMagenta);
  g2->SetMarkerStyle(20);
  g2->Draw("pl");

  auto g3= new TGraphErrors(n,M15000L,resolM15000L,M15000Le,resolM15000Le);
  g3->SetMarkerColor(kOrange);
  //g3->SetLineColor(kOrange);
  g3->SetMarkerStyle(20);
  g3->Draw("pl");

  //auto g4= new TGraphErrors(n,M5000L,resolM5000L,M5000Le,resolM5000Le);
  //g4->SetMarkerColor(kRed);
  //g4->SetLineColor(kRed);
  //g4->SetMarkerStyle(20);
  //g4->Draw("pl");


  auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
  //leg1->AddEntry(g4, "M=5000","pl");
  leg1->AddEntry(g1, "M=8000","pl");
  leg1->AddEntry(g3, "M=15000","pl");
  leg1->AddEntry(g2, "M=20000","pl");
  leg1->Draw("same");

}
