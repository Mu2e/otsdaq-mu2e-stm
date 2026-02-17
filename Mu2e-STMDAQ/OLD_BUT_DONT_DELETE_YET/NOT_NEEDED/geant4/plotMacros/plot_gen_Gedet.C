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

void plot_gen_Gedet(std::string filename){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);
  gStyle->SetOptStat(0);
 
  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);

  TFile* input = new TFile(filename.c_str(),"read");

  double Nphot_gen = 1000000;

  //Energy of the file
  
  int pos1 = filename.find("10to6_") + 6;
  
  int pos2 = filename.find("keV_");

  int diff = pos2-pos1;
  std::cout<<"pos1: "<<pos1<<" pos2: "<<pos2<<" diff: "<<diff<<std::endl;

  double energy = std::stod(filename.substr(pos1, diff));

  energy = energy*1000;
  
  std::cout<<"Energy: "<<energy<<" keV"<<std::endl;
  std::string plot_name = "10to6_"+filename.substr(pos1, diff)+"keV_099_1_geometry";

  std::string plot_name_png = plot_name+".png";
  std::string plot_name_pdf = plot_name+".pdf";

  char* imagename_png = const_cast<char*>(plot_name_png.c_str());
  char* imagename_pdf = const_cast<char*>(plot_name_pdf.c_str());


  
  TH2D* hyx_gen = new TH2D("hyx_gen", "xy position of the hit in each step", 1000, -2, 2, 1000, -2, 2);
  hyx_gen->GetXaxis()->SetTitle("x_{gen #gamma collimator} [cm]");
  hyx_gen->GetYaxis()->SetTitle("y_{gen #gamma collimator} [cm]");
  TH2D* hyx_stm_nohole = new TH2D("hyx_stm_nohole", "xy position of the hit in each step", 1000, -10, 10, 1000, -10, 10);
  hyx_stm_nohole->GetXaxis()->SetTitle("x_{#gamma STM, #Deltaz=0.88} [cm]");
  hyx_stm_nohole->GetYaxis()->SetTitle("y_{#gamma STM, #Deltaz=0.88} [cm]");
  TH2D* hyx_stm_hole = new TH2D("hyx_stm_hole", "xy position of the hit in each step", 1000, -10, 10, 1000, -10, 10);
  hyx_stm_hole->GetXaxis()->SetTitle("x_{#gamma STM, #Deltaz=6.97} [cm]");
  hyx_stm_hole->GetYaxis()->SetTitle("y_{#gamma STM, #Deltaz=6.97} [cm]");
  TH2D* hyz_stm = new TH2D("hyz_stm", "yz position of the hit in each step", 1000, -1, 10, 1000, -10, 10);
  hyz_stm->GetXaxis()->SetTitle("z_{#gamma STM} [cm]");
  hyz_stm->GetYaxis()->SetTitle("y_{#gamma STM} [cm]");
  


  double pre_px,pre_py,pre_pz,pxgen,pygen,pzgen,xgen,ygen,zgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("StepsGe");


  tree1->SetBranchAddress("pxgen", &pxgen);
  tree1->SetBranchAddress("pygen", &pygen);
  tree1->SetBranchAddress("pzgen", &pzgen);
  tree1->SetBranchAddress("EdepEvent", &EdepEvent);
  tree1->SetBranchAddress("xgen", &xgen);
  tree1->SetBranchAddress("ygen", &ygen);
  tree1->SetBranchAddress("zgen", &zgen);

  tree2->SetBranchAddress("preXpos", &preXpos);
  tree2->SetBranchAddress("preYpos", &preYpos);
  tree2->SetBranchAddress("preZpos", &preZpos);

  int entries = tree1->GetEntries();
  cout<<entries<<endl;

  //Each entry is an event (a photon generated)
  for(unsigned long i=0; i<entries; i++)
    {
      tree1->GetEntry(i);
      hyx_gen->Fill(xgen, ygen);
    }
  


  int entries2 = tree2->GetEntries();
  cout<<entries2<<endl;

  double nohole_limit = 0.8801; //cm

  for(unsigned long i=0; i<entries2; i++)
    {
      tree2->GetEntry(i);
      if(preZpos < -0.05) {std::cout<<"preXpos: "<<preXpos<<" preYpos: "<<preYpos<<" preZpos: "<<preZpos<<std::endl;}
      if(preZpos <= nohole_limit){
	hyx_stm_nohole->Fill(preXpos, preYpos);
	//std::cout<<"preZpos: "<<preZpos<<std::endl;
      }
      else{ hyx_stm_hole->Fill(preXpos, preYpos); }

      hyz_stm->Fill(preZpos, preYpos);
    }
 


  c1->cd(1);
  hyx_gen->SetFillStyle(3001);
  hyx_gen->SetLineColor(kBlack);
  hyx_gen->SetLineWidth(1);
  hyx_gen->SetFillColor(kBlue);
  hyx_gen->Draw("HIST");
 
  std::string str_latex1 = "E_{#gamma}="+std::to_string(int(energy))+" keV";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.2,.8,char_latex1);
  
  c1->cd(2);
  hyx_stm_nohole->SetFillStyle(3001);
  hyx_stm_nohole->SetLineColor(kBlack);
  hyx_stm_nohole->SetLineWidth(1);
  hyx_stm_nohole->SetFillColor(kCyan-3);
  hyx_stm_nohole->Draw("HIST,SAMES");

  c1->cd(3);
  hyx_stm_hole->SetFillStyle(3001);
  hyx_stm_hole->SetLineColor(kBlack);
  hyx_stm_hole->SetLineWidth(1);
  hyx_stm_hole->SetFillColor(kCyan-3);
  hyx_stm_hole->Draw("HIST,SAMES");

  
  c1->cd(4);
  hyz_stm->SetFillStyle(3001);
  hyz_stm->SetLineColor(kBlack);
  hyz_stm->SetLineWidth(1);
  hyz_stm->SetFillColor(kCyan-3);
  hyz_stm->Draw("HIST,SAMES");
  

  c1->Print("500keV_Aluminium_v08.png");
  //c1->Print(imagename_png);
  //c1->Print(imagename_pdf);

  
}
