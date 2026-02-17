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


void ReadVariousMWDrootfiles(std::string rate) {

  gROOT->SetStyle("ATLAS");
  //auto c1= new TCanvas("c1","Title",400,10,1500,500);
  auto c1= new TCanvas("c1");  

  double xx1[2]={330, 360};
  double yy1[2]={0,10100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);      
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  //graph1->Draw("ap");

  TH1D* htrue = new TH1D("hTrue","",100,xx1[0],xx1[1]);
  TH1D* hreco = new TH1D("hReco","",100,xx1[0],xx1[1]);
  
  std::string name;
  fstream readfile;
  std::vector<string> file_name;
  //TRUE
  readfile.open("/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/FlashXrayOverlappedSim/"+rate+"kHz_Flash/"+rate+"kHz_FlashTrue.txt",ios::in);
  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    }


  std::cout<<"Size: "<<file_name.size()<<std::endl;

  for (int file=0;file<(file_name.size()-1);file++){
 
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());

    TTree* tree=(TTree*)input->Get("Etree");
    double E;
    tree->SetBranchAddress("Energy",&E);
        
    unsigned long entries=tree->GetEntries();
    
    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);
      htrue->Fill(E);
    }
    
  }//for int file

  //for the true histogram add 600 peaks
  int Ntrue_Xrays;
  if(stod(rate)<20){Ntrue_Xrays=600;}
  else{Ntrue_Xrays=60;}
  for(int k=0; k<Ntrue_Xrays; k++){htrue->Fill(347);}
    
  htrue->Draw("same");


  //RECO
  fstream readfile_reco;
  std::vector<string> file_name_reco;
  readfile_reco.open("/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/FlashXrayOverlappedSim/SUM_FlashXrays/"+rate+"kHz/MWDM400L200_Noise/"+rate+"kHz_FlashReco.txt",ios::in);
  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile_reco>>name;
    file_name_reco.push_back(name);
    if(readfile_reco.eof())break;
    }


  std::cout<<"Size: "<<file_name_reco.size()<<std::endl;

  for (int file=0;file<(file_name_reco.size()-1);file++){
 
    string path;
    path=file_name_reco[file];
    std::cout<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());

    TTree* tree=(TTree*)input->Get("treeADC");
    double p;
    tree->SetBranchAddress("peaks",&p);
    
    unsigned long entries=tree->GetEntries();
    
    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);
      double E = p*(-0.57);
      hreco->Fill(E);
    }
    
  }//for int file
    
  htrue->Draw("same");
  hreco->SetLineColor(kBlue);
  hreco->Draw("same");

}
