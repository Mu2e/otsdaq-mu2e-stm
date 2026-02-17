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

void plotGenMomentumTree(std::string filename) {

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);

  auto c1= new TCanvas("c1");

  double Nphot_gen = 500000;
  //TFile *input= new TFile("/data1/cgarcia/Geant4_K/BlockGe_1460.8keVphotons_momsphere.root","read");
  //TFile *input= new TFile("/data1/cgarcia/Geant4_K/CylinderGe_1460.8keVphotons_momsphere.root","read");

  TFile *input= new TFile(filename.c_str(),"read");

  //TREES GENERATION POSITIONS AND MOMENTUM
  TH3D* hpxpypzgen=new TH3D("hpxpypzgen", "Momentum at Production; px; pz; py", 100, -1, 1, 100, -1, 1, 100, -1, 1);
  TH3D* hxyzgen=new TH3D("hxyzgen", "Position at Production; x [cm]; z [cm]; y [cm]", 100, -8, 8, 100, -8, 8, 100, -10, 10);
  //TH1D* hEtot=new TH1D("hEtot", "E deposited at the STM per event; E deposited in a cylinder of Ge (r_{ext}=4.5,r_{int}=0.88,z=10cm) [keV]; Events", 100, 0, 1900);
  TH1D* hEtot=new TH1D("hEtot", "E deposited at the STM per event; E deposited in HPGe [keV]; Events/X-ray", 100, 0, 1900); 
  TH2D* hyx = new TH2D("hyx", "xy position at Production", 1000, -30, 30, 1000, -30, 30);
  hyx->GetXaxis()->SetTitle("x [cm]");
  hyx->GetYaxis()->SetTitle("y [cm]");

  double pxgen,xgen;
  double pygen,ygen;
  double pzgen,zgen;
  double EdepEvent;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");

  tree1->SetBranchAddress("pxgen", &pxgen);
  tree1->SetBranchAddress("pygen", &pygen);
  tree1->SetBranchAddress("pzgen", &pzgen);
  tree1->SetBranchAddress("EdepEvent", &EdepEvent);
  tree1->SetBranchAddress("xgen", &xgen);
  tree1->SetBranchAddress("ygen", &ygen);
  tree1->SetBranchAddress("zgen", &zgen);

  int entries=tree1->GetEntries();
  cout<<entries<<endl;

  //Each entry is an event (a photon generated)
  for(unsigned long i=0;i<entries;i++)
    {
      tree1->GetEntry(i);

      hpxpypzgen->Fill(pxgen,pzgen,pygen);
      hxyzgen->Fill(xgen,zgen,ygen);
      hyx->Fill(xgen,ygen);
     
      if(EdepEvent!=0){
	hEtot->Fill(EdepEvent);
	std::cout<<"Edep: "<<EdepEvent<<" keV"<<std::endl;}
    }

  hpxpypzgen->SetMarkerColor(kCyan-3);
  hpxpypzgen->SetMarkerStyle(1);
  hpxpypzgen->GetXaxis()->SetLabelSize(0.04);
  hpxpypzgen->GetYaxis()->SetLabelSize(0.04);
  hpxpypzgen->GetZaxis()->SetLabelSize(0.04);
  //hpxpypzgen->Draw("");

  hxyzgen->SetMarkerColor(kOrange-3);
  hxyzgen->SetMarkerStyle(1);
  hxyzgen->GetXaxis()->SetLabelSize(0.04);
  hxyzgen->GetYaxis()->SetLabelSize(0.04);
  hxyzgen->GetZaxis()->SetLabelSize(0.04);
  //hxyzgen->Draw(""); 

  hEtot->Scale(1./Nphot_gen);
  hEtot->SetFillStyle(3001);
  hEtot->SetLineColor(kBlue);
  hEtot->SetLineWidth(1);
  hEtot->SetFillColor(kCyan-3);
  //hEtot->Draw("HIST");

  hyx->Draw("colz");
  
  double xmin = 1449;
  double xmax = 1450;
  double hits_integral = hEtot->Integral(hEtot->FindFixBin(xmin), hEtot->FindFixBin(xmax), "");
  std::cout<<"Integral between: "<<xmin<<" and "<<xmax<<": "<<hits_integral<<std::endl;

  
  std::string str_latex1 = "#splitline{1460.8 keV photons,}{detector starts at z=20.395 cm}";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.2,.6,char_latex1);

  gPad->Update();

  TPaveStats *stat[1];
  stat[0] = (TPaveStats*)hEtot->FindObject("stats"); 
  stat[0]->SetY1NDC(.74);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.45);
  //stat[0]->SetTextColor(4);
  stat[0]->SetTextSize(0.043);
  stat[0]->Draw("same");


  //c1->Print("EdepGe(0,0,-10)_fulldetector_2cylinders_photcyl_0.png");
}
