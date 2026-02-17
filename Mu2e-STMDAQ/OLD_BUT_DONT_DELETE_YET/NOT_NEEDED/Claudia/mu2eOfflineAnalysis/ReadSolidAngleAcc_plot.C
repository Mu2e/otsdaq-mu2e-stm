#include <iostream>
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
#include<iomanip>

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

#define mm_to_cm 10
#define PI 3.14159265358979323846  /* pi */

//================================================================
//This program reads readGens tree
//Reads the root files from a txt and analyse the trees
//================================================================  
void PrintGraph(vector<string> file_name, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);
  int palette_number = 55;
  gStyle->SetPalette(palette_number);
  //just for TH2D
  gStyle->SetPadRightMargin(0.14);
  //gStyle->SetPadRightMargin(0.1);

  double xmuonmu2e,ymuonmu2e,zmuonmu2e,px,py,pz,momentum,theta,costheta,phi,x_STM,y_STM,z_STM;

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  TGraph *gr = new TGraph ();
  TH2D *h = new TH2D("TH2D","",100,xmin,xmax,100,ymin,ymax);

  std::cout<<"---Loop over files---"<<std::endl;
  unsigned long int p=0;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<""<<std::endl;
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<path.c_str()<<std::endl;

   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("generate/STMHits/STMHits");

    tree->SetBranchAddress("xmuonmu2e",&xmuonmu2e);
    tree->SetBranchAddress("ymuonmu2e",&ymuonmu2e);
    tree->SetBranchAddress("zmuonmu2e",&zmuonmu2e);
    tree->SetBranchAddress("momentum",&momentum);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("theta",&theta);
    tree->SetBranchAddress("costheta",&costheta);   
    tree->SetBranchAddress("phi",&phi);
    tree->SetBranchAddress("x_STM",&x_STM);
    tree->SetBranchAddress("y_STM",&y_STM);
    tree->SetBranchAddress("z_STM",&z_STM);

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;


    for(unsigned long i=0;i<entries;i++){

      
      tree->GetEntry(i);
      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
      }
      
     
      if(branch==0){h->Fill(xmuonmu2e/mm_to_cm,ymuonmu2e/mm_to_cm);}

    }//entries    
    
    input->Close();
  }//for int file


  gr->SetTitle("");
  gr->GetXaxis()->SetTitle(Xtitle);
  gr->GetYaxis()->SetTitle(Ytitle);
  gr->SetMarkerStyle(7);
  gr->SetMarkerColor(kBlack);
  //gr->Draw("same,p");
  

  h->GetXaxis()->SetTitle(Xtitle);
  h->GetYaxis()->SetTitle(Ytitle);
  h->GetZaxis()->SetLabelSize(0.04);
  h->Draw("colz");


  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.62); //new x start position
  ps->SetX2NDC(0.82); //new x end position
  ps->SetY1NDC(0.76); //new y start position
  ps->SetY2NDC(0.9); //new y end position

  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats
  h->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);
  ps->GetLineWith("Entries")->SetTextSize(0.027);
  c1->Modified();



  //c1->Print("50Mxypositions_z>615_8.png");
  //c1->Print("50Mxypositions_z>615_8.pdf");
}

//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //Removes title of the histogram (TH1)
  gStyle->SetOptStat(1110);

  double xmuonmu2e,ymuonmu2e,zmuonmu2e,px,py,pz,momentum,theta,costheta,phi,x_STM,y_STM,z_STM;

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  
  TH1F*h1 = new TH1F("TH1","", nbins, Xrange[0], Xrange[1]);

  std::cout<<"---Loop over files---"<<std::endl;
  unsigned long int p=0;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<""<<std::endl;
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<path.c_str()<<std::endl;


    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("generate/STMHits/STMHits");

    tree->SetBranchAddress("xmuonmu2e",&xmuonmu2e);
    tree->SetBranchAddress("ymuonmu2e",&ymuonmu2e);
    tree->SetBranchAddress("zmuonmu2e",&zmuonmu2e);
    tree->SetBranchAddress("momentum",&momentum);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("theta",&theta);
    tree->SetBranchAddress("costheta",&costheta);
    tree->SetBranchAddress("phi",&phi);
    tree->SetBranchAddress("x_STM",&x_STM);
    tree->SetBranchAddress("y_STM",&y_STM);
    tree->SetBranchAddress("z_STM",&z_STM);

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;
    

    for(unsigned long i=0;i<entries;i++){

      
      tree->GetEntry(i);
      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;}

      if(branch==0){h1->Fill(momentum);}
      //if(branch==0){h1->Fill(px);} 
      //if(branch==0){h1->Fill(py);} 
      //if(branch==0){h1->Fill(pz);}


    }//entries    
    

    input->Close();
  }//for int file



  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);


    
  if(branch==0){h1->SetFillColor(kRed+2);}


  //gPad->SetLogy();

  h1->Draw("");  
  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.71); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.76); //new y start position
  ps->SetY2NDC(0.9); //new y end position
  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats
  h1->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);
  ps->GetLineWith("Entries")->SetTextSize(0.027);
  c1->Modified();
    


  c1->Print("10to7PhotGun_SolidAng_099999_1_VD89_stoppedpos.pdf");
  c1->Print("10to7PhotGun_SolidAng_099999_1_VD89_stoppedpos.png");
}



//================================================================
void ReadSolidAngleAcc_plot(std::string ArtFiles_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 

  //Open txt
  fstream readfile;
  readfile.open(ArtFiles_location,ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();
  int i=0;
  //Read each art file from txt
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    std::cout<<i<<" "<<name<<std::endl;
    i++;
  }

  
  //0 is to print momentum
  PrintHisto(file_name, 100, -5, 5, 0, 3000, "p_{#gamma, VD=89} [MeV]", "Counts", 0); 
  
  //0 is to print x, y coordinates in Mu2e system
  //PrintGraph(file_name,-410, -370, -15, 15, "x_{gen #gamma, ST (z>615cm)}(Mu2e) [cm]", "y_{gen #gamma, ST (z>615cm)} [cm]", 0);
  
  readfile.close();
}

//================================================================
