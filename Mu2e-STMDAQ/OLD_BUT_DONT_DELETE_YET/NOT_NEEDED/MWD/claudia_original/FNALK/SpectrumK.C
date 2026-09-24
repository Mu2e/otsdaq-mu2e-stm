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

void SpectrumK(std::string pathfile) {
  gROOT->SetStyle("ATLAS");

  auto c1= new TCanvas("c1");

  double xx1[2]={-4000,-3800};
  double yy1[2]={0,100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  //Complete
  TH1F*h1 = new TH1F("TH1","", 4000, -8000, 0);
 
  //Abrimos el txt
  fstream readfile;
  readfile.open(pathfile,ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  double range1 = -1640;
  double range2 = -1610;
  double counts_K = 0;


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
      if((peaks>=range1)&&(peaks<=range2)){counts_K++;}
    }
  }//for int file


  h1->GetXaxis()->SetTitle("ADC Counts");
  h1->SetTitle("");
  //h1->SetStats(1);
  //h1->SetLineColor(kBlue);
  h1->Draw("same");

  TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", -3930, -3850);
  Fit->SetParameters(1.61968e+02,-3892.31,-9.29169e+0);
  h1->Fit(Fit,"0","", -3950, -3850);
  Fit->SetLineColor(kRed);
  Fit->SetLineStyle(2);
  Fit->Draw("same");

  //TLine *linemean=new TLine(-1.09731e+03,0,-1.09731e+03,280);
  cout<<"Mean: "<<h1->GetMean()<<endl;
  cout<<"RMS: "<<h1->GetRMS()<<endl;
  cout<<"MeanError: "<<h1->GetMeanError()<<endl;
  cout<<"RMSError: "<<h1->GetRMSError()<<endl;

  //double integral_fullhisto = h1->Integral(h1->FindFixBin(xx1[0]), h1->FindFixBin(xx1[1]));
  //std::cout<<"(back+peaks) Integral between "<<xx1[0]<<" and "<<xx1[1]<<" keV: "<<integral_fullhisto<<std::endl;

  double integral_fullhisto = h1->Integral(h1->FindFixBin(xx1[0]), h1->FindFixBin(xx1[1]));
  std::cout<<"(back+peaks) Integral between "<<xx1[0]<<" and "<<xx1[1]<<" keV: "<<integral_fullhisto<<std::endl;

  double integral_K = h1->Integral(h1->FindFixBin(range1), h1->FindFixBin(range2));
  std::cout<<"Number of counts K Integral between "<<range1<<" and "<<range2<<" keV: "<<integral_K<<std::endl;

  std::cout<<"Number of counts K, between "<<range1<<" and "<<range2<<": "<<counts_K<<std::endl;

  //linemean->Draw("same");
  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(h1,"^{40}K data","F");
  legend->AddEntry(Fit,"fit","L");
  legend->Draw("same");

  //gPad->SetLogy();

  //c1->Print("Fit.png","png"); 

}
