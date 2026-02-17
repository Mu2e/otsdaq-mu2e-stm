#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TPaveStats.h"

using namespace std;

void plotGeom_KCl_Ge(std::string filename){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);

  TFile* input = new TFile(filename.c_str(),"read");


  TH3D* hgeom=new TH3D("hgeom", "KCl geom; x; z; y", 100, -10, 10, 100, -10, 10, 100, -10, 10);


  double pxgen,pygen,pzgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas;

  //Leemos el TTree
  TTree* tree3=(TTree*)input->Get("StepsKCl");

  tree3->SetBranchAddress("preXpos", &preXpos);
  tree3->SetBranchAddress("preYpos", &preYpos);
  tree3->SetBranchAddress("preZpos", &preZpos);




  int entries3=tree3->GetEntries();
  cout<<entries3<<endl;

  for(unsigned long i=0;i<entries3;i++)
    {
      tree3->GetEntry(i);
      hgeom->Fill(preXpos,preZpos,preYpos);
    }

  hgeom->Draw("");

}
