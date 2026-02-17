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
#include "TPaveStats.h"

void Spectrum109() {
  gROOT->SetStyle("ATLAS");
  
  auto c1= new TCanvas("c1");

  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1); gStyle->SetStatBorderSize(0);
  gStyle->SetStatX(.89); gStyle->SetStatY(.89);
  gStyle->SetFitFormat("5.7g");
  
  //Complete
  TH1F*h1 = new TH1F("TH1","", 1000, -1200, -1100);
  //Abrimos el txt
  fstream readfile;
  readfile.open("/work/mu2e/data1/cgarcia/DATA/MWD_Analysis/RUN109/M1000L500/run_00109.txt",ios::in);
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

  double xfit[2];
  xfit[0] = -1180;
  xfit[1] = -1145;
  
  double counts = h1->Integral(h1->FindFixBin(xfit[0]), h1->FindFixBin(xfit[1]), "");
  std::cout<<"Number of counts in the photopeak: "<<counts<<endl;
  std::cout<<"bin numbers: "<<h1->FindFixBin(xfit[0])<<" "<<h1->FindFixBin(xfit[1])<<std::endl;

  int zerocontentbins = 0;
  for(int i = h1->FindFixBin(xfit[0]) ; i < h1->FindFixBin(xfit[1]); i++){
    if(h1->GetBinContent(i)==0){zerocontentbins++;}
  }

  std::cout<<"Bins with no content: "<<zerocontentbins<<std::endl;
  double ndof = h1->FindFixBin(xfit[1])-h1->FindFixBin(xfit[0]) - 3 - zerocontentbins ; 

  std::cout<<"ndof = n bins in fit range - parameters used for fit - bins with zero content = "<<ndof<<std::endl;
  
  //Fit Energy peak Cessium
   TF1*Fitwide = new TF1("Fitwide", "[0]*TMath::Gaus(x,[1],[2])", xfit[0], xfit[1]);
   Fitwide->SetParameters(200,-1160,4.79156);   
   h1->Fit(Fitwide,"0","",xfit[0], xfit[1]);
   Fitwide->SetLineColor(kRed);
   Fitwide->SetLineStyle(2);
    
  /*
   cout<<"Mean: "<<h1->GetMean()<<endl;
   cout<<"RMS: "<<h1->GetRMS()<<endl;
   cout<<"MeanError: "<<h1->GetMeanError()<<endl;
   cout<<"RMSError: "<<h1->GetRMSError()<<endl;
   
  */

   h1->Draw();
   c1->Modified();
   c1->Update();
  
  gPad->Update();
  TPaveStats* ps = (TPaveStats *)h1->GetListOfFunctions()->FindObject("stats");
  //TPaveStats* ps = (TPaveStats *)h1->FindObject("stats");
  ps->SetY1NDC(.62);
  ps->SetY2NDC(.87);
  ps->SetX1NDC(0.5);
  ps->SetX2NDC(0.91);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);
  
  c1->Modified();
  c1->Update();
  
  double xx1[2]={-1200, -1100};
  double yy1[2];
  yy1[0]=0;
  yy1[1]=h1->GetMaximum()+5;
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);

  graph1->Draw("ap"); 
  h1->Draw("same");  
  ps->Draw("same");
  Fitwide->Draw("same"); 

  auto legend = new TLegend(0.52,0.35,0.91,0.55);
  legend->AddEntry(h1,"raw data","F");
  legend->AddEntry(Fitwide,"fit","L");
  legend->Draw("same");
  

  //c1->Print("109CsCalibration_M1000L500_newChi2.pdf","pdf"); 
  //c1->Print("109CsCalibration_M1000L500_newChi2.png","png");

}
