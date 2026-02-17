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


//================================================================
void PlotTimes_prof(std::string ArtFiles_location, double xmin, double xmax, string Xtitlest, string Ytitlest, int VDnumber, double branch){

  gROOT->SetStyle("ATLAS");

  gStyle->SetOptStat(1110);
  gStyle->SetOptFit(01111);
  gStyle->SetPadRightMargin(0.08);
 
  TCanvas *c1 = new TCanvas("");

  double Xrange[2]={xmin,xmax};
  double Yrange[2]={0,3}; //meV

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

  int Ninitprofiles = 4;
  int Nprofiles = 3;
  int binsprofile = 25;
  //int binsprofile = 50;
  
  TProfile *hprofx_phot[Nprofiles];
  TProfile *hprofy_phot[Nprofiles];
  
  TGraph *gr_entries = new TGraph();

  TH1D* h1 = new TH1D("h1","",100,Xrange[0],Xrange[1]);
    
  for(int i=0;i<Ninitprofiles;i++){
    hprofx_phot[i] = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");
    hprofy_phot[i] = new TProfile("","",binsprofile,Xrange[0], Xrange[1],"");
  }

  
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

   double check_totphotons = 0;
   double check_highEphotons = 0;
  
  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

  
  //Read tree
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    //for (long unsigned int file=0;file<1;file++){  

    string path;
    path = file_name[file];
    std::cout<<"file #: "<<file<<" "<<path.c_str()<<std::endl;

    TFile *input = new TFile(path.c_str());
    TTree* tree = (TTree*)input->Get("readvd/ntvd");

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
    
    unsigned long entries = tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;


     for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);

      //Just fill at Virtual detector ID number:

      if(sid==VDnumber){

	double mom = sqrt(px*px+py*py+pz*pz);

	if( pdg==22 ){
	  
	  double xcentre = -3904; //mm
          double ycentre = 0;
          double r = sqrt((x-xcentre)*(x-xcentre)+(y-ycentre)*(y-ycentre));

          if((r < 75.0224)&&(pz > 0)){

	    //Time restriction
	    if(time<2000){ //2us 
	      hprofx_phot[0]->Fill(time,time);
	      hprofy_phot[0]->Fill(time,mom);/*MeV*/

	      //if(code==115){
		hprofx_phot[3]->Fill(mom,mom);
		hprofy_phot[3]->Fill(mom,time);
		//}
	      /*
	      if((code!=16)&&(mom<1)){
		h1->Fill(mom);
		std::cout<<"Mom: "<<mom<<"MeV, time: "<<time<<" ns, code: "<<code<<std::endl;}*/
	      if((mom)<0.5){
		hprofx_phot[1]->Fill(time,time);
		hprofy_phot[1]->Fill(time,mom);/*MeV*/
	      }
	      if((mom)<5){
		hprofx_phot[2]->Fill(time,time);
		hprofy_phot[2]->Fill(time,mom);/*MeV*/
	      }
	    }//time
	  }//r
	}//pdg
      }//sid
      
     }//entries
     
  }//file
  



TGraph *GraphEntries[Ninitprofiles];
TGraphErrors *ProfileErrors[Ninitprofiles];

//first 3 profiles
for(int i=0;i<Nprofiles;i++){
  GraphEntries[i]= new TGraph ();
  vector<double> x_v, y_v, x_error_v, y_error_v;
  x_v.clear();
  y_v.clear();
  x_error_v.clear();
  y_error_v.clear();
  TAxis *xaxis = hprofx_phot[0]->GetXaxis();
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
    std::cout<<"time: "<<x_value<<"+-"<<x_error<<", mom: "<<y_value<<"+-"<<y_error<<" Entries in mom: "<<y_entries<<std::endl;
    
    //Fill it just if energy is bigger than 0                                                                     
    if(y_value!=0){
      GraphEntries[i]->SetPoint(p,x_value,y_entries); p++;
      x_v.push_back(x_value);
      y_v.push_back(y_value);
      x_error_v.push_back(x_error);
      y_error_v.push_back(y_error);
    } //yvalue
  }//bin
  
  ProfileErrors[i] = new TGraphErrors(x_v.size(),&x_v[0],&y_v[0],0,&y_error_v[0]);
  ProfileErrors[i]->SetMarkerStyle(21);
  GraphEntries[i]->SetMarkerStyle(21);
 }
 

 if(branch==0) {
   ProfileErrors[0]->SetMarkerColor(kGreen-3);
   GraphEntries[0]->SetMarkerColor(kGreen-3);
   ProfileErrors[1]->SetMarkerColor(kPink-3);
   GraphEntries[1]->SetMarkerColor(kPink-3);
   ProfileErrors[2]->SetMarkerColor(kBlue-3);
   GraphEntries[2]->SetMarkerColor(kBlue-3);
  
   ProfileErrors[0]->Draw("same,p");
   ProfileErrors[1]->Draw("same,p");
   ProfileErrors[2]->Draw("same,p");

   auto leg = new TLegend(0.2,0.7,0.68,0.9);
   
   leg->AddEntry(ProfileErrors[0], "All E_{#gamma}","p");
   leg->AddEntry(ProfileErrors[1], "E_{#gamma} < 0.5 MeV","p");
   leg->AddEntry(ProfileErrors[2], "E_{#gamma} < 5 MeV","p");
   leg->Draw("same");
 }

 
 if(branch==1){
   int j =0;
   
   for(int i =0; i< binsprofile; i++){
     double time_bin = hprofx_phot[0]->GetBinContent(i);
     double bincontent = hprofy_phot[0]->GetBinEntries(i);
     if(bincontent!=0){
     gr_entries->SetPoint(j, time_bin, bincontent);
     std::cout<<"time: "<<time_bin<<", Bin entries: "<<bincontent<<std::endl;
     j++;
     }
   }

   gPad->SetLogy();
   gr_entries->GetHistogram()->SetMinimum(0);
   gr_entries->SetMarkerColor(kGreen-3);
   gr_entries->SetMarkerStyle(21);
   gr_entries->GetXaxis()->SetTitle(Xtitle);
   gr_entries->GetYaxis()->SetTitle(Ytitle);
   gr_entries->Draw("ap");
 }


if(branch==2){
   int j =0;

   vector<double> x_v, y_v, x_error_v, y_error_v;
   x_v.clear();
   y_v.clear();
   x_error_v.clear();
   y_error_v.clear();

   for(int i =0; i< binsprofile; i++){
     double mom_bin = hprofx_phot[3]->GetBinContent(i);
     double time_bin = hprofy_phot[3]->GetBinContent(i);

     double mom_error = hprofx_phot[3]->GetBinError(i);
     double time_error = hprofy_phot[3]->GetBinError(i);
     if(time_bin!=0){
       x_v.push_back(mom_bin);
       y_v.push_back(time_bin);
       x_error_v.push_back(mom_error);
       y_error_v.push_back(time_error);
       std::cout<<"mom: "<<mom_bin<<", time bin: "<<time_bin<<std::endl;
       j++;
      }
   }

   ProfileErrors[3] = new TGraphErrors(x_v.size(),&x_v[0],&y_v[0],0,&y_error_v[0]);
   ProfileErrors[3]->SetMarkerStyle(21);

   //gPad->SetLogy();
   //gr_entries->GetHistogram()->SetMinimum(0);
   ProfileErrors[3]->SetMarkerColor(kGreen-3);
   ProfileErrors[3]->GetXaxis()->SetTitle(Xtitle);
   ProfileErrors[3]->GetYaxis()->SetTitle(Ytitle);
   ProfileErrors[3]->GetYaxis()->SetRangeUser(60,110);
   ProfileErrors[3]->Draw("ap");
 }


 
c1->Print("ArrivalPhotonTimes_cut2us_VD10_Nentries_zoom.png");
//c1->Print("ArrivalPhotonTimes_VD10_Nentries_300ns.png");
}


//================================================================

  void Arrivaltimes_VD(std::string ArtFiles_location, double VDnumber){
  
    //PlotTimes_prof(ArtFiles_location, 0, 3000, "t [ns]","E_{#gamma,VD=10 (cut ST size, p_{z}>0)} [MeV]", VDnumber, 0);  

    //PlotTimes_prof(ArtFiles_location, 0, 300, "t [ns]","#Photons_{VD=10 (cut ST size, p_{z}>0)}", VDnumber, 1);

    PlotTimes_prof(ArtFiles_location, 0, 20, "E_{#gamma,VD=10 (cut ST size, p_{z}>0)} [MeV]","t [ns]", VDnumber, 2);

  }
