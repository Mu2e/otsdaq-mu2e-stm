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

#define readROOTfromtxt

void Create1rootFile() {

  gROOT->SetStyle("ATLAS");
  //auto c1= new TCanvas("c1","Title",400,10,1500,500);
  auto c1= new TCanvas("c1");  

  double xx1[2]={300, 800};
  double yy1[2]={0,1100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);      
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("{}^{137}Cs");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

#ifdef readROOTfromtxt   

  std::string rate="10";
  int rateint=int(stoi(rate));
  std::string noise="0.32";
  std::string energy="844";
  std::string Ms="400";
  std::string Ls="200";

  //Create this .root
  std::string rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/"+energy+"keV_"+noise+"mV/M"+Ms+"L"+Ls+"/MWDData"+energy+"keV_"+rate+"kHz_energypeaks.root";
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");
  TTree*treenew = new TTree("treeADC","treeADC");
  double peaks;
  treenew->Branch("peaks",&peaks);  

  string name;
  vector<string> file_name;
  file_name.clear();

  
  fstream readfile;
  readfile.open("/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/"+energy+"keV_"+noise+"mV/M"+Ms+"L"+Ls+"/run"+std::to_string(rateint)+"kHz.txt",ios::in); //ROOT files at 1kHz (different jobs) 
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
    TTree* tree=(TTree*)input->Get("treeADC");
    double p;
    double peakEnergy;
    
    tree->SetBranchAddress("peaks",&p);

    unsigned long entries=tree->GetEntries();
    
    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);
      peaks=p;
      //std::cout<<"ADC peak: "<<peaks<<std::endl;
      treenew->Fill();
    }
  }//for int file
  rootfile->Write();
  rootfile->Close();

  std::cout<<"ROOT file created: "<<rootname<<std::endl;
#else
  string name;

  std::string rate="1";
  int rateint=int(stoi(rate));
  std::string noise="0.32";
  std::string energy="1809";
  std::string Ms="400";
  std::string Ls="200";

  int Mnum = 9;
  int Lnum = 20;

  int M[9]={300,400,500,600,700,800,900,1000,2000};
  int L[20]={10,20,30,40,50,60,70,80,90,100,200,300,400,500,600,700,800,900,1000,2000};
  std::string rootname;
  TFile *rootfile;
  double peaks;


  for(int i = 0; i < Mnum ;i++){
    for (int j = 0; j < Lnum ; j++){
      if(L[j] > M[i]){continue;}

      rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/"+energy+"keV_"+noise+"mV/M"+Ms+"L"+Ls+"/"+std::to_string(rateint)+"kHz/MWDData"+energy+"keV_"+rate+"kHz_M"+std::to_string(M[i])+"_L"+std::to_string(L[j])+"_energypeaks.root";
      rootfile=new TFile(rootname.c_str(),"recreate");
      TTree*treenew = new TTree("treeADC","treeADC");
   
      treenew->Branch("peaks",&peaks);

      for(int jobs=0;jobs<20;jobs++){
	if(jobs<10){
	  name="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/"+energy+"keV_"+noise+"mV/M"+Ms+"L"+Ls+"/"+std::to_string(rateint)+"kHz/MWDData"+energy+"keV_"+rate+"kHz_job0"+std::to_string(jobs)+"._M"+std::to_string(M[i])+"_L"+std::to_string(L[j])+"_energypeaks.root";
	}
	else{name="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/"+energy+"keV_"+noise+"mV/M"+Ms+"L"+Ls+"/"+std::to_string(rateint)+"kHz/MWDData"+energy+"keV_"+rate+"kHz_job"+std::to_string(jobs).+"_M"+std::to_string(M[i])+"_L"+std::to_string(L[j])+"_energypeaks.root";}

	std::cout<<name.c_str()<<std::endl;


	TFile *input=new TFile(name.c_str());
	TTree* tree=(TTree*)input->Get("treeADC");
	double p;
	double peakEnergy;

	tree->SetBranchAddress("peaks",&p);

	unsigned long entries=tree->GetEntries();

	cout<<"entries: "<<entries<<endl;
	for(unsigned long i=0;i<entries;i++){
	  tree->GetEntry(i);
	  peaks=p;
	  //std::cout<<"ADC peak: "<<peaks<<std::endl;
	  treenew->Fill();
	}//entries
	input->Close();
      }//jobs
      std::cout<<"ROOTFILE CREATED: "<<rootname<<std::endl;
      std::cout<<""<<std::endl;
      rootfile->Write();
      rootfile->Close();
    }//Lnum
  }//Mnum

#endif


}
