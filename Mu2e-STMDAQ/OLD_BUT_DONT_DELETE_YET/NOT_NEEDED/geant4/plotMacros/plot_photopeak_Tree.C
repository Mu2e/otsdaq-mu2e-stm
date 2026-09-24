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

void plot_photopeak_Tree() {

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 500);
  c1->Divide(2,1);

  //TFile *input= new TFile("/data1/cgarcia/Geant4_K/BlockGe_1460.8keVphotons_momsphere.root","read");
  //TFile *input= new TFile("/data1/cgarcia/Geant4_K/CylinderGe_1460.8keVphotons_momsphere.root","read");  
  //TFile *input= new TFile("/data1/cgarcia/Geant4_K/CylinderGe_1460.8keVphotons_momsphere_15cmlong.root","read");
  TFile *input= new TFile("/data1/cgarcia/Geant4_K/CylinderGe_1460.8keVphotons_momsphere_6cmlong.root","read");

  TH1F* hEdep = new TH1F("hEdep", "Total Energy deposited by electrons in each event", 100, 0, 1900);
  TH1F* htracklengthevent_phot = new TH1F("htracklengthevent_phot", "Total Track Length by photons in each event", 100, -1, 30);

  hEdep->GetYaxis()->SetTitleOffset(1.7);
  hEdep->GetYaxis()->SetLabelSize(0.04);
  hEdep->GetXaxis()->SetLabelSize(0.04);
  hEdep->GetYaxis()->SetTitleSize(0.04);
  hEdep->GetXaxis()->SetTitleSize(0.03);
  hEdep->GetXaxis()->SetTitle("E deposited in a cylinder of Ge (r_{ext}=4.5,r_{int}=0.88,z=6cm) [keV]");
  hEdep->GetYaxis()->SetTitle("Events");

  htracklengthevent_phot->GetYaxis()->SetTitleOffset(1.7);
  htracklengthevent_phot->GetYaxis()->SetLabelSize(0.04);
  htracklengthevent_phot->GetXaxis()->SetLabelSize(0.04);
  htracklengthevent_phot->GetYaxis()->SetTitleSize(0.04);
  htracklengthevent_phot->GetXaxis()->SetTitleSize(0.03);
  htracklengthevent_phot->GetXaxis()->SetTitle("Total #gamma Track Length in Ge detector [cm]");
  htracklengthevent_phot->GetYaxis()->SetTitle("Events");

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
      //photopeak
      //if((EdepEvent>=1449)&&(EdepEvent<=1450)){
      //outside photopeak
      //if((EdepEvent>0)&&(EdepEvent<1449)){
      //tracklength between 9.5 and 12 cm
      //if((TrackLengthEventgammas>=9.5)&&(TrackLengthEventgammas<=12)&&(EdepEvent>0)&&(EdepEvent<1449)){ 
	hEdep->Fill(EdepEvent);
	htracklengthevent_phot->Fill(TrackLengthEventgammas);
	std::cout<<"Edep event: "<<EdepEvent<<" Track length phot: "<<TrackLengthEventgammas<<std::endl;
      }
    }

  double x1=0;
  double x2=1400;
  double x3=1600;

  double hits_integral1 = hEdep->Integral(hEdep->FindFixBin(x1), hEdep->FindFixBin(x2), "");
  double hits_integral2 = hEdep->Integral(hEdep->FindFixBin(x2), hEdep->FindFixBin(x3), "");
  std::cout<<"Integral: "<<x1<<"-"<<x2<<": "<<hits_integral1<<std::endl;
  std::cout<<"Integral: "<<x2<<"-"<<x3<<": "<<hits_integral2<<std::endl;

  TPaveStats *stat[2];

  c1->cd(1);
  htracklengthevent_phot->SetFillStyle(3001);
  htracklengthevent_phot->SetLineColor(kBlue);
  htracklengthevent_phot->SetLineWidth(1);
  htracklengthevent_phot->SetFillColor(kOrange+1);
  htracklengthevent_phot->Draw("");

  gPad->Update();

  stat[0] = (TPaveStats*)htracklengthevent_phot->FindObject("stats"); 
  stat[0]->SetY1NDC(.74);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.45);
  stat[0]->SetTextSize(0.03);
  stat[0]->Draw("same");


  c1->cd(2);
  hEdep->SetFillStyle(3001);
  hEdep->SetLineColor(kBlue);
  hEdep->SetLineWidth(1);
  hEdep->SetFillColor(kCyan-3);
  hEdep->Draw("");

  gPad->Update();

  stat[1] = (TPaveStats*)hEdep->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.03);
  stat[1]->Draw("same");

}
