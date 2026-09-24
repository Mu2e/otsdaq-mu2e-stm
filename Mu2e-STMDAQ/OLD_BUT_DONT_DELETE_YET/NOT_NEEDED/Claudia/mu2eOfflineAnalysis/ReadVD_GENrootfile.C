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
#include "TF1.h"

//================================================================
//This program reads readvd/ntvd or readvd/ntvext trees: Virtual detectors analyser
//Reads the root files from a txt and analyse the trees
//================================================================  

void GenRootFile(vector<string> file_name, int nbins, double xmin, double xmax, string Xtitlest, string Ytitlest, int branch, int savetorootfile, string outputfilename){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  TCanvas *c1 = new TCanvas("");
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  float px_photVD101;

  TFile*output=new TFile(outputfilename.c_str(),"recreate");
  TTree*PXtree=new TTree("PXtree", "PXtree");
  
  PXtree->Branch("px_photVD101",&px_photVD101);


  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

  TH1F*h1 = new TH1F("TH1","", nbins, xmin, xmax);
  
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
  
  string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");

    tree->SetBranchAddress("evt",&evt);
    tree->SetBranchAddress("trk",&trk);
    tree->SetBranchAddress("sid",&sid);
    tree->SetBranchAddress("pdg",&pdg);
    tree->SetBranchAddress("run",&run);
    tree->SetBranchAddress("subrun",&subrun);
    tree->SetBranchAddress("time",&time);
    tree->SetBranchAddress("x",&x);
    tree->SetBranchAddress("y",&y);
    tree->SetBranchAddress("z",&z);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("ke",&ke);
    tree->SetBranchAddress("code",&code);

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;

    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);
      
      //Just fill at Virtual detector ID number:
      float VDnumber = 101;
      if(sid==VDnumber){
	
	double mom=sqrt(px*px+py*py+pz*pz);
	double pt = sqrt(px*px+py*py);
	
	if((branch==1)&&(pdg==22)){
	  h1->Fill(px);
	  px_photVD101 = px;
          PXtree->Fill();
	}
      } //if sid
      
    }//entries
    input->Close();
  }//loop over files

  h1->Draw("");
  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
 
  if(branch==1){h1->SetFillColor(kGreen+2);}

  //if save to root file is 1, the histogram is not plotted on screen, it is stored in the root file 
   if(savetorootfile==1){
    output->Write();
    output->Close();
   }
  

  //c1->Print("");
  //c1->Print("");
  
}



//================================================================
void ReadVD_GENrootfile(std::string ArtFiles_location) {
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
    //std::cout<<name<<std::endl;
  }


  //1 is to print momentum of photons of particles at VD, branch, savetorootfile, root file
  GenRootFile(file_name, 100, -0.02, 0.02, "p_{VD=101, #gamma} [MeV]", "Counts", 1, 1, "VD101_px_photons_fit.root");
  readfile.close();
}

//================================================================
