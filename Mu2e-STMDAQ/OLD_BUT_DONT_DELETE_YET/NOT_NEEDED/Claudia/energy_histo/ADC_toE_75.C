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

void ADC_toE_75() {

  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={0, 2000};
  double yy1[2]={0,45};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,2000);      
  graph1->GetYaxis()->SetRangeUser(0,45);
  graph1->SetTitle("{}^{60}Co");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //Complete
  
  
   TH1F*h1 = new TH1F("TH1","", 3508, 0, 2000);//binning of 0.57 kev
  

   TFile *run0= new TFile("run00075_energypeaks_time.root","read");
   double peaks;

   TTree* tree0=(TTree*)run0->Get("treeADC");

   tree0->SetBranchAddress("peaks",&peaks);

   int entries=tree0->GetEntries();

   cout<<"entries: "<<entries<<endl;
   for(Int_t i=0;i<entries;i++){
     //Cada punto es una entrada del arbol, tiene 10 entradas:
     tree0->GetEntry(i);
     h1->Fill((peaks+0.2704)/(-1.742));
   }
   h1->GetXaxis()->SetTitle("E (keV)");
   h1->SetTitle("");
   h1->Draw("same");

   cout<<"Mean: "<<h1->GetMean()<<endl;
   cout<<"RMS: "<<h1->GetRMS()<<endl;
   cout<<"MeanError: "<<h1->GetMeanError()<<endl;
   cout<<"RMSError: "<<h1->GetRMSError()<<endl;


   //Fit Energy peak Cessium
   TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])", 1100,1300);
   Fit1->SetParameters(3.51407e+01,1200,1.50849e+00);
   h1->Fit(Fit1,"0","",1100,1300);
   Fit1->SetLineColor(kRed);
   Fit1->SetLineStyle(2);
   Fit1->Draw("same");

   TF1*Fit2 = new TF1("Fit2", "[0]*TMath::Gaus(x,[1],[2])", 1300,1400);
   Fit2->SetParameters(3.51407e+01,1300,1.50849e+00);
   h1->Fit(Fit2,"0","",1300,1400);
   Fit2->SetLineColor(kRed);
   Fit2->SetLineStyle(2);
   Fit2->Draw("same");
}
