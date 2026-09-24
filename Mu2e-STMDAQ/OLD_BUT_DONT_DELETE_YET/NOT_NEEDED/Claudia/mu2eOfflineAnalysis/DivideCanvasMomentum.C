#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
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
#define PI 3.14159265358979323846  /* pi */

//================================================================
//This program reads readST/treeST trees: Particles at Stopping Target
//Reads the root files from a txt and analyse the trees
//================================================================

void ReadRootFile(std::string rootinput,int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);

  std::fstream writefile;
  bool write_to_file=false;
  if(write_to_file==true){
    //writefile.open("BremsstrahlungElectrons_mompos_ST.txt",std::ios::out);
    writefile.open("AllPhotons_mompos_ST.txt",std::ios::out);
  }

  //For long canvas
  //TCanvas *c1 = new TCanvas("c1","c1",0,650,1400,400);
  //Default
  TCanvas *c1 = new TCanvas("c1");

  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());


  int Nhisto = 4;
  TH1F*h[Nhisto];
  for(int i = 0; i<Nhisto; i++){
    h[i] = new TH1F("","", nbins, Xrange[0], Xrange[1]);
    h[i]->GetYaxis()->SetTitle(Ytitle);
  }
  if(branch==0){
    h[0]->GetXaxis()->SetTitle("p_{x, #gamma} at ST [keV]");
    h[1]->GetXaxis()->SetTitle("p_{y, #gamma} at ST [keV]");
    h[2]->GetXaxis()->SetTitle("p_{z, #gamma} at ST [keV]");
    h[3]->GetXaxis()->SetTitle("p_{#gamma} at ST [keV]");
  }

  if(branch==1){
    h[0]->GetXaxis()->SetTitle("p_{x, e^{#pm}} bremsstrahlung at ST [keV]");
    h[1]->GetXaxis()->SetTitle("p_{y, e^{#pm}} bremsstrahlung at ST [keV]");
    h[2]->GetXaxis()->SetTitle("p_{z, e^{#pm}} bremsstrahlung at ST [keV]");
    h[3]->GetXaxis()->SetTitle("p_{e^{#pm}} bremsstrahlung at ST [keV]");
  }

  if(branch==2){
    h[0]->GetXaxis()->SetTitle("p_{x, e^{+}} annihilation at ST [keV]");
    h[1]->GetXaxis()->SetTitle("p_{y, e^{+}} annihilation at ST [keV]");
    h[2]->GetXaxis()->SetTitle("p_{z, e^{+}} annihilation at ST [keV]");
    h[3]->GetXaxis()->SetTitle("p_{e^{+}} annihilation at ST [keV]");
  }

  if(branch==3){
    h[0]->GetXaxis()->SetTitle(Xtitle);
  }
  if(branch==4){
    h[0]->GetXaxis()->SetTitle(Xtitle);
  }
  if(branch==5){
    h[0]->GetXaxis()->SetTitle(Xtitle);
  }
  if(branch==6){
    h[0]->GetXaxis()->SetTitle(Xtitle);
  }
  if(branch==7){
    h[0]->GetXaxis()->SetTitle(Xtitle);
  }
  TPad *pad1 = new TPad("","",0,0,0.25,1);
  TPad *pad2 = new TPad("","",0.25,0,0.5,1);
  TPad *pad3 = new TPad("","",0.5,0,0.75,1);
  TPad *pad4 = new TPad("","",0.75,0,1,1);

  pad1->Draw();
  pad2->Draw();
  pad3->Draw();
  pad4->Draw();


  TFile *input=new TFile(rootinput.c_str());
  TTree* Eventtree=(TTree*)input->Get("Eventtree");
  TTree* Bremstree=(TTree*)input->Get("Bremstree");
  TTree* Annihtree=(TTree*)input->Get("Annihtree");
  TTree* Muontree=(TTree*)input->Get("Muontree");
  TTree* Photontree=(TTree*)input->Get("Photontree");

  float time_event, energy_event, pdgID_event, pcode_event, px_event, py_event, pz_event, p_event, x_event, y_event, z_event;
  float time_brems_e, energy_brems_e, pdgID_brems_e, pcode_brems_e, event_num_brems_e, px_brems_e, py_brems_e, pz_brems_e, p_brems_e, x_brems_e, y_brems_e, z_brems_e;
  float time_annih_e, energy_annih_e, pdgID_annih_e, pcode_annih_e, event_num_annih_e, px_annih_e, py_annih_e, pz_annih_e, p_annih_e, x_annih_e, y_annih_e, z_annih_e;
  float time_muon, energy_muon, pdgID_muon, pcode_muon;
  float time_photon, energy_photon, pdgID_photon, pcode_photon, trackID_photon, event_num_photon, px_photon, py_photon, pz_photon, p_photon, x_photon, y_photon, z_photon;


  Eventtree->SetBranchAddress("time_event",&time_event);
  Eventtree->SetBranchAddress("energy_event",&energy_event);
  Eventtree->SetBranchAddress("pdgID_event",&pdgID_event);
  Eventtree->SetBranchAddress("pcode_event",&pcode_event);
  Eventtree->SetBranchAddress("px_event",&px_event);
  Eventtree->SetBranchAddress("py_event",&py_event);
  Eventtree->SetBranchAddress("pz_event",&pz_event);
  Eventtree->SetBranchAddress("p_event",&p_event);
  Eventtree->SetBranchAddress("x_event",&x_event);
  Eventtree->SetBranchAddress("y_event",&y_event);
  Eventtree->SetBranchAddress("z_event",&z_event);


  Bremstree->SetBranchAddress("time_brems_e",&time_brems_e);
  Bremstree->SetBranchAddress("energy_brems_e",&energy_brems_e);
  Bremstree->SetBranchAddress("pdgID_brems_e",&pdgID_brems_e);
  Bremstree->SetBranchAddress("pcode_brems_e",&pcode_brems_e);
  Bremstree->SetBranchAddress("event_num_brems_e",&event_num_brems_e);
  Bremstree->SetBranchAddress("px_brems_e",&px_brems_e);
  Bremstree->SetBranchAddress("py_brems_e",&py_brems_e);
  Bremstree->SetBranchAddress("pz_brems_e",&pz_brems_e);
  Bremstree->SetBranchAddress("p_brems_e",&p_brems_e);
  Bremstree->SetBranchAddress("x_brems_e",&x_brems_e);
  Bremstree->SetBranchAddress("y_brems_e",&y_brems_e);
  Bremstree->SetBranchAddress("z_brems_e",&z_brems_e);

  Annihtree->SetBranchAddress("event_num_annih_e",&event_num_annih_e);
  Annihtree->SetBranchAddress("time_annih_e",&time_annih_e);
  Annihtree->SetBranchAddress("energy_annih_e",&energy_annih_e);
  Annihtree->SetBranchAddress("pdgID_annih_e",&pdgID_annih_e);
  Annihtree->SetBranchAddress("pcode_annih_e",&pcode_annih_e);
  Annihtree->SetBranchAddress("px_annih_e",&px_annih_e);
  Annihtree->SetBranchAddress("py_annih_e",&py_annih_e);
  Annihtree->SetBranchAddress("pz_annih_e",&pz_annih_e);
  Annihtree->SetBranchAddress("p_annih_e",&p_annih_e);
  Annihtree->SetBranchAddress("x_annih_e",&x_annih_e);
  Annihtree->SetBranchAddress("y_annih_e",&y_annih_e);
  Annihtree->SetBranchAddress("z_annih_e",&z_annih_e);

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
  Photontree->SetBranchAddress("px_photon",&px_photon);
  Photontree->SetBranchAddress("py_photon",&py_photon);
  Photontree->SetBranchAddress("pz_photon",&pz_photon);
  Photontree->SetBranchAddress("p_photon",&p_photon);
  Photontree->SetBranchAddress("x_photon",&x_photon);
  Photontree->SetBranchAddress("y_photon",&y_photon);
  Photontree->SetBranchAddress("z_photon",&z_photon);

  double auxtrack_phot=200;
  double aux_event_num_photon=200;

  unsigned long entriesEvent=Eventtree->GetEntries();
  std::cout<<"Entries Event (starting with mu, e+- or photon): "<<entriesEvent<<std::endl;
  unsigned long entriesBrems=Bremstree->GetEntries();
  std::cout<<"Entries Brems hit ST (e+-): "<<entriesBrems<<std::endl;
  unsigned long entriesAnnih=Annihtree->GetEntries();
  std::cout<<"Entries Annih hit ST (e+-): "<<entriesAnnih<<std::endl;
  unsigned long entriesMuon=Muontree->GetEntries();
  std::cout<<"Entries Muon hit ST: "<<entriesMuon<<std::endl;
  unsigned long entriesPhoton=Photontree->GetEntries();
  std::cout<<"Entries Photon hit ST: "<<entriesPhoton<<std::endl;



  for(unsigned long i=0;i<entriesEvent;i++){
    Eventtree->GetEntry(i);
  }

  for(unsigned long i=0;i<entriesBrems;i++){
    Bremstree->GetEntry(i);
      double px_brems_e_keV=px_brems_e*1000;
      double py_brems_e_keV=py_brems_e*1000;
      double pz_brems_e_keV=pz_brems_e*1000;
      double p_brems_e_keV=p_brems_e*1000;
      /*if(write_to_file==true){
        double eventi = i+1;
        writefile<<eventi<<" "<<px_brems_e_keV<<" "<<py_brems_e_keV<<" "<<pz_brems_e_keV<<" "<<p_brems_e_keV<<" "<<x_brems_e<<" "<<y_brems_e<<" "<<z_brems_e<<std::endl;
      }*/
      if(branch==1){
        h[0]->Fill(px_brems_e_keV);
        h[1]->Fill(py_brems_e_keV);
        h[2]->Fill(pz_brems_e_keV);
        h[3]->Fill(p_brems_e_keV);
      }

      if(branch==3){ //theta
        double theta = acos(pz_brems_e/p_brems_e);
        h[0]->Fill(theta);
      }

      if(branch==4){ //phi
        double phi = atan(py_brems_e/px_brems_e);
        //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
        if((px_brems_e<0)&&(py_brems_e>0)){
  //these angles are negative angles (-phi) in the 4th quadrant that should be in 2nd quadrant
  phi = PI + phi;
  //std::cout<<"px_brems_e: "<<px_brems_e<<" py_brems_e: "<<py_brems_e<<" phi: "<<phi<<std::endl;
}
        if((px_brems_e<0)&&(py_brems_e<0)){
  //These are positive angles in the 1st quadrant that should be in 3rd quadrant
  phi = PI + phi;
  //std::cout<<"px_brems_e: "<<px_brems_e<<" py_brems_e: "<<py_brems_e<<" phi: "<<phi<<std::endl;
}
        //Conversion so that 4th quadrant returns positive angles
        //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
        if((px_brems_e>0)&&(py_brems_e<0)){
  //these angles are negative angles (-phi) in the 4th quadrant that should be positive angles in the 4th quadrant
  phi = 2*PI + phi;
  //std::cout<<"px_brems_e: "<<px_brems_e<<" py_brems_e: "<<py_brems_e<<" phi: "<<phi<<std::endl;
}
        //std::cout<<"px_brems_e: "<<px_brems_e<<" py_brems_e: "<<py_brems_e<<" phi: "<<phi<<std::endl;

        h[0]->Fill(phi);
}

}


  for(unsigned long i=0;i<entriesAnnih;i++){
  Annihtree->GetEntry(i);
  double px_annih_e_keV=px_annih_e*1000;
  double py_annih_e_keV=py_annih_e*1000;
  double pz_annih_e_keV=pz_annih_e*1000;
  double p_annih_e_keV=p_annih_e*1000;
  if(pdgID_annih_e!=-11){
  std::cout<<"pdgID: "<<pdgID_annih_e<<std::endl;
}
  if(branch==2){
  if(p_annih_e_keV>0){
  h[0]->Fill(px_annih_e_keV);
  h[1]->Fill(py_annih_e_keV);
  h[2]->Fill(pz_annih_e_keV);
  h[3]->Fill(p_annih_e_keV);}
}
}


  for(unsigned long i=0;i<entriesPhoton;i++){
  Photontree->GetEntry(i);
  //std::cout<<"Event ID: "<<event_num_photon<<" track id: "<<trackID_photon<<" E: "<<energy_photon<<" time: "<<time_photon<<" pcode_photon: "<<pcode_photon<<" px: "<<px_photon<<" py: "<<py_photon<<" pz: "<<pz_photon<<std::endl;
  if((trackID_photon!=auxtrack_phot)||(event_num_photon!=aux_event_num_photon)){
  double px_photon_keV=px_photon*1000;
  double py_photon_keV=py_photon*1000;
  double pz_photon_keV=pz_photon*1000;
  double p_photon_keV=p_photon*1000;
  if(write_to_file==true){
  double eventi = i+1;
  writefile<<eventi<<" "<<px_photon_keV<<" "<<py_photon_keV<<" "<<pz_photon_keV<<" "<<p_photon_keV<<" "<<x_photon<<" "<<y_photon<<" "<<z_photon<<std::endl;
}
  //std::cout<<"STORE event: "<<event_num_photon<<" trackID: "<<trackID_photon<<" E: "<<energy_photon<<" time: "<<time_photon<<" pcode_photon: "<<pcode_photon<<std::endl;
  if(branch==0){
  if(p_photon_keV>=500){
  h[0]->Fill(px_photon_keV);
  h[1]->Fill(py_photon_keV);
  h[2]->Fill(pz_photon_keV);
  h[3]->Fill(p_photon_keV);}
}
  if(branch==5){ //theta
  double theta = acos(pz_photon/p_photon);
  //std::cout<<"pz_photon: "<<pz_photon<<" p_photon: "<<p_photon<<" theta: "<<theta<<std::endl;
  h[0]->Fill(theta);
}

  if(branch==6){ //phi
  double phi = atan(py_photon/px_photon);
  //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
  if((px_photon<0)&&(py_photon>0)){
  //these angles are negative angles (-phi) in the 4th quadrant that should be in 2nd quadrant
  phi = PI + phi;
  //std::cout<<"px_photon: "<<px_photon<<" py_photon: "<<py_photon<<" phi: "<<phi<<std::endl;
  }
  if((px_photon<0)&&(py_photon<0)){
  //These are positive angles in the 1st quadrant that should be in 3rd quadrant
  phi = PI + phi;
  //std::cout<<"px_photon: "<<px_photon<<" py_photon: "<<py_photon<<" phi: "<<phi<<std::endl;
  }
  //Conversion so that 4th quadrant returns positive angles
  //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
  if((px_photon>0)&&(py_photon<0)){
  //these angles are negative angles (-phi) in the 4th quadrant that should be positive angles in the 4th quadrant
  phi = 2*PI + phi;
  //std::cout<<"px_photon: "<<px_photon<<" py_photon: "<<py_photon<<" phi: "<<phi<<std::endl;
  }
  //std::cout<<"px_photon: "<<px_photon<<" py_photon: "<<py_photon<<" phi: "<<phi<<std::endl;

  h[0]->Fill(phi);
  }

  if(branch==7){ //cos(theta)
    double theta = acos(pz_photon/p_photon);
    double costheta = cos(theta);
    h[0]->Fill(costheta);
  }
  
  }
  auxtrack_phot=trackID_photon;
  aux_event_num_photon=event_num_photon;
  }


  for(unsigned long i=0;i<entriesMuon;i++){
    Muontree->GetEntry(i);
  }

  TPaveStats *stat[Nhisto];

  if(branch==0){
  pad1->cd();
  h[0]->SetLineColor(kGreen+2);
  h[0]->SetFillColor(kGreen+2);
  h[0]->SetFillStyle(3001);
  h[0]->GetYaxis()->SetTitleOffset(1.65);
  h[0]->Draw("");

  std::string str_latex1 = "p_{#gamma} > 500 keV";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.3,.85,char_latex1);

  gPad->Update();
  stat[0] = (TPaveStats*)h[0]->FindObject("stats");
  stat[0]->SetY1NDC(.75);
  stat[0]->SetY2NDC(.9);
  stat[0]->SetX1NDC(0.5); //new x start position
  stat[0]->SetX2NDC(0.9);
  stat[0]->SetTextColor(4);
  //stat[0]->SetTextSize(0.043);
  stat[0]->Draw("same");


  pad2->cd();
  h[1]->SetLineColor(kGreen+2);
  h[1]->SetFillColor(kGreen+2);
  h[1]->SetFillStyle(3001);
  h[1]->GetYaxis()->SetTitleOffset(1.65);
  h[1]->Draw("");
  /*  std::string str_latex2 = std::to_string(int(t2))+" < t < "+std::to_string(int(t3))+" ns";
    char* char_latex2 = const_cast<char*>(str_latex2.c_str());
    TLatex latex2;
    latex2.DrawLatexNDC(.3,.85,char_latex2);
  */
  gPad->Update();
  stat[1] = (TPaveStats*)h[1]->FindObject("stats");
  stat[1]->SetY1NDC(.75);
  stat[1]->SetY2NDC(.9);
  stat[1]->SetX1NDC(0.5); //new x start position
  stat[1]->SetX2NDC(0.9);
  stat[1]->SetTextColor(4);
  stat[1]->Draw("same");


  pad3->cd();
  h[2]->SetLineColor(kGreen+2);
  h[2]->SetFillColor(kGreen+2);
  h[2]->SetFillStyle(3001);
  h[2]->GetYaxis()->SetTitleOffset(1.71);
  h[2]->Draw("");
  /*std::string str_latex3 = std::to_string(int(t3))+" < t < "+std::to_string(int(t4))+" ns";
  char* char_latex3 = const_cast<char*>(str_latex3.c_str());
  TLatex latex3;
  latex3.DrawLatexNDC(.3,.85,char_latex3);
*/
  gPad->Update();
  stat[2] = (TPaveStats*)h[2]->FindObject("stats");
  stat[2]->SetY1NDC(.75);
  stat[2]->SetY2NDC(.9);
  stat[2]->SetX1NDC(0.5); //new x start position
  stat[2]->SetX2NDC(0.9);
  stat[2]->SetTextColor(4);
  stat[2]->Draw("same");


  pad4->cd();
  h[3]->SetLineColor(kGreen+2);
  h[3]->SetFillColor(kGreen+2);
  //h[3]->SetFillStyle(3001);
  h[3]->GetYaxis()->SetTitleOffset(1.65);
  h[3]->Draw("");
  /*std::string str_latex4 = std::to_string(int(t4))+" < t < "+std::to_string(int(t5))+" ns";
    char* char_latex4 = const_cast<char*>(str_latex4.c_str());
    TLatex latex4;
    latex4.DrawLatexNDC(.3,.85,char_latex4);
  */
  gPad->Update();
  stat[3] = (TPaveStats*)h[3]->FindObject("stats");
  stat[3]->SetY1NDC(.75);
  stat[3]->SetY2NDC(.9);
  stat[3]->SetX1NDC(0.5); //new x start position
  stat[3]->SetX2NDC(0.9);
  stat[3]->SetTextColor(4);
  stat[3]->Draw("same");


  }


  if(branch==1){
    pad1->cd();
    h[0]->SetLineColor(kViolet+2);
    h[0]->SetFillColor(kViolet+2);
    h[0]->SetFillStyle(3001);
    h[0]->GetYaxis()->SetTitleOffset(1.65);
    h[0]->Draw("");


    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetY1NDC(.75);
    stat[0]->SetY2NDC(.9);
    stat[0]->SetX1NDC(0.5); //new x start position
    stat[0]->SetX2NDC(0.9);
    stat[0]->SetTextColor(4);
    //stat[0]->SetTextSize(0.043);
    stat[0]->Draw("same");


    pad2->cd();
    h[1]->SetLineColor(kViolet+2);
    h[1]->SetFillColor(kViolet+2);
    h[1]->SetFillStyle(3001);
    h[1]->GetYaxis()->SetTitleOffset(1.65);
    h[1]->Draw("");

    gPad->Update();
    stat[1] = (TPaveStats*)h[1]->FindObject("stats");
    stat[1]->SetY1NDC(.75);
    stat[1]->SetY2NDC(.9);
    stat[1]->SetX1NDC(0.5); //new x start position
    stat[1]->SetX2NDC(0.9);
    stat[1]->SetTextColor(4);
    stat[1]->Draw("same");


    pad3->cd();
    h[2]->SetLineColor(kViolet+2);
    h[2]->SetFillColor(kViolet+2);
    h[2]->SetFillStyle(3001);
    h[2]->GetYaxis()->SetTitleOffset(1.65);
    h[2]->Draw("");

    gPad->Update();
    stat[2] = (TPaveStats*)h[2]->FindObject("stats");
    stat[2]->SetY1NDC(.75);
    stat[2]->SetY2NDC(.9);
    stat[2]->SetX1NDC(0.5); //new x start position
    stat[2]->SetX2NDC(0.9);
    stat[2]->SetTextColor(4);
    stat[2]->Draw("same");


    pad4->cd();
    h[3]->SetLineColor(kViolet+2);
    h[3]->SetFillColor(kViolet+2);
    //h[3]->SetFillStyle(3001);
    h[3]->GetYaxis()->SetTitleOffset(1.65);
    h[3]->Draw("");

    gPad->Update();
    stat[3] = (TPaveStats*)h[3]->FindObject("stats");
    stat[3]->SetY1NDC(.75);
    stat[3]->SetY2NDC(.9);
    stat[3]->SetX1NDC(0.5); //new x start position
    stat[3]->SetX2NDC(0.9);
    stat[3]->SetTextColor(4);
    stat[3]->Draw("same");


  }




  if(branch==2){
    pad1->cd();
    h[0]->SetLineColor(kMagenta+2);
    h[0]->SetFillColor(kMagenta+2);
    h[0]->SetFillStyle(3001);
    h[0]->GetYaxis()->SetTitleOffset(1.71);
    h[0]->Draw("");
    std::string str_latex1 = "p_{e^{+}} > 0 keV";
    char* char_latex1 = const_cast<char*>(str_latex1.c_str());
    TLatex latex1;
    latex1.DrawLatexNDC(.3,.85,char_latex1);

    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetY1NDC(.75);
    stat[0]->SetY2NDC(.9);
    stat[0]->SetX1NDC(0.5); //new x start position
    stat[0]->SetX2NDC(0.9);
    stat[0]->SetTextColor(4);
    //stat[0]->SetTextSize(0.043);
    stat[0]->Draw("same");


    pad2->cd();
    h[1]->SetLineColor(kMagenta+2);
    h[1]->SetFillColor(kMagenta+2);
    h[1]->SetFillStyle(3001);
    h[1]->GetYaxis()->SetTitleOffset(1.71);
    h[1]->Draw("");

    gPad->Update();
    stat[1] = (TPaveStats*)h[1]->FindObject("stats");
    stat[1]->SetY1NDC(.75);
    stat[1]->SetY2NDC(.9);
    stat[1]->SetX1NDC(0.5); //new x start position
    stat[1]->SetX2NDC(0.9);
    stat[1]->SetTextColor(4);
    stat[1]->Draw("same");


    pad3->cd();
    h[2]->SetLineColor(kMagenta+2);
    h[2]->SetFillColor(kMagenta+2);
    h[2]->SetFillStyle(3001);
    h[2]->GetYaxis()->SetTitleOffset(1.71);
    h[2]->Draw("");

    gPad->Update();
    stat[2] = (TPaveStats*)h[2]->FindObject("stats");
    stat[2]->SetY1NDC(.75);
    stat[2]->SetY2NDC(.9);
    stat[2]->SetX1NDC(0.5); //new x start position
    stat[2]->SetX2NDC(0.9);
    stat[2]->SetTextColor(4);
    stat[2]->Draw("same");


    pad4->cd();
    h[3]->SetLineColor(kMagenta+2);
    h[3]->SetFillColor(kMagenta+2);
    //h[3]->SetFillStyle(3001);
    h[3]->GetYaxis()->SetTitleOffset(1.71);
    h[3]->Draw("");

    gPad->Update();
    stat[3] = (TPaveStats*)h[3]->FindObject("stats");
    stat[3]->SetY1NDC(.75);
    stat[3]->SetY2NDC(.9);
    stat[3]->SetX1NDC(0.5); //new x start position
    stat[3]->SetX2NDC(0.9);
    stat[3]->SetTextColor(4);
    stat[3]->Draw("same");


  }

  if(branch==3){ //theta brems
    h[0]->SetLineColor(kViolet+3);
    h[0]->SetFillColor(kViolet+3);
    h[0]->Draw("");

    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetX1NDC(0.71); //new x start position
    stat[0]->SetX2NDC(0.91); //new x end position
    stat[0]->SetY1NDC(0.76); //new y start position
    stat[0]->SetY2NDC(0.9); //new y end position
    stat[0]->SetTextColor(4);
    stat[0]->GetLineWith("Entries")->SetTextSize(0.027);
    stat[0]->GetLineWith("Mean")->SetTextSize(0.027);
    stat[0]->GetLineWith("Std Dev")->SetTextSize(0.027);
    stat[0]->Draw("same");
  }

  if(branch==4){ //phi brems
    h[0]->SetLineColor(kViolet+5);
    h[0]->SetFillColor(kViolet+5);
    h[0]->Draw("");

    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetX1NDC(0.71); //new x start position
    stat[0]->SetX2NDC(0.91); //new x end position
    stat[0]->SetY1NDC(0.76); //new y start position
    stat[0]->SetY2NDC(0.9); //new y end position
    stat[0]->SetTextColor(4);
    stat[0]->GetLineWith("Entries")->SetTextSize(0.027);
    stat[0]->GetLineWith("Mean")->SetTextSize(0.027);
    stat[0]->GetLineWith("Std Dev")->SetTextSize(0.027);
    stat[0]->Draw("same");
  }

  if(branch==5){ //theta phot
    //Integral theta
    double theta1 = 0;
    double theta2 = 0.003;
    double integral1 = h[0]->Integral(h[0]->FindFixBin(theta1), h[0]->FindFixBin(theta2));
    double integral2 = h[0]->GetEntries()-integral1;

    std::cout<<"Theta integral between: ["<<theta1<<","<<theta2<<"]rad: "<<integral1<<std::endl;
    std::cout<<"Rest of events: "<<setprecision(8)<<integral2<<std::endl;

    h[0]->SetLineColor(kGreen+3);
    h[0]->SetFillColor(kGreen+3);
    h[0]->Draw("");
    /*
    std::string str_latex1 = "Integral #theta["+std::to_string(int(theta1))+","+std::to_string(theta2)+"]rad = "+std::to_string(int(integral1));
    char* char_latex1 = const_cast<char*>(str_latex1.c_str());
    TLatex latex1;
    latex1.DrawLatexNDC(.3,.85,char_latex1);
    */
    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetX1NDC(0.71); //new x start position
    stat[0]->SetX2NDC(0.91); //new x end position
    stat[0]->SetY1NDC(0.76); //new y start position
    stat[0]->SetY2NDC(0.9); //new y end position
    stat[0]->SetTextColor(4);
    stat[0]->GetLineWith("Entries")->SetTextSize(0.027);
    stat[0]->GetLineWith("Mean")->SetTextSize(0.027);
    stat[0]->GetLineWith("Std Dev")->SetTextSize(0.027);
    stat[0]->Draw("same");
  }

  if(branch==6){ //phi phot
    h[0]->SetLineColor(kGreen-5);
    h[0]->SetFillColor(kGreen-5);
    h[0]->Draw("");

    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetX1NDC(0.71); //new x start position
    stat[0]->SetX2NDC(0.91); //new x end position
    stat[0]->SetY1NDC(0.76); //new y start position
    stat[0]->SetY2NDC(0.9); //new y end position
    stat[0]->SetTextColor(4);
    stat[0]->GetLineWith("Entries")->SetTextSize(0.027);
    stat[0]->GetLineWith("Mean")->SetTextSize(0.027);
    stat[0]->GetLineWith("Std Dev")->SetTextSize(0.027);
    stat[0]->Draw("same");
  }

  if(branch==7){ //costheta
    h[0]->SetFillColor(kGreen-6);
    h[0]->SetLineColor(kGreen-6);
    h[0]->Draw("");

    //Integral cos(theta)
    double costheta1 = cos(0.003);
    double costheta2 = cos(0);
    double integral1 = h[0]->Integral(h[0]->FindFixBin(costheta1), h[0]->FindFixBin(costheta2));
    double integral2 = h[0]->GetEntries()-integral1;

    std::cout<<"cos(theta) integral between: ["<<costheta1<<","<<costheta2<<"]: "<<integral1<<std::endl;
    std::cout<<"Rest of events: "<<setprecision(8)<<integral2<<std::endl;


    gPad->Update();
    stat[0] = (TPaveStats*)h[0]->FindObject("stats");
    stat[0]->SetX1NDC(0.71); //new x start position
    stat[0]->SetX2NDC(0.91); //new x end position
    stat[0]->SetY1NDC(0.76); //new y start position
    stat[0]->SetY2NDC(0.9); //new y end position
    stat[0]->SetTextColor(4);
    stat[0]->GetLineWith("Entries")->SetTextSize(0.027);
    stat[0]->GetLineWith("Mean")->SetTextSize(0.027);
    stat[0]->GetLineWith("Std Dev")->SetTextSize(0.027);
    stat[0]->Draw("same");
  }

  writefile.close();

}



//================================================================
void DivideCanvasMomentum(std::string ArtFiles_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log");

  //Open txt
  fstream readfile;
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



  //GENERATE PLOTS WITH 4 CANVAS
  //ReRun Ele beam photons momentum
  //ReadRootFile("/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 100, -600, 2000, 0, 400, "p_{#gamma} [keV]", "Counts", 0);
  //ReRun Ele beam e+- bremsstrahlung momentum
  //ReadRootFile("/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 100, -1000, 50000, 0, 400, "p_{e^{#pm}} [keV]", "Counts", 1);
  //ReRun Ele beam e+ annihilation momentum
  //ReadRootFile("/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 100, -3000, 8000, 0, 400, "p_{e^{+}} [keV]", "Counts", 2);

  //GENERATE PLOTS WITH 1 CANVAS
  double xmin_theta = -2*PI;
  double xmax_theta = 2*PI;
  double xmin_phi = -2*PI;
  double xmax_phi = 3*PI;
  //Electron and positrons that undergo bremsstrahlung angles
  //ReadRootFile("/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 1000, 1, 2, 0, 500, "#theta_{e^{#pm}} bremsstrahlung [rad]", "", 3);
  //ReadRootFile("/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 1000, -1, xmax_phi, 0, 500, "#phi_{e^{#pm}} bremsstrahlung [rad]", "", 4);
  //All photons generated at ST angles
  //ReadRootFile("/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 1000, -1, 3.5, 0, 500, "#theta_{#gamma} [rad]", "", 5);
  //ReadRootFile("/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 1000, -1, xmax_phi, 0, 500, "#phi_{#gamma} [rad]", "", 6);
  ReadRootFile("/work/mu2e/data1/cgarcia/ReRunEleBeamtogg/ST_events_bremsstrahlung_ReRunEleBeam.root", 1000, -1.5, 2.5, 0, 500, "cos(#theta_{#gamma})", "", 7);

  readfile.close();
}

//================================================================
