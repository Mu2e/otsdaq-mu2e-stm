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

void plotEdep_KCl(std::string filename){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1100);

  TCanvas* c1 = new TCanvas();

  TFile* input = new TFile(filename.c_str(),"read");

  double Nphot_gen=500000;

  double xrange_low = 0;
  double xrange_high = 1900;
  //int nbins =50;
  int nbins =100; 
  TH1F* hEtot = new TH1F("hEtot", "Total Edep by e+e- in each event", nbins, xrange_low, xrange_high);

  hEtot->GetXaxis()->SetTitle("E deposited in KCl [keV]");
  hEtot->GetYaxis()->SetTitle("Events/X-ray");
  hEtot->GetYaxis()->SetTitleOffset(1.7);

  double pxgen,pygen,pzgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  TTree* tree2=(TTree*)input->Get("StepsGe");
  TTree* tree3=(TTree*)input->Get("StepsKCl");

  
  //KCl step
  tree3->SetBranchAddress("preXpos", &preXpos);
  tree3->SetBranchAddress("preYpos", &preYpos);
  tree3->SetBranchAddress("preZpos", &preZpos);
  tree3->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree3->SetBranchAddress("PDGID", &PDGID);
  tree3->SetBranchAddress("EventID", &EventID);
  tree3->SetBranchAddress("EdepStep", &EdepStep);


  int entries3=tree3->GetEntries();
  cout<<entries3<<endl;

  double aux1=0;
  double aux2=0;
  double edep_event=0;

  for(unsigned long i=0;i<entries3;i++)
    {
      tree3->GetEntry(i);
      //std:cout<<"EventID: "<<EventID<<" PDGID: "<<PDGID<<" EdepStep: "<<EdepStep<<std::endl;
      
  
      if(i==0){aux2=EventID;}
      aux1 = EventID;

      //std::cout<<"aux1: "<<aux1<<" aux2: "<<aux2<<std::endl;

      if(aux2!=aux1){
	if(edep_event!=0){
	  if((edep_event>=xrange_low)&&(edep_event<=xrange_high)){
	    hEtot->Fill(edep_event);}
	  }
	//std::cout<<"Total Edep by e+e- in event "<<aux2<<": "<<edep_event<<std::endl;
	edep_event=0;
	aux2 =EventID;
      }


      if(((PDGID==11)||(PDGID==-11))&&(aux1==aux2)){
	edep_event=edep_event+EdepStep;  
	aux2 =EventID;
	//std::cout<<"sum edep: "<<edep_event<<std::endl;
      }

    }


  hEtot->Scale(1./Nphot_gen);

  hEtot->SetFillStyle(3001);
  hEtot->SetLineColor(kBlack);
  hEtot->SetLineWidth(1);
  hEtot->SetFillColor(kGreen-3);
  hEtot->Draw("HIST");


  double xmin = 1440;
  double xmax = 1470;
  double hits_integral = hEtot->Integral(hEtot->FindFixBin(xmin), hEtot->FindFixBin(xmax), "");
  std::cout<<"Integral between: "<<xmin<<" and "<<xmax<<": "<<hits_integral<<std::endl;


  std::string str_latex1 = "#splitline{1460.8 keV photons }{generated at KCl cylinder}";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.2,.65,char_latex1);

  gPad->SetLogy();

  gPad->Update();

  TPaveStats *stat[1];
  stat[0] = (TPaveStats*)hEtot->FindObject("stats");
  stat[0]->SetY1NDC(.74);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.45);
  stat[0]->SetTextSize(0.043);
  //stat[0]->SetTextColor(kGreen-3);
  stat[0]->SetTextColor(kBlack);
  stat[0]->Draw("same");

  //c1->Print("EdepKCl(0,0,0)_fulldetector_2cylinders_photcyl_0_not0events.png");
}
