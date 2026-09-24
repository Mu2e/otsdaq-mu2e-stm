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

void plotGen_AlWindow_VD(std::string filename){

  double Nphot_gen = 1000000;
  
  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);


  TFile* input = new TFile(filename.c_str(),"read");


  //Gen
   TH2D* hyx_gen = new TH2D("hyx", "xy position at Production", 1000, -1, 1, 1000, -1, 1);
   hyx_gen->GetXaxis()->SetTitle("x_{#gamma} (generated 347 keV) [cm]");
   hyx_gen->GetYaxis()->SetTitle("y_{#gamma} (generated 347 keV) [cm]");

  //VD
  TH2D* hyx_VD = new TH2D("hyx_VD", "xy position of the hit in each step VD", 1000, -10, 10, 1000, -10, 10);
  hyx_VD->GetXaxis()->SetTitle("x_{#gamma} (detected VD) [cm]");
  hyx_VD->GetYaxis()->SetTitle("y_{#gamma} (detected VD) [cm]");
  
  TH1F* hE_phot = new TH1F("hE_phot", "E photon in event", 100, 0, 100);
  hE_phot->GetXaxis()->SetTitle("p_{#gamma} in Vacuum VD [keV]");
  hE_phot->GetYaxis()->SetTitle("Events/#gamma generated");
  hE_phot->GetYaxis()->SetTitleOffset(1.7);

  //Al windows
  TH2D* hzy = new TH2D("hzy", "zy position of the hit in each step Al Windows", 1000, -5, -4.5, 1000, -10, 10);
  hzy->GetXaxis()->SetTitle("z_{#gamma} (Al Windows) [cm]");
  hzy->GetYaxis()->SetTitle("y_{#gamma} (Al Windows) [cm]");
  TH3D* hgeom=new TH3D("hgeom", "Al Windows geom; x; z; y", 100, -10, 10, 100, 1.4, 19.6, 100, -10, 10);
 
  double pre_px,pre_py,pre_pz;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;
  double pxgen,xgen;
  double pygen,ygen;
  double pzgen,zgen;
 
  
  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("StepsVacuumVirtualPlane");  
  TTree* tree3=(TTree*)input->Get("StepsAlWindow");

  //Al window step
  tree3->SetBranchAddress("preXpos", &preXpos);
  tree3->SetBranchAddress("preYpos", &preYpos);
  tree3->SetBranchAddress("preZpos", &preZpos);
  tree3->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree3->SetBranchAddress("PDGID", &PDGID);
  tree3->SetBranchAddress("EventID", &EventID);
  tree3->SetBranchAddress("EdepStep", &EdepStep);
  
  //Production tree
  tree1->SetBranchAddress("pxgen", &pxgen);
  tree1->SetBranchAddress("pygen", &pygen);
  tree1->SetBranchAddress("pzgen", &pzgen);
  tree1->SetBranchAddress("EdepEvent", &EdepEvent);
  tree1->SetBranchAddress("xgen", &xgen);
  tree1->SetBranchAddress("ygen", &ygen);
  tree1->SetBranchAddress("zgen", &zgen);
  
  //Vacuum plane step
  tree2->SetBranchAddress("preXpos", &preXpos);
  tree2->SetBranchAddress("preYpos", &preYpos);
  tree2->SetBranchAddress("preZpos", &preZpos);
  tree2->SetBranchAddress("pre_px", &pre_px);
  tree2->SetBranchAddress("pre_py", &pre_py);
  tree2->SetBranchAddress("pre_pz", &pre_pz);
  tree2->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree2->SetBranchAddress("PDGID", &PDGID);
  tree2->SetBranchAddress("EventID", &EventID);
  tree2->SetBranchAddress("EdepStep", &EdepStep);



  int entries1=tree1->GetEntries();
  int entries2=tree2->GetEntries();
  int entries3=tree3->GetEntries();

  double aux_EventID;
  
  double mom;

  for(unsigned long i=0;i<entries1;i++)
    {
      tree1->GetEntry(i);
      hyx_gen->Fill(xgen, ygen);
      //std::cout<<"px gen: "<<pxgen<<" py gen: "<<pygen<<" pz gen: "<<pzgen<<std::endl;
    }

  for(unsigned long i=0;i<entries2;i++)
    {
      tree2->GetEntry(i);

      if(i==0){aux_EventID = aux_EventID+1;}

      //Just 1st plane
      //if((PDGID==22)&&(preZpos<10.3)){
      
      //std::cout<<"PDGID "<<PDGID<<" preXpos: "<<preXpos<<" preYpos: "<<preYpos<<" preZpos: "<<preZpos<<" EventID: "<<EventID<<std::endl;
      //std::cout<<"Mom: pre_px: "<<pre_px<<" pre_py: "<<pre_py<<" pre_pz: "<<pre_pz<<std::endl;

      if((PDGID==22)&&(aux_EventID!=EventID)&&(pre_pz>0)) {
      //if((PDGID==11)&&(aux_EventID!=EventID)&&(pre_pz>0)) { 
	mom = sqrt(pre_px*pre_px+pre_py*pre_py+pre_pz*pre_pz)*1000;
	//std::cout<<"mom: "<<mom<<std::endl;
	hE_phot->Fill(mom);
	hyx_VD->Fill(preXpos,preYpos);
	aux_EventID=EventID;
      }
    }
 

  for(unsigned long i=0;i<entries3;i++)
    {
      tree3->GetEntry(i);
      hzy->Fill(preZpos,preYpos);
      hgeom->Fill(preXpos,preZpos,preYpos);
      
    }


  
  TPaveStats *stat[4];

  c1->cd(1);
  hyx_gen->SetMarkerColor(kBlue);
  hyx_gen->SetMarkerStyle(1);
  hyx_gen->SetFillColor(kBlue);
  hyx_gen->GetXaxis()->SetLabelSize(0.04);
  hyx_gen->GetYaxis()->SetLabelSize(0.04);
  hyx_gen->Draw("");

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
  hzy->SetMarkerStyle(1);
  hzy->SetMarkerColor(kBlue);
  hzy->GetXaxis()->SetLabelSize(0.04);
  hzy->GetYaxis()->SetLabelSize(0.04);
  hzy->Draw("");
  
  //hgeom->SetMarkerStyle(1);
  //hgeom->SetMarkerColor(kBlue);
  /*hgeom->GetXaxis()->SetLabelSize(0.04);
  hgeom->GetYaxis()->SetLabelSize(0.04);
  hgeom->Draw("");
  */
  gPad->Update();

  /*stat[1] = (TPaveStats*)hyx->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.043);
  stat[1]->Draw("same");*/

  c1->cd(3);
  hE_phot->SetFillStyle(3001);
  hE_phot->Scale(1./Nphot_gen);
  hE_phot->SetMarkerStyle(1);
  hE_phot->SetMarkerColor(kBlue);
  hE_phot->SetLineColor(kBlue);
  hE_phot->SetFillColor(kBlue);
  hE_phot->SetLineWidth(1);
  hE_phot->GetXaxis()->SetLabelSize(0.04);
  hE_phot->GetYaxis()->SetLabelSize(0.04);
  hE_phot->Draw("HIST");
  gPad->SetLogy();
  gPad->Update();

  /*stat[2] = (TPaveStats*)hzy->FindObject("stats");
  stat[2]->SetY1NDC(.74);
  stat[2]->SetY2NDC(.91);
  stat[2]->SetX1NDC(0.2);
  stat[2]->SetX2NDC(0.45);
  stat[2]->SetTextSize(0.043);
  stat[2]->Draw("same");*/

  c1->cd(4);
  hyx_VD->SetMarkerStyle(1);
  hyx_VD->SetMarkerColor(kBlue);
  hyx_VD->GetXaxis()->SetLabelSize(0.04);
  hyx_VD->GetYaxis()->SetLabelSize(0.04);
  hyx_VD->Draw("");

  gPad->Update();

  /* stat[1] = (TPaveStats*)hzx->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.043);
  stat[1]->Draw("same");*/

  //c1->Print("Particlesdet_VD_694keV_10cmpoly_diskinitial_nostats.pdf");
  //c1->Print("Particlesdet_VD_694keV_10cmpoly_diskinitial_nostats.png");
}
