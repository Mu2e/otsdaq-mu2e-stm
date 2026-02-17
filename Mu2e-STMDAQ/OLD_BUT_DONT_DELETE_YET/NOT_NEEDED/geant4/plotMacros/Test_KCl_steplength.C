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

void Test_KCl_steplength(std::string filename) {

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);

  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 500);
  c1->Divide(2,1);

  TFile* input = new TFile(filename.c_str(),"read");

  double xrange_low=0;
  double xrange_high=1900;

  TH1F* hEdep = new TH1F("hEdep", "Total Energy deposited by electrons in each event in KCl", 100, xrange_low, xrange_high);
  TH1F* htracklengthevent_phot = new TH1F("htracklengthevent_phot", "Total Track Length by photons in each event in KCl", 100, -1, 30);

  hEdep->GetYaxis()->SetTitleOffset(1.7);
  hEdep->GetYaxis()->SetLabelSize(0.04);
  hEdep->GetXaxis()->SetLabelSize(0.04);
  hEdep->GetYaxis()->SetTitleSize(0.04);
  hEdep->GetXaxis()->SetTitleSize(0.03);
  hEdep->GetXaxis()->SetTitle("Total E deposited by e+- in KCl [keV]");
  hEdep->GetYaxis()->SetTitle("Events");

  htracklengthevent_phot->GetYaxis()->SetTitleOffset(1.7);
  htracklengthevent_phot->GetYaxis()->SetLabelSize(0.04);
  htracklengthevent_phot->GetXaxis()->SetLabelSize(0.04);
  htracklengthevent_phot->GetYaxis()->SetTitleSize(0.04);
  htracklengthevent_phot->GetXaxis()->SetTitleSize(0.03);
  htracklengthevent_phot->GetXaxis()->SetTitle("Total #gamma Track Length in KCl [cm]");
  htracklengthevent_phot->GetYaxis()->SetTitle("Events");

  double preXpos,preYpos,preZpos,postXpos,postYpos,postZpos,StepLengthStep,EdepStep;
  int PDGID,EventID;

  std::vector<int> Event;

  //Leemos el TTree
  TTree* tree3=(TTree*)input->Get("StepsKCl");

  //KCl step
  tree3->SetBranchAddress("preXpos", &preXpos);
  tree3->SetBranchAddress("preYpos", &preYpos);
  tree3->SetBranchAddress("preZpos", &preZpos);
  tree3->SetBranchAddress("postXpos", &postXpos);
  tree3->SetBranchAddress("postYpos", &postYpos);
  tree3->SetBranchAddress("postZpos", &postZpos);
  tree3->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree3->SetBranchAddress("PDGID", &PDGID);
  tree3->SetBranchAddress("EventID", &EventID);
  tree3->SetBranchAddress("EdepStep", &EdepStep);



  int entries=tree3->GetEntries();
  cout<<entries<<endl;


  double aux1=0;
  double aux2=0;
  double edep_event=0;
  double tracklength_event=0;

  //fill Edep
  for(unsigned long i=0;i<entries;i++)
    {
      tree3->GetEntry(i);
      std:cout<<"EventID: "<<EventID<<" PDGID: "<<PDGID<<" EdepStep: "<<EdepStep<<std::endl;
  
      if(i==0){aux2=EventID;}
      aux1 = EventID;

      std::cout<<"aux1: "<<aux1<<" aux2: "<<aux2<<std::endl;

      if(aux2!=aux1){
	//if(edep_event!=0){
	if(edep_event==0){ 
	if((edep_event>=xrange_low)&&(edep_event<=xrange_high)){
	  hEdep->Fill(edep_event);
	  std::cout<<"Fill, Total Edep by e+e- in event "<<aux2<<": "<<edep_event<<std::endl;
	  Event.push_back(aux2);}
	}
	edep_event=0;
	aux2 =EventID;
      }


      if(((PDGID==11)||(PDGID==-11))&&(aux1==aux2)){
	edep_event=edep_event+EdepStep;  
	aux2 =EventID;
	std::cout<<"sum edep: "<<edep_event<<std::endl;
      }

    }

  std::cout<<"STEPLENGTH---"<<std::endl;

  double k=0;

  for(unsigned long i=0;i<entries;i++)
    {   
      tree3->GetEntry(i);
      std::cout<<"EventID: "<<EventID<<" PDGID: "<<PDGID<<" Step length: "<<StepLengthStep<<std::endl;
      if(k<Event.size()){
	std::cout<<" eventk: "<<Event.at(k)<<std::endl;
	if(EventID==Event.at(k)){
	  if(PDGID==22){
	    tracklength_event=tracklength_event+StepLengthStep;}
	}
	else if (EventID<Event.at(k)){continue;}
	else{
	  std::cout<<"Fill Total track length: "<<tracklength_event<<std::endl;
	  htracklengthevent_phot->Fill(tracklength_event);
	  tracklength_event=0;
	  k++;
	  if((k<Event.size())&&(EventID==Event.at(k))){tracklength_event=tracklength_event+StepLengthStep;}
	  }
	}
      }




  /*
  aux1=0;
  aux2=0;

  //Fill tracklength by photons per event and the positions of all particles in KCl
  for(unsigned long i=0;i<entries;i++)
    {
      tree3->GetEntry(i);
      //std:cout<<"EventID: "<<EventID<<" PDGID: "<<PDGID<<" Step length: "<<StepLengthStep<<std::endl;
      
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


  double x1=0;
  double x2=1400;
  double x3=1600;

  double hits_integral1 = hEdep->Integral(hEdep->FindFixBin(x1), hEdep->FindFixBin(x2), "");
  double hits_integral2 = hEdep->Integral(hEdep->FindFixBin(x2), hEdep->FindFixBin(x3), "");
  std::cout<<"Integral: "<<x1<<"-"<<x2<<": "<<hits_integral1<<std::endl;
  std::cout<<"Integral: "<<x2<<"-"<<x3<<": "<<hits_integral2<<std::endl;

  TPaveStats *stat[2];

  c1->cd(1);

  gPad->SetLogy();

  hEdep->SetFillStyle(3001);
  hEdep->SetLineColor(kBlack);
  hEdep->SetLineWidth(1);
  hEdep->SetFillColor(kGreen+1);
  hEdep->Draw("");

  gPad->Update();

  stat[1] = (TPaveStats*)hEdep->FindObject("stats");
  stat[1]->SetY1NDC(.74);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2);
  stat[1]->SetX2NDC(0.45);
  stat[1]->SetTextSize(0.03);
  stat[1]->Draw("same");


  c1->cd(2);

  gPad->SetLogy();

  htracklengthevent_phot->SetFillStyle(3001);
  htracklengthevent_phot->SetLineColor(kBlack);
  htracklengthevent_phot->SetLineWidth(1);
  htracklengthevent_phot->SetFillColor(kRed+1);
  htracklengthevent_phot->Draw("");

  gPad->Update();

  stat[0] = (TPaveStats*)htracklengthevent_phot->FindObject("stats"); 
  stat[0]->SetY1NDC(.74);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.45);
  stat[0]->SetTextSize(0.03);
  stat[0]->Draw("same");

}
