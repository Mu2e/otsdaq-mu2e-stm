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

void ADC_toE_109() {

  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={0, 800};
  double yy1[2]={0,700};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,800);      
  graph1->GetYaxis()->SetRangeUser(0,700);
  graph1->SetTitle("{}^{137}Cs");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //Complete
  
  //    TH1F*h1 = new TH1F("TH1","", 4848, 0, 800);//binning of 0.165 kev
   TH1F*h1 = new TH1F("TH1","", 1404, 0, 800);//binning of 0.57 kev
  // TH1F*h1 = new TH1F("TH1","", 400, 0, 800);//binning of 2kev     
   //Abrimos el txt
  fstream readfile;
  readfile.open("run_00109.txt",ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    //cout<<name<<endl;
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
      h1->Fill((peaks-6.50022)/(-1.66617));
    }
  }//for int file


  h1->GetXaxis()->SetTitle("E (keV)");
  h1->SetTitle("");
  h1->SetStats(1);
  h1->Draw("same");

   //Fit Energy peak Cessium
  TF1*Fitwide = new TF1("Fitwide", "[0]*TMath::Gaus(x,[1],[2])", 680, 720);
  Fitwide->SetParameters(1.31429e+02,6.95250e+02,2.01827e+00 );
  h1->Fit(Fitwide,"0","",680, 720);

  
    Fitwide->SetLineColor(kRed);
    Fitwide->SetLineStyle(2);
    Fitwide->Draw("same");

  
  cout<<"Mean: "<<h1->GetMean()<<endl;
  cout<<"RMS: "<<h1->GetRMS()<<endl;
  cout<<"MeanError: "<<h1->GetMeanError()<<endl;
  cout<<"RMSError: "<<h1->GetRMSError()<<endl;

  //TLine *line661=new TLine(661.7,0,661.7,800);
  // line661->Draw("same");
  //  auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
  //leg1->AddEntry(line661, "661.7 keV","l");
  //leg1->Draw("same");


  //c1->Print("EnergyCs_run00109.pdf","pdf");
  //c1->Print("EnergyCs_run00109.png","png");
  //c1->Print("Energy109.pdf","pdf");                                                                           
  //c1->Print("Energy109.png","png"); 
}
