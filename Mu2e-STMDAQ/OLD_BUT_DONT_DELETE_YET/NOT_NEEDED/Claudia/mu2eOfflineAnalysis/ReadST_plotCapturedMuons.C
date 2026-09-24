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

#define muonmass 105.65
//================================================================
//This program reads readST/treeST trees: Particles at Stopping Target
//Reads the root files from a txt and analyse the trees
//================================================================  
/*void PrintGraph(vector<string> file_name, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);

  //step branches 
  std::vector<double> *VolIdstep=0, *Edepstep=0, *pathlengthstep=0, *Xmu2estep=0, *Ymu2estep=0, *Zmu2estep=0, *Xmu2estepend=0, *Ymu2estepend=0, *Zmu2estepend=0, *momstartstep=0, *momendstep=0, *pxstartstep=0, *pystartstep=0, *pzstartstep=0, *pxendstep=0, *pyendstep=0,*pzendstep=0, *pdgidstep=0, *Kestartstep=0, *Keendstep=0, *Etotstartstep=0, *Etotendstep=0, *ProcessCode=0, *trackIdnum=0, *timestep=0;
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
 

  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
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
      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
	//std::cout<<"size: "<<NhitsSTevt->size()<<std::endl;
      }
      double NhitsST_evt = NhitsSTevt->at(0);
      double Edep_evt = Edepevt->at(0);
      double pathlength_evt = pathlengthevt->at(0);

      if(NhitsST_evt!=0){
       


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



	  if((debugout==true)&&(file==0)&&(j<100)){
	    std::cout<<"--step #: "<<j<<" (plotting only first 100 steps-hits  per event) "<<std::endl;
	    std::cout<<"PDG Id: "<<pdgid_step<<std::endl;
	    std::cout<<"VolIdstep (foil #): "<<VolId_step<<std::endl;
	    std::cout<<"Edepstep: "<<Edep_step<<std::endl;
	    std::cout<<"pathlengthstep: "<<pathlength_step<<" mm"<<std::endl;
	    std::cout<<"Xmu2e_step: "<<Xmu2e_step<<" Ymu2e_step: "<<Ymu2e_step<<" Zmu2e_step: "<<Zmu2e_step<<" mm"<<std::endl;
	    std::cout<<"momstart_step: "<<momstart_step<<" momend_step: "<<momend_step<<" MeV"<<std::endl;
	    std::cout<<"Etotstart_step: "<<Etotstart_step<<" MeV"<<std::endl;
	    std::cout<<"Process_Code: "<<Process_Code<<" trackId_num: "<<trackId_num<<std::endl;
	  }


	  if(branch==0){gr->SetPoint(p,Xmu2e_step,Ymu2e_step); p++;}
	  if(branch==1){gr->SetPoint(p,Zmu2e_step,Ymu2e_step); p++;}

	}//NhitsST_evt
	
      //n=n+NhitsST_evt;
      //int Nfoils = Nhitsfoilevt->size()/entries;
      //std::cout<<"Number of foils in ST: "<<Nfoils<<std::endl;
      //for (int k = 0; k<Nfoils; k++){
      //std::cout<<"Foil n: "<<k<<" with "<<Nhitsfoilevt->at(k)<<" hits"<<std::endl;
      //}

      } //if(NhitsST_evt!=0)
    }//entries    
    input->Close();
  }//for int file

 

  gr->SetTitle("");
  gr->GetXaxis()->SetTitle(Xtitle);
  gr->GetYaxis()->SetTitle(Ytitle);
  gr->SetMarkerStyle(1);
  gr->SetMarkerColor(kBlack);
  gr->Draw("same,p");
  

  //c1->Print("zyhit_ST_byedepositparticles.png");
  //c1->Print("zyhit_ST_byedepositparticles.pdf");
}
*/
//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  
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


  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;
   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readST/treeST");


    tree->SetBranchAddress("VolIdstep",&VolIdstep);
    //tree->SetBranchAddress("Edepstep",&Edepstep);
    //tree->SetBranchAddress("pathlengthstep",&pathlengthstep);
    tree->SetBranchAddress("Xmu2estep",&Xmu2estep);
    tree->SetBranchAddress("Ymu2estep",&Ymu2estep);
    tree->SetBranchAddress("Zmu2estep",&Zmu2estep);
    //tree->SetBranchAddress("Xmu2estepend",&Xmu2estepend);
    //tree->SetBranchAddress("Ymu2estepend",&Ymu2estepend);
    //tree->SetBranchAddress("Zmu2estepend",&Zmu2estepend);
    tree->SetBranchAddress("momstartstep",&momstartstep);
    //tree->SetBranchAddress("momendstep",&momendstep);
    //tree->SetBranchAddress("pxstartstep",&pxstartstep);
    //tree->SetBranchAddress("pystartstep",&pystartstep);
    //tree->SetBranchAddress("pzstartstep",&pzstartstep);
    //tree->SetBranchAddress("pxendstep",&pxendstep);
    //tree->SetBranchAddress("pyendstep",&pyendstep);
    //tree->SetBranchAddress("pzendstep",&pzendstep);
    tree->SetBranchAddress("pdgidstep",&pdgidstep);
    //tree->SetBranchAddress("Kestartstep",&Kestartstep);
    //tree->SetBranchAddress("Keendstep",&Keendstep);
    tree->SetBranchAddress("Etotstartstep",&Etotstartstep);
    //tree->SetBranchAddress("Etotendstep",&Etotendstep);
    tree->SetBranchAddress("Etotstartsimpart",&Etotstartsimpart);
    //tree->SetBranchAddress("Etotendsimpart",&Etotendsimpart);
    tree->SetBranchAddress("ProcessCode",&ProcessCode);
    //tree->SetBranchAddress("trackIdnum",&trackIdnum);
    tree->SetBranchAddress("NhitsSTevt",&NhitsSTevt);
    tree->SetBranchAddress("Edepevt",&Edepevt);
    //tree->SetBranchAddress("pathlengthevt",&pathlengthevt);
    //tree->SetBranchAddress("Nhitsfoilevt",&Nhitsfoilevt);
    //tree->SetBranchAddress("timestep",&timestep);


    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;
 
    for(unsigned long i=0;i<entries;i++){
      
      tree->GetEntry(i);
      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
	//std::cout<<"size: "<<NhitsSTevt->size()<<std::endl;
      }
      double NhitsST_evt = NhitsSTevt->at(0);
      double Edep_evt = Edepevt->at(0);
      //double pathlength_evt = pathlengthevt->at(0);
     
    
      if(NhitsST_evt!=0){

	for(unsigned long int j = 0; j < NhitsST_evt; j++){

	  double VolId_step = VolIdstep->at(j);
	  //double Edep_step = Edepstep->at(j);
	  //double pathlength_step = pathlengthstep->at(j);
	  double Xmu2e_step = Xmu2estep->at(j);
	  double Ymu2e_step = Ymu2estep->at(j);
	  double Zmu2e_step = Zmu2estep->at(j);
	  //double Xmu2eend_step = Xmu2estepend->at(j);
          //double Ymu2eend_step = Ymu2estepend->at(j);
          //double Zmu2eend_step = Zmu2estepend->at(j);
	  double momstart_step = momstartstep->at(j);
	  //double momend_step = momendstep->at(j);
	  //double pxstart_step = pxstartstep->at(j); 
	  //double pystart_step = pystartstep->at(j);
	  //double pzstart_step = pzstartstep->at(j);
	  //double pxend_step = pxendstep->at(j);
          //double pyend_step = pyendstep->at(j);
          //double pzend_step = pzendstep->at(j);
	  double pdgid_step = pdgidstep->at(j);
	  //double Kestart_step = Kestartstep->at(j);
	  double Etotstart_step = Etotstartstep->at(j);
	  double Etotstart_simpart = Etotstartsimpart->at(j);
	  //double Etotend_simpart = Etotendsimpart->at(j);
	  double Process_Code = ProcessCode->at(j);
	  //double trackId_num = trackIdnum->at(j);
	  //double time_step = timestep->at(j);



	  if(pdgid_step==13){
	    //std::cout<<"Event: "<<i<<" Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<" momentum start hit: "<<momstart_step<<" Etotstart_simpart: "<<Etotstart_simpart<<" Zmu2e_step: "<<Zmu2e_step<<std::endl;
	  }

	  //muminus capture at rest ST
	  if(Process_Code==32){

	    if((branch==0)&&(pdgid_step==13)){
	      double momstartevent=sqrt(Etotstart_simpart*Etotstart_simpart-muonmass*muonmass);
	      //std::cout<<"CAPTURE"<<std::endl;
	      //std::cout<<"Event: "<<i<<" Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<" momentum start hit: "<<momstart_step<<" Etotstart_simpart: "<<Etotstart_simpart<<" momentum simparticle: "<<momstartevent<<" Zmu2e_step: "<<Zmu2e_step<<std::endl;
	      h1->Fill(momstartevent);
	      }
	    if((branch==1)&&(pdgid_step==13)){
	      h1->Fill(Xmu2e_step);
	      //h1->Fill(Ymu2e_step);
	      //h1->Fill(Zmu2e_step);
	    }

	    if(pdgid_step!=13){std::cout<<"Muminus capture without muminus!!"<<std::endl; 
	      std::cout<<"Process_Code: "<<Process_Code<<", pdgid_step: "<<pdgid_step<<std::endl;}
	  }
  
	  
	  
	}//NhitsST_evt      
      } //if(NhitsST_evt!=0)
    }//entries    
    input->Close();
  }//for int file
  
  
  h1->SetTitle("");
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  
  if(branch==0){h1->SetFillColor(kYellow+2);}
  if(branch==1){h1->SetLineColor(kBlue+1); ;}
  /*  if(branch==2){h1->SetFillColor(kViolet+2);}
  if(branch==3){h1->SetFillColor(kGreen+2);}
  if(branch==4){h1->SetFillColor(kMagenta+2);}
  if(branch==5){h1->SetFillColor(kOrange+2);}
  if(branch==6){h1->SetFillColor(kRed+2);}
  if(branch==7){h1->SetFillColor(kRed+2);}
  if(branch==8){h1->SetFillColor(kOrange+2);}
  if(branch==9){h1->SetFillColor(kGreen+2);}
  if(branch==11){h1->SetFillColor(kAzure+2);}
  if(branch==12){h1->SetFillColor(kCyan+2);}
  if(branch==13){h1->SetFillColor(kGreen+2);}
  */

  h1->Draw("");
  
    
  //c1->Print("EdepeventST_epluseminus_brems.png");
  //c1->Print("EdepeventST_epluseminus_brems.pdf");

}



//================================================================
void ReadST_plotCapturedMuons(std::string ArtFiles_location) {
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


  //0 is to print number of muminuscaptureatrest, process code = 32
  //PrintHisto(file_name, 100, -2, 110, 0, 3000, "p_{init #mu^{-}, capture at rest event ST} [MeV]", "Counts", 0);
  //1 is to print x,y,z positions of muminuscaptureatrest, process code = 32
  PrintHisto(file_name, 100, -3990, -3820, 0, 3000, "x_{#mu^{-}, capture at rest ST} [mm]", "", 1);
  //PrintHisto(file_name, 100, -80, 80, 0, 3000, "y_{#mu^{-}, capture at rest ST} [mm]", "", 1);
  //PrintHisto(file_name, 100, 5400, 6300, 0, 3000, "z_{#mu^{-}, capture at rest ST} [mm]", "", 1);


  //0 is to print x, y coordinates in Mu2e system
  //PrintGraph(file_name, -4200, -3600, -300, 300, "x_{ST}(Mu2e) [mm]", "y_{ST}(Mu2e) [mm]", 0); 
  //1 is to print y, z coordinates in Mu2e system 
  //PrintGraph(file_name,5400, 6300, -300, 300, "z_{ST}(Mu2e) [mm]", "y_{ST}(Mu2e) [mm]", 1); 
  
  readfile.close();
}

//================================================================
