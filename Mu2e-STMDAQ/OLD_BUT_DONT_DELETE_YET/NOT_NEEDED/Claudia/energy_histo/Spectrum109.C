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

void Spectrum109() {

  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={-1500, -1000};
  double yy1[2]={0,700};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(-1500, -1000);
  graph1->GetYaxis()->SetRangeUser(0,700);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //Complete
  //TH1F*h1 = new TH1F("TH1","", 1000, -3000, 0);
  TH1F*h1 = new TH1F("TH1","", 1000, -1500, -1000);
  //Read the first .root
  //TFile *run0= new TFile("run00109_energypeaks_bin_00.root","read");
  //Abrimos el txt
  fstream readfile;
  readfile.open("run_00109.txt",ios::in);
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


  h1->GetXaxis()->SetTitle("ADC Counts");
  h1->SetTitle("");
  //  h1->SetStats(1);
  h1->Draw("same");

   //Fit Energy peak Cessium
   TF1*Fitwide = new TF1("Fitwide", "[0]*TMath::Gaus(x,[1],[2])", -1190, -1110);
   Fitwide->SetParameters(2.01505e+02,-1.09731e+03,-4.79156e+00);
  
   
   h1->Fit(Fitwide,"0","",-1190, -1110);
  

  Fitwide->SetLineColor(kRed);
  Fitwide->SetLineStyle(2);
  Fitwide->Draw("same");

  //  TLine *linemean=new TLine(-1.09731e+03,0,-1.09731e+03,280);
  cout<<"Mean: "<<h1->GetMean()<<endl;
  cout<<"RMS: "<<h1->GetRMS()<<endl;
  cout<<"MeanError: "<<h1->GetMeanError()<<endl;
  cout<<"RMSError: "<<h1->GetRMSError()<<endl;

  //linemean->Draw("same");
  //c1->Print("ADCCounts_run00109.pdf","pdf");                                                                           
  //c1->Print("ADCCounts_run00109.png","png");  
   c1->Print("ADCCountsFit_run00109_newprogram.pdf","pdf");
  c1->Print("ADCCountsFit_run00109_newprogram.png","png");
 

}
