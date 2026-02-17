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

void plotGenPositionsTree_VirtualPlane(std::string filename){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);

  double Nphot_gen = 500000;

  TFile* input = new TFile(filename.c_str(),"read");

  //Ge detector
  TH2D* hzx = new TH2D("hzx", "zx position of the hit in each step", 1000, 11, 15, 1000, -40, 40);
  hzx->GetXaxis()->SetTitle("z [cm]");
  hzx->GetYaxis()->SetTitle("x [cm]");
  TH2D* hzy = new TH2D("hzy", "zy position of the hit in each step", 1000, 11, 15, 1000, -40, 40);
  hzy->GetXaxis()->SetTitle("z [cm]");
  hzy->GetYaxis()->SetTitle("y [cm]");
  TH2D* hyx = new TH2D("hyx", "xy position of the hit in each step", 1000, -40, 40, 1000, -40, 40);
  hyx->GetXaxis()->SetTitle("x [cm]");
  hyx->GetYaxis()->SetTitle("y [cm]");
  

  TH1F* hE_phot = new TH1F("hE_phot", "E photon in event", 100, 0, 1900);
  
  hE_phot->GetXaxis()->SetTitle("E_{#gamma} in Virtual Vacuum Plane (z<13.4cm) [keV]");
  hE_phot->GetYaxis()->SetTitle("Events/X-ray");
  hE_phot->GetYaxis()->SetTitleOffset(1.7);


  double pre_px,pre_py,pre_pz;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("StepsGe");
  TTree* tree3=(TTree*)input->Get("StepsKCl");
  TTree* tree4=(TTree*)input->Get("StepsVacuumVirtualPlane");  

  //Vacuum plane step
  tree4->SetBranchAddress("preXpos", &preXpos);
  tree4->SetBranchAddress("preYpos", &preYpos);
  tree4->SetBranchAddress("preZpos", &preZpos);
  tree4->SetBranchAddress("pre_px", &pre_px);
  tree4->SetBranchAddress("pre_py", &pre_py);
  tree4->SetBranchAddress("pre_pz", &pre_pz);
  tree4->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree4->SetBranchAddress("PDGID", &PDGID);
  tree4->SetBranchAddress("EventID", &EventID);
  tree4->SetBranchAddress("EdepStep", &EdepStep);



  int entries4=tree4->GetEntries();
  cout<<entries4<<endl;

  double Ephoton_Energy;

  for(unsigned long i=0;i<entries4;i++)
    {
      tree4->GetEntry(i);
     
	hzx->Fill(preZpos,preXpos);
	hzy->Fill(preZpos,preYpos);
	hyx->Fill(preXpos,preYpos);

	//Just 1st plane
	//if((PDGID==22)&&(preZpos<20.2)){
	if((PDGID==22)&&(preZpos<13.4)){
	  Ephoton_Energy = sqrt(pre_px*pre_px+pre_py*pre_py+pre_pz*pre_pz)*1000;
	  //std::cout<<"photon energy: "<<photon_Energy<<std::endl;
	  hE_phot->Fill(Ephoton_Energy);
	}
    }
 

  double xmin = 1440;
  double xmax = 1470;
  double hits_integral = hE_phot->Integral(hE_phot->FindFixBin(xmin), hE_phot->FindFixBin(xmax), "");
  std::cout<<"Integral between: "<<xmin<<" and "<<xmax<<": "<<hits_integral<<std::endl;

 

  TPaveStats *stat[4];

  c1->cd(1);
  hE_phot->Scale(1./Nphot_gen);
  hE_phot->SetFillStyle(3001);
  hE_phot->SetLineColor(kBlue);
  hE_phot->SetLineWidth(1);
  hE_phot->SetFillColor(kBlue);
  hE_phot->GetXaxis()->SetLabelSize(0.04);
  hE_phot->GetYaxis()->SetLabelSize(0.04);
  hE_phot->Draw("HIST");

  gPad->SetLogy();

  gPad->Update();

  /*stat[0] = (TPaveStats*)hE_phot->FindObject("stats"); 
  stat[0]->SetY1NDC(.74);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.45);
  stat[0]->SetTextSize(0.043);
  stat[0]->Draw("same");
  */

  c1->cd(2);
  hyx->SetMarkerStyle(1);
  hyx->SetMarkerColor(kBlue);
  hyx->GetXaxis()->SetLabelSize(0.04);
  hyx->GetYaxis()->SetLabelSize(0.04);
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
  hzy->SetMarkerColor(kBlue);
  hzy->GetXaxis()->SetLabelSize(0.04);
  hzy->GetYaxis()->SetLabelSize(0.04);
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
  hzx->SetMarkerColor(kBlue);
  hzx->GetXaxis()->SetLabelSize(0.04);
  hzx->GetYaxis()->SetLabelSize(0.04);
  hzx->Draw("");

  gPad->Update();

  /* stat[1] = (TPaveStats*)hzx->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.043);
  stat[1]->Draw("same");*/

  //c1->Print("Particlesdet_VD_347keV_10cmpoly_diskinitial.pdf");
  //c1->Print("Particlesdet_VD_347keV_10cmpoly_diskinitial.png");
}
