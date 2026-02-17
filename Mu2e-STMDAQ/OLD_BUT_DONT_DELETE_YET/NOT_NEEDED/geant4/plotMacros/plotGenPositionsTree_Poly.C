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

void plotGenPositionsTree_Poly(std::string filename){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);


  TFile* input = new TFile(filename.c_str(),"read");

  double Nphot_gen = 500000;

  //Ge detector
  TH2D* hzx = new TH2D("hzx", "zx position of the hit in each step", 1000, 20, 30, 1000, -10, 10);
  hzx->GetXaxis()->SetTitle("z [cm]");
  hzx->GetYaxis()->SetTitle("x [cm]");
  TH2D* hzy = new TH2D("hzy", "zy position of the hit in each step", 1000, 20, 30, 1000, -10, 10);
  hzy->GetXaxis()->SetTitle("z [cm]");
  hzy->GetYaxis()->SetTitle("y [cm]");
  TH2D* hyx = new TH2D("hyx", "xy position of the hit in each step", 1000, -10, 10, 1000, -10, 10);
  hyx->GetXaxis()->SetTitle("x [cm]");
  hyx->GetYaxis()->SetTitle("y [cm]");
  

  //KCl detector
  /*TH2D* hzx = new TH2D("hzx", "zx position of the hit in each step", 1000, -8, 8, 1000, -8, 8);
  hzx->GetXaxis()->SetTitle("z [cm]");
  hzx->GetYaxis()->SetTitle("x [cm]");
  TH2D* hzy = new TH2D("hzy", "zy position of the hit in each step", 1000, -8, 8, 1000, -10, 10);
  hzy->GetXaxis()->SetTitle("z [cm]");
  hzy->GetYaxis()->SetTitle("y [cm]");
  TH2D* hyx = new TH2D("hyx", "xy position of the hit in each step", 1000, -8, 8, 1000, -10, 10);
  hyx->GetXaxis()->SetTitle("x [cm]");
  hyx->GetYaxis()->SetTitle("y [cm]");
  */


  TH1F* htracklengthevent_e = new TH1F("htracklengthevent_e", "Total Track Length by electrons in each event", 100, -0.5, 0.5);
  TH1F* htracklengthevent_phot = new TH1F("htracklengthevent_phot", "Total Track Length of photons depositing energy in Ge", 100, -1, 30);

  //htracklengthevent_phot->GetXaxis()->SetTitle("Total #gamma Track Length in KCl detector [cm]");
  htracklengthevent_phot->GetXaxis()->SetTitle("Total #gamma Track Length (#neq0) in Ge detector [cm]"); 
  htracklengthevent_phot->GetYaxis()->SetTitle("Events/X-ray");
  htracklengthevent_phot->GetYaxis()->SetTitleOffset(1.7);


  double pxgen,pygen,pzgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;

  //Leemos el TTree
  TTree* tree3=(TTree*)input->Get("StepsPoly");
  
  //Poly step
  tree3->SetBranchAddress("preXpos", &preXpos);
  tree3->SetBranchAddress("preYpos", &preYpos);
  tree3->SetBranchAddress("preZpos", &preZpos);
  tree3->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree3->SetBranchAddress("PDGID", &PDGID);
  tree3->SetBranchAddress("EventID", &EventID);
  tree3->SetBranchAddress("EdepStep", &EdepStep);

  int entries=tree3->GetEntries();
  cout<<entries<<endl;

  for(unsigned long i=0;i<entries;i++)
    {
      tree3->GetEntry(i);
      //std::cout<<"zpos: "<<preZpos<<" xpos: "<<preXpos<<" ypos: "<<preYpos<<std::endl;
      hzx->Fill(preZpos,preXpos);
      hzy->Fill(preZpos,preYpos);
      hyx->Fill(preXpos,preYpos);
  
    }
 

  TPaveStats *stat[4];

  c1->cd(1);
  //htracklengthevent_phot->Scale(1./Nphot_gen);
  // htracklengthevent_phot->SetFillStyle(3001);
  //htracklengthevent_phot->SetLineColor(kOrange+1);
  //htracklengthevent_phot->SetLineWidth(1);
  //htracklengthevent_phot->SetFillColor(kOrange+1);
  //htracklengthevent_phot->Draw("HIST");

  gPad->Update();

  /*stat[0] = (TPaveStats*)htracklengthevent->FindObject("stats"); 
  stat[0]->SetY1NDC(.74);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.45);
  stat[0]->SetTextSize(0.043);
  stat[0]->Draw("same");*/



  c1->cd(2);
  hyx->SetMarkerStyle(1);
  hyx->SetMarkerColor(kOrange+1);
  hyx->Draw("");

  gPad->Update();

  /*stat[1] = (TPaveStats*)hyx->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.043);
  stat[1]->Draw("same");*/

  c1->cd(3);
  hzy->SetMarkerStyle(1);
  hzy->SetMarkerColor(kOrange+1);
  hzy->Draw("");

  gPad->Update();

  /*stat[2] = (TPaveStats*)hzy->FindObject("stats");
  stat[2]->SetY1NDC(.74);
  stat[2]->SetY2NDC(.91);
  stat[2]->SetX1NDC(0.2);
  stat[2]->SetX2NDC(0.45);
  stat[2]->SetTextSize(0.043);
  stat[2]->Draw("same");*/

  c1->cd(4);
  hzx->SetMarkerStyle(1);
  hzx->SetMarkerColor(kOrange+1);
  hzx->Draw("");

  gPad->Update();

  /* stat[1] = (TPaveStats*)hzx->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.043);
  stat[1]->Draw("same");*/



  //c1->Print("");
}
