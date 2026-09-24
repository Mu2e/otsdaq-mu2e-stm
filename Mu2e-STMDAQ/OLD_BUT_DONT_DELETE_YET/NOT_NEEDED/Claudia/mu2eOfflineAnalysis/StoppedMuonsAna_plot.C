#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include "TH1.h"
#include "TF1.h"
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

using namespace std;

void StoppedMuonsAna_plot(std::string ArtFiles_location) {

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111); 
  gStyle->SetOptStat(1110);

  std::fstream writefile;
  bool write_to_file=false;
  if(write_to_file==true){
    writefile.open("10M_StoppedMuons_xyz.txt",std::ios::out);
  }

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

  //1 file
  string path;
  path=file_name[0];


  TCanvas *c1 = new TCanvas("");

  TFile *input= new TFile(path.c_str(),"read");

  //HISTOGRAMS
  //Momentum for the generated photons
  TH1D* hist1 = (TH1D*)input->Get("generate/StoppedMuonXRayGammaRayGun/hmomentum");
  hist1->GetYaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetTitle("E (MeV)");
  hist1->GetYaxis()->SetTitleSize(0.025);
  hist1->GetXaxis()->SetTitleSize(0.025);
  hist1->GetYaxis()->SetTitleOffset(2);
  hist1->GetXaxis()->SetTitleOffset(2);

  hist1->SetFillStyle( 3001);
  hist1->SetFillColor( kBlue+1);
  //hist1->Draw("HIST");

  //x-y positions for the generated photons
  TH2D* hist2 = (TH2D*)input->Get("generate/StoppedMuonXRayGammaRayGun/hxyPos");
  hist2->GetYaxis()->SetLabelSize(0.03);
  hist2->GetXaxis()->SetLabelSize(0.03);

  hist2->GetYaxis()->SetTitleSize(0.025);
  hist2->GetXaxis()->SetTitleSize(0.025);
  hist2->GetYaxis()->SetTitleOffset(2);
  hist2->GetXaxis()->SetTitleOffset(2);
  hist2->SetFillStyle( 3001);
  hist2->SetFillColor( kBlue+1);
  //hist2->Draw("HIST");

  //z positions for the generated photons
  TH1D* hist3 = (TH1D*)input->Get("generate/StoppedMuonXRayGammaRayGun/hcz");
  hist3->GetYaxis()->SetLabelSize(0.03);
  hist3->GetXaxis()->SetLabelSize(0.03);

  hist3->GetYaxis()->SetTitleSize(0.025);
  hist3->GetXaxis()->SetTitleSize(0.025);
  hist3->GetYaxis()->SetTitleOffset(2);
  hist3->GetXaxis()->SetTitleOffset(2);
  hist3->SetFillStyle( 3001);
  hist3->SetFillColor( kBlue+1);
  //hist3->Draw("HIST");



  //TREES GENERATION POSITIONS FOR STOPPED MUONS AT ST
  TH1F* hxmu2e=new TH1F("hxmu2e", "x at Production Mu2e System", 100, -3990, -3800);
  TH1F* hymu2e=new TH1F("hymu2e", "y at Production Mu2e System", 100, -80, 100);
  TH1F* hzmu2e=new TH1F("hzmu2e", "z at Production Mu2e System", 100, 5400, 6300);


  double xmuonmu2e, ymuonmu2e, zmuonmu2e, t, nphots_generated;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("generate/StoppedMuonXRayGammaRayGun/treeStopMuons");

  tree1->SetBranchAddress("xmuonmu2e", &xmuonmu2e);
  tree1->SetBranchAddress("ymuonmu2e", &ymuonmu2e);
  tree1->SetBranchAddress("zmuonmu2e", &zmuonmu2e);
  tree1->SetBranchAddress("t", &t);
  //tree1->SetBranchAddress("nphots_generated", &nphots_generated);

  unsigned long int entries=tree1->GetEntries();
  std::cout<<entries<<std::endl;

  //Each entry is an event (a photon generated)
  for(unsigned long int i=0;i<entries;i++)
    {
      tree1->GetEntry(i);
      hxmu2e->Fill(xmuonmu2e);
      hymu2e->Fill(ymuonmu2e);
      hzmu2e->Fill(zmuonmu2e);

      if(write_to_file==true){
        writefile<<xmuonmu2e<<" "<<ymuonmu2e<<" "<<zmuonmu2e<<std::endl;
      }
    
}

  hxmu2e->GetXaxis()->SetTitle("x_{stopped #mu^{-}} [mm]");
  hxmu2e->SetLineColor( kBlue+1);
  hxmu2e->Draw("");

  hymu2e->GetXaxis()->SetTitle("y_{stopped #mu^{-}} [mm]");
  hymu2e->SetLineColor( kBlue+1);
  //hymu2e->Draw("");

  hzmu2e->GetXaxis()->SetTitle("z_{stopped #mu^{-}} [mm]");
  hzmu2e->SetLineColor( kBlue+1);
  //hzmu2e->Draw("");




  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.71); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.76); //new y start position
  ps->SetY2NDC(0.9); //new y end position
  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats
  //hxmu2e->SetStats(0);
  //hymu2e->SetStats(0);
  hzmu2e->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);

  /*
  ps->GetLineWith("Entries")->SetTextSize(0.027);
  ps->GetLineWith("Mean")->SetTextSize(0.027);
  ps->GetLineWith("Std Dev")->SetTextSize(0.027);
  */
  c1->Modified();


  writefile.close();

  c1->Print("x_10Mstoppedmuons.png");
  c1->Print("x_10Mstoppedmuons.pdf");
}
