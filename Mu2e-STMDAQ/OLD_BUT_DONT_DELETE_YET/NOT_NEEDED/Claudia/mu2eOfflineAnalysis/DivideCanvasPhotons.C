#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cstdio>
#include<stdio.h>
#include<stdlib.h>
#include <iomanip>


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TPaveStats.h"

#define mm_to_cm 10
#define muonmass 105.65

//================================================================
//This program reads readST/treeST trees: Particles at Stopping Target
//Reads the root files from a txt and analyse the trees
//================================================================

void ReadRootFile(std::string rootinput,int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  double scale_POT=200000000;
  
  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);

  TCanvas *c1 ;
  if(branch==0){
    c1= new TCanvas("c1","c1",0,650,800,600);
  }
  if((branch==1)||(branch==2)){
    c1= new TCanvas("c1");
  }
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());


  int Nhisto = 7;
  TH1F*h[Nhisto];
  for(int i = 0; i<Nhisto; i++){
    h[i] = new TH1F("","", nbins, Xrange[0], Xrange[1]);
    h[i]->GetXaxis()->SetTitle(Xtitle);
    h[i]->GetYaxis()->SetTitle(Ytitle);
  }

  TPad *pad1 = new TPad("","",0,0.5,0.33,1);
  TPad *pad2 = new TPad("","",0.33,0.5,0.66,1);
  TPad *pad3 = new TPad("","",0.66,0.5,1,1);
  TPad *pad4 = new TPad("","",0,0,0.5,0.5);
  TPad *pad5 = new TPad("","",0.5,0,1,0.5);

  pad1->Draw();
  pad2->Draw();
  pad3->Draw();
  pad4->Draw();
  pad5->Draw();

  TFile *input=new TFile(rootinput.c_str());
  TTree* Eventtree=(TTree*)input->Get("Eventtree");
  TTree* Bremstree=(TTree*)input->Get("Bremstree");
  TTree* Muontree=(TTree*)input->Get("Muontree");
  TTree* Photontree=(TTree*)input->Get("Photontree");

  float time_event, energy_event, pdgID_event, pcode_event;
  float time_brems_e, energy_brems_e, pdgID_brems_e, pcode_brems_e, event_num_brems_e;
  float time_muon, energy_muon, pdgID_muon, pcode_muon;
  float time_photon, energy_photon, pdgID_photon, pcode_photon, trackID_photon, event_num_photon, edepstep_photon;


  Eventtree->SetBranchAddress("time_event",&time_event);
  Eventtree->SetBranchAddress("energy_event",&energy_event);
  Eventtree->SetBranchAddress("pdgID_event",&pdgID_event);
  Eventtree->SetBranchAddress("pcode_event",&pcode_event);

  Bremstree->SetBranchAddress("time_brems_e",&time_brems_e);
  Bremstree->SetBranchAddress("energy_brems_e",&energy_brems_e);
  Bremstree->SetBranchAddress("pdgID_brems_e",&pdgID_brems_e);
  Bremstree->SetBranchAddress("pcode_brems_e",&pcode_brems_e);
  Bremstree->SetBranchAddress("event_num_brems_e",&event_num_brems_e);

  Muontree->SetBranchAddress("time_muon",&time_muon);
  Muontree->SetBranchAddress("energy_muon",&energy_muon);
  Muontree->SetBranchAddress("pdgID_muon",&pdgID_muon);
  Muontree->SetBranchAddress("pcode_muon",&pcode_muon);

  Photontree->SetBranchAddress("event_num_photon",&event_num_photon);
  Photontree->SetBranchAddress("time_photon",&time_photon);
  Photontree->SetBranchAddress("energy_photon",&energy_photon);
  Photontree->SetBranchAddress("pdgID_photon",&pdgID_photon);
  Photontree->SetBranchAddress("pcode_photon",&pcode_photon);
  Photontree->SetBranchAddress("trackID_photon",&trackID_photon);
  //Photontree->SetBranchAddress("edepstep_photon",&edepstep_photon);

  double auxtrack_phot=200;
  double aux_event_num_photon=200;

  unsigned long entriesEvent=Eventtree->GetEntries();
  std::cout<<"Entries Event (starting with mu, e+- or photon): "<<entriesEvent<<std::endl;
  unsigned long entriesBrems=Bremstree->GetEntries();
  std::cout<<"Entries Brems hit ST (e+-): "<<entriesBrems<<std::endl;
  unsigned long entriesMuon=Muontree->GetEntries();
  std::cout<<"Entries Muon hit ST: "<<entriesMuon<<std::endl;
  unsigned long entriesPhoton=Photontree->GetEntries();
  std::cout<<"Entries Photon hit ST: "<<entriesPhoton<<std::endl;


  //Ranges
  double t1 = 50;
  double t2 = 100;
  double t3 = 150;
  double t4 = 200;
  double t5 = 250;
  double t6 = 300;

  for(unsigned long i=0;i<entriesEvent;i++){
    Eventtree->GetEntry(i);
    //if((i==2)||(i==16)||(i==17)||(i==18)||(i==25)||(i==40)){std::cout<<"Event: "<<i<<" Event sim particle: "<<pdgID_event<<std::endl;}
  }

  for(unsigned long i=0;i<entriesBrems;i++){
    Bremstree->GetEntry(i);
  }
  

  for(unsigned long i=0;i<entriesPhoton;i++){
    Photontree->GetEntry(i);
    //std::cout<<"Event ID: "<<event_num_photon<<" track id: "<<trackID_photon<<" E: "<<energy_photon<<" time: "<<time_photon<<" pcode_photon: "<<pcode_photon<<std::endl;
    //std::cout<<"edep: "<<edepstep_photon<<std::endl;
    if((trackID_photon!=auxtrack_phot)||(event_num_photon!=aux_event_num_photon)){
      double energy_photon_keV=energy_photon*1000;
      //std::cout<<"STORE: Brems photon in event: "<<event_num_photon<<" trackID: "<<trackID_photon<<" E: "<<energy_photon<<" time: "<<time_photon<<" pcode_photon: "<<pcode_photon<<std::endl;

      if((branch==0)||(branch==1)||(branch==2)){
	if(energy_photon_keV<Xrange[1]){
	  if((time_photon>=t1)&&(time_photon<t2)){
	    h[0]->Fill(energy_photon_keV);}
	  if((time_photon>=t2)&&(time_photon<t3)){
	    h[1]->Fill(energy_photon_keV);}
	  if((time_photon>=t3)&&(time_photon<t4)){
	    h[2]->Fill(energy_photon_keV);}
	  if((time_photon>=t4)&&(time_photon<t5)){
	    h[3]->Fill(energy_photon_keV);}
	  if((time_photon>=t5)&&(time_photon<t6)){
	    //std::cout<<"Energy: "<<energy_photon_keV<<" keV"<<std::endl;
	    h[4]->Fill(energy_photon_keV);}
	}//energyphotonkeV
	
	if((energy_photon>Xrange[0])&&(energy_photon<Xrange[1])){
	  //if(time_photon>t2){//MeV
	  h[5]->Fill(energy_photon);
	  //std::cout<<"Energy: "<<energy_photon<<" MeV, time: "<<time_photon<<" ns"<<std::endl;
	}
	 //}
	
	//if((energy_photon>1.75)&&(energy_photon<1.85)){
	//if((energy_photon>0.842)&&(energy_photon<0.8445)){   
	//if((energy_photon>0.334)&&(energy_photon<0.3354)){
	if((energy_photon>0.064)&&(energy_photon<0.068)){ 
	  //h[6]->Fill((time_photon/1000000000)/60);
	  h[6]->Fill(time_photon);
	  std::cout<<"Energy: "<<energy_photon<<" MeV, time: "<<time_photon<<" ns event: "<<event_num_photon<<std::endl;
	}//if energy photon
      }//branch
    }//if trackid
    auxtrack_phot=trackID_photon;
    aux_event_num_photon=event_num_photon;
  }//entries photon

  
  for(unsigned long i=0;i<entriesMuon;i++){
    Muontree->GetEntry(i);
  }

  TPaveStats *stat[Nhisto];

  if(branch==0){
    pad1->cd();
    h[0]->SetLineColor(kCyan+2);
    h[0]->SetFillColor(kCyan+2);
    h[0]->SetFillStyle(3001);
    h[0]->GetYaxis()->SetTitleOffset(1.65);
    h[0]->GetYaxis()->SetLabelSize(0.04);
    h[0]->Draw("");
    std::string str_latex1 = std::to_string(int(t1))+" < t < "+std::to_string(int(t2))+" ns";
    char* char_latex1 = const_cast<char*>(str_latex1.c_str());
    TLatex latex1;
    latex1.DrawLatexNDC(.3,.85,char_latex1);

    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetY1NDC(.64);
    stat[0]->SetY2NDC(.81);
    stat[0]->SetX1NDC(0.4); //new x start position
    stat[0]->SetX2NDC(0.85);
    stat[0]->SetTextColor(4);
    stat[0]->SetTextSize(0.04);
    stat[0]->Draw("same");


    pad2->cd();
    h[1]->SetLineColor(kCyan+2);
    h[1]->SetFillColor(kCyan+2);
    h[1]->SetFillStyle(3001);
    h[1]->GetYaxis()->SetTitleOffset(1.65);
    h[1]->GetYaxis()->SetLabelSize(0.04);
    h[1]->Draw("");
    std::string str_latex2 = std::to_string(int(t2))+" < t < "+std::to_string(int(t3))+" ns";
    char* char_latex2 = const_cast<char*>(str_latex2.c_str());
    TLatex latex2;
    latex2.DrawLatexNDC(.3,.85,char_latex2);

    gPad->Update();
    stat[1] = (TPaveStats*)h[1]->FindObject("stats");
    stat[1]->SetY1NDC(.64);
    stat[1]->SetY2NDC(.81);
    stat[1]->SetX1NDC(0.4); //new x start position
    stat[1]->SetX2NDC(0.85);
    stat[1]->SetTextColor(4);
    stat[1]->Draw("same");


    pad3->cd();
    h[2]->SetLineColor(kCyan+2);
    h[2]->SetFillColor(kCyan+2);
    h[2]->SetFillStyle(3001);
    h[2]->GetYaxis()->SetTitleOffset(1.71);
    h[2]->GetYaxis()->SetLabelSize(0.04);
    h[2]->Draw("");
    std::string str_latex3 = std::to_string(int(t3))+" < t < "+std::to_string(int(t4))+" ns";
    char* char_latex3 = const_cast<char*>(str_latex3.c_str());
    TLatex latex3;
    latex3.DrawLatexNDC(.3,.85,char_latex3);

    gPad->Update();
    stat[2] = (TPaveStats*)h[2]->FindObject("stats");
    stat[2]->SetY1NDC(.64);
    stat[2]->SetY2NDC(.81);
    stat[2]->SetX1NDC(0.4); //new x start position
    stat[2]->SetX2NDC(0.85);
    stat[2]->SetTextColor(4);
    stat[2]->Draw("same");


    pad4->cd();
    h[3]->SetLineColor(kCyan+2);
    h[3]->SetFillColor(kCyan+2);
    h[3]->SetFillStyle(3001);
    h[3]->GetYaxis()->SetTitleOffset(1.65);
    h[3]->GetYaxis()->SetLabelSize(0.04);
    h[3]->Draw("");
    std::string str_latex4 = std::to_string(int(t4))+" < t < "+std::to_string(int(t5))+" ns";
    char* char_latex4 = const_cast<char*>(str_latex4.c_str());
    TLatex latex4;
    latex4.DrawLatexNDC(.3,.85,char_latex4);

    gPad->Update();
    stat[3] = (TPaveStats*)h[3]->FindObject("stats");
    stat[3]->SetY1NDC(.64);
    stat[3]->SetY2NDC(.81);
    stat[3]->SetX1NDC(0.6); //new x start position
    stat[3]->SetX2NDC(0.85);
    stat[3]->SetTextColor(4);
    stat[3]->Draw("same");


    pad5->cd();
    h[4]->SetLineColor(kCyan+2);
    h[4]->SetFillColor(kCyan+2);
    h[4]->SetFillStyle(3001);
    h[4]->GetYaxis()->SetTitleOffset(1.65);
    h[4]->GetYaxis()->SetLabelSize(0.04);
    h[4]->Draw("");
    std::string str_latex5 = std::to_string(int(t5))+" < t < "+std::to_string(int(t6))+" ns";
    char* char_latex5 = const_cast<char*>(str_latex5.c_str());
    TLatex latex5;
    latex5.DrawLatexNDC(.3,.85,char_latex5);

    gPad->Update();
    stat[4] = (TPaveStats*)h[4]->FindObject("stats");
    stat[4]->SetY1NDC(.64);
    stat[4]->SetY2NDC(.81);
    stat[4]->SetX1NDC(0.6); //new x start position
    stat[4]->SetX2NDC(0.85);
    stat[4]->SetTextColor(4);
    stat[4]->Draw("same");

  }

  if(branch==1){
    h[5]->Scale(1./scale_POT);
    h[5]->SetLineColor(kCyan+2);
    h[5]->SetFillColor(kCyan+2);
    h[5]->SetFillStyle(3001);
    h[5]->GetYaxis()->SetTitleOffset(1.65);
    h[5]->GetYaxis()->SetRangeUser(0,0.00135);
    h[5]->Draw("HIST");
    std::string str_latex6 = "t > "+std::to_string(int(t2))+" ns";
    char* char_latex6 = const_cast<char*>(str_latex6.c_str());
    TLatex latex6;
    //latex6.DrawLatexNDC(.3,.85,char_latex6);

    //gPad->SetLogy();
 
    //gPad->Update();
    c1->Update();
    stat[5] = (TPaveStats*)c1->GetPrimitive("stats");
    stat[5]->SetX1NDC(0.71); //new x start position
    stat[5]->SetX2NDC(0.91); //new x end position
    stat[5]->SetY1NDC(0.76); //new y start position
    stat[5]->SetY2NDC(0.9); //new y end position
    stat[5]->SetTextSize(0.032);
    stat[5]->SetName("mystats");

    // the following line is needed to avoid that the automatic redrawing of stats
    h[5]->SetStats(0);
    stat[5]->GetLineWith("Entries")->SetTextColor(kBlue);
    c1->Modified();
    stat[5]->Draw("same");
  }

  
  if(branch==2){
    h[6]->Scale(1./scale_POT);
    h[6]->SetLineColor(kMagenta+2);
    h[6]->SetFillColor(kMagenta+2);
    h[6]->SetFillStyle(3001);
    h[6]->GetYaxis()->SetTitleOffset(1.65);
    h[6]->GetYaxis()->SetRangeUser(0,0.000002);
    h[6]->Draw("HIST");
    
    c1->Update();
    stat[6] = (TPaveStats*)c1->GetPrimitive("stats");
    stat[6]->SetX1NDC(0.71); //new x start position
    stat[6]->SetX2NDC(0.91); //new x end position
    stat[6]->SetY1NDC(0.76); //new y start position
    stat[6]->SetY2NDC(0.9); //new y end position
    stat[6]->SetTextSize(0.032);
    stat[6]->SetName("mystats");

    // the following line is needed to avoid that the automatic redrawing of stats
    h[6]->SetStats(0);
    stat[6]->GetLineWith("Entries")->SetTextColor(kBlue);
    c1->Modified();
    stat[6]->Draw("same");
  }



  //c1->Print("MuonBeam_Photons_t0ns_perPOT_E1.75_1.85MeV_ST.png");
  //c1->Print("MuonBeam_Photons_t0ns_perPOT_1809_range.png"); 
  //c1->Print("MuonBeam_Photons_t0ns_perPOT_500_520.pdf");
 
}



//================================================================
void DivideCanvasPhotons(std::string ArtFiles_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log");

  //Open txt
  /*  fstream readfile;
  readfile.open(ArtFiles_location,ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Read each art file from txt
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    std::cout<<name<<std::endl;
  }
  */

 
  //ReadRootFile(ArtFiles_location, 100, 0, 550, 0, 400, "E_{#gamma} [keV]", "Counts", 0);
  //ReadRootFile(ArtFiles_location, 100, 0.34, 0.35, 0, 400, "E_{#gamma} [MeV]", "Counts/POT", 1);
  //ReadRootFile(ArtFiles_location, 100, 0, 2000, 0, 400, "t_{(1.75<E_{#gamma}<1.85 MeV), ST} [ns]", "Counts/POT", 2);
  ReadRootFile(ArtFiles_location, 100, 0, 6000, 0, 400, "t_{(0.334<E_{#gamma}<0.3354 MeV), ST} [ns]", "Counts/POT", 2); 
  
  //readfile.close();
}

//================================================================
