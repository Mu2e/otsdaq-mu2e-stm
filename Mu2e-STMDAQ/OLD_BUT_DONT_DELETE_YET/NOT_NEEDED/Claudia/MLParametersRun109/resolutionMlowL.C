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

void resolutionMlowL() {
  gROOT->SetStyle("ATLAS");
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1");
  c1->SetGrid();


  double xx1[2]={0,6000};
  double yy1[2]={1, 4.1};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,6000);
  graph1->GetYaxis()->SetRangeUser(1, 4.1);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("L");
  graph1->GetYaxis()->SetTitle("#sigma_{TOT} [keV]");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  const Int_t n1=5;
  const Int_t n2=4;
  const Int_t n3=3;
  double M1000L[n3]={100,500,1000};
  double M2000L[n2]={100,500,1000,2000};
  double M5000L[n1]={100,500,1000,2000,5000};
  
  double M1000Le[n3]={0,0,0};
  double M2000Le[n2]={0,0,0,0};
  double M5000Le[n1]={0,0,0,0,0};

  double resolM1000L[n3]={2.002,1.238,4.03};
  double resolM2000L[n2]={2.145,1.447,1.269,2.54};
  double resolM5000L[n1]={2.4,1.844,1.752,1.65,1.768};

  double resolM1000Le[n3]={0.021,0.014,0.05};
  double resolM2000Le[n2]={0.024,0.017,0.015,0.05};
  double resolM5000Le[n1]={0.03,0.022,0.021,0.02,0.022};

  auto g1= new TGraphErrors(n3,M1000L,resolM1000L,M1000Le,resolM1000Le);
  g1->SetMarkerStyle(20);
  g1->SetMarkerColor(kBlue);
  //g1->SetLineColor(kBlue);
  g1->Draw("pl");

  auto g2= new TGraphErrors(n1,M5000L,resolM5000L,M5000Le,resolM5000Le);
  g2->SetMarkerColor(kMagenta);
  //g2->SetLineColor(kMagenta);
  g2->SetMarkerStyle(20);
  g2->Draw("pl");

  auto g3= new TGraphErrors(n2,M2000L,resolM2000L,M2000Le,resolM2000Le);
  g3->SetMarkerColor(kOrange);
  //g3->SetLineColor(kOrange);
  g3->SetMarkerStyle(20);
  g3->Draw("pl");

 


  auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
  leg1->AddEntry(g1, "M=1000","pl");
  leg1->AddEntry(g3, "M=2000","pl");
  leg1->AddEntry(g2, "M=5000","pl");
  leg1->Draw("same");

}
