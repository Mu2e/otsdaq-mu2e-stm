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

void plotPeaksRawSup() {

  gROOT->SetStyle("ATLAS");
  auto c1= new TCanvas("c1");



  double xx1[2]={-2000, 0};
  double yy1[2]={0,20000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(-2000, 0);      
  graph1->GetYaxis()->SetRangeUser(0,20000);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  //Complete
  
  
  TH1F*hsup = new TH1F("hsup","",100 , -2000, 0);
  TH1F*hraw = new TH1F("hraw","",100 , -2000, 0);

  //TH1F*hsup = new TH1F("hsup","",100 , -1500, -1000);
  //TH1F*hraw = new TH1F("hraw","",100 , -1500, -1000);  
 
  //Gradient suppression file run109_00 M=8000 L=1000
  //TFile *runsup= new TFile("energypeakssuppressed_gradient.root","read");   
  //My old supression code file run109_00
  //TFile *runsup= new TFile("energypeakssuppressed_oldcode.root","read"); 
  //Dan's suppression code run109_00
  //TFile *runsup= new TFile("../Suppression_AlgorithmDan/energypeakssuppressed_Dan.root","read");
  

  //Gradient suppression file for all the run 109
  //TFile *runsup= new TFile("/work/cgarcia/DATA/Claudia/Suppression_Algorithm/tests/energypeakssuppressed_all109run_gradient.root","read");
  //No supressed data
  //TFile *runnosup= new TFile("/work/cgarcia/DATA/Claudia/Suppression_Algorithm/tests/run00109.new_energypeaks_bin_00.root","read"); 
 

  //Gradient suppression file for all the Simulated data
  //TFile *runsup= new TFile("/work/cgarcia/DATA/Claudia/GenData/Generateddata_peaks_Suppressed.root","read");
  //No supressed data
  //TFile *runnosup= new TFile("/work/cgarcia/DATA/Claudia/GenData/Generateddata_peaks.root","read");  

  //Gradient suppression file for different rates 5us PRE sup
  //TFile *runsup= new TFile("/work/cgarcia/DATA/Claudia/GenData/Rates/Supdata_50kHz_Peaks.root","read");
  //No supressed data for different rates
  //TFile *runnosup= new TFile("/work/cgarcia/DATA/Claudia/GenData/Rates/Gendata_50kHz_Peaks.root","read");  

  //Gradient suppression file for different rates 2us PRE sup noise
  TFile *runsup= new TFile("/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/MWDSupdataNoise_50kHz_Peaks.root","read");
  //No supressed data for different rates
  TFile *runnosup= new TFile("/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/MWDGendataNoise_50kHz_Peaks.root","read"); 

  

  TLatex latex;
  latex.SetTextSize(0.06);
  latex.DrawLatex(-1866,16000,"50 kHz");


  //-----SUPPRESSED DATA FILE 

  double peaks;
  
  TTree* tree0=(TTree*)runsup->Get("treeADC");

  tree0->SetBranchAddress("peaks",&peaks);

  int entries0=tree0->GetEntries(); 

 
  for(Int_t i=0;i<entries0;i++){
    
    tree0->GetEntry(i);
    //cout<<"Sup: "<<peaks<<endl;
    //hsup->Fill((peaks-0.3549)/(-1.755));
    hsup->Fill(peaks);
  }
  hsup->GetXaxis()->SetTitle("ADC Counts");
  hsup->SetTitle("");
  hsup->SetLineColor(kBlue);
  

  //-----RAW DATA FILE
 

  TTree* tree1=(TTree*)runnosup->Get("treeADC");

  tree1->SetBranchAddress("peaks",&peaks);

  int entries1=tree1->GetEntries();


  for(Int_t i=0;i<entries1;i++){

    tree1->GetEntry(i);
    //hraw->Fill((peaks-0.3549)/(-1.755)); 
    //cout<<"Raw: "<<peaks<<endl;
    hraw->Fill(peaks);
  }

  hraw->GetXaxis()->SetTitle("ADC Counts");
  hraw->SetTitle("");
  hraw->SetLineColor(kGreen);
  
  cout<<"entries raw: "<<entries1<<endl;
  cout<<"entries sup: "<<entries0<<endl;

  double countsRaw = hraw->Integral(hraw->FindFixBin(-1300), hraw->FindFixBin(-1100), "");
  std::cout<<"Number of counts in the raw-data photopeak: "<<countsRaw<<endl;
  double countsSup = hsup->Integral(hsup->FindFixBin(-1300), hsup->FindFixBin(-1100), "");
  std::cout<<"Number of counts in the sup-data photopeak: "<<countsSup<<endl;

  hraw->Draw("same");  
  hsup->Draw("same");



 auto legend = new TLegend(0.1,0.7,0.48,0.9);
 legend->AddEntry(hraw,"Raw Data","l");
 legend->AddEntry(hsup,"Suppressed Data","l");
 legend->Draw("same");

}
