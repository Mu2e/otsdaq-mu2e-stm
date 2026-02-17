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
#include "TRandom.h"

#define mm_to_cm 10
#define muonmass 105.65

//================================================================
//This program reads readST/treeST trees: Particles at Stopping Target
//Reads the root files from a txt and analyse the trees
//================================================================
void ReadRootFile(std::string rootinput,int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);

  bool smearing=true;
  double sigma_smearing = 80; //ns

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
  gPad->SetLogy();
  graph1->SetMinimum(0.00000001);
  graph1->SetMaximum(1);


  std::cout<<"Reading code from: "<<rootinput<<std::endl;

  TH1F*h1 = new TH1F("TH1","", nbins, Xrange[0], Xrange[1]);
  TH1F*h2 = new TH1F("TH2","", nbins, Xrange[0], Xrange[1]);
  TH1F*h3 = new TH1F("TH3","", nbins, Xrange[0], Xrange[1]);
  TH1F*h4 = new TH1F("TH4","", nbins, Xrange[0], Xrange[1]);
  TH1F*h5 = new TH1F("TH5","", nbins, Xrange[0], Xrange[1]);

  TH2D *h = new TH2D("TH2D","", nbins, Xrange[0], Xrange[1], nbins,Yrange[0],Yrange[1]);

  TGraph *gr = new TGraph ();


  double binsprofile = 25;
  TProfile *hprofx = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");
  TProfile *hprofy = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");

  int Nprofiles = 3;
  TProfile *hprofx_phot[Nprofiles];
  TProfile *hprofy_phot[Nprofiles];
  for(int i=0;i<Nprofiles;i++){
      hprofx_phot[i] = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");
      hprofy_phot[i] = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");
  }



  TFile *input=new TFile(rootinput.c_str());
  TTree* Eventtree=(TTree*)input->Get("Eventtree");
  TTree* Bremstree=(TTree*)input->Get("Bremstree");
  TTree* Muontree=(TTree*)input->Get("Muontree");
  TTree* Photontree=(TTree*)input->Get("Photontree");

  float time_event, energy_event, pdgID_event, pcode_event;
  float time_brems_e, energy_brems_e, pdgID_brems_e, pcode_brems_e, event_num_brems_e;
  float time_muon, energy_muon, pdgID_muon, pcode_muon;
  float time_photon, energy_photon, pdgID_photon, pcode_photon, trackID_photon, event_num_photon;


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

  Double_t scale_photons_flash = 1;
  Double_t scale_pions = 1;
  Double_t scale_muons = 1;
  Double_t scale_electrons = 1;
  Double_t scale_muons_stopped = 1;
  Double_t scale_eplusminus = 1;
  Double_t scale_photons = 1;

  Double_t scale_POT = 200000000;

  /*Double_t scale_photons_flash = 100000;
  Double_t scale_pions = 70000;
  Double_t scale_muons = 300;
  Double_t scale_electrons = 4;
  Double_t scale_muons_stopped = 300;
  Double_t scale_eplusminus = 4;
  Double_t scale_photons = 8;*/


  double window_stoppedmuons=0;
  double window_muons=0;
  double window_eplusminusbrems=0;
  double window_photons=0;
  double stoppedmuons=0;
  double muons=0;
  double eplusminusbrems=0;
  double photons=0;


  double t1 = 0;
  double t2 = 400000000; //ns

  //double t1 = 1000;
  //double t2 = 1600; //ns

  //For TGraph
  double j=0;

  double cutEnergyBrems_keV = 0;
  double cutEnergyBrems_MeV = cutEnergyBrems_keV*0.001;

  for(unsigned long i=0;i<entriesEvent;i++){
    Eventtree->GetEntry(i);
    if(pdgID_event==13){if(smearing==true){double t = gRandom->Gaus(time_event,sigma_smearing); h1->Fill(t);}
      else{h1->Fill(time_event);}
      muons++;
      if((time_event>=t1)&&(time_event<=t2)){window_muons++;}}
    if((pdgID_event==11)||(pdgID_event==-11)){
      double energy_event_keV = energy_event*1000;
      if(time_event<1695){
        gr->SetPoint(j,energy_event_keV,time_event); j++;
      }
      h->Fill(time_event,energy_event);
      if((branch==0)||(branch==2)){hprofx->Fill(time_event,time_event);}
      if(branch==0){hprofy->Fill(time_event,energy_event);}
      if(branch==2){hprofy->Fill(time_event,energy_event_keV);}
      if(branch==5){
        if(time_event<1695){
          hprofx_phot[1]->Fill(energy_event_keV,energy_event_keV);
          hprofy_phot[1]->Fill(energy_event_keV,time_event);/*keV*/
      }//if time photon
    }
  }
}

  for(unsigned long i=0;i<entriesBrems;i++){
    Bremstree->GetEntry(i);
    if(smearing==true){double t = gRandom->Gaus(time_brems_e,sigma_smearing); h2->Fill(t);}
    else{h2->Fill(time_brems_e);}
    eplusminusbrems++;

    if((time_brems_e>=t1)&&(time_brems_e<=t2)){window_eplusminusbrems++;}
    if(branch==3){
      hprofx->Fill(time_brems_e,time_brems_e);
      hprofy->Fill(time_brems_e,energy_brems_e*1000);/*keV*/}

      //if(event_num_brems_e==0){std::cout<<"Brems e+- in event: "<<event_num_brems_e<<" E: "<<energy_brems_e<<" time: "<<time_brems_e<<std::endl;}
  }

  for(unsigned long i=0;i<entriesPhoton;i++){
    Photontree->GetEntry(i);
    //std::cout<<"track id: "<<trackID_photon<<" E: "<<energy_photon<<" time: "<<time_photon<<" pcode_photon: "<<pcode_photon<<std::endl;
    //if(trackID_photon!=auxtrack_phot){ //use this for ST_events_bremsstrahlung_new.root where the event number is not stored
    if((trackID_photon!=auxtrack_phot)||(event_num_photon!=aux_event_num_photon)){
      //std::cout<<"STORE: Brems photon in event: "<<event_num_photon<<" trackID: "<<trackID_photon<<" E: "<<energy_photon<<" time: "<<time_photon<<" pcode_photon: "<<pcode_photon<<std::endl;
        if(branch==4){
          hprofx_phot[0]->Fill(time_photon,time_photon);
          hprofy_phot[0]->Fill(time_photon,energy_photon*1000);/*keV*/
          if((energy_photon*1000)>100){
            hprofx_phot[1]->Fill(time_photon,time_photon);
            hprofy_phot[1]->Fill(time_photon,energy_photon*1000);/*keV*/
          }
          if((energy_photon*1000)>200){
            hprofx_phot[2]->Fill(time_photon,time_photon);
            hprofy_phot[2]->Fill(time_photon,energy_photon*1000);/*keV*/
          }
        }//if branch==4

        if(branch==5){
          //std::cout<<"Energy photon: "<<energy_photon*1000<<" keV, time: "<<time_photon<<" ns"<<std::endl;
          if(time_photon<1695){
          hprofx_phot[0]->Fill(energy_photon*1000,energy_photon*1000);
          hprofy_phot[0]->Fill(energy_photon*1000,time_photon);/*keV*/
        }//if time photon
        }//if branch==5


        if(energy_photon>cutEnergyBrems_MeV){
          if(smearing==true){double t = gRandom->Gaus(time_photon,sigma_smearing); h4->Fill(t);}
          else{h4->Fill(time_photon);}
          photons++;
          if((time_photon>=t1)&&(time_photon<=t2)){window_photons++;}
        }

      }
    auxtrack_phot=trackID_photon;
    aux_event_num_photon=event_num_photon;
  }


  for(unsigned long i=0;i<entriesMuon;i++){
    Muontree->GetEntry(i);
    if(pcode_muon==32){
      if(smearing==true){double t = gRandom->Gaus(time_muon,sigma_smearing); h3->Fill(t);}
      else{h3->Fill(time_muon);}
      stoppedmuons++; if((time_muon>=t1)&&(time_muon<=t2)){window_stoppedmuons++;}}
  }


  auto leg = new TLegend(0.1,0.7,0.48,0.9);

  if((branch==0)||(branch==2)||(branch==3)){gr->SetMarkerStyle(1); gr->SetMarkerColor(kOrange+2);
    gr->Draw("same,p");
    //h->Draw("colz");
    vector<double> time_event_v,energy_event_v,time_event_error, energy_event_error;
    time_event_v.clear();
    energy_event_v.clear();
    time_event_error.clear();
    energy_event_error.clear();
    TAxis *xaxis = hprofx->GetXaxis();
    int nbins_prof = xaxis->GetNbins();
    cout<<"nbins profile: "<<nbins_prof<<std::endl;
    for(int bin=0; bin<nbins_prof;bin++){
      double x_value = hprofx->GetBinContent(bin);
      double y_value = hprofy->GetBinContent(bin);
      double x_error = hprofx->GetBinError(bin);
      double y_error = hprofy->GetBinError(bin);
      std::cout<<"time_event: "<<x_value<<" ns, energy_event: "<<y_value<<" error energy: "<<y_error<<std::endl;
      //Fill it just if energy is bigger than 0
      if(y_value!=0){
        time_event_v.push_back(x_value);
        energy_event_v.push_back(y_value);
        time_event_error.push_back(x_error);
        energy_event_error.push_back(y_error);
      }
    }
    TGraphErrors *ProfileErrors = new TGraphErrors(time_event_v.size(),&time_event_v[0],&energy_event_v[0],0,&energy_event_error[0]);
    ProfileErrors->SetMarkerStyle(21);
    //ProfileErrors->SetMarkerColor(kOrange+2);
    ProfileErrors->SetMarkerColor(kGreen-3);
    //ProfileErrors->Draw("same,p");

  }

  if(branch==1){

    h1->SetLineColor(kBlue+2);
    h2->SetLineColor(kOrange+2);
    h3->SetLineColor(kRed-7);
    h4->SetLineColor(kGreen-3);
    h1->SetFillColor(kBlue+2);
    h2->SetFillColor(kOrange+2);
    h3->SetFillColor(kRed-7);
    h4->SetFillColor(kGreen-3);
    h1->SetFillStyle(3001);
    h2->SetFillStyle(3001);
    h3->SetFillStyle(3001);
    h4->SetFillStyle(3001);

    /*h1->Scale(scale_muons);
    h2->Scale(scale_eplusminus);
    h3->Scale(scale_muons_stopped);
    h4->Scale(scale_photons);*/

    h1->Scale(1./scale_POT);
    h2->Scale(1./scale_POT);
    h3->Scale(1./scale_POT);
    h4->Scale(1./scale_POT);

    h4->Draw("HIST,same");
    h2->Draw("HIST,same");
    h1->Draw("HIST,same");
    h3->Draw("HIST,same");

    std::string str_latex = "#scale[1]{Mu2e simulation (2#times10^{8} POT)}";
    char* char_latex = const_cast<char*>(str_latex.c_str());
    TLatex latex;
    latex.DrawLatexNDC(.3,.85,char_latex);

    std::string str_scale_eplusminus = std::to_string(int(scale_eplusminus));
    //std::string str_scale_eplusminus_chain = "Bremsstrahlung e^{#pm} arrival time at ST (#times"+str_scale_eplusminus+")";
    std::string str_scale_eplusminus_chain = "#font[42]{e^{#pm} to #gamma (Bremsstrahlung)}";
    char* char_brems_eplusminus = const_cast<char*>(str_scale_eplusminus_chain.c_str());

    std::string str_scale_muons = std::to_string(int(scale_muons));
    //std::string str_scale_muons_chain = "#mu^{-} arrival time at ST (#times"+str_scale_muons+")";
    std::string str_scale_muons_chain = "#font[42]{#mu^{-} Beam}";
    char* char_scale_muons = const_cast<char*>(str_scale_muons_chain.c_str());


    std::string str_scale_muons_stopped = std::to_string(int(scale_muons_stopped));
    //std::string str_scale_muons_stopped_chain = "Stopped #mu^{-} arrival time at ST (#times"+str_scale_muons_stopped+")";
    std::string str_scale_muons_stopped_chain = "#font[42]{Stopped #mu^{-}}";
    char* char_scale_muons_stopped = const_cast<char*>(str_scale_muons_stopped_chain.c_str());

    std::string str_scale_photons = std::to_string(int(scale_photons));
    //std::string str_scale_photons_chain = "Flash #gamma at ST (#times"+str_scale_photons+")";
    std::string str_scale_photons_chain = "#font[42]{Flash #gamma}";
    char* char_scale_photons = const_cast<char*>(str_scale_photons_chain.c_str());

    leg->AddEntry(h4, char_scale_photons,"l");
    leg->AddEntry(h2, char_brems_eplusminus,"l");
    leg->AddEntry(h1, char_scale_muons,"l");
    leg->AddEntry(h3, char_scale_muons_stopped,"l");
    leg->Draw("same");
  }



  if((branch==4)||(branch==5)){
    TGraph *GraphEntries[Nprofiles];
    TGraphErrors *ProfileErrors[Nprofiles];
    for(int i=0;i<Nprofiles;i++){
    GraphEntries[i]= new TGraph ();
    vector<double> x_v, y_v, x_error_v, y_error_v;
    x_v.clear();
    y_v.clear();
    x_error_v.clear();
    y_error_v.clear();
    TAxis *xaxis = hprofx->GetXaxis();
    int nbins_prof = xaxis->GetNbins();
    cout<<"nbins profile: "<<nbins_prof<<std::endl;
    double p=0;
    for(int bin=0; bin<nbins_prof;bin++){
      double x_value = hprofx_phot[i]->GetBinContent(bin);
      double y_value = hprofy_phot[i]->GetBinContent(bin);
      double x_error = hprofx_phot[i]->GetBinError(bin);
      double y_error = hprofy_phot[i]->GetBinError(bin);
      //std::cout<<"time_event: "<<x_value<<" ns, energy_event: "<<y_value<<" error energy: "<<y_error<<std::endl;
      //Number of entries: One entry in x
      double x_entries = hprofx_phot[i]->GetBinEntries(bin);
      double y_entries = hprofy_phot[i]->GetBinEntries(bin);
      std::cout<<"x: "<<x_value<<" , y: "<<y_value<<" Entries in y: "<<y_entries<<std::endl;

      //Fill it just if energy is bigger than 0
      if(y_value!=0){
        GraphEntries[i]->SetPoint(p,x_value,y_entries); p++;
        x_v.push_back(x_value);
        y_v.push_back(y_value);
        x_error_v.push_back(x_error);
        y_error_v.push_back(y_error);
      }
    }
    ProfileErrors[i] = new TGraphErrors(x_v.size(),&x_v[0],&y_v[0],0,&y_error_v[0]);
    ProfileErrors[i]->SetMarkerStyle(21);
    GraphEntries[i]->SetMarkerStyle(21);

  }

  if(branch==4){
  ProfileErrors[0]->SetMarkerColor(kGreen-3);
  GraphEntries[0]->SetMarkerColor(kGreen-3);
  ProfileErrors[1]->SetMarkerColor(kPink-3);
  GraphEntries[1]->SetMarkerColor(kPink-3);
  ProfileErrors[2]->SetMarkerColor(kBlue-3);
  GraphEntries[2]->SetMarkerColor(kBlue-3);

  ProfileErrors[0]->Draw("same,p");
  ProfileErrors[1]->Draw("same,p");
  ProfileErrors[2]->Draw("same,p");

  leg->AddEntry(ProfileErrors[0], "All E_{#gamma}","p");
  leg->AddEntry(ProfileErrors[1], "E_{#gamma} > 100 keV","p");
  leg->AddEntry(ProfileErrors[2], "E_{#gamma} > 200 keV","p");
  leg->Draw("same");

  /*GraphEntries[0]->Draw("same,p");
  GraphEntries[1]->Draw("same,p");
  GraphEntries[2]->Draw("same,p");

  leg->AddEntry(GraphEntries[0], "All E_{#gamma}","p");
  leg->AddEntry(GraphEntries[1], "E_{#gamma} > 100 keV","p");
  leg->AddEntry(GraphEntries[2], "E_{#gamma} > 200 keV","p");
  leg->Draw("same");*/
  }

  if(branch==5){
    ProfileErrors[0]->SetMarkerColor(kGreen-3);
    ProfileErrors[1]->SetMarkerColor(kOrange+2);
    ProfileErrors[0]->Draw("same,p");
    ProfileErrors[1]->Draw("same,p");

    leg->AddEntry(ProfileErrors[1], "e^{#pm} ST event, t < 1695 ns","p");
    leg->AddEntry(ProfileErrors[0], "All flash #gamma gen at ST, t < 1695 ns","p");
    leg->Draw("same");
  }

  }

  //Black axis
  gPad->RedrawAxis();


  std::cout<<""<<std::endl;
  std::cout<<"Total number of mu- arriving at ST: "<<muons<<" mean: "<<h1->GetMean()<<" ns"<<std::endl;
  std::cout<<"Total number of mu- stopped at ST: "<<stoppedmuons<<" mean: "<<h3->GetMean()<<" ns"<<std::endl;
  std::cout<<"Total number of e+- emmiting bremsstrahlung at ST: "<<eplusminusbrems<<" mean: "<<h2->GetMean()<<" ns"<<std::endl;
  std::cout<<"Photons generated at ST (Egamma>"<<cutEnergyBrems_keV<< " keV): "<<photons<<" mean: "<<h4->GetMean()<<" ns"<<std::endl;
  std::cout<<""<<std::endl;
  std::cout<<"Number of mu- arriving at ST between "<<t1<<"ns and "<<t2<<" ns: "<<window_muons<<std::endl;
  std::cout<<"Number of mu- stopped at ST between "<<t1<<"ns and "<<t2<<" ns: "<<window_stoppedmuons<<std::endl;
  std::cout<<"Number of e+- emmiting bremsstrahlung at ST between "<<t1<<" ns and "<<t2<<" ns: "<<window_eplusminusbrems<<std::endl;
  std::cout<<"Number of flash photons generated at ST between "<<t1<<" ns and "<<t2<<" ns with (Egamma>"<<cutEnergyBrems_keV<< " keV):"<<window_photons<<std::endl;
}



//================================================================
void LogScale_arrivaltimes(std::string ArtFiles_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log");

  //Open txt
/*fstream readfile;
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
  }*/


  //branch 0 prints brems electrons hits, muon arrival time event and stopped muons (process code =32) in y log scale
  //ReadRootFile(ArtFiles_location, 170, 0, 1700, 0, 70000000, "t (at ST) [ns]", "(N / 10 ns) / POT", 1);
  ReadRootFile(ArtFiles_location, 170, -500, 1700, 0, 70000000, "t+smearing (at ST) [ns]", "(N / 10 ns) / POT", 1);


  //ReadRootFile("ST_events_bremsstrahlung_ReRunEleBeam.root", 1000, 0, 1200, 0, 500, "E [keV]", "t [ns]", 5);


  //readfile.close();
}

//================================================================
