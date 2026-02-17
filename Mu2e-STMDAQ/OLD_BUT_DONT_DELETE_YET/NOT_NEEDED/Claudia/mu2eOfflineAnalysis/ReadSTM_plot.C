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
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TGraph2D.h"
#include "TLatex.h"
#include "TProfile2D.h"


#define mm_to_cm 10
#define muonmass 105.65


//================================================================
//This program reads readSTM/treeSTM trees: Particles at Stopping Target Monitor
//Reads the root files from a txt and analyse the trees
//================================================================  
void PrintGraph(vector<string> file_name, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetPadRightMargin(0.14);
  gStyle->SetOptStat(1110);
  int palette_number = 55;
  gStyle->SetPalette(palette_number);

  //step branches
  std::vector<int> *pdgidstep=0;
  std::vector<double> *momstartstep=0, *momendstep=0, *Edepstep=0, *StepLength=0, *Xmu2estep=0, *Ymu2estep=0, *Zmu2estep=0, *Xmu2estepend=0, *Ymu2estepend=0, *Zmu2estepend=0,*XdetS=0, *YdetS=0, *ZdetS=0, *pxstartstep=0, *pystartstep=0, *pzstartstep=0, *pxendstep=0, *pyendstep=0, *pzendstep=0, *Kestartstep=0, *Keendstep=0, *Etotstartstep=0, *Etotendstep=0, *ProcessCode=0, *timestep=0, *Nelectronsholesstep=0;
  //event branches
  std::vector<double> *Etotstartsimpart=0, *Etotendsimpart=0 /*Etotstartsimpart and Etotendsimpart is for the original sim particle not for the hit, for the hit use momend and momstart*/, *Edepevt=0 /*electrons and positrons*/, *NhitsSTMevt=0, *Nelectronsholesevt=0;

  std::vector<double> Xv,Yv,mom_initphotv;

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
  auto hprof2d = new TProfile2D("hprof2d","",60,xmin,xmax,40,ymin,ymax); 
  TH2D *h = new TH2D("TH2D","",100,xmin,xmax,100,ymin,ymax);

  std::cout<<"---Loop over files---"<<std::endl;
  unsigned long int p=0;

  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    //for (long unsigned int file=0;file<1;file++){
  string path;
    path=file_name[file];
    std::cout<<""<<std::endl;
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<path.c_str()<<std::endl;

   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readSTM/treeSTM");
    //TTree* tree=(TTree*)input->Get("MyAnalizer/treeSTM"); 


    tree->SetBranchAddress("pdgidstep",&pdgidstep);
    tree->SetBranchAddress("momstartstep",&momstartstep);
    tree->SetBranchAddress("pxstartstep",&pxstartstep);
    tree->SetBranchAddress("pystartstep",&pystartstep);
    tree->SetBranchAddress("pzstartstep",&pzstartstep);
    tree->SetBranchAddress("momendstep",&momendstep);
    tree->SetBranchAddress("pxendstep",&pxendstep);
    tree->SetBranchAddress("pyendstep",&pyendstep);
    tree->SetBranchAddress("pzendstep",&pzendstep);
    tree->SetBranchAddress("Xmu2estep",&Xmu2estep);
    tree->SetBranchAddress("Ymu2estep",&Ymu2estep);
    tree->SetBranchAddress("Zmu2estep",&Zmu2estep);
    tree->SetBranchAddress("Xmu2estepend",&Xmu2estepend);
    tree->SetBranchAddress("Ymu2estepend",&Ymu2estepend);
    tree->SetBranchAddress("Zmu2estepend",&Zmu2estepend);
    tree->SetBranchAddress("XdetS",&XdetS);
    tree->SetBranchAddress("YdetS",&YdetS);
    tree->SetBranchAddress("ZdetS",&ZdetS);
    tree->SetBranchAddress("StepLength",&StepLength);
    tree->SetBranchAddress("Edepstep",&Edepstep);
    tree->SetBranchAddress("Kestartstep",&Kestartstep);
    //tree->SetBranchAddress("Keendstep",&Keendstep);
    tree->SetBranchAddress("Etotstartstep",&Etotstartstep);
    //tree->SetBranchAddress("Etotendstep",&Etotendstep);
    tree->SetBranchAddress("Etotstartsimpart",&Etotstartsimpart);
    tree->SetBranchAddress("Etotendsimpart",&Etotendsimpart);
    tree->SetBranchAddress("ProcessCode",&ProcessCode);
    tree->SetBranchAddress("timestep",&timestep);
    tree->SetBranchAddress("Edepevt",&Edepevt);
    tree->SetBranchAddress("NhitsSTMevt",&NhitsSTMevt);
    tree->SetBranchAddress("Nelectronsholesstep",&Nelectronsholesstep);
    tree->SetBranchAddress("Nelectronsholesevt",&Nelectronsholesevt);   
   

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;


    for(unsigned long i=0;i<entries;i++){

      
      tree->GetEntry(i);
      //if(debugout==true){
      //std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
      //}

     
      //stored once per event
      double Etot_deposit = Edepevt->at(0);
      double NhitsSTM_evt = NhitsSTMevt->at(0);
      double Nelectronsholes_evt = Nelectronsholesevt->at(0);

      if(NhitsSTM_evt!=0){
	if(debugout==true){
	  std::cout<<"Etotstart steps size (=nsteps): "<<Etotstartstep->size()<<" Edepevt size (=1): "<<Edepevt->size()<<std::endl;
	  std::cout<<"Energy deposited in STM (just by e+-, no photons, Edep by photons is just the binding energy in Ge): "<<Etot_deposit<<" MeV, in event: "<<i<<std::endl;
	}
	//Exception
	if((Xmu2estep->size())!=NhitsSTM_evt){
	  std::cout<<"EXCEPTION THIS SIZE IS SUPPOSED TO BE THE SAME"<<std::endl;
	  std::cout<<"Check sized: "<<Xmu2estep->size()<<" = "<<NhitsSTM_evt<<std::endl;
	  exit(0);
	}
 
      /*  if((debugout==true)&&(file==0)){
std::cout<<"Number of hits in ST in this event: "<<Xmu2e->size()<<" Etot deposited event: "<<Etot_deposit<<" Start energy: "<<Etot_start<<" End energy: "<<Etot_end<<std::endl;
}*/
//Loop over hits
      for(unsigned long int j = 0; j < NhitsSTM_evt; j++){
	
	if(debugout==true){
	  std::cout<<""<<std::endl;
	  std::cout<<"hit: "<<j<<std::endl;
	}
	int pdgID_step = pdgidstep->at(j);
	double momstart_step = momstartstep->at(j);
	double pxstart_step = pxstartstep->at(j);
	double pystart_step = pystartstep->at(j);
	double pzstart_step = pzstartstep->at(j);
	double momend_step = momendstep->at(j);
	double pxend_step = pxendstep->at(j);
	double pyend_step = pyendstep->at(j);
	double pzend_step = pzendstep->at(j);
	double Xmu2e_step = Xmu2estep->at(j);
	double Ymu2e_step = Ymu2estep->at(j);
	double Zmu2e_step = Zmu2estep->at(j);
	double Xmu2eend_step = Xmu2estepend->at(j);
	double Ymu2eend_step = Ymu2estepend->at(j);
	double Zmu2eend_step = Zmu2estepend->at(j);
	double XdetS_step = XdetS->at(j);
	double YdetS_step = YdetS->at(j);
	double ZdetS_step = ZdetS->at(j);
	double StepLength_step = StepLength->at(j);
	double Edeposit_step = Edepstep->at(j);
	double Kestart_step = Kestartstep->at(j);
	//double Keend_step = Keendstep->at(j);
	double Etotstart_step = Etotstartstep->at(j);
	//double Etotend_step = Etotendstep->at(j);
	double Etotstart_simpart = Etotstartsimpart->at(j);
	double Etotend_simpart = Etotendsimpart->at(j);
	double Process_Code = ProcessCode->at(j);
	double time_step = timestep->at(j);
	double Nelectronsholes_step = Nelectronsholesstep->at(j);


	if((debugout==true)&&(file==0)&&(j<100)){
	std::cout<<"--step #: "<<j<<std::endl;
	std::cout<<"PDG Id: "<<pdgID_step<<std::endl;
	std::cout<<"Process Code: "<<Process_Code<<std::endl;
	std::cout<<"Edepstep: "<<Edeposit_step<<" MeV"<<std::endl;
	std::cout<<"pathlengthstep: "<<StepLength_step<<" mm"<<std::endl;
	std::cout<<"Xmu2e_step: "<<Xmu2e_step<<" Ymu2e_step: "<<Ymu2e_step<<" Zmu2e_step: "<<Zmu2e_step<<" mm"<<std::endl;
	std::cout<<"XdetS_step: "<<XdetS_step<<" YdetS_step: "<<YdetS_step<<" ZdetS_step: "<<ZdetS_step<<" mm"<<std::endl;
	std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" MeV"<<std::endl;
	std::cout<<"Primary particle for this hit: Etot_start: "<<Etotstart_simpart<<" MeV"<<std::endl;
	}
	
	if(branch==0){gr->SetPoint(p,Xmu2e_step,Ymu2e_step); p++;}
	if(branch==1){gr->SetPoint(p,Zmu2e_step,Ymu2e_step); p++;}
	//fill vectors (first photon hit in the STM)
	if((branch==2)&&(j==0)&&(pdgID_step==22)/*&&((Xmu2e_step/mm_to_cm)<-390)*/){Xv.push_back(Xmu2e_step/mm_to_cm); Yv.push_back(Ymu2e_step/mm_to_cm); mom_initphotv.push_back(momstart_step);
	  hprof2d->Fill(Xmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm,momstart_step,1);
	}
	if((branch==3)&&(j==0)&&(pdgID_step==22)&&((Xmu2e_step/mm_to_cm)<=-390)){h->Fill(Xmu2e_step/mm_to_cm,Ymu2e_step/mm_to_cm);}

      }//NhitsSTM_evt
      if(debugout==true){
	std::cout<<" "<<std::endl;}
      } //if(NhitsSTM_evt!=0)
 
    }//entries    
    
    input->Close();
  }//for int file


  TGraph2D *grbr2 = new TGraph2D(Xv.size(),&Xv[0],&Yv[0],&mom_initphotv[0]);

  if((branch==0)||(branch==1)){
    gr->SetTitle("");
    gr->GetXaxis()->SetTitle(Xtitle);
    gr->GetYaxis()->SetTitle(Ytitle);
    gr->SetMarkerStyle(7);
    gr->SetMarkerColor(kBlack);
    gr->Draw("same,p");
  }

  TLatex latex;
  if(branch==2){
    //for(unsigned long int i=0;i<Xv.size();i++){
    //std::cout<<Xv.at(i)<<" "<<Yv.at(i)<<" "<<mom_initphotv.at(i)<<std::endl;
    //}

    //grbr2->GetHistogram()->GetZaxis()->SetLabelSize(0.03);
    std::cout<<"Entries: "<<grbr2->GetHistogram()->GetEntries()<<" size: "<<Xv.size()<<std::endl;
    //TGraph2D binning
    grbr2->SetNpx(100);
    grbr2->SetNpy(100);
    //grbr2->Draw("same,colz");
    latex.SetTextSize(0.06);
    latex.SetTextAlign(13);
    latex.DrawLatex(xmin+2,ymax-3,"p_{init #gamma STM}");
    hprof2d->Draw("same,colz");

  }

  if(branch==3){
    TAxis *X = h->GetXaxis();
    X->SetNdivisions(5,3,0);
    h->GetXaxis()->SetTitle(Xtitle);
    h->GetYaxis()->SetTitle(Ytitle);
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
    c1->Modified();

  }



  c1->Print("epluseminusphot_xy_STM_1_347keV.png");
  c1->Print("epluseminusphot_xy_STM_1_347keV.pdf");
}

//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);


  //step branches
  std::vector<int> *pdgidstep=0;
  std::vector<double> *momstartstep=0, *momendstep=0, *Edepstep=0, *StepLength=0, *Xmu2estep=0, *Ymu2estep=0, *Zmu2estep=0, *Xmu2estepend=0, *Ymu2estepend=0, *Zmu2estepend=0,*XdetS=0, *YdetS=0, *ZdetS=0, *pxstartstep=0, *pystartstep=0, *pzstartstep=0, *pxendstep=0, *pyendstep=0, *pzendstep=0, *Kestartstep=0, *Keendstep=0, *Etotstartstep=0, *Etotendstep=0, *ProcessCode=0, *timestep=0, *Nelectronsholesstep=0;
  //event branches
  std::vector<double> *Etotstartsimpart=0, *Etotendsimpart=0 /*Etotstartsimpart and Etotendsimpart is for the original sim particle not for the hit, for the hit use momend and momstart*/, *Edepevt=0 /*electrons and positrons*/, *NhitsSTMevt=0, *Nelectronsholesevt=0;


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

  TH3D* h4=new TH3D("TH3D", "Sopping Target; x(Mu2e) [mm]; z(Mu2e) [mm]; y(Mu2e) [mm]", 100, -4200, -3800, 100, 40700, 40850, 100, -80, 80);


  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<""<<std::endl;
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<file<<" "<<path.c_str()<<std::endl;
   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readSTM/treeSTM");
    //TTree* tree=(TTree*)input->Get("MyAnalizer/treeSTM"); 


    tree->SetBranchAddress("pdgidstep",&pdgidstep);
    tree->SetBranchAddress("momstartstep",&momstartstep);
    tree->SetBranchAddress("pxstartstep",&pxstartstep);
    tree->SetBranchAddress("pystartstep",&pystartstep);
    tree->SetBranchAddress("pzstartstep",&pzstartstep);
    tree->SetBranchAddress("momendstep",&momendstep);
    tree->SetBranchAddress("pxendstep",&pxendstep);
    tree->SetBranchAddress("pyendstep",&pyendstep);
    tree->SetBranchAddress("pzendstep",&pzendstep);
    tree->SetBranchAddress("Xmu2estep",&Xmu2estep);
    tree->SetBranchAddress("Ymu2estep",&Ymu2estep);
    tree->SetBranchAddress("Zmu2estep",&Zmu2estep);
    tree->SetBranchAddress("Xmu2estepend",&Xmu2estepend);
    tree->SetBranchAddress("Ymu2estepend",&Ymu2estepend);
    tree->SetBranchAddress("Zmu2estepend",&Zmu2estepend);
    tree->SetBranchAddress("XdetS",&XdetS);
    tree->SetBranchAddress("YdetS",&YdetS);
    tree->SetBranchAddress("ZdetS",&ZdetS);
    tree->SetBranchAddress("StepLength",&StepLength);
    tree->SetBranchAddress("Edepstep",&Edepstep);
    tree->SetBranchAddress("Kestartstep",&Kestartstep);
    //tree->SetBranchAddress("Keendstep",&Keendstep); //wrong stored (ke start energy)
    tree->SetBranchAddress("Etotstartstep",&Etotstartstep);
    //tree->SetBranchAddress("Etotendstep",&Etotendstep); //wrong stored (start energy)
    tree->SetBranchAddress("Etotstartsimpart",&Etotstartsimpart);
    tree->SetBranchAddress("Etotendsimpart",&Etotendsimpart);
    tree->SetBranchAddress("ProcessCode",&ProcessCode);
    tree->SetBranchAddress("timestep",&timestep);
    tree->SetBranchAddress("Edepevt",&Edepevt);
    tree->SetBranchAddress("NhitsSTMevt",&NhitsSTMevt);
    tree->SetBranchAddress("Nelectronsholesstep",&Nelectronsholesstep);
    tree->SetBranchAddress("Nelectronsholesevt",&Nelectronsholesevt);   

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;


    for(unsigned long i=0;i<entries;i++){
      
      tree->GetEntry(i);
      /*if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
	}*/

      double Edepcheck = 0;


      //stored once per event
      double Etot_deposit = Edepevt->at(0);
      double NhitsSTM_evt = NhitsSTMevt->at(0);
      double Nelectronsholes_evt = Nelectronsholesevt->at(0);
      
      if(NhitsSTM_evt!=0){
	std::cout<<"evt: "<<i<<" pdg init: "<<pdgidstep->at(0)<<std::endl;
	if(debugout==true){
	  std::cout<<"Etotstart steps size (=nsteps): "<<Etotstartstep->size()<<" Edepevt size (=1): "<<Edepevt->size()<<std::endl;
	  std::cout<<"Energy deposited in STM (just by e+-, no photons, Edep by photons is just the binding energy in Ge): "<<Etot_deposit<<" MeV, in event: "<<i<<std::endl;
	}

	//Exception
	if((Xmu2estep->size())!=NhitsSTM_evt){
	  std::cout<<"EXCEPTION THIS SIZE IS SUPPOSED TO BE THE SAME"<<std::endl;
	  std::cout<<"Check sized: "<<Xmu2estep->size()<<" = "<<NhitsSTM_evt<<std::endl;
	  exit(0);
	}

	if(branch==0){h1->Fill(Etot_deposit);}
 
	/*  if((debugout==true)&&(file==0)){
	    std::cout<<"Number of hits in ST in this event: "<<Xmu2e->size()<<" Etot deposited event: "<<Etot_deposit<<" Start energy: "<<Etot_start<<" End energy: "<<Etot_end<<std::endl;
	    }*/

	//Loop over hits
	for(unsigned long int j = 0; j < NhitsSTM_evt; j++){
	  
	  if(debugout==true){
	    std::cout<<""<<std::endl;
	    std::cout<<"hit: "<<j<<std::endl;
	  }
	  int pdgID_step = pdgidstep->at(j);
	  double momstart_step = momstartstep->at(j);
	  double pxstart_step = pxstartstep->at(j);
	  double pystart_step = pystartstep->at(j);
	  double pzstart_step = pzstartstep->at(j);
	  double momend_step = momendstep->at(j);
	  double pxend_step = pxendstep->at(j);
          double pyend_step = pyendstep->at(j);
          double pzend_step = pzendstep->at(j);
	  double Xmu2e_step = Xmu2estep->at(j);
	  double Ymu2e_step = Ymu2estep->at(j);
	  double Zmu2e_step = Zmu2estep->at(j);
	  double Xmu2eend_step = Xmu2estepend->at(j);
          double Ymu2eend_step = Ymu2estepend->at(j);
          double Zmu2eend_step = Zmu2estepend->at(j);
	  double XdetS_step = XdetS->at(j);
          double YdetS_step = YdetS->at(j);
          double ZdetS_step = ZdetS->at(j);
	  double StepLength_step = StepLength->at(j);
	  double Edeposit_step = Edepstep->at(j);
	  double Kestart_step = Kestartstep->at(j);
	  //double Keend_step = Keendstep->at(j);
	  double Etotstart_step = Etotstartstep->at(j);
	  //double Etotend_step = Etotendstep->at(j);
	  double Etotstart_simpart = Etotstartsimpart->at(j); 
	  double Etotend_simpart = Etotendsimpart->at(j);
	  double Process_Code = ProcessCode->at(j);
	  double time_step = timestep->at(j);
	  double Nelectronsholes_step = Nelectronsholesstep->at(j);

	  if((pdgID_step==11)||(pdgID_step==-11)){
	    Edepcheck=Edepcheck+Edeposit_step;
	  }
	  if((Edeposit_step!=0)&&(pdgID_step!=11)&&(pdgID_step!=-11)&&(pdgID_step!=22)){
	    std::cout<<"Energy deposited in the STM not by e+e- or photons: "<<Edeposit_step<<" MeV"<<std::endl;
	  }

	  //if((debugout==true)&&(file==0)&&(j<100)){
	  if(debugout==true){
	  std::cout<<"--step #: "<<j<<" time: "<<time_step<<std::endl;
	  std::cout<<"PDG Id: "<<pdgID_step<<std::endl;
	  std::cout<<"Process Code: "<<Process_Code<<std::endl;
	  std::cout<<"Edepstep: "<<Edeposit_step<<" MeV"<<std::endl;
	  std::cout<<"pathlengthstep: "<<StepLength_step<<" mm"<<std::endl;
	  std::cout<<"Xmu2e_step: "<<Xmu2e_step<<" Ymu2e_step: "<<Ymu2e_step<<" Zmu2e_step: "<<Zmu2e_step<<" mm"<<std::endl;
	  std::cout<<"Xmu2eend_step: "<<Xmu2eend_step<<" Ymu2eend_step: "<<Ymu2eend_step<<" Zmu2eend_step: "<<Zmu2eend_step<<" mm"<<std::endl;
	  std::cout<<"XdetS_step: "<<XdetS_step<<" YdetS_step: "<<YdetS_step<<" ZdetS_step: "<<ZdetS_step<<" mm"<<std::endl;
	  std::cout<<"Etot start step: "<<Etotstart_step<<" MeV"<<std::endl;
	  std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" MeV"<<std::endl;
	  std::cout<<"p start step: ("<<pxstart_step<<","<<pystart_step<<","<<pzstart_step<<") MeV"<<std::endl;
	  std::cout<<"p end step: ("<<pxend_step<<","<<pyend_step<<","<<pzend_step<<") MeV"<<std::endl;
	  std::cout<<"ke start step: "<<Kestart_step<<" MeV"<<std::endl;
	  std::cout<<"Primary particle for this hit: Etot_start: "<<Etotstart_simpart<<" MeV"<<std::endl;

	  }
     
	  if(branch==1){h4->Fill(Xmu2e_step, Zmu2e_step, Ymu2e_step);}
	  if((branch==2)&&((pdgID_step==11)||(pdgID_step==-11))){h1->Fill(Edeposit_step);}  
	}//NhitsSTM_evt
	
	if(debugout==true){
	  std::cout<<"ETOT DEPOSIT EVENT CHECK (e+- and photons): "<<Edepcheck<<std::endl;
	  std::cout<<" "<<std::endl;
	}

	//Exception
	if(Edepcheck!=Etot_deposit){
	  std::cout<<"Total energy deposited in the detector by e+e- is not the same as summing energy deposited in steps..."<<std::endl;
	  exit(0);
	}

      } //if(NhitsSTM_evt!=0)
 
    }//entries    
    input->Close();
  }//for int file



  
  if((branch==0)||(branch==2)){
    h1->SetTitle("");
    //h1->SetStats(0);
    h1->GetXaxis()->SetTitle(Xtitle);
    h1->GetYaxis()->SetTitle(Ytitle);

    h1->SetFillColor(kAzure+2);}
  
  if(branch==1){
    h4->SetTitle("");
    h4->SetStats(0);
    h4->SetMarkerStyle(1);
  }

  /*
  if(branch==2){h1->SetFillColor(kViolet+2);}
  if(branch==3){h1->SetFillColor(kMagenta+2);}
  if(branch==4){h1->SetFillColor(kOrange+2);}
  if(branch==5){h1->SetFillColor(kRed+2);}
  if(branch==6){h1->SetLineColor(kBlue+2);
  h2->SetLineColor(kMagenta-7);
  h3->SetLineColor(kGreen+2);
}*/

  h1->Draw("");
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



  //h1->Draw("same");
  //gPad->SetLogy();  
  

  //c1->Print("EdepEvent_electrons_STM_10to6photons_ST_347keV.png");
  //c1->Print("EdepEvent_electrons_STM_10to6photons_ST_347keV.pdf");
}



//================================================================
void ReadSTM_plot(std::string ArtFiles_location) {
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

    
  //0 is to print total energy deposited in each event by e+- at the STM
  //PrintHisto(file_name, 100, -0.1, 4, 0, 3000, "E_{dep event STM, e^{#pm}} [MeV]", "Counts", 0);
  //1 is to print hits position in x,y,z in Mu2e system 
  //PrintHisto(file_name, 100, -2, 10, 0, 3000, "", "", 1);
  //2 is to print energy deposited in each step by e+- at the STM  
  //PrintHisto(file_name, 100, -1, 2.5, 0, 3000, "E_{dep hit STM, e^{#pm}} [MeV]", "Counts", 2);


  //0 is to print x, y coordinates in Mu2e system
  //PrintGraph(file_name, -4000, -3800, -80, 80, "x_{STM}(Mu2e) [mm]", "y_{STM}(Mu2e) [mm]", 0); 
  //1 is to print y, z coordinates in Mu2e system 
  //PrintGraph(file_name,40740, 40850, -80, 80, "z_{STM}(Mu2e) [mm]", "y_{STM}(Mu2e) [mm]", 1); 
  //2 TGraph2D/Tprofile2D is to print the momentum of the incident photon in STM as a function of x,y
  //PrintGraph(file_name, -405, -375, -10, 10, "x_{STM}(Mu2e) [cm]", "y_{STM}(Mu2e) [cm]", 2);
  //3 is x,y position hit of gammas (TH2)
  PrintGraph(file_name, -400, -390, -4, 4, "x_{#gamma, STM}(Mu2e) [cm]", "y_{#gamma, STM}(Mu2e) [cm]", 3);

  readfile.close();
}

//================================================================
