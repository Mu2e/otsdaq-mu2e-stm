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

  gROOT->SetStyle("ATLAS");
  //auto c1= new TCanvas("c1","Title",400,10,1500,500);
  auto c1= new TCanvas("c1");  
  double countphot1 = 658;
  double countphot2 = 666;
  
  double xx1[2]={630, 680};
  double yy1[2]={0,1000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);      
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("{}^{137}Cs");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  //Complete  
  //TH1F*h1 = new TH1F("TH1","", 4848, 0, 800);//binning of 0.165 kev
  TH1F*h1 = new TH1F("TH1","", 90, 630, 680);//binning of 0.57 kev
  //TH1F*h1 = new TH1F("TH1","",877 , 300, 800);//binning of 0.57 kev  
  //TH1F*h1 = new TH1F("TH1","", 400, 0, 800);//binning of 2kev     
  //Abrimos el txt
  fstream readfile;
  //Open old ROOT files for RUN109 using M=8000,L=1000
  //readfile.open("/work/cgarcia/DATA/MWD_Analysis/run_00109.txt",ios::in);

  //Open ROOT files for RUN109 using M=400,L=200
  //readfile.open("/work/cgarcia/DATA/MWD_Analysis/RUN109/M400L200/run_00109.txt",ios::in);
  //Open ROOT files for RUN109 using M=8000,L=1000
  //readfile.open("/work/cgarcia/DATA/MWD_Analysis/RUN109/M8000L1000/run_00109.txt",ios::in);
  //Open ROOT files for RUN109 using M=1000,L=500 
  //readfile.open("/work/cgarcia/DATA/MWD_Analysis/RUN109/M1000L500/run_00109.txt",ios::in);
  readfile.open("/work/mu2e/data1/cgarcia/DATA/MWD_Analysis/RUN109/M1000L500/run_00109.txt",ios::in);

  string name;
  vector<string> file_name;
  file_name.clear();

  int xrays=0;

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
    TTree *tree=(TTree*)input->Get("treeADC");
    double peaks;
    double peakEnergy;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();
  
    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree->GetEntry(i);
      
      //Using (old root files) Liverpool Calibration with M=8000, L=1000
      //peakEnergy=(peaks-0.354947)/(-1.75514);

      //Using Liverpool Calibration with M=400, L=200
      //peakEnergy=(peaks-1.738)/(-1.767);
      
      //Using Liverpool Calibration with M=8000, L=1000
      //peakEnergy=(peaks+3.392)/(-1.753);

      //Using Liverpool Calibration with M=1000, L=500
      //peakEnergy=(peaks-2.73)/(-1.76);

      //Standard
      peakEnergy=peaks*(-0.57);

      //Cutting noise, signal above a threshold
      if(peakEnergy>30){
	//Number of X-Rays inside the gaussian
	if((peakEnergy<countphot2)&&(peakEnergy>countphot1)){xrays++;}
	h1->Fill(peakEnergy);
      }
    }
  }//for int file

  std::cout<<"Number of X-Rays for the 661.7 keV peak counting: "<<xrays<<std::endl;

  double numberentries661 = h1->Integral(h1->FindFixBin(countphot1), h1->FindFixBin(countphot2), "");
  std::cout<<"Number of X-Rays for the 661.7 keV peak integral: "<<numberentries661<<std::endl;
  h1->GetXaxis()->SetTitle("E [keV]");
  h1->SetTitle("");
  h1->SetStats(1);
  h1->Draw("same");

   //Fit Energy peak Cessium
  TF1*Fitwide = new TF1("Fitwide", "[0]*TMath::Gaus(x,[1],[2])", 630, 680);
  Fitwide->SetParameters(1.31429e+02,660,2.01827e+00 );
  
  h1->Fit(Fitwide,"0","",630, 680);

  
  Fitwide->SetLineColor(kRed);
  Fitwide->SetLineStyle(2);
  Fitwide->Draw("same");

  
  cout<<"Mean: "<<h1->GetMean()<<endl;
  cout<<"RMS: "<<h1->GetRMS()<<endl;
  cout<<"MeanError: "<<h1->GetMeanError()<<endl;
  cout<<"RMSError: "<<h1->GetRMSError()<<endl;

  TLine *line661=new TLine(661.7,yy1[0],661.7,yy1[1]);
  line661->SetLineStyle(3);
  line661->SetLineColor(kBlue);
  line661->SetLineWidth(2);
  line661->Draw("same");
   auto leg1 = new TLegend(0.2,0.6,0.5,0.8);
   leg1->AddEntry(h1, "raw data","f");
   leg1->AddEntry(Fitwide, "fit","l");
   leg1->AddEntry(line661, "661.7 keV","l");
   leg1->Draw("same");


   //c1->Print("EnergyCs_run00109_newmidas.pdf","pdf");
   //c1->Print("EnergyCs_run00109_0.57Calib.png","png");
   //c1->Print("Energy109.pdf","pdf");                                                                           
   //c1->Print("Energy109.png","png"); 
}
