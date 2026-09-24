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

void plotTree() {

  //gROOT->SetStyle("ATLAS");
  int palette_number = 57;
  gStyle->SetPalette(palette_number);
  
  auto c1= new TCanvas("c1","c1",0,0,700,500);

  //Abrimos el txt
  fstream readfile;
  readfile.open("/work/cgarcia/DATA/geant4/CompPhotEffectStudy/roots.txt",ios::in);
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

  int num;
  double Egen;
  //Fill 2D histogram                                                                                                                                                                             
  TH2D *h = new TH2D("histo","",11,0,11,9,100,1000);
  h->GetXaxis()->SetTitle("# Comptons & Photoelectric Effects");
  h->GetYaxis()->SetTitle("Energy [keV]");
  h->GetXaxis()->SetTitleColor(kBlack);
  
  for (int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    //cout<<file_name.size()<<endl;
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("Events");
    double peaks;

    tree->SetBranchAddress("num",&num);
    tree->SetBranchAddress("Egen",&Egen);

    unsigned long entries=tree->GetEntries();
    cout<<"entries: "<<entries<<endl;


    //Frequency of events
    int maxfreq=12;
    int freq[12]={0,0,0,0,0,0,0,0,0,0,0,0};


    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);
      h->Fill(num,1000*Egen);//keV
      //increase the frequency of the element num
      freq[num]++; //if num=2 increase the element freq[2] by 1
      std::cout<<"Event: "<<i<<" #comptons and phot effect: "<<num<<" Energy x-ray: "<<Egen<<" MeV"<<std::endl;
    }
    
    for(int i=0;i<maxfreq;i++){
      std::cout<<"freq of "<<i<<" compt+phot: "<<freq[i]<<std::endl;}
  }//for int file

 
  h->Draw("colz");
}
