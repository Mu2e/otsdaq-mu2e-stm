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
#include "TAxis.h"
#include "TH1F.h"

void plotPeaksRaw(string rootfile) {

  gROOT->SetStyle("ATLAS");
  auto c1= new TCanvas("c1");

  double xx1[2]={-1500, -1000};
  double yy1[2]={0,30000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);      
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E[keV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  //graph1->Draw("ap");

  double fitx=0;
  double fity=50;
  double fitmean=717.439;

  TH1F*hraw = new TH1F("hraw","",100 , fitx, fity);
  hraw->GetXaxis()->SetTitle("E[keV]");
  //No supressed data for different rates
  //TFile *runnosup= new TFile("/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/MWDGendataNoise_120kHz_Peaks.root","read"); 
  TFile *runnosup= new TFile(rootfile.c_str(),"read");
  

  TLatex latex;
  latex.SetTextSize(0.06);
  latex.DrawLatex(-1866,26000,"1 kHz");


 
  double peaks;
  
 
  //-----RAW DATA FILE
  TTree* tree1=(TTree*)runnosup->Get("treeADC");
  tree1->SetBranchAddress("peaks",&peaks);
  int entries1=tree1->GetEntries();


  for(Int_t i=0;i<entries1;i++){

    tree1->GetEntry(i);
    hraw->Fill((peaks-0.354947)/(-1.75514)); 
    //cout<<"Raw: "<<peaks<<endl;
    hraw->Fill(peaks);
  }

  hraw->SetTitle("");
  hraw->SetLineColor(kOrange);
  
  cout<<"Entries raw (total number of peaks): "<<entries1<<endl;
 

  double countsRaw = hraw->Integral(hraw->FindFixBin(-1300), hraw->FindFixBin(-1100), "");
  std::cout<<"Number of counts in the raw-data photopeak, between -1300 and -1100: "<<countsRaw<<endl;
 
  TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", fitx, fity);
  //Fit->SetParameters(1,-1195,1);
  Fit->SetParameters(1,fitmean,1);  
  hraw->Fit(Fit,"0","",fitx, fity);
  Fit->SetLineColor(kRed);
  Fit->SetLineStyle(2);
  
  hraw->Draw("");  
  Fit->Draw("same");

  //Return the chi2 to see if it's right
  double Chi2errors;
  Chi2errors=hraw->GetFunction("Fit")->GetChisquare();
  std::cout<<"Chi2= "<<Chi2errors<<std::endl;


 auto legend = new TLegend(0.1,0.7,0.48,0.9);
 legend->AddEntry(hraw,"Raw Data","l");
 //legend->AddEntry(hsup,"Suppressed Data","l");
 legend->Draw("same");

}
