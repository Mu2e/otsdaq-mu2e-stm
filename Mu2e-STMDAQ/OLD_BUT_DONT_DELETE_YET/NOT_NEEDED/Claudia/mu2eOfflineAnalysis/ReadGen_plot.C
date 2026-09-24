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
#include "TPaveStats.h"

//================================================================
//This program reads readvd/ntvd trees: Generated particles analyser
//Reads the root files from a txt and analyse the trees
//================================================================  
void PrintGraph(vector<string> file_name, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight()=0*/, g4bl_time /*extra.time()=0*/, ke /*MeV*/, code /*sim.creationCode()*/;

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

  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;
   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");


    tree->SetBranchAddress("evt",&evt);
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


    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;

    for(unsigned long i=0;i<entries;i++){
      
      tree->GetEntry(i);

      if((branch==1)&&((pdg==11)||(pdg==-11))){gr->SetPoint(i,x,y);} 
      if((branch==2)&&(pdg!=11)&&(pdg!=-11)){gr->SetPoint(i,x,y);}
    }//entries
    input->Close();
  }//for int files

  gr->SetTitle("");
  gr->GetXaxis()->SetTitle(Xtitle);
  gr->GetYaxis()->SetTitle(Ytitle);
  gr->SetMarkerStyle(1);
  if(branch==1){gr->SetMarkerColor(kBlack);}
  gr->Draw("same,p");

  //c1->Print("electronsandpositrons_xy.png");
  //c1->Print("electronsandpositrons_xy.pdf"); 

}

//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;
  unsigned long int countelectrons=0;
  unsigned long int countentries=0;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);
  //Underflows and overflows
  //gStyle->SetOptStat(111110);
  
  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight()=0*/, g4bl_time /*extra.time()=0*/, ke /*MeV*/, code /*sim.creationCode()*/;


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
  TH1F*h2 = new TH1F("TH2","", nbins, Xrange[0], Xrange[1]);
  TH1F*h3 = new TH1F("TH3","", nbins, Xrange[0], Xrange[1]);


  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    
    TTree* tree=(TTree*)input->Get("readvd/ntvd");

    tree->SetBranchAddress("evt",&evt);
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


    unsigned long entries=tree->GetEntries();
    countentries=countentries+entries;
    std::cout<<"entries: "<<entries<<std::endl;

    for(unsigned long i=0;i<entries;i++){
      
      tree->GetEntry(i);
      double mom = sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2));

      //if(sid!=8){std::cout<<"Virtual detector ID: "<<sid<<std::endl;}

      if((branch==0)&&((pdg==11)||(pdg==-11))&&(sid==8)){h1->Fill(mom);}
      //if((branch==0)&&(pdg==-11)&&(sid==8)){h1->Fill(mom);}
      //if((branch==0)&&(pdg==11)&&(sid==8)){h1->Fill(mom);}
      //if((branch==1)&&((pdg==11)||(pdg==-11))&&(sid!=8)&&(sid!=20)){h1->Fill(mom); std::cout<<"sid: "<<sid<<" pz: "<<pz<<std::endl;}
      if((branch==1)&&(pdg==13)&&(sid==8)){h1->Fill(mom);}
      //if((branch==1)&&((pdg==13)||(pdg==-13))){h1->Fill(mom);}
      if((branch==2)&&(pdg==22)&&(sid==8)){h1->Fill(mom); if(mom>40){std::cout<<"mom gamma: "<<mom<<std::endl;}}
      if((branch==3)&&(pdg==2212)){h1->Fill(mom);}
      if((branch==4)&&(pdg==-211)){h1->Fill(mom);}

      //if((pdg!=11)&&(pdg!=-11)&&(pdg!=13)&&(pdg!=-13)&&(pdg!=22)&&(pdg!=2212)&&(pdg!=-211)){
      //std::cout<<"Different generated particle with pdgid: "<<pdg<<std::endl;}
    
    }//entries

    //std::cout<<"Number of e- and e+: "<<countelectrons<<std::endl;
    input->Close();
  }//for int files

  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);

  if(branch==0){h1->SetFillColor(kOrange+2);}
  if(branch==1){h1->SetFillColor(kRed+2);}
  if(branch==2){h1->SetFillColor(kGreen+2);}
  if(branch==3){h1->SetFillColor(kRed+2);}
  if(branch==4){h1->SetFillColor(kRed+2);}
  
  if(branch==5){h1->SetFillColor(kRed+2);}
  if(branch==6){h1->SetLineColor(kBlue+2);
    h2->SetLineColor(kMagenta-7);
    h3->SetLineColor(kGreen+2);}

  h1->Draw("");

  auto leg = new TLegend(0.1,0.7,0.48,0.9);
  TH1F*hnorm1 = (TH1F*)(h1->Clone("TH1"));
  hnorm1->Scale(1./hnorm1->Integral());
  TH1F*hnorm2 = (TH1F*)(h2->Clone("TH2"));
  hnorm2->Scale(1./hnorm2->Integral());
  TH1F*hnorm3 = (TH1F*)(h3->Clone("TH3"));
  hnorm3->Scale(1./hnorm3->Integral());
  if(branch==6){leg->AddEntry(h2, "#pi^{-} arrival time","l");
    leg->AddEntry(h1, "#mu^{#pm} arrival time","l");
    leg->AddEntry(h3, "#gamma arrival time","l");
    //h2->Draw("same");
    //h3->Draw("same");
    hnorm1->Draw("HIST,same"); 
    hnorm2->Draw("HIST,same");
    hnorm3->Draw("HIST,same");
    leg->Draw("same");
  }



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

  //ps->GetLineWith("Mean")->SetTextSize(0.027);
  //ps->GetLineWith("Std Dev")->SetTextSize(0.027);

  c1->Modified();


  std::cout<<"Total number of entries(=events=particles) in all the .root files: "<<countentries<<std::endl;
  std::cout<<"Number of electrons+positrons: "<<countelectrons<<std::endl;
 
  //h1->Draw("same");
  //gPad->SetLogy();  
  
  c1->Print("VD8e+-.png");
  //c1->Print("electronspositrons_analyserVD8.pdf");
 
}



//================================================================
void ReadGen_plot(std::string ArtFiles_location) {
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

    
  //0 is to print gen momentum of e+, e- at VD=8
  PrintHisto(file_name, 100, -5, 80, 0, 15000000, "p_{e^{#pm}, VD=8} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, -5, 80, 0, 15000000, "p_{e^{+}, VD=8} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, -5, 80, 0, 15000000, "p_{e^{-}, VD=8} [MeV]", "Counts", 0);
  //1 is to print gen momentum of mu+, mu- at VD=8
  //PrintHisto(file_name, 100, -5, 100, 0, 15000000, "p_{#mu^{#pm}, VD=8} [MeV]", "Counts", 1);    
  //PrintHisto(file_name, 100, -5, 100, 0, 15000000, "p_{#mu^{-}, VD=8} [MeV]", "Counts", 1); 
  //PrintHisto(file_name, 100, -5, 80, 0, 15000000, "p_{e^{#pm}, VD=8} [MeV]", "Counts", 1);  
  //PrintHisto(file_name, 100, -5, 100, 0, 15000000, "p_{#mu^{-}, VD=8} [MeV]", "Counts", 1);
  
  //2 is to print gen momentum of gammas at VD=8
  //PrintHisto(file_name, 100, -5, 40, 0, 15000000, "p_{#gamma, VD=8} [MeV]", "Counts", 2);
  //3 is to print gen momentum of p at VD=8
  //PrintHisto(file_name, 100, -5, 150, 0, 15000000, "p_{p, VD=8} [MeV]", "Counts",3);
  //4 is to print gen momentum of pi- at VD=8
  //PrintHisto(file_name, 100, -5, 150, 0, 15000000, "p_{#pi^{-}, VD=8} [MeV]", "Counts", 4);



  //1 is x,y position hit of e+, e- Gen pos
  //PrintGraph(file_name, -4200, -3600, -300, 300, "x_{e^{#pm}}(Mu2e) [mm]", "y_{e^{#pm}}(Mu2e) [mm]", 1); 
  //2 is x,y position hit of rest of particles in VD
  //PrintGraph(file_name, -4200, -3600, -300, 300, "x_{#mu^{#pm},#pi^{-},p,#gamma}(Mu2e) [mm]", "y_{#mu^{#pm},#pi^{-},p,#gamma}(Mu2e) [mm]", 2);

  readfile.close();
}

//================================================================
