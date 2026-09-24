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

void plot_gen_Gedet_zxrot(std::string filename, std::string HPGe_rotation){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);
  gStyle->SetOptStat(0);

  //std::string HPGe_rotation = "40";
  
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

  std::cout<<"Energy: "<<energy<<std::endl;
  std::string plot_name = "10to6_"+filename.substr(pos1, diff)+"keV_099_1_geometry";

  std::string plot_name_png = plot_name+"_"+HPGe_rotation+"yrot_coll0.png";
  //std::string plot_name_pdf = plot_name+".pdf";

  char* imagename_png = const_cast<char*>(plot_name_png.c_str());
  //char* imagename_pdf = const_cast<char*>(plot_name_pdf.c_str());


  
  TH2D* hyx_gen = new TH2D("hyx_gen", "xy position of the hit in each step", 1000, -2, 3, 1000, -3, 3);
  hyx_gen->GetXaxis()->SetTitle("x_{gen #gamma collimator} [cm]");
  hyx_gen->GetYaxis()->SetTitle("y_{gen #gamma collimator} [cm]");
  TH2D* hyz_gen = new TH2D("hyz_gen", "zy position of the hit in each step", 1000, -12, 10, 1000, -10, 10);
  hyz_gen->GetXaxis()->SetTitle("z_{gen #gamma collimator} [cm]");
  hyz_gen->GetYaxis()->SetTitle("y_{gen #gamma collimator} [cm]");
  TH2D* hyx_stm = new TH2D("hyx_stm_nohole", "xy position of the hit in each step", 1000, -10, 10, 1000, -10, 10);
  hyx_stm->GetXaxis()->SetTitle("x_{#gamma STM} [cm]");
  hyx_stm->GetYaxis()->SetTitle("y_{#gamma STM} [cm]");
  TH2D* hxz_stm = new TH2D("hxz_stm", "xz position of the hit in each step", 1000, -12, 10, 1000, -10, 10);
  hxz_stm->GetXaxis()->SetTitle("z_{#gamma STM} [cm]");
  hxz_stm->GetYaxis()->SetTitle("x_{#gamma STM} [cm]");
  TH2D* hyz_stm = new TH2D("hyz_stm", "yz position of the hit in each step", 1000, -4, 10, 1000, -10, 10);
  hyz_stm->GetXaxis()->SetTitle("z_{#gamma STM} [cm]");
  hyz_stm->GetYaxis()->SetTitle("y_{#gamma STM} [cm]");
  TH2D* hxz_vd = new TH2D("hxz_vd", "xz position of the hit in each step", 1000, -12, 10, 1000, -10, 10);
  hxz_vd->GetXaxis()->SetTitle("z_{#gamma STM} [cm]");
  hxz_vd->GetYaxis()->SetTitle("x_{#gamma STM} [cm]");  
  TH2D* hyz_vd = new TH2D("hyz_vd", "yz position of the hit in each step", 1000, -4, 10, 1000, -10, 10);
  hyz_vd->GetXaxis()->SetTitle("z_{#gamma STM} [cm]");
  hyz_vd->GetYaxis()->SetTitle("y_{#gamma STM} [cm]");


  double pre_px,pre_py,pre_pz,pxgen,pygen,pzgen,xgen,ygen,zgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("StepsGe");
  TTree* tree3=(TTree*)input->Get("StepsVacuumVirtualPlane");
  
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

  tree3->SetBranchAddress("preXpos", &preXpos);
  tree3->SetBranchAddress("preYpos", &preYpos);
  tree3->SetBranchAddress("preZpos", &preZpos);
  tree3->SetBranchAddress("pre_px", &pre_px);
  tree3->SetBranchAddress("pre_py", &pre_py);
  tree3->SetBranchAddress("pre_pz", &pre_pz);
  tree3->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree3->SetBranchAddress("PDGID", &PDGID);
  tree3->SetBranchAddress("EventID", &EventID);
  tree3->SetBranchAddress("EdepStep", &EdepStep);

  int entries = tree1->GetEntries();
  cout<<entries<<endl;

  //Each entry is an event (a photon generated)
  for(unsigned long i=0; i<entries; i++)
    {
      tree1->GetEntry(i);
      hyx_gen->Fill(xgen, ygen);
      hyz_gen->Fill(zgen, ygen);
    }
  


  int entries2 = tree2->GetEntries();
  cout<<entries2<<endl;

  double nohle_limit = 0.831; //cm

  for(unsigned long i=0; i<entries2; i++)
    {
      tree2->GetEntry(i);
      hyx_stm->Fill(preXpos, preYpos);
     
      hxz_stm->Fill(preZpos, preXpos);
   
      hyz_stm->Fill(preZpos, preYpos);
    }

  
  int entries3 = tree3->GetEntries();
  cout<<entries3<<endl;

  double aux_EventID=1;
  
  for(unsigned long i=0; i<entries3; i++)
    {
      tree3->GetEntry(i);
      if(aux_EventID!=EventID) {
          hxz_vd->Fill(preZpos, preXpos);
	  hyz_vd->Fill(preZpos, preYpos);
	  aux_EventID=EventID;
      }
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
  hyx_stm->SetFillStyle(3001);
  hyx_stm->SetLineColor(kBlack);
  hyx_stm->SetLineWidth(1);
  hyx_stm->SetFillColor(kCyan-3);
  hyx_stm->Draw("HIST,SAMES");

  c1->cd(3);
  hxz_stm->SetFillStyle(3001);
  hxz_stm->SetLineColor(kBlack);
  hxz_stm->SetLineWidth(1);
  hxz_stm->SetFillColor(kCyan-3);
  hxz_stm->Draw("HIST,SAMES");

  hyz_gen->SetFillStyle(3001);
  hyz_gen->SetLineColor(kBlack);
  hyz_gen->SetLineWidth(1);
  hyz_gen->SetFillColor(kBlue);
  hyz_gen->Draw("HIST,SAMES");

  hxz_vd->SetFillStyle(3001);
  hxz_vd->SetLineColor(kBlack);
  hxz_vd->SetLineWidth(1);
  hxz_vd->SetFillColor(kBlue);
  hxz_vd->Draw("HIST,SAMES");
    
  std::string str_latex2 = "STM rotation angle: "+HPGe_rotation+"#circ";
  char* char_latex2 = const_cast<char*>(str_latex2.c_str());
  TLatex latex2;
  latex2.DrawLatexNDC(.2,.8,char_latex2);

  
  c1->cd(4);
  hyz_vd->SetFillStyle(3001);
  hyz_vd->SetLineColor(kBlack);
  hyz_vd->SetLineWidth(1);
  hyz_vd->SetFillColor(kCyan-3);
  hyz_vd->Draw("HIST,SAMES");
  
  hyz_stm->SetFillStyle(3001);
  hyz_stm->SetLineColor(kBlack);
  hyz_stm->SetLineWidth(1);
  hyz_stm->SetFillColor(kCyan-3);
  hyz_stm->Draw("HIST,SAMES");
  

  
  c1->Print(imagename_png);
  //c1->Print(imagename_pdf);

  
}
