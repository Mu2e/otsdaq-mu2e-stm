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
#include "TProfile.h"
#include "TGraphErrors.h"


#define mm_to_cm 10
#define muonmass 105.65

//================================================================
//This program reads readST/treeST trees: Particles at Stopping Target
//Reads the root files from a txt and analyse the trees
//================================================================  
void PrintGraph(vector<string> file_name, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);


  //just for TH2D
  int palette_number = 55;
  gStyle->SetPalette(palette_number);
  gStyle->SetPadRightMargin(0.14);

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
  
  TGraph *gr = new TGraph ();
  TH2D *h = new TH2D("TH2D","",100,xmin,xmax,100,ymin,ymax);
  double binsprofile = 11;
  TProfile *hprofx = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");
  TProfile *hprofy = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");

  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
  //for (long unsigned int file=0;file<1;file++){
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readST/treeST");


    tree->SetBranchAddress("VolIdstep",&VolIdstep);
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
    tree->SetBranchAddress("pzendstep",&pzendstep);
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
    unsigned long int p=0;


    for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);      
      double NhitsST_evt = NhitsSTevt->at(0);
      double Edep_evt = Edepevt->at(0);
      double pathlength_evt = pathlengthevt->at(0);

      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
        std::cout<<"size hits: "<<NhitsST_evt<<std::endl;
      }
      if(NhitsST_evt!=0){
       
	/*if(file==0){
	  std::cout<<"Event with edep #: "<<i<<", Number of hits in ST in this event: "<<NhitsST_evt<<" (should be the same as: "<<VolIdstep->size()<<")"<<" Etot deposited event: "<<Edep_evt<<" path length event: "<<pathlength_evt<<std::endl;
	  }*/

	for(unsigned long int j = 0; j < NhitsST_evt; j++){
       
	  double VolId_step = VolIdstep->at(j);
	  double Edep_step = Edepstep->at(j);
	  double pathlength_step = pathlengthstep->at(j);
	  double Xmu2e_step = Xmu2estep->at(j);
	  double Ymu2e_step = Ymu2estep->at(j);
	  double Zmu2e_step = Zmu2estep->at(j);
	  double Xmu2eend_step = Xmu2estepend->at(j);
          double Ymu2eend_step = Ymu2estepend->at(j);
          double Zmu2eend_step = Zmu2estepend->at(j);
	  double momstart_step = momstartstep->at(j);
	  double momend_step = momendstep->at(j);
	  double pxstart_step = pxstartstep->at(j); 
	  double pystart_step = pystartstep->at(j);
	  double pzstart_step = pzstartstep->at(j);
	  double pxend_step = pxendstep->at(j);
          double pyend_step = pyendstep->at(j);
          double pzend_step = pzendstep->at(j);
	  double pdgid_step = pdgidstep->at(j);
	  double Kestart_step = Kestartstep->at(j);
	  //double Keend_step = Keendstep->at(j); //this is wrong stored initial
	  double Etotstart_step = Etotstartstep->at(j);
	  //double Etotend_step = Etotendstep->at(j); //this is wrong stored initial 
	  double Etotstart_simpart = Etotstartsimpart->at(j);
	  double Etotend_simpart = Etotendsimpart->at(j);
	  double Process_Code = ProcessCode->at(j);
	  double trackId_num = trackIdnum->at(j);
	  double time_step = timestep->at(j);


	 
	  if((debugout==true)&&(file==0)){
	    std::cout<<"--step #: "<<j<<std::endl;
	    std::cout<<"PDG Id: "<<pdgid_step<<std::endl;
	    std::cout<<"VolIdstep (foil #): "<<VolId_step<<std::endl;
	    std::cout<<"Edepstep: "<<Edep_step<<std::endl;
	    std::cout<<"pathlengthstep: "<<pathlength_step<<" mm"<<std::endl;
	    std::cout<<"Xmu2e_step: "<<Xmu2e_step<<" Ymu2e_step: "<<Ymu2e_step<<" Zmu2e_step: "<<Zmu2e_step<<" mm"<<std::endl;
	    std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" MeV"<<std::endl;
	    std::cout<<"Etotstart_step: "<<Etotstart_step<<" MeV"<<std::endl;
	    std::cout<<"Process_Code: "<<Process_Code<<" trackId_num: "<<trackId_num<<std::endl;
	  }

	  double xcentre = -3904;//mm
	  double ycentre = 0;
	  
	  if(branch==0){gr->SetPoint(p,Xmu2e_step,Ymu2e_step); p++;}
	  if(branch==1){gr->SetPoint(p,Zmu2e_step,Ymu2e_step); p++;}
	  //Photons at ST
	  if((branch==2)&&(pdgid_step==22)&&((Zmu2e_step/mm_to_cm)>615)){h->Fill(Xmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm);}
	  //Stopped muons at ST
	  //if((branch==3)&&(Process_Cod==32)&&(pdgid_step==13)&&((Zmu2e_step/mm_to_cm)<560)){h->Fill(Xmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm);}
	  //if((branch==3)&&(Process_Code==32)&&(pdgid_step==13)&&((Zmu2e_step/mm_to_cm)>615)){h->Fill(Xmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm);}  
	  if((branch==3)&&(Process_Code==32)&&(pdgid_step==13)){h->Fill(Xmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm);}
	  
	 
	  double r = sqrt((Xmu2e_step-xcentre)*(Xmu2e_step-xcentre)+(Ymu2e_step-ycentre)*(Ymu2e_step-ycentre));
	  //if((branch==3)&&(Process_Code==32)&&(pdgid_step==13)&&(r > 75)){h->Fill(Xmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm);}

	  //Profile momentum stopped muons
	  if((branch==4)&&(Process_Code==32)&&(pdgid_step==13)){
	    hprofx->Fill(Zmu2e_step/mm_to_cm,Zmu2e_step/mm_to_cm);
	    //Because muons are stopped with momentum =0
	    hprofy->Fill(Zmu2e_step/mm_to_cm,sqrt(Etotstart_simpart*Etotstart_simpart-muonmass*muonmass));
	    //hprofy->Fill(Zmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm); 
	    //hprofy->Fill(Zmu2e_step/mm_to_cm,Xmu2e_step/mm_to_cm);
	  }

	}//NhitsST_evt
	/*
      n=n+NhitsST_evt;
      int Nfoils = Nhitsfoilevt->size()/entries;
      std::cout<<"Number of foils in ST: "<<Nfoils<<std::endl;
      for (int k = 0; k<Nfoils; k++){
      std::cout<<"Foil n: "<<k<<" with "<<Nhitsfoilevt->at(k)<<" hits"<<std::endl;
      }
	*/
      } //if(NhitsST_evt!=0)
    }//entries    
    input->Close();
  }//for int file

 

  if((branch==0)||(branch==1)){
    gr->SetTitle("");
    gr->GetXaxis()->SetTitle(Xtitle);
    gr->GetYaxis()->SetTitle(Ytitle);
    gr->SetMarkerStyle(1);
    gr->SetMarkerColor(kBlack);
    //gr->Draw("same,p");
  }
  
  if((branch==2)||(branch==3)){
  h->GetXaxis()->SetTitle(Xtitle);
  h->GetYaxis()->SetTitle(Ytitle);
  h->GetZaxis()->SetLabelSize(0.04);
  h->Draw("colz");
}
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



  if(branch==4){
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
      double x_value = hprofx->GetBinContent(bin);
      double y_value = hprofy->GetBinContent(bin);
      double x_error = hprofx->GetBinError(bin);
      double y_error = hprofy->GetBinError(bin);
      //std::cout<<"time_event: "<<x_value<<" ns, energy_event: "<<y_value<<" error energy: "<<y_error<<std::endl;
      double x_entries = hprofx->GetBinEntries(bin);
      double y_entries = hprofy->GetBinEntries(bin);
      std::cout<<"x: "<<x_value<<" , y: "<<y_value<<" Entries in y: "<<y_entries<<std::endl;

     if(y_value!=0){
	x_v.push_back(x_value);
	y_v.push_back(y_value);
	x_error_v.push_back(x_error);
	y_error_v.push_back(y_error);
	}
    }
      TGraphErrors *ProfileErrors = new TGraphErrors(x_v.size(),&x_v[0],&y_v[0],0,&y_error_v[0]);
      ProfileErrors->SetMarkerStyle(21);
      ProfileErrors->SetMarkerStyle(21);
      ProfileErrors->SetMarkerColor(kMagenta-3);
      ProfileErrors->Draw("same,p");
}



  //c1->Print("MuonBeamMDC2020_xypositions_allstoppedmuons_externalfoils.pdf");
  c1->Print("xyheatmap-stoppedmuons_MDC2020.png");
  
}

//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout = false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);  

  //Number of POT in Mu2e sim
  //double POT = 200000000;
  double POT = 12500000;
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

  TH3D* h4=new TH3D("TH3D", "Sopping Target; x(Mu2e) [mm]; z(Mu2e) [mm]; y(Mu2e) [mm]", 100, -4200, -3600, 100, 5400, 6300, 100, -300, 300);

  TH3D* hpxpypzgen=new TH3D("hpxpypzgen", "Momentum at Production; px [MeV]; pz [MeV]; py [MeV]", 100, -100, 100, 100, -100, 100, 100, -100, 100);

  double counterlowmommuonsST = 0;

  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    std::cout<<"File: "<<file<<std::endl;
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;
   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readST/treeST");
   

    tree->SetBranchAddress("VolIdstep",&VolIdstep);
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
    tree->SetBranchAddress("pzendstep",&pzendstep);
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
      double Edepele = 0;
      double Erest = 0;
      double Edepmuons_evt = 0;
      double startEsimmuon = 0;
      double simpartmomphot_aux = 0;
      double simpartmomphot = 0;
      double initialpz_Event = 0;
      double event_aux = 0;
      //double trackid_aux = 0;
      double VolId_step, Edep_step, pathlength_step, Xmu2e_step, Ymu2e_step, Zmu2e_step, Xmu2eend_step, Ymu2eend_step, Zmu2eend_step, momstart_step, momend_step, pxstart_step, pystart_step, pzstart_step, pxend_step, pyend_step, pzend_step, pdgid_step, Kestart_step, Etotstart_step, Etotstart_simpart, Etotend_simpart, Process_Code, trackId_num, time_step;

      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
	std::cout<<"size hits: "<<NhitsST_evt<<std::endl;
      }
    
      if(NhitsST_evt!=0){

	if(debugout==true){
	  std::cout<<"Event with edep #: "<<i<<", Number of hits in ST in this event: "<<NhitsST_evt<<" (should be the same as: "<<VolIdstep->size()<<")"<<" Etot deposited event: "<<Edep_evt<<" path length event: "<<pathlength_evt<<std::endl;
       }

	//trackid_aux = 0;
	
	for(unsigned long int j = 0; j < NhitsST_evt; j++){
	  
	   VolId_step = VolIdstep->at(j);
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
	   pdgid_step = pdgidstep->at(j);
	   Kestart_step = Kestartstep->at(j);
	  //Keend_step = Keendstep->at(j); //this is wrong stored initial
	   Etotstart_step = Etotstartstep->at(j); 
	  //Etotend_step = Etotendstep->at(j); //this is wrong stored initial
	   Etotstart_simpart = Etotstartsimpart->at(j);
	   Etotend_simpart = Etotendsimpart->at(j);
	   Process_Code = ProcessCode->at(j);
	   trackId_num = trackIdnum->at(j);
	   time_step = timestep->at(j);

	   if(j==0){initialpz_Event = pzstart_step;}

	   if(pdgid_step==13){startEsimmuon=Etotstart_simpart;}

	   if((pdgid_step==11)||(pdgid_step==-11)){Edepele = Edepele+Edep_step;}
	   if((Edep_step!=0)&&(pdgid_step!=11)&&(pdgid_step!=-11)&&(pdgid_step!=22)){
	     Erest=Erest+Edep_step;
	     /*std::cout<<"Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;
	       std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" Edepstep: "<<Edep_step<<" MeV"<<std::endl;*/
	   }
	   
	   
	  //Electrons and positrons that emit brems at ST
	   if(Process_Code==16){
	     //std::cout<<"BREMSSTRAHLUNG, Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;
	     //std::cout<<"Etotstart_step: "<<Etotstart_step<<" momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" Edepstep: "<<Edep_step<<" MeV"<<std::endl;
	     if(branch==2){ 
	       //if electron or positron fill the histogram
	      if((pdgid_step==11)||(pdgid_step==-11)){h1->Fill(momstart_step);}
	      //if a different particle emitting bremsstrahlung cout
	      if((pdgid_step!=11)&&(pdgid_step!=-11)){std::cout<<"BREMSSTRAHLUNG particle: pdgid_step: "<<pdgid_step<<std::endl;}
	      //if(pdgid_step==-11){std::cout<<"BREMSSTRAHLUNG particle: pdgid_step: "<<pdgid_step<<std::endl;}
	     }
	  } //if Process_Code==16
	  
	  //Annihilated positrons at ST
	   if(Process_Code==2){
	     if(branch==4){
	       h1->Fill(momstart_step);
	     }
	     if(pdgid_step!=-11){
	       std::cout<<"ANNIHILATION, Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;
	      std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" Edepstep: "<<Edep_step<<" MeV"<<std::endl;
	     }
	   }
	   
	   //Muons at the ST
	   if((branch==6)&&((pdgid_step==13)||(pdgid_step==-13))){h1->Fill(momstart_step); 
	    //std::cout<<"Event: "<<i<<" Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<" trackId_num: "<<trackId_num<<std::endl;
	    //std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" Edepstep: "<<Edep_step<<" MeV"<<std::endl;
	   }
	   
	  //Protons at ST
	   if((branch==7)&&(pdgid_step==2212)){h1->Fill(momstart_step);
	     std::cout<<"Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;
	     std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" Edepstep: "<<Edep_step<<" MeV"<<std::endl;}
	  
	   //e- at ST
	   //if((branch==8)&&(momstart_step!=0)&&(pdgid_step==11)){h1->Fill(momstart_step); 
	   //e+ at ST
	   //if((branch==8)&&(momstart_step!=0)&&(pdgid_step==-11)){h1->Fill(momstart_step);
	   //e+e- at ST  
	   if((branch==8)&&(momstart_step!=0)&&((pdgid_step==11)||(pdgid_step==-11))){h1->Fill(momstart_step);
	     //if((branch==8)&&((pdgid_step==11)||(pdgid_step==-11))){h1->Fill(momstart_step);  
	     /* std::cout<<"Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;
		std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" Edepstep: "<<Edep_step<<" MeV"<<std::endl;*/
	  }
	   
	   
	   //photons at ST hit
	   if((branch==9)&&(pdgid_step==22)){
	       h1->Fill(momstart_step); std::cout<<"Energy: "<<momstart_step<<"event: "<<i<<std::endl;
	   }
	   
	   //muminus capture at rest ST
	   if(Process_Code==32){
	     double stopped_muons_mom = sqrt(Etotstart_simpart*Etotstart_simpart-muonmass*muonmass);

	     if(debugout==true){
	       std::cout<<"Event: "<<i<<" Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<" momstart_step: "<<momstart_step<<" Etot_simpart: "<<Etotstart_simpart<<" initial pz: "<<initialpz_Event<<" stopped muon mom: "<<stopped_muons_mom<<std::endl;
	     }

	     if((branch==11)&&(pdgid_step==13)){
	       //h1->Fill(momstart_step);
	       h1->Fill(stopped_muons_mom);
	       std::cout<<"Event: "<<i<<std::endl;
	     }

	     if(branch==19){
	       double costheta_stopped_muons = initialpz_Event / stopped_muons_mom;
	       std::cout<<"cos(theta) stopped muons: "<<costheta_stopped_muons<<std::endl;
	       h1->Fill(costheta_stopped_muons);
	     }

	     //For stopped muons
	     if(branch==17){h2->Fill(stopped_muons_mom);}
	     //if(branch==18){h1->Fill(Xmu2e_step/mm_to_cm);}
	     //if(branch==18){h1->Fill(Ymu2e_step/mm_to_cm);}
	     //if(branch==18){h1->Fill(Zmu2e_step/mm_to_cm);}
	     //if((branch==18)&&((Zmu2e_step/mm_to_cm)>615)){
	       //h1->Fill(Xmu2e_step/mm_to_cm); 
	       //h1->Fill(Ymu2e_step/mm_to_cm);
	     //}
	     /*if((branch==18)&&((Zmu2e_step/mm_to_cm)<560)){
	       //h1->Fill(Xmu2e_step/mm_to_cm);
	       h1->Fill(Ymu2e_step/mm_to_cm);
	       }*/

	     
	     if(pdgid_step!=13){std::cout<<"Muminus capture without muminus!!"<<std::endl; 
	      std::cout<<"Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;}
	   } //process code=32

	   if((branch==18)&&(pdgid_step==11)){h1->Fill(Xmu2e_step/mm_to_cm);}
	   
	   
	   //All processes at ST
	   if((Process_Code!=17)&&(Process_Code!=40)&&(Process_Code!=49)&&(Process_Code!=16)&&(Process_Code!=2)&&(Process_Code!=12)&&(Process_Code!=13)&&(Process_Code!=31)&&(Process_Code!=21)&&(Process_Code!=29)&&(Process_Code!=65)&&(Process_Code!=32)&&(Process_Code!=23)&&(Process_Code!=78)&&(Process_Code!=133)&&(Process_Code!=58)&&(Process_Code!=108)&&(Process_Code!=14)&&(Process_Code!=59)&&(Process_Code!=100)&&(Process_Code!=109)&&(Process_Code!=97)){
	     std::cout<<"PROCESS CODE: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;
	     std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" Edepstep: "<<Edep_step<<" MeV"<<std::endl;
	   }

	   
	   //Edep of muons per event
	   if(branch==12){
	    if(pdgid_step==13){
	      Edepmuons_evt=Edepmuons_evt+Edep_step;
	      //std::cout<<"Event: "<<i<<" Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<" Edep_step "<<Edep_step<<" momstart_step "<<momstart_step<<" Etotstart_step: "<<Etotstart_step<<" Etotstart_simpart "<<Etotstart_simpart<<std::endl;
	    }
	   }
	   /* if((pdgid_step!=11)&&(pdgid_step!=-11)&&(pdgid_step!=22)&&(pdgid_step!=-211)&&(pdgid_step!=13)&&(pdgid_step!=-13)&&(pdgid_step!=2212)&&(pdgid_step!=2112)){
	     std::cout<<"Process_Code: "<<Process_Code<<", PARTICLE ID: "<<std::setprecision(10)<<pdgid_step<<std::setprecision(6)<<std::endl;
	     }*/
	   
	   
	  //photons mom simparticle at ST
	   if((branch==13)&&(pdgid_step==22)){
	     simpartmomphot=Etotstart_simpart;
	     //std::cout<<"Event: "<<i<<" Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<" Edep_step "<<Edep_step<<" momstart_step "<<momstart_step<<" Etotstart_step: "<<Etotstart_step<<" Etotstart_simpart "<<Etotstart_simpart<<" = p(phot): "<<Etotstart_simpart<<std::endl;
	     if(simpartmomphot_aux!=simpartmomphot){h1->Fill(simpartmomphot);simpartmomphot_aux=Etotstart_simpart;/*std::cout<<"Photon simpart mom: "<<simpartmomphot<<std::endl;*/}
	  }
	   
	   //Fill with energy of simparticle for the 1st hit of the event (energy of the DIO generated)
	   if((branch==14)&&(j==0)){h1->Fill(Etotstart_simpart);
	     if(Etotstart_simpart>80){
	     std::cout<<"SIMPARTICLE: pdgid: "<<pdgid_step<<std::endl;
	     std::cout<<"Etotstart_simpart: "<<Etotstart_simpart<<" MeV"<<std::endl;}
	   }
	  if((branch==15)&&(j==0)){
	    h1->Fill(Zmu2e_step);
	  }
	  if((branch==16)&&(j==0)){
	    hpxpypzgen->Fill(pxstart_step,pzstart_step,pystart_step);
	    std::cout<<"SIMPARTICLE: pdgid: "<<pdgid_step<<std::endl;
	    std::cout<<"Etotstart_simpart: "<<Etotstart_simpart<<" MeV px: "<<pxstart_step<<" py: "<<pystart_step<<" pz: "<<pzstart_step<<std::endl;
	    
	  }


	  
	  if((branch==20)&&(pdgid_step==22)){
	    //if((momstart_step>0.334)&&(momstart_step<0.3354)){
	    if((momstart_step>0.843)&&(momstart_step<0.8445)){
	    //if((momstart_step>0.064)&&(momstart_step<0.068)){
	    //if((momstart_step>1.77)&&(momstart_step<1.85)){ 
	    //if((momstart_step>1.808)&&(momstart_step<1.81)){
	      if((i==0)||(i!=event_aux)){
		   h1->Fill(momstart_step);
		   //std::cout<<"Etotstart_simpart: "<<Etotstart_simpart<<" MeV, mom step: "<<momstart_step<<" event: "<<i<<" trackId_num: "<<trackId_num<<std::endl;
		   event_aux=i;
	      }
	    }
	  }


	  

	  if((debugout==true)&&(file==0)/*&&((pdgid_step==22)||(Process_Code==16))*/){
	    //if((i==1956)&&(file==0)){
	  std::cout<<" "<<std::endl;
 
	    std::cout<<"--step #: "<<j<<std::endl;
	    std::cout<<"PDG Id: "<<std::setprecision(10)<<pdgid_step<<std::setprecision(4)<<std::endl;
	    std::cout<<"Time from VD8 to ST step: "<<time_step<<std::endl;
	    std::cout<<"VolIdstep (foil #): "<<VolId_step<<std::endl;
	    std::cout<<"Edepstep: "<<Edep_step<<std::endl;
	    std::cout<<"pathlengthstep: "<<pathlength_step<<" mm"<<std::endl;
	    std::cout<<"Xmu2e_step: "<<Xmu2e_step<<" Ymu2e_step: "<<Ymu2e_step<<" Zmu2e_step: "<<Zmu2e_step<<" mm"<<std::endl;
	    std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" MeV"<<std::endl;
	    std::cout<<"(px,py,pz)start: "<<pxstart_step<<" "<<pystart_step<<" "<<pzstart_step<<std::endl;
	    std::cout<<"Etotstart_step: "<<Etotstart_step<<" MeV"<<std::endl;
	    std::cout<<"Process_Code: "<<Process_Code<<" trackId_num: "<<trackId_num<<std::endl;
	    std::cout<<"Etotstart_simpart: "<<Etotstart_simpart<<" MeV"<<std::endl;
	  }
	  
	  if(branch==1){h4->Fill(Xmu2e_step, Zmu2e_step, Ymu2e_step);}
	  
	}//NhitsST_evt
	
	/*
	  n=n+NhitsST_evt;
	  int Nfoils = Nhitsfoilevt->size()/entries;
	  std::cout<<"Number of foils in ST: "<<Nfoils<<std::endl;
	  for (int k = 0; k<Nfoils; k++){
	  std::cout<<"Foil n: "<<k<<" with "<<Nhitsfoilevt->at(k)<<" hits"<<std::endl;
	  }
	*/
	if((Edepele!=0)&&(branch==0)){h1->Fill(Edepele);}
	if((Erest!=0)&&(branch==3)){h1->Fill(Erest);}
	if((Edep_evt!=0)&&(branch==5)){h1->Fill(Edep_evt);}
	/*if((Edepmuons_evt!=0)&&(branch==12)){
	  h1->Fill(Edepmuons_evt);
	  std::cout<<"Event: "<<i<<" Edepmuons_evt: "<<Edepmuons_evt<<std::endl;
	  }*/
	
	if((startEsimmuon!=0)&&((branch==12)||(branch==17))){
	  double momstartevent=sqrt(startEsimmuon*startEsimmuon-muonmass*muonmass);
	  //std::cout<<"Event: "<<i<<" Mom start event: "<<momstartevent<<std::endl;
	  h1->Fill(momstartevent); 
	  if(momstartevent<70){counterlowmommuonsST++;}
	  //std::cout<<"Muons with momentum<70MeV: "<<counterlowmommuonsST<<std::endl;
	}
	
      } //if(NhitsST_evt!=0)
    }//entries    
    input->Close();
  }//for int file
  
  
  h1->SetTitle("");
  //Comment to get the statistical box of histogram 
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  
  if(branch==0){h1->SetFillColor(kAzure+2);}
  if(branch==2){h1->SetFillColor(kViolet+2);}
  if(branch==3){h1->SetFillColor(kGreen+2);}
  if(branch==4){h1->SetFillColor(kMagenta+2);}
  if(branch==5){h1->SetFillColor(kOrange+2);}
  if(branch==6){h1->SetFillColor(kRed+2);}
  if(branch==7){h1->SetFillColor(kRed+2);}
  if(branch==8){h1->SetFillColor(kOrange+2);}
  if(branch==9){h1->SetFillColor(kGreen+2);}
  if(branch==11){h1->SetFillColor(kYellow+2);}
  if(branch==12){h1->SetFillColor(kCyan+2);}
  if(branch==13){h1->SetFillColor(kGreen+2);}
  if(branch==14){h1->SetFillColor(kCyan-3);
                 h1->SetLineColor(kCyan-3);
		 h1->SetFillStyle(3001);
		 //Scale by number of POT 
		 h1->Scale(1./POT);
                 }
  if(branch==15){h1->SetLineColor(kBlue+1);}
  if(branch==16){hpxpypzgen->SetMarkerColor(kCyan-3);hpxpypzgen->SetMarkerStyle(1);}
  if(branch==17){h1->SetFillColor(kCyan+2); 
                 h1->SetLineColor(kCyan+2); 
		 h2->SetFillColor(kYellow+2); 
		 h2->SetLineColor(kYellow+2); 
		 h1->SetFillStyle(3001);
		 h2->SetFillStyle(3001);
		 //Scale by number of POT
		 h1->Scale(1./POT);
		 h2->Scale(1./POT);
                 }
  if(branch==18){h1->SetLineColor(kBlue+1);}
  if(branch==19){h1->SetFillColor(kGreen-6);
                 h1->SetLineColor(kGreen-6);
  }
if(branch==20){h1->SetFillColor(kCyan-3);
                 h1->SetLineColor(kCyan-3);
                 h1->SetFillStyle(3001);
                 //Scale by number of POT
                 h1->Scale(1./POT);
                 }

 
  std::cout<<"h1 entries: "<<h1->GetEntries()<<" h2 entries: "<<h2->GetEntries()<<std::endl;
  if(branch==20){
    h1->Draw("HIST");
  }
  else{
  h1->Draw("");
  }
  //hpxpypzgen->Draw("");

  if(branch==1){
    h4->SetTitle("");
    h4->SetStats(0);
    h4->SetMarkerStyle(1);
  }
  //h4->Draw(""); 

  auto leg = new TLegend(0.1,0.7,0.48,0.9);
  TH1F*hnorm1 = (TH1F*)(h1->Clone("TH1"));
  hnorm1->Scale(1./hnorm1->Integral());
  /* TH1F*hnorm2 = (TH1F*)(h2->Clone("TH2"));
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
}*/

  if(branch==17){h2->Draw("same,HIST"); leg->AddEntry(h1,"All","f"); leg->AddEntry(h2,"Stopped muons","f"); leg->Draw("same");}


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
  /*
  ps->GetLineWith("Entries")->SetTextSize(0.027);
  ps->GetLineWith("Mean")->SetTextSize(0.027);
  ps->GetLineWith("Std Dev")->SetTextSize(0.027);   
  */

  c1->Modified();

  gPad->RedrawAxis();
  ps->Draw("same");
  //h1->Draw("same");
  //gPad->SetLogy();  

  std::cout<<"Histogram max height: "<<h1->GetMaximum()<<std::endl;
  c1->Print("MDC2020_xelectronsST.png");
  //c1->Print("Costheta_MDC2020_stoppedmuons.pdf");

}



//================================================================
void ReadST_plot(std::string ArtFiles_location) {
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

    
  //0 is to print total energy deposited in ST by e+e-
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "E_{dep event ST, e^{#pm}} [MeV]", "Counts", 0);
  //1 is to print hits position in x,y,z in Mu2e system 
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "", "", 1);
  //2 is to print initial energies of e+, e- that emit bremmstrahlung processcode = 16
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "p_{init e^{#pm}, bremsstrahlung hit ST} [MeV]", "Counts", 2);
  //3 is to print total energy deposited in ST by rest of particles
  //PrintHisto(file_name, 100, -2, 60, 0, 3000, "E_{dep event ST, #mu^{#pm},#pi^{-},p} [MeV]", "Counts", 3);
  //4 is to print initial energies of e+ annihilates in ST process code =2
  //PrintHisto(file_name, 100, -2, 5, 0, 3000, "p_{init e^{+}, annihilation hit ST} [MeV]", "Counts", 4);  
  //5 is to print total energy deposited in ST by all particles
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "E_{dep event ST} [MeV]", "Counts", 5);
  //6 is to print the initial momentum of muons at ST
  //PrintHisto(file_name, 100, -2, 100, 0, 3000, "p_{init #mu^{#pm}, hit ST} [MeV]", "Counts", 6);  
  //7 is to print the initial momentum of protons at ST
  //PrintHisto(file_name, 100, -2, 150, 0, 3000, "p_{init p, hit ST} [MeV]", "Counts", 7);
  //8 is to print the initial momentum of e+e- at ST
  //PrintHisto(file_name, 100, -2, 20, 0, 3000, "p_{init e^{#pm}, hit ST} [MeV]", "Counts", 8);
  //8 is to print the initial momentum of e- at ST
  //PrintHisto(file_name, 100, -2, 80, 0, 3000, "p_{init e^{-}, hit ST} [MeV]", "Counts", 8); 
  //8 is to print the initial momentum of e- at ST
  //PrintHisto(file_name, 100, -2, 80, 0, 3000, "p_{init e^{+}, hit ST} [MeV]", "Counts", 8); 
  //9 is to print the initial momentum of gamma at ST
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "p_{init #gamma, hit ST} [MeV]", "Counts", 9); 
  //10 is to print the initial momentum of neutrons at ST
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "p_{init n, hit ST} [MeV]", "Counts", 10); 
  //11 is to print number of muminuscaptureatrest, process code = 32
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "p_{init #mu^{-}, capture at rest hit ST} [MeV]", "Counts", 11);
  //PrintHisto(file_name, 100, -2, 100, 0, 3000, "p_{init stopped #mu^{-}} [MeV]", "Counts", 11); 
  //12 is to print Edep per event per muon
  //PrintHisto(file_name, 100, -2, 30, 0, 3000, "E_{dep #mu^{-}, event ST} [MeV]", "Counts", 12);
  //12 is to print muon start momentum of event (simparticle)
  //PrintHisto(file_name, 100, -2, 100, 0, 3000, "p_{init #mu^{-}, event ST} [MeV]", "Counts", 12);   
  //13 is to print photon start momentum of each simparticle
  //PrintHisto(file_name, 100, -2, 100, 0, 3000, "p_{init #gamma, event ST} [MeV]", "Counts", 13); 
  //14 is to print DIO electrons start momentum of each simparticle  
  //PrintHisto(file_name, 100, -2, 110, 0, 3000, "E_{gen DIO e^{-}} [MeV]", "Counts/POT", 14);
  //15 is to print z of DIO electrons (first simparticle of the event)
  //PrintHisto(file_name, 100, 5400, 6300, 0, 3000, "z_{gen DIO e^{-}} [mm]", "", 15);
  //16 is to print px, py, pz of generated DIO electrons (useless arguments)
  //PrintHisto(file_name, 100, 0, 0, 0, 3000, "", "", 16);
  //17 Print all muons and stopped muons
  //PrintHisto(file_name, 100, -2, 120, 0, 3000, "p_{#mu^{-}} [MeV]", "Counts/POT", 17);
  //18 Print x,y,z of stopped muons
  //PrintHisto(file_name, 100, -405, -376, 0, 3000, "x_{MDC2020-#gamma} ST [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -10, 17, 0, 3000, "y_{MDC2020-#gamma} ST [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, 544, 630, 0, 3000, "z_{MDC2020-#gamma} ST [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -401, -381, 0, 3000, "x_{gen #gamma, ST (z<560cm)} [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -8, 8, 0, 3000, "y_{gen #gamma, ST (z<560cm)} [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -401, -381, 0, 3000, "x_{gen #gamma, ST (z>615cm)} [cm]", "Counts", 18);
  PrintHisto(file_name, 100, -400, -380, 0, 3000, "x_{e^{-}} ST [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -405, -376, 0, 3000, "x_{#gamma} ST [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -400, -380, 0, 3000, "x_{stopped #mu^{-}} ST [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -8, 8, 0, 3000, "y_{gen #gamma, ST (z>615cm)} [cm]", "Counts", 18);
  //PrintHisto(file_name, 100, -1.5, 2.5, 0, 3000, "cos(#theta_{stopped #mu^{-}})", "", 19);
  //PrintHisto(file_name, 100, 0.334, 0.3354, 0, 3000, "E_{#gamma} [MeV]", "Counts/POT", 20);
  //PrintHisto(file_name, 100, 0.843, 0.8445, 0, 3000, "E_{#gamma} [MeV]", "Counts/POT", 20);
  //PrintHisto(file_name, 100, 0.064, 0.068, 0, 3000, "E_{#gamma} [MeV]", "Counts/POT", 20);
  //PrintHisto(file_name, 100, 1.77, 1.85, 0, 3000, "E_{#gamma} [MeV]", "Counts/POT", 20);
  //PrintHisto(file_name, 100, 1.808, 1.81, 0, 3000, "E_{#gamma} [MeV]", "Counts/POT", 20);
  //0 is to print x, y coordinates in Mu2e system
  //PrintGraph(file_name, -4200, -3600, -300, 300, "x_{ST}(Mu2e) [mm]", "y_{ST}(Mu2e) [mm]", 0); 
  //1 is to print y, z coordinates in Mu2e system 
  //PrintGraph(file_name,5400, 6300, -300, 300, "z_{ST}(Mu2e) [mm]", "y_{ST}(Mu2e) [mm]", 1); 

  //2 is to Print y x coordinates hit of gammas in ST a TH2
  //PrintGraph(file_name,-415, -365, -20, 20, "x_{#gamma at ST (z<560cm)}(Mu2e) [cm]", "y_{#gamma, ST (z<560cm)} [cm]", 2);
  //PrintGraph(file_name,-415, -365, -20, 20, "x_{#gamma at ST (z>615cm)}(Mu2e) [cm]", "y_{#gamma, ST (z>615cm)} [cm]", 2);
  //3 is to Print y x coordinates hit of stopped muons in ST a TH2 
  //PrintGraph(file_name,-415, -365, -20, 20, "x_{stopped #mu^{-} at ST (z<560cm)}(Mu2e) [cm]", "y_{stopped #mu^{-} at ST (z<560cm)} [cm]", 3);
  //PrintGraph(file_name,-415, -365, -20, 20, "x_{stopped #mu^{-} at ST (z>615cm)}(Mu2e) [cm]", "y_{stopped #mu^{-} at ST (z>615cm)} [cm]", 3);  
  //PrintGraph(file_name,-405, -376, -10, 17, "x_{MDC2020-#gamma} ST [cm]", "y_{MDC2020-#gamma} ST [cm]", 3);
  //PrintGraph(file_name,-415, -365, -20, 20, "x_{stopped #mu^{-} at ST-external foil} (Mu2e) [cm]", "y_{stopped #mu^{-} at ST-external foil} [cm]", 3);
   
  //4 is to print TProfile with z and momentum of stopped muons
  //PrintGraph(file_name, 545, 630, 0, 50, "z_{stopped #mu^{-} at ST} (Mu2e) [cm]", "p_{stopped #mu^{-} at ST} [MeV]", 4); 
  //PrintGraph(file_name,540, 630, -5, 5, "z_{stopped #mu^{-} at ST} (Mu2e) [cm]", "y_{stopped #mu^{-} at ST} [cm]", 4);
  //PrintGraph(file_name,540, 630, -415, -365, "z_{stopped #mu^{-} at ST} (Mu2e) [cm]", "x_{stopped #mu^{-} at ST} [cm]", 4);  


  readfile.close();
}

//================================================================
