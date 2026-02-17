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

void AllPeaksRawSup(std::string inputraw, std::string inputsuppressed) {

  gROOT->SetStyle("ATLAS");
  auto c1= new TCanvas("c1");

  bool ADC = false; //if ADC true plot ADC spectrum, if ADC false plot energy spectrum
  double calibration = 1; //1 for 1ADC=0.57keV, 2 for Liverpool Calibration M=400, L=200, 3 for ELBE calibration M=400, L=200

  //Histogram range
  double xx1[2]={630,680};
  int bins=int((xx1[1]-xx1[0])/0.57); 

  TH1F*hsup = new TH1F("hsup","",100 , -1500, -1000);
  TH1F*hraw = new TH1F("hraw","",100 , -1500, -1000);  
  

  TH1F*henergysup = new TH1F("henergysup","", bins , xx1[0], xx1[1]); //bin 0.57 keV
  TH1F*henergyraw = new TH1F("henergyraw","", bins , xx1[0],xx1[1]);

  //TH1F*henergysup = new TH1F("henergysup","", bins , 0, 1000);
  //TH1F*henergyraw = new TH1F("henergyraw","", bins , 0,1000);

  //Counting peaks range
  int countpeaks_raw=0;
  int countpeaks_sup=0;
  int countpeaks_low=655;
  int countpeaks_up=675;

  //int countpeaks_low=0;
  //int countpeaks_up=70;      
  
  double energyraw, energysup;

  //Fit range
  double fitrange[2]={655,675};



  //-----SUPPRESSED DATA FILE

  fstream readfile0;
  readfile0.open(inputsuppressed,ios::in);
  string name0;
  vector<string> file_name0;
  file_name0.clear();

  while(1){
    readfile0>>name0;
    file_name0.push_back(name0);
    if(readfile0.eof())break;
    cout<<name0<<endl;
  }

  std::cout<<"Size: "<<file_name0.size()<<std::endl;
  for (int file=0;file<(file_name0.size()-1);file++){
    string path;
    path=file_name0[file];
    
    cout<<path.c_str()<<endl;

    TFile *runsup=new TFile(path.c_str());
    TTree* tree0=(TTree*)runsup->Get("treeADC");
    double peaks;

    tree0->SetBranchAddress("peaks",&peaks);

    unsigned long entries0=tree0->GetEntries();

    cout<<"entries: "<<entries0<<endl;
    for(unsigned long i=0;i<entries0;i++){

      tree0->GetEntry(i);
      hsup->Fill(peaks);


      if(calibration==1){
	energysup=(peaks*(-0.57)); 
	//energysup=(peaks-0.354947)/(-1.75514);
	henergysup->Fill(energysup);
	if((energysup>=countpeaks_low)&&(energysup<=countpeaks_up)){countpeaks_sup++; std::cout<<"Sup: "<<energysup<<std::endl;}
      }
      if(calibration==2){
	energysup=(peaks-1.738)/(-1.767);
	henergysup->Fill(energysup);
	if((energysup>=countpeaks_low)&&(energysup<=countpeaks_up)){countpeaks_sup++;}
      }
      if(calibration==3){
	energysup=(peaks+1.76)/(-1.785);
	henergysup->Fill(energysup);
	if((energysup>=countpeaks_low)&&(energysup<=countpeaks_up)){countpeaks_sup++;}
      }


    }
  }//for int file
 


  //-----RAW DATA FILE

  fstream readfile1;
  readfile1.open(inputraw,ios::in);
  string name1;
  vector<string> file_name1;
  file_name1.clear();

  while(1){
    readfile1>>name1;
    file_name1.push_back(name1);
    if(readfile1.eof())break;
    cout<<name1<<endl;
  }

  std::cout<<"Size: "<<file_name1.size()<<std::endl;
  for (int file=0;file<(file_name1.size()-1);file++){
    string path;
    path=file_name1[file];

    cout<<path.c_str()<<endl;

    TFile *runraw=new TFile(path.c_str());
    TTree* tree1=(TTree*)runraw->Get("treeADC");
    double peaks;

    tree1->SetBranchAddress("peaks",&peaks);

    unsigned long entries1=tree1->GetEntries();

    cout<<"entries: "<<entries1<<endl;
    for(unsigned long i=0;i<entries1;i++){
                            
      tree1->GetEntry(i);
      hraw->Fill(peaks);

      if(calibration==1){
	energyraw=(peaks*(-0.57));
	//energyraw=(peaks-0.354947)/(-1.75514);
	henergyraw->Fill(energyraw);
	if((energyraw>=countpeaks_low)&&(energyraw<=countpeaks_up)){countpeaks_raw++; std::cout<<"Raw: "<<energyraw<<std::endl;}
      }
      if(calibration==2){
	energyraw=(peaks-1.738)/(-1.767);
	henergyraw->Fill(energyraw);
	if((energyraw>=countpeaks_low)&&(energyraw<=countpeaks_up)){countpeaks_raw++;}
      }
      if(calibration==3){
	energyraw=(peaks+1.76)/(-1.785);
	henergyraw->Fill(energyraw);
	if((energyraw>=countpeaks_low)&&(energyraw<=countpeaks_up)){countpeaks_raw++;}
      }

    }
  }//for int file        



  //double countsRaw = hraw->Integral(hraw->FindFixBin(-1200), hraw->FindFixBin(-1100), "");
  std::cout<<"Number of counts in the raw-data photopeak between "<<countpeaks_low <<" and "<< countpeaks_up<< ": "<<countpeaks_raw<<endl;
  //double countsSup = hsup->Integral(hsup->FindFixBin(-1200), hsup->FindFixBin(-1100), "");
  std::cout<<"Number of counts in the sup-data photopeak between "<<countpeaks_low <<" and " <<countpeaks_up << ": "<<countpeaks_sup<<endl;


  if(ADC==true){
  hsup->GetXaxis()->SetTitle("ADC Counts");
  hsup->SetTitle("");
  hsup->SetLineColor(kBlue);

  hraw->GetXaxis()->SetTitle("ADC Counts");
  hraw->SetTitle("");
  hraw->SetLineColor(kGreen);

  hraw->Draw("same");
  hsup->Draw("same");

 TLatex latex;
 latex.SetTextSize(0.04);
 latex.DrawLatex(640,0.05,"M=400, L=200");
 
 auto legend = new TLegend(0.1,0.7,0.48,0.9);
 legend->AddEntry(hraw,"Raw Data","f");
 legend->AddEntry(hsup,"Suppressed Data","f");
 legend->Draw("same");
  }


  if(ADC==false){
    henergysup->GetXaxis()->SetTitle("Energy [keV]");
    henergysup->SetTitle("");
    henergysup->SetLineColor(kBlue);

    henergyraw->GetXaxis()->SetTitle("Energy [keV]");
    henergyraw->SetTitle("");
    henergyraw->SetLineWidth(3);
    henergyraw->SetLineColor(kGreen);

  
    henergyraw->Draw("same");
    henergysup->Draw("same");

    //Fit suppressed data
    std::cout<<"Fit for suppressed data"<<std::endl;
    TF1*Fitsup = new TF1("Fitsup", "[0]*TMath::Gaus(x,[1],[2])", fitrange[0], fitrange[1]);
    Fitsup->SetParameters(6.89704e+02,665,2);
    henergysup->Fit(Fitsup,"0","",fitrange[0], fitrange[1]);
    Fitsup->SetLineColor(kBlue);
    Fitsup->SetLineStyle(2);
    //Fitsup->Draw("same");

    //Fit raw data
    std::cout<<"Fit for raw data"<<std::endl;
    TF1*Fitraw = new TF1("Fitraw", "[0]*TMath::Gaus(x,[1],[2])", fitrange[0], fitrange[1]);
    Fitraw->SetParameters(6.89704e+02,665,2);
    henergyraw->Fit(Fitraw,"0","",fitrange[0], fitrange[1]);
    Fitraw->SetLineColor(kGreen);
    Fitraw->SetLineStyle(2);
    //Fitraw->Draw("same");


    TLatex latex, latex1, latex2;
    latex.SetTextSize(0.05);
    latex.DrawLatex(635,0,"M=400, L=200");
    latex1.SetTextSize(0.03);
    latex1.DrawLatex(635,0,"5 Block RAMs");
    latex2.SetTextSize(0.05);
    latex2.DrawLatex(635,0,"#font[6]{Spill Mode}");


    auto legend = new TLegend(0.23,0.6,0.53,0.8);
    legend->AddEntry(henergyraw,"Raw Data","f");
    //legend->AddEntry(Fitraw,"Fit Raw Data","l");
    legend->AddEntry(henergysup,"HLS Suppressed Data","f");
    //legend->AddEntry(Fitsup,"HLS Suppressed Data","l");
    legend->Draw("same");


    //c1->Print("run109_M1000L500_livCaliM400_5us_HZDRcode.png");
    //c1->Print("run109_M1000L500_livCaliM400_5us_HZDRcode.pdf");
  }



}
