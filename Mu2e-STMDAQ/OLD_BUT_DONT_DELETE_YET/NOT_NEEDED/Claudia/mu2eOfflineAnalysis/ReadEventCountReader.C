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
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TF1.h"
#include "TPaveStats.h"

//================================================================
//This program reads genCountLogger/numEvents histogram
//================================================================  

void EntriesHisto(std::string rootinput, int nbins, double xmin, double xmax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0010);

  TCanvas *c1 = new TCanvas("");
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  std::cout<<"Reading code from: "<<rootinput<<std::endl;

  TFile *input=new TFile(rootinput.c_str()); 
  //TFile *input = TFile::Open(rootinput.c_str());

  TH1F *hist = (TH1F*)input->Get("genCountLogger/numEvents");
  double entries_evt = hist->GetBinContent(1); 
  TH1F *hist_subruns = (TH1F*)input->Get("genCountLogger/numSubRuns");
  double entries_subruns = hist_subruns->GetBinContent(1);
  
  std::cout<<"Events: "<<entries_evt<<std::endl;
  std::cout<<"Subruns: "<<entries_subruns<<std::endl;

  //TH1F*h1 = new TH1F("TH1","", nbins, xmin, xmax);
  hist->Draw("");

  //c1->Print("");
  //c1->Print("");

}



//================================================================
void ReadEventCountReader(std::string rootfile_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 
  
  //To read entries in histogram in rootfile and print it
  EntriesHisto(rootfile_location, 100, -1, 1, "", "# Events in 1 bin", 0); 

}

//================================================================
