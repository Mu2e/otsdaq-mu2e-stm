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

#define mm_to_cm 10
#define muonmass 105.65

//================================================================
//This program reads readST/treeST trees: Particles at Stopping Target
//Reads the root files from a txt and analyse the trees
//================================================================  

void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout = false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);  
  
  //step branches 
  std::vector<double> *VolIdstep=0, *Edepstep=0, *pathlengthstep=0, *Xmu2estep=0, *Ymu2estep=0, *Zmu2estep=0, *Xmu2estepend=0, *Ymu2estepend=0, *Zmu2estepend=0, *momstartstep=0, *momendstep=0, *pxstartstep=0, *pystartstep=0, *pzstartstep=0, *pxendstep=0, *pyendstep=0,*pzendstep=0, *pdgidstep=0, *Kestartstep=0, *Keendstep=0, *Etotstartstep=0, *Etotendstep=0 /*Etotstart and Etotend is for the original sim particle not for the hit, for the hit use momend and momstart*/, *ProcessCode=0, *trackIdnum=0, *timestep=0;
  //event branches          
  std::vector<double> *Etotstartsimpart=0, *Etotendsimpart=0, *NhitsSTevt=0, *Edepevt=0, *pathlengthevt=0, *Nhitsfoilevt=0;


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
  TH1F*h4 = new TH1F("TH4","", nbins, Xrange[0], Xrange[1]);
  TH1F*h5 = new TH1F("TH5","", nbins, Xrange[0], Xrange[1]);

  double photons=0;
  double pions=0;
  double muons=0;
  double electrons=0;
  double stoppedmuons=0;
  double ebrems=0;

  Double_t scale_photons = 100000;
  Double_t scale_pions = 70000;
  Double_t scale_muons = 300;
  Double_t scale_electrons = 4;
  Double_t scale_muons_stopped = 300;
  Double_t scale_eplusminus = 4;

  double window_stoppedmuons=0;
  double window_muons=0;
  double window_eplusminusbrems=0;

  double t1 = 0;
  double t2 = 500; //ns

  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<"file #: "<<file<<" "<<path.c_str()<<std::endl;
   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readST/treeST");
   

    /*tree->SetBranchAddress("VolIdstep",&VolIdstep);
    tree->SetBranchAddress("Edepstep",&Edepstep);
    tree->SetBranchAddress("pathlengthstep",&pathlengthstep);
    tree->SetBranchAddress("Xmu2estep",&Xmu2estep);
    tree->SetBranchAddress("Ymu2estep",&Ymu2estep);
    tree->SetBranchAddress("Zmu2estep",&Zmu2estep);
    tree->SetBranchAddress("Xmu2estepend",&Xmu2estepend);
    tree->SetBranchAddress("Ymu2estepend",&Ymu2estepend);
    tree->SetBranchAddress("Zmu2estepend",&Zmu2estepend);
    tree->SetBranchAddress("momstartstep",&momstartstep);
    tree->SetBranchAddress("momendstep",&momendstep);
    tree->SetBranchAddress("pxstartstep",&pxstartstep);
    tree->SetBranchAddress("pystartstep",&pystartstep);
    tree->SetBranchAddress("pzstartstep",&pzstartstep);
    tree->SetBranchAddress("pxendstep",&pxendstep);
    tree->SetBranchAddress("pyendstep",&pyendstep);
    tree->SetBranchAddress("pzendstep",&pzendstep);*/
    tree->SetBranchAddress("pdgidstep",&pdgidstep);
    tree->SetBranchAddress("Kestartstep",&Kestartstep);
    tree->SetBranchAddress("Keendstep",&Keendstep);
    tree->SetBranchAddress("Etotstartstep",&Etotstartstep);
    tree->SetBranchAddress("Etotendstep",&Etotendstep);
    tree->SetBranchAddress("Etotstartsimpart",&Etotstartsimpart);
    tree->SetBranchAddress("Etotendsimpart",&Etotendsimpart);
    tree->SetBranchAddress("ProcessCode",&ProcessCode);
    tree->SetBranchAddress("trackIdnum",&trackIdnum);
    tree->SetBranchAddress("NhitsSTevt",&NhitsSTevt);
    tree->SetBranchAddress("Edepevt",&Edepevt);
    tree->SetBranchAddress("pathlengthevt",&pathlengthevt);
    tree->SetBranchAddress("Nhitsfoilevt",&Nhitsfoilevt);
    tree->SetBranchAddress("timestep",&timestep);


    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;
 
    for(unsigned long i=0;i<entries;i++){
      
      tree->GetEntry(i);

      double NhitsST_evt = NhitsSTevt->at(0);
      double Edep_evt = Edepevt->at(0);
      double pathlength_evt = pathlengthevt->at(0);

      double VolId_step, Edep_step, pathlength_step, Xmu2e_step, Ymu2e_step, Zmu2e_step, Xmu2eend_step, Ymu2eend_step, Zmu2eend_step, momstart_step, momend_step, pxstart_step, pystart_step, pzstart_step, pxend_step, pyend_step, pzend_step, pdgid_step, Kestart_step, Etotstart_step, Etotstart_simpart, Etotend_simpart, Process_Code, trackId_num, time_step;

      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
	std::cout<<"size hits: "<<NhitsST_evt<<std::endl;
      }
    
      if(NhitsST_evt!=0){

	/*if(file==0){
	  std::cout<<"Event with edep #: "<<i<<", Number of hits in ST in this event: "<<NhitsST_evt<<" (should be the same as: "<<VolIdstep->size()<<")"<<" Etot deposited event: "<<Edep_evt<<" path length event: "<<pathlength_evt<<std::endl;
	  }*/



	for(unsigned long int j = 0; j < NhitsST_evt; j++){

	  /*VolId_step = VolIdstep->at(j);
	   Edep_step = Edepstep->at(j);
	   pathlength_step = pathlengthstep->at(j);
	   Xmu2e_step = Xmu2estep->at(j);
	   Ymu2e_step = Ymu2estep->at(j);
	   Zmu2e_step = Zmu2estep->at(j);
	   Xmu2eend_step = Xmu2estepend->at(j);
           Ymu2eend_step = Ymu2estepend->at(j);
           Zmu2eend_step = Zmu2estepend->at(j);
	   momstart_step = momstartstep->at(j);
	   momend_step = momendstep->at(j);
	   pxstart_step = pxstartstep->at(j); 
	   pystart_step = pystartstep->at(j);
	   pzstart_step = pzstartstep->at(j);
	   pxend_step = pxendstep->at(j);
           pyend_step = pyendstep->at(j);
           pzend_step = pzendstep->at(j);
	   Kestart_step = Kestartstep->at(j);*/
	   Etotstart_step = Etotstartstep->at(j);
	   Etotstart_simpart = Etotstartsimpart->at(j);
	   Etotend_simpart = Etotendsimpart->at(j);
	   Process_Code = ProcessCode->at(j);
	   trackId_num = trackIdnum->at(j);
	   pdgid_step = pdgidstep->at(j);   
	   time_step = timestep->at(j);


	   if((debugout==true)&&(file==0)){
	     std::cout<<" "<<std::endl;
	     std::cout<<"--step #: "<<j<<std::endl;
	     std::cout<<"PDG Id: "<<pdgid_step<<std::endl;
	     std::cout<<"Time from VD8 to ST step: "<<time_step<<std::endl;
	     /* std::cout<<"VolIdstep (foil #): "<<VolId_step<<std::endl;
	     std::cout<<"Edepstep: "<<Edep_step<<std::endl;
	     std::cout<<"pathlengthstep: "<<pathlength_step<<" mm"<<std::endl;
	     std::cout<<"Xmu2e_step: "<<Xmu2e_step<<" Ymu2e_step: "<<Ymu2e_step<<" Zmu2e_step: "<<Zmu2e_step<<" mm"<<std::endl;
	     std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" MeV"<<std::endl;
	     std::cout<<"(px,py,pz)start: "<<pxstart_step<<" "<<pystart_step<<" "<<pzstart_step<<std::endl;
	     std::cout<<"Etotstart_step: "<<Etotstart_step<<" MeV"<<std::endl;*/
	     std::cout<<"Process_Code: "<<Process_Code<<" trackId_num: "<<trackId_num<<std::endl;
	     std::cout<<"Etotstart_simpart: "<<Etotstart_simpart<<" MeV"<<std::endl;
	   }



	   //time of first hit which would be the time of the simparticle
	   if(j==0){
	     //if(Etotstart_step!=Etotstart_simpart){std::cout<<"Warning first particle in the event doesn't correspond to the simparticle"<<std::endl; exit(0);}
	     if((branch==0)&&(pdgid_step==13)){h1->Fill(time_step);muons++;}
	     if((branch==0)&&(pdgid_step==-211)){h2->Fill(time_step);pions++;}
	     if((branch==0)&&(pdgid_step==22)){h3->Fill(time_step);photons++; std::cout<<"Photon arrival time: "<<time_step<<" ns"<<std::endl;}
	     if((branch==0)&&((pdgid_step==11)||(pdgid_step==-11))){h4->Fill(time_step);electrons++;}
	     //first muon which is the sim particle not all the muon hits
	     if((branch==1)&&(pdgid_step==13)){h1->Fill(time_step);muons++; if((time_step>=t1)&&(time_step<=t2)){window_muons++;}}
	   }


	   //muminus capture at rest ST
	   if(Process_Code==32){
             if((branch==0)&&(pdgid_step==13)){
               //std::cout<<"Event: "<<i<<" Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<" mom_simpart: "<<Etotstart_simpart<<std::endl;
               h5->Fill(time_step);
	       stoppedmuons++;
	     }
	     if((branch==1)&&(pdgid_step==13)){
               h3->Fill(time_step);
               stoppedmuons++;
	       if((time_step>=t1)&&(time_step<=t2)){window_stoppedmuons++;}
             }


           }//if Process_Code==32

	   //Electrons and positrons that emit brems at ST
           if(Process_Code==16){
             if(branch==1){
               //if electron or positron fill the histogram
	       if((pdgid_step==11)||(pdgid_step==-11)){h2->Fill(time_step);ebrems++;if((time_step>=t1)&&(time_step<=t2)){window_eplusminusbrems++;}}
	     }
	   } //if Process_Code==16 


	}//NhitsST_evt
	
      } //if(NhitsST_evt!=0)
    }//entries    
    input->Close();
  }//for int file
    
  h1->SetTitle("");
  //Comment to get the statistical box of histogram 
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  
  auto leg = new TLegend(0.1,0.7,0.48,0.9);

  if(branch==0){h1->SetLineColor(kBlue+2);
    h2->SetLineColor(kMagenta-7);
    h3->SetLineColor(kGreen+2);
    h4->SetLineColor(kBlue-7);
    h5->SetLineColor(kRed-7);
    h1->SetFillColor(kBlue+2);
    h2->SetFillColor(kMagenta-7);
    h3->SetFillColor(kGreen+2);
    h4->SetFillColor(kBlue-7);
    h5->SetFillColor(kRed-7);
    h1->SetFillStyle(3001);
    h2->SetFillStyle(3001);
    h3->SetFillStyle(3001);
    h4->SetFillStyle(3001);
    h5->SetFillStyle(3001);


  //Scaled histograms to these numbers
  std::string str_scale_muons = std::to_string(int(scale_muons));
  std::string str_scale_muons_chain = "#mu^{-} arrival time at ST (#times"+str_scale_muons+")";
  char* char_scale_muons = const_cast<char*>(str_scale_muons_chain.c_str());

  std::string str_scale_pions = std::to_string(int(scale_pions));
  std::string str_scale_pions_chain = "#pi^{-} arrival time at ST (#times"+str_scale_pions+")";
  char* char_scale_pions = const_cast<char*>(str_scale_pions_chain.c_str());

  std::string str_scale_photons = std::to_string(int(scale_photons));
  std::string str_scale_photons_chain = "#gamma arrival time at ST (#times"+str_scale_photons+")";
  char* char_scale_photons = const_cast<char*>(str_scale_photons_chain.c_str());

  std::string str_scale_electrons = std::to_string(int(scale_electrons));
  std::string str_scale_electrons_chain = "#font[42]{e}^{#pm} arrival time at ST (#times"+str_scale_electrons+")";
  char* char_scale_electrons = const_cast<char*>(str_scale_electrons_chain.c_str());

  std::string str_scale_muons_stopped = std::to_string(int(scale_muons_stopped));
  std::string str_scale_muons_stopped_chain = "Stopped #mu^{-} arrival time at ST (#times"+str_scale_muons_stopped+")";
  char* char_scale_muons_stopped = const_cast<char*>(str_scale_muons_stopped_chain.c_str());

  h1->Scale(scale_muons);
  h2->Scale(scale_pions);
  h3->Scale(scale_photons);
  h4->Scale(scale_electrons);
  h5->Scale(scale_muons_stopped);
  
  h2->Draw("HIST,same");
  h1->Draw("HIST,same");
  h4->Draw("HIST,same");
  h3->Draw("HIST,same");
  h5->Draw("HIST,same");
  std::string str_latex = "#scale[1]{Mu2e simulation, 2#times10^{8} POT}";
  char* char_latex = const_cast<char*>(str_latex.c_str());
  TLatex latex;
  latex.DrawLatexNDC(.3,.85,char_latex);

  leg->AddEntry(h3, char_scale_photons,"l");
  leg->AddEntry(h4, char_scale_electrons,"l");
  leg->AddEntry(h2, char_scale_pions,"l");
  leg->AddEntry(h1, char_scale_muons,"l");
  leg->AddEntry(h5, char_scale_muons_stopped,"l");
  leg->Draw("same");
  }//if branch=0


  if(branch==1){h1->SetLineColor(kBlue+2);
    h2->SetLineColor(kOrange+2);
    h3->SetLineColor(kRed-7);
    h1->SetFillColor(kBlue+2);
    h2->SetFillColor(kOrange+2);
    h3->SetFillColor(kRed-7);
    h1->SetFillStyle(3001);
    h2->SetFillStyle(3001);
    h3->SetFillStyle(3001);

    h1->Scale(scale_muons);
    h2->Scale(scale_eplusminus);
    h3->Scale(scale_muons_stopped);

    h1->Draw("HIST,same");
    h2->Draw("HIST,same");
    h3->Draw("HIST,same");

    std::string str_latex = "#scale[1]{Mu2e simulation, 2#times10^{8} POT}";
    char* char_latex = const_cast<char*>(str_latex.c_str());
    TLatex latex;
    latex.DrawLatexNDC(.3,.85,char_latex);
 
    std::string str_scale_eplusminus = std::to_string(int(scale_eplusminus));
    std::string str_scale_eplusminus_chain = "Bremsstrahlung e^{#pm} arrival time at ST (#times"+str_scale_eplusminus+")";
    char* char_brems_eplusminus = const_cast<char*>(str_scale_eplusminus_chain.c_str());

    std::string str_scale_muons = std::to_string(int(scale_muons));
    std::string str_scale_muons_chain = "#mu^{-} arrival time at ST (#times"+str_scale_muons+")";
    char* char_scale_muons = const_cast<char*>(str_scale_muons_chain.c_str());


    std::string str_scale_muons_stopped = std::to_string(int(scale_muons_stopped));
    std::string str_scale_muons_stopped_chain = "Stopped #mu^{-} arrival time at ST (#times"+str_scale_muons_stopped+")";
    char* char_scale_muons_stopped = const_cast<char*>(str_scale_muons_stopped_chain.c_str());

    leg->AddEntry(h2, char_brems_eplusminus,"l");
    leg->AddEntry(h1, char_scale_muons,"l");
    leg->AddEntry(h3, char_scale_muons_stopped,"l");
    leg->Draw("same");
  }


  std::cout<<"Total number of muons: "<<muons<<std::endl;
  std::cout<<"Total number of electrons: "<<electrons<<std::endl;
  std::cout<<"Total number of pions: "<<pions<<std::endl;
  std::cout<<"Total number of photons: "<<photons<<std::endl;
  std::cout<<"Total number of stopped muons: "<<stoppedmuons<<std::endl;
  std::cout<<" "<<std::endl;

  std::cout<<"Number of mu- arriving at ST between "<<t1<<"ns and"<<t2<<"ns: "<<window_muons<<std::endl;
  std::cout<<"Number of mu- stopped at ST between "<<t1<<"ns and"<<t2<<"ns: "<<window_stoppedmuons<<std::endl;
  std::cout<<"Number of e+- emmiting bremsstrahlung at ST between "<<t1<<"ns and"<<t2<<"ns: "<<window_eplusminusbrems<<std::endl;
    
  c1->Print("Bremsphotons_stoppedmuons_ST_EleMuonBeam.pdf");
  c1->Print("Bremsphotons_stoppedmuons_ST_EleMuonBeam.png");

}



//================================================================
void ReadST_arrivaltimes(std::string ArtFiles_location) {
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



  //Branch 0 is to print the arrival times of sim particles at the ST
  //PrintHisto(file_name, 50, 0, 500, 0, 70000000, "t [ns]", "N / 10 ns", 0);
  //Branch 1 is to print the arrival times of bremsstrahlung e+-and muons at the ST or stopped muons
  PrintHisto(file_name, 50, 0, 500, 0, 70000000, "t [ns]", "N / 10 ns", 1);


  readfile.close();
}

//================================================================
