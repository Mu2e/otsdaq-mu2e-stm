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

void ADC_toE_106() {

  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={0, 800};
  double yy1[2]={0,80};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,800);      
  graph1->GetYaxis()->SetRangeUser(0,80);
  graph1->SetTitle("{}^{137}Cs");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //Complete
  
  
   TH1F*h1 = new TH1F("TH1","",1404 , 0, 800);//binning of 0.57 kev
  

 
   // TFile *run0= new TFile("../../../DATA/MWD_Analysis/RUN106/run00106_energypeaks_sup_double.root","read");
    TFile *run0= new TFile("../../../DATA/MWD_Analysis/RUN106/run00106_energypeaks_nosup_float.root","read");   
   //  TFile *run0= new TFile("../../../DATA/MWD_Analysis/RUN106/run00106_energypeaks_nosup_double.root","read"); 
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
   TF1*Fitwide = new TF1("Fitwide", "[0]*TMath::Gaus(x,[1],[2])", 640, 680);
   Fitwide->SetParameters(1.31429e+02,660,2.01827e+00 );
   h1->Fit(Fitwide,"0","",640, 680);


   Fitwide->SetLineColor(kRed);
   Fitwide->SetLineStyle(2);
   Fitwide->Draw("same");
   //Fit Energy peak Cessium
   //TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", 640,670);
   //Fit->SetParameters(3.51407e+01,6.55428e+02,1.50849e+00);
   //h1->Fit(Fit,"0","",640,670);
   //Fit->SetLineColor(kRed);
   //Fit->SetLineStyle(2);
   //Fit->Draw("same");

}
