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

void plotGedetector_VirtualPlane(std::string filename){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas();

  TFile* input = new TFile(filename.c_str(),"read");

  double Nphot_gen = 500000;
  
  //double xrange_low = 1449;
  //double xrange_high = 1462;
  //int nbins =50;

  double xrange_low = 0;
  double xrange_high = 1900;
  int nbins = 100;

  TH1F* hE_phot_virtual = new TH1F("hE_phot_virtual", "E photon in event", nbins, xrange_low, xrange_high);
  TH1D* hEtot_Ge = new TH1D("hEtot", "E deposited at the STM per event; E deposited in HPGe [keV]; Events", nbins, xrange_low, xrange_high);
  
  hE_phot_virtual->GetXaxis()->SetTitle("E_{#gamma} before HPGe (dark) and E deposited in HPGe (light) [keV]");
  hE_phot_virtual->GetYaxis()->SetTitle("Events/X-ray");
  //hE_phot_virtual->GetYaxis()->SetTitle("Events");  
  hE_phot_virtual->GetYaxis()->SetTitleOffset(1.7);


  double pre_px,pre_py,pre_pz,pxgen,pygen,pzgen,xgen,ygen,zgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("StepsGe");
  TTree* tree3=(TTree*)input->Get("StepsKCl");
  TTree* tree4=(TTree*)input->Get("StepsVacuumVirtualPlane");  


  tree1->SetBranchAddress("pxgen", &pxgen);
  tree1->SetBranchAddress("pygen", &pygen);
  tree1->SetBranchAddress("pzgen", &pzgen);
  tree1->SetBranchAddress("EdepEvent", &EdepEvent);
  tree1->SetBranchAddress("xgen", &xgen);
  tree1->SetBranchAddress("ygen", &ygen);
  tree1->SetBranchAddress("zgen", &zgen);


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


  int entries=tree1->GetEntries();
  cout<<entries<<endl;

  //Each entry is an event (a photon generated)
  for(unsigned long i=0;i<entries;i++)
    {
      tree1->GetEntry(i);
      //if(EdepEvent!=0){
	if((EdepEvent>=xrange_low)&&(EdepEvent<=xrange_high)){
	  hEtot_Ge->Fill(EdepEvent);
	  }
	//std::cout<<"Edep: "<<EdepEvent<<" keV"<<std::endl;}
    }



  int entries4=tree4->GetEntries();
  cout<<entries4<<endl;

  double Ephoton_Energy;

  for(unsigned long i=0;i<entries4;i++)
    {
      tree4->GetEntry(i);
	//Just 1st plane
	if((PDGID==22)&&(preZpos<20.2)){
	  Ephoton_Energy = sqrt(pre_px*pre_px+pre_py*pre_py+pre_pz*pre_pz)*1000;
	  if((Ephoton_Energy>=xrange_low)&&(Ephoton_Energy<=xrange_high)){
	  //std::cout<<"photon energy: "<<photon_Energy<<std::endl;
	  hE_phot_virtual->Fill(Ephoton_Energy);
	  }
	}
    }
 

  double xmin = 1440;
  double xmax = 1470;
  double hits_integral = hE_phot_virtual->Integral(hE_phot_virtual->FindFixBin(xmin), hE_phot_virtual->FindFixBin(xmax), "");
  std::cout<<"Integral between: "<<xmin<<" and "<<xmax<<": "<<hits_integral<<std::endl;

  xmin = 1449;
  xmax = 1450;
  hits_integral = hEtot_Ge->Integral(hEtot_Ge->FindFixBin(xmin), hEtot_Ge->FindFixBin(xmax), "");
  std::cout<<"Integral between: "<<xmin<<" and "<<xmax<<": "<<hits_integral<<std::endl;


  TPaveStats *stat[2];
  hE_phot_virtual->Scale(1./Nphot_gen);
  hE_phot_virtual->SetFillStyle(3001);
  hE_phot_virtual->SetLineColor(kBlack);
  hE_phot_virtual->SetLineWidth(1);
  hE_phot_virtual->SetFillColor(kBlue);
  hE_phot_virtual->Draw("HIST");
  //hE_phot_virtual->Draw("");
 
  gPad->Update();

  stat[0] = (TPaveStats*)hE_phot_virtual->FindObject("stats"); 
  stat[0]->SetY1NDC(.74);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.45);
  stat[0]->SetTextSize(0.043);
  stat[0]->SetTextColor(kBlue);
  stat[0]->Draw("same");
  
  hEtot_Ge->Scale(1./Nphot_gen);
  hEtot_Ge->SetFillStyle(3001);
  hEtot_Ge->SetLineColor(kBlack);
  hEtot_Ge->SetLineWidth(1);
  hEtot_Ge->SetFillColor(kCyan-3);
  hEtot_Ge->Draw("HIST,SAMES");
  //hEtot_Ge->Draw("SAMES");

  gPad->Update();

  stat[1] = (TPaveStats*)hEtot_Ge->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2); //new x start position
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.043);
  stat[1]->SetTextColor(kCyan-3);
  stat[1]->Draw("same");

  gPad->SetLogy();

  //c1->Print("Photopeak_before_after_HPGe_cylVirtual_KCl(0,0,-10)_full_0.png");
  //c1->Print("Photopeak_before_after_HPGe_cylVirtual_KCl(0,0,-10)_0.pdf");
}
