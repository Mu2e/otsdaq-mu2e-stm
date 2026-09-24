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

void Spectrum() {

  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={-1500, -1000};
  double yy1[2]={0,45};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(-1500, -1000);
  graph1->GetYaxis()->SetRangeUser(0,45);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //Complete
  //TH1F*h1 = new TH1F("TH1","", 1000, -3000, 0);
  TH1F*h1 = new TH1F("TH1","", 1000, -1500, -1000);
  //Read the first .root
  TFile *run0= new TFile("run_00109_energypeaks0.root","read");
  double peaks;

  TTree* tree0=(TTree*)run0->Get("treeADC");

  tree0->SetBranchAddress("peaks",&peaks);

  int entries=tree0->GetEntries();

  cout<<"entries: "<<entries<<endl;
  for(Int_t i=0;i<entries;i++){
    //Cada punto es una entrada del arbol, tiene 10 entradas:
    tree0->GetEntry(i);
    h1->Fill(peaks);
  }

  h1->GetXaxis()->SetTitle("ADC Counts");
  h1->SetTitle("");
  h1->SetStats(1);
  h1->Draw("same");

  /*  //Fit Energy peak Cessium
    TF1*Fitwide = new TF1("Fitwide", "[0]*TMath::Gaus(x,[1],[2])", -1500, -1000);
    //Fitwide->SetParameters(7.78161,-1.18138e+03,-2.94654e+01);
    Fitwide->SetParameters(2.73250e+01,-1.18276e+03,-2.80617e+01);

    TF1*Fitthin = new TF1("Fitthin", "[0]*TMath::Gaus(x,[1],[2])", -1500, -1000);
    //Fitthin->SetParameters(1.18775e+01,-1.17852e+03,1.10360e+01 );
    Fitthin->SetParameters(3.94760e+01,-1.17938e+03,9.01608);

    TF1*FitCombined = new TF1("FitCombined", "[0]*TMath::Gaus(x,[1],[2])+[3]*TMath::Gaus(x,[4],[5])", -1500, -1000);



    h1->Fit(Fitwide,"0","",-1500, -1000);
    h1->Fit(Fitthin,"0","",-1190, -1170);

   //FitCombined->SetParameters(Fitwide->GetParameter(0),Fitwide->GetParameter(1), Fitwide->GetParameter(2),Fitthin->GetParameter(0),Fitthin->GetParameter(1),Fitthin->GetParameter(2));
   //h1->Fit(FitCombined,"0","",-1500, -1000);


    Fitwide->SetLineColor(kRed);
    Fitwide->SetLineStyle(2);
    Fitwide->Draw("same");

    Fitthin->SetLineColor(kRed);
    Fitthin->SetLineStyle(2);
    Fitthin->Draw("same");

    //FitCombined->SetLineColor(kRed);
    //FitCombined->Draw("same");

    //Lleno un histograma con numeros aleatorios siguiendo la forma de la funcion suma FitCombined, la media de ese histograma es la que uso
    TH1F*h2 = new TH1F("TH2","", 100, -1500, -1000);
    h2->FillRandom("FitCombined",10000); */
  //h2->Draw("same");

  //TLine *linemean=new TLine(h2->GetMean(),0,h2->GetMean(),45);
  //cout<<"Mean: "<<h2->GetMean()<<endl;
  //cout<<"RMS: "<<h2->GetRMS()<<endl;
  //Uso la media del histograma original (h1), no la media del histograma que llenamos usando el fit final (h2)
  //TLine *linemean=new TLine(h1->GetMean(),0,h1->GetMean(),45);
  cout<<"Mean: "<<h1->GetMean()<<endl;
  cout<<"RMS: "<<h1->GetRMS()<<endl;
  cout<<"MeanError: "<<h1->GetMeanError()<<endl;
  cout<<"RMSError: "<<h1->GetRMSError()<<endl;

  //linemean->Draw("same");

}
