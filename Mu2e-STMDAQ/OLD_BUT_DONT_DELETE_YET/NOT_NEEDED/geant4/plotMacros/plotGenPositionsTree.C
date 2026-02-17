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

void plotGenPositionsTree(std::string filename){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);

  //TFile* input= new TFile("/data1/cgarcia/Geant4_K/BlockGe_1460.8keVphotons_momsphere.root","read");
  //TFile* input= new TFile("/data1/cgarcia/Geant4_K/CylinderGe_1460.8keVphotons_momsphere.root","read");  

  TFile* input = new TFile(filename.c_str(),"read");

  TH2D* hzx = new TH2D("hzx", "zx position of the hit in each step", 1000, 15, 35, 1000, -10, 10);
  hzx->GetXaxis()->SetTitle("z [cm]");
  hzx->GetYaxis()->SetTitle("x [cm]");
  TH2D* hzy = new TH2D("hzy", "zy position of the hit in each step", 1000, 15, 35, 1000, -10, 10);
  hzy->GetXaxis()->SetTitle("z [cm]");
  hzy->GetYaxis()->SetTitle("y [cm]");
  TH2D* hyx = new TH2D("hyx", "xy position of the hit in each step", 1000, -10, 10, 1000, -10, 10);
  hyx->GetXaxis()->SetTitle("x [cm]");
  hyx->GetYaxis()->SetTitle("y [cm]");
  TH1F* htracklengthevent_e = new TH1F("htracklengthevent_e", "Total Track Length by electrons in each event", 100, -0.5, 0.5);
  TH1F* htracklengthevent_phot = new TH1F("htracklengthevent_phot", "Total Track Length by photons in each event", 100, -1, 30);

  htracklengthevent_phot->GetXaxis()->SetTitle("Total #gamma Track Length in Ge detector [cm]");
  htracklengthevent_phot->GetYaxis()->SetTitle("Events");
  htracklengthevent_phot->GetYaxis()->SetTitleOffset(1.7);

  double pxgen,pygen,pzgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("Steps");

  tree1->SetBranchAddress("pxgen", &pxgen);
  tree1->SetBranchAddress("pygen", &pygen);
  tree1->SetBranchAddress("pzgen", &pzgen);
  tree1->SetBranchAddress("EdepEvent", &EdepEvent);
  tree1->SetBranchAddress("TrackLengthEvent", &TrackLengthEvent);
  tree1->SetBranchAddress("TrackLengthEventgammas", &TrackLengthEventgammas);

  tree2->SetBranchAddress("preXpos", &preXpos);
  tree2->SetBranchAddress("preYpos", &preYpos);
  tree2->SetBranchAddress("preZpos", &preZpos);
 

  int entries=tree1->GetEntries();
  cout<<entries<<endl;

  //Each entry is an event (a photon generated)
  for(unsigned long i=0;i<entries;i++)
    {
      tree1->GetEntry(i);

      if(EdepEvent!=0){
	htracklengthevent_e->Fill(TrackLengthEvent);
	htracklengthevent_phot->Fill(TrackLengthEventgammas);
	std::cout<<"Edep event: "<<EdepEvent<<" Track length phot: "<<TrackLengthEventgammas<<std::endl;
      }
    }


  int entries2=tree2->GetEntries();
  cout<<entries2<<endl;


  for(unsigned long i=0;i<entries2;i++)
    {
      tree2->GetEntry(i);
     
      //if(preZpos<20.395){
      //if((preZpos>20.395)&&(preZpos<21.225)){
      if((preZpos>21.225)&&(preZpos<28.775)){  
      //if(preZpos>28.775){ 
      //if((preZpos>28.36)&&(preZpos<28.775)){ 	
      //if((preZpos>21.225)&&(preZpos<21.4)){ 
      std::cout<<"zpos: "<<preZpos<<" xpos: "<<preXpos<<" ypos: "<<preYpos<<std::endl;
	hzx->Fill(preZpos,preXpos);
	hzy->Fill(preZpos,preYpos);
	hyx->Fill(preXpos,preYpos);
      }
  }

  TPaveStats *stat[4];

  c1->cd(1);
  htracklengthevent_phot->SetFillStyle(3001);
  htracklengthevent_phot->SetLineColor(kBlue);
  htracklengthevent_phot->SetLineWidth(1);
  htracklengthevent_phot->SetFillColor(kOrange+1);
  htracklengthevent_phot->Draw("");

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
  hyx->SetMarkerColor(kGray);
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
  hzy->SetMarkerColor(kGray);
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
  hzx->SetMarkerColor(kGray);
  hzx->Draw("");

  gPad->Update();

  /* stat[1] = (TPaveStats*)hzx->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.043);
  stat[1]->Draw("same");*/


}
