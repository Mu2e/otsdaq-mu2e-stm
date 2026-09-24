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

void plotGenPositionsTree_KCl_Ge(std::string filename){

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
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("StepsGe");
  TTree* tree3=(TTree*)input->Get("StepsKCl");

  tree1->SetBranchAddress("pxgen", &pxgen);
  tree1->SetBranchAddress("pygen", &pygen);
  tree1->SetBranchAddress("pzgen", &pzgen);
  tree1->SetBranchAddress("EdepEvent", &EdepEvent);
  tree1->SetBranchAddress("TrackLengthEvent", &TrackLengthEvent);
  tree1->SetBranchAddress("TrackLengthEventgammas", &TrackLengthEventgammas);

  //HPGe step
  tree2->SetBranchAddress("preXpos", &preXpos);
  tree2->SetBranchAddress("preYpos", &preYpos);
  tree2->SetBranchAddress("preZpos", &preZpos);
  
  //KCl step
  //tree3->SetBranchAddress("preXpos", &preXpos);
  //tree3->SetBranchAddress("preYpos", &preYpos);
  //tree3->SetBranchAddress("preZpos", &preZpos);
  //tree3->SetBranchAddress("StepLengthStep", &StepLengthStep);
  //tree3->SetBranchAddress("PDGID", &PDGID);
  //tree3->SetBranchAddress("EventID", &EventID);
  //tree3->SetBranchAddress("EdepStep", &EdepStep);

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
      if((preZpos>20.395)&&(preZpos<21.225)){
      //if((preZpos>21.225)&&(preZpos<28.775)){  
      //if(preZpos>28.775){ 
      //if((preZpos>28.36)&&(preZpos<28.775)){ 	
      //if((preZpos>21.22)&&(preZpos<21.4)){ 
      //std::cout<<"zpos: "<<preZpos<<" xpos: "<<preXpos<<" ypos: "<<preYpos<<std::endl;
	hzx->Fill(preZpos,preXpos);
	hzy->Fill(preZpos,preYpos);
	hyx->Fill(preXpos,preYpos);
	}
    }
 

  /*  int entries3=tree3->GetEntries();
  cout<<entries3<<endl;

  double aux1=0;
  double aux2=0;
  double tracklength_event=0;
  double edep_event=0;

  //Fill tracklength by photons per event and the positions of all particles in KCl
  for(unsigned long i=0;i<entries3;i++)
    {
      tree3->GetEntry(i);
      //std:cout<<"EventID: "<<EventID<<" PDGID: "<<PDGID<<" Step length: "<<StepLengthStep<<std::endl;
      
      hzx->Fill(preZpos,preXpos);
      hzy->Fill(preZpos,preYpos);
      hyx->Fill(preXpos,preYpos);

      if(i==0){aux2=EventID;}
      aux1 = EventID;

      //std::cout<<"aux1: "<<aux1<<" aux2: "<<aux2<<std::endl;

      if(aux2!=aux1){
        htracklengthevent_phot->Fill(tracklength_event);
	//std::cout<<"Total track length by photons: "<<tracklength_event<<std::endl;
	tracklength_event=0;
	aux2 =EventID;
      }


      if((PDGID==22)&&(aux1==aux2)){
	tracklength_event=tracklength_event+StepLengthStep;  
	aux2 =EventID;
	//std::cout<<"sum step: "<<tracklength_event<<std::endl;
      }

    }
 
  */

  TPaveStats *stat[4];

  c1->cd(1);
  htracklengthevent_phot->Scale(1./Nphot_gen);
  htracklengthevent_phot->SetFillStyle(3001);
  htracklengthevent_phot->SetLineColor(kOrange+1);
  htracklengthevent_phot->SetLineWidth(1);
  htracklengthevent_phot->SetFillColor(kOrange+1);
  htracklengthevent_phot->Draw("HIST");

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



  c1->Print("HitsGe-20.395-21.225detector_photcyl_KCl(0,0,-10)_0.png");
}
