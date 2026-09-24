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
#include "TH1F.h"

void Spectrum110() {

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1); gStyle->SetStatBorderSize(0);
  gStyle->SetStatX(.89); gStyle->SetStatY(.89);
  gStyle->SetFitFormat("5.7g");

  
  auto c1= new TCanvas("c1");

  TH1F*h1 = new TH1F("TH1","", 28300, -2800, -30);

  //Abrimos el txt
  fstream readfile;
  readfile.open("/work/mu2e/data1/cgarcia/DATA/MWD_Analysis/RUN110/M1000L500/run_00110.txt",ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .csv
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    cout<<name<<endl;
  }
  std::cout<<"Size: "<<file_name.size()<<std::endl;
  for (int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    //cout<<file_name.size()<<endl;
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree->GetEntry(i);
      h1->Fill(peaks);
    }
  }//for int file

  //Fit Energy peak Europium
  TF1*Fitpeak1 = new TF1("Fitpeak1", "[0]*TMath::Gaus(x,[1],[2])", -100, -30);
  Fitpeak1->SetParameters(3.76986e+02 ,-7.31723e+01,-8.51890e+00 );
  TF1*Fitpeak2 = new TF1("Fitpeak2", "[0]*TMath::Gaus(x,[1],[2])", -220, -200);
  Fitpeak2->SetParameters( 6.09078e+02 ,-2.13458e+02, -4.16254e+00);
  TF1*Fitpeak3 = new TF1("Fitpeak3", "[0]*TMath::Gaus(x,[1],[2])", -440, -410);
  Fitpeak3->SetParameters(9.85416e+01 , -4.23283e+02,-6.86131e+00 );
  TF1*Fitpeak4 = new TF1("Fitpeak4", "[0]*TMath::Gaus(x,[1],[2])", -620, -580);
  Fitpeak4->SetParameters( 2.84262e+02 ,-5.97022e+02,4.32377e+00);
  TF1*Fitpeak5 = new TF1("Fitpeak5", "[0]*TMath::Gaus(x,[1],[2])",-727,-715);
  Fitpeak5->SetParameters( 2.06218e+01 ,-720,-1.19732e+01 );
  TF1*Fitpeak6 = new TF1("Fitpeak6", "[0]*TMath::Gaus(x,[1],[2])",-786,-773);
  Fitpeak6->SetParameters(2.77554e+01,-7.72270e+02,-7.12861e+00);

  TF1*Fitpeak7 = new TF1("Fitpeak7", "[0]*TMath::Gaus(x,[1],[2])", -1385, -1350);
  Fitpeak7->SetParameters(1.07648e+02,-1.35799e+03 ,4.83114e+00);
  TF1*Fitpeak8 = new TF1("Fitpeak8", "[0]*TMath::Gaus(x,[1],[2])", -1710, -1680);
  Fitpeak8->SetParameters(1.16572e+02 ,-1.70191e+03 ,4.26439e+00 );
  TF1*Fitpeak9 = new TF1("Fitpeak9", "[0]*TMath::Gaus(x,[1],[2])", -1925, -1900);   
  Fitpeak9->SetParameters(7.33733e+01 ,-1910,4.96903e+00);
  TF1*Fitpeak10 = new TF1("Fitpeak10", "[0]*TMath::Gaus(x,[1],[2])", -1970, -1945);
  Fitpeak10->SetParameters(8.80746e+01,-1.95990e+03 ,-4.33598e+00 );
  TF1*Fitpeak11 = new TF1("Fitpeak11", "[0]*TMath::Gaus(x,[1],[2])", -2490, -2450);
  Fitpeak11->SetParameters(1.36845e+02 ,-2.45713e+03,3.64893e+00);
  TF1*Fitpeak12 = new TF1("Fitpeak12", "[0]*TMath::Gaus(x,[1],[2])", -1535, -1515);
  Fitpeak12->SetParameters(2.42267e+01 , -1525,1.03950e+01);
  
  
  h1->Draw("");
  

  //h1->Fit(Fitpeak1,"0","",-75, -60);
  //h1->Fit(Fitpeak2,"0","",-220, -200);
  //h1->Fit(Fitpeak3,"0","",-430, -420);
  //h1->Fit(Fitpeak4,"0","",-630, -590);
  //h1->Fit(Fitpeak5,"0","",-727,-715);
  //h1->Fit(Fitpeak6,"0","",-786,-773);

  //h1->Fit(Fitpeak7,"0","",-1385, -1350);
  //h1->Fit(Fitpeak8,"0","",-1710, -1680);
  //h1->Fit(Fitpeak9,"0","",-1925, -1900);
  //h1->Fit(Fitpeak10,"0","",-1970, -1945);
  //h1->Fit(Fitpeak11,"0","",-2490, -2450);
  h1->Fit(Fitpeak12,"0","",-1535, -1515);
  

  //double xx1[2]={-200, -30};
  //double xx1[2]={-300, -180};
  //double xx1[2]={-500, -400};
  //double xx1[2]={-640, -590};
  //double xx1[2]={-740,-710};
  //double xx1[2]={-795,-755}; 
  //double xx1[2]={-1410, -1355};
  //double xx1[2]={-1750, -1680};
  //double xx1[2]={-1940, -1900};
  //double xx1[2]={-2000, -1940};
  //double xx1[2]={-2550, -2450};
  double xx1[2]={-1580, -1490};
  
  double yy1[2];
  yy1[0]= 0;
  //yy1[1]= h1->GetMaximum()+5;
  yy1[1]= 50;
  
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  
  
  Fitpeak1->SetLineColor(kRed);
  Fitpeak1->SetLineStyle(2);
   
  Fitpeak2->SetLineColor(kRed);
  Fitpeak2->SetLineStyle(2);
  
  Fitpeak3->SetLineColor(kRed);
  Fitpeak3->SetLineStyle(2);
  
  Fitpeak4->SetLineColor(kRed);
  Fitpeak4->SetLineStyle(2);
  
  Fitpeak5->SetLineColor(kRed);
  Fitpeak5->SetLineStyle(2);
  
  Fitpeak6->SetLineColor(kRed);
  Fitpeak6->SetLineStyle(2);
   
  Fitpeak7->SetLineColor(kRed);
  Fitpeak7->SetLineStyle(2);
  
  Fitpeak8->SetLineColor(kRed);
  Fitpeak8->SetLineStyle(2);
  
  Fitpeak9->SetLineColor(kRed);
  Fitpeak9->SetLineStyle(2);
    
  Fitpeak10->SetLineColor(kRed);
  Fitpeak10->SetLineStyle(2);
  
  Fitpeak11->SetLineColor(kRed);
  Fitpeak11->SetLineStyle(2);
  
  Fitpeak12->SetLineColor(kRed);
  Fitpeak12->SetLineStyle(2);
    
  c1->Modified();
  c1->Update();

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)h1->GetListOfFunctions()->FindObject("stats");
  ps->SetY1NDC(.62);
  ps->SetY2NDC(.87);
  ps->SetX1NDC(0.21);
  ps->SetX2NDC(0.61);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  c1->Modified();
  c1->Update();
  
  graph1->Draw("ap");
  h1->Draw("same");
  
  Fitpeak1->Draw("same");
  Fitpeak2->Draw("same");
  Fitpeak3->Draw("same");
  Fitpeak4->Draw("same");
  Fitpeak5->Draw("same");
  Fitpeak6->Draw("same");
  Fitpeak7->Draw("same");
  Fitpeak8->Draw("same");
  Fitpeak9->Draw("same");
  Fitpeak10->Draw("same");
  Fitpeak11->Draw("same");
  Fitpeak12->Draw("same");
  
  auto legend = new TLegend(0.23,0.35,0.7,0.56);
  legend->AddEntry(h1,"raw data","F");
  legend->AddEntry(Fitpeak1,"fit","L");
  legend->Draw("same");
  
  c1->Print("110EuCalibration_M1000L500_peak12_newChi2.pdf","pdf");
  c1->Print("110EuCalibration_M1000L500_peak12_newChi2.png","png");

}
