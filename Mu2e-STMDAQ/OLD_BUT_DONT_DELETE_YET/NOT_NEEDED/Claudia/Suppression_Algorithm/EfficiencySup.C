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

void EfficiencySup() {
  gROOT->SetStyle("ATLAS");

  double xx1[2]={0,50};
  double yy1[2]={0, 0.5};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,210);
  graph1->GetYaxis()->SetRangeUser(0, 1);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("Fraction of raw data kept");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  const Int_t n=13;
  Double_t fractionkept[n];
  //Number of adc values in original file
  Double_t original_data_file=320052083;
  Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
  //Number of adc values in the files after suppressing
  Double_t datakept[13]={3840000,19184640,38338560,76788480,115115520,153365760,188394240,218127360,223265280,234658560,250106880,259872000,264960000};

  for(int i=0;i<n;i++){
    fractionkept[i]=(datakept[i])/original_data_file;
    }


  TGraph *grdata_kept = new TGraph(n,&rate[0],&fractionkept[0]);
  grdata_kept->SetMarkerColor(kViolet-8);
  grdata_kept->Draw("same,p");


  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(grdata_kept,"Suppressed Data","p");
  legend->Draw("same");

}
