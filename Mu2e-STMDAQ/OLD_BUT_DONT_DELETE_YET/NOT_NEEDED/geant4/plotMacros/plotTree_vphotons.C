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
#include "TAxis.h"
#include "TH1F.h"

void plotTree_vphotons() {

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);  
  auto c1= new TCanvas("");

  int PDGID, EventID, particlenum;
  double preXpos, preYpos, preZpos, postXpos, postYpos, postZpos, stepinitialtime, stepfinaltime, velocity, deltat, deltax, deltay,deltaz,distance;
  
  TFile *input=new TFile("/work/cgarcia/DATA/geant4/FixedPointEvent/Times/STM100cmaway_E844keV.root");
  TTree* tree=(TTree*)input->Get("Steps");


  tree->SetBranchAddress("PDGID",&PDGID);
  tree->SetBranchAddress("EventID",&EventID);
  tree->SetBranchAddress("preXpos",&preXpos);
  tree->SetBranchAddress("preYpos",&preYpos);
  tree->SetBranchAddress("preZpos",&preZpos);
  tree->SetBranchAddress("postXpos",&postXpos);
  tree->SetBranchAddress("postYpos",&postYpos);
  tree->SetBranchAddress("postZpos",&postZpos);
  tree->SetBranchAddress("step_initialtime",&stepinitialtime);
  tree->SetBranchAddress("step_finaltime",&stepfinaltime);
  tree->SetBranchAddress("particlenum",&particlenum);

    unsigned long entries=tree->GetEntries();
    cout<<"entries: "<<entries<<endl;

    //Initialise arrays of histograms
    TH1F *hVel = new TH1F("hVel","",10000, 290000000,310000000);
    TH1F *ht = new TH1F("hDeltat","",100, -0.1,0.6);


    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);

      //if particle is a photon
      //Calculate the distance travelled and difference in time


      if(PDGID==22){
	deltax=postXpos-preXpos;//cm
	deltay=postYpos-preYpos;
	deltaz=postZpos-preZpos;
	std::cout<<"Initial time: "<<stepinitialtime<<"ns, Final time: "<<stepfinaltime<<" ns"<<std::endl;
	deltat=stepfinaltime-stepinitialtime;//ns
	distance=sqrt(deltax*deltax+deltay*deltay+deltaz*deltaz);
	velocity=distance/deltat; //cm/ns
	double velocitySI= velocity*1e7;//m/s

	hVel->Fill(velocitySI);
	ht->Fill(deltat);
	
	std::cout<<"Distance: "<<distance<<" cm, delta_time: "<<deltat<<"ns, velocity: "<<velocitySI<<" m/s"<<std::endl;
	std::cout<<"PDGID: "<<PDGID<<std::endl;
	}
    }
    

    //Velocity Plot
    hVel->GetXaxis()->SetTitle("v_{#gamma}(Ge) [m/s]");
    hVel->GetYaxis()->SetTitle("");
    hVel->SetFillColor(kViolet+1);    
    hVel->SetLineColor(kViolet+1);
    //hVel->Draw();
    //Velocity Plot normalised 
    TH1F*hnormVel = (TH1F*)(hVel->Clone("hSpeed"));
    hnormVel->Scale(1./hnormVel->Integral());
    //hnormVel->GetYaxis()->SetRangeUser(yminnorm,ymaxnorm);
    hnormVel->Draw("HIST");
   

    //Deltat Plot
    ht->GetXaxis()->SetTitle("#Deltat_{#gamma} [ns]");
    ht->GetYaxis()->SetTitle("");
    ht->SetFillColor(kRed+1);
    ht->SetLineColor(kRed+1);
    //ht->Draw();
    //Deltat Plot normalised                                                                                                                                                   
    TH1F*hnormt = (TH1F*)(ht->Clone("htime"));
    hnormt->Scale(1./hnormt->Integral());
    //hnormt->GetYaxis()->SetRangeUser(yminnorm,ymaxnorm);
    //hnormt->Draw("HIST");


      

    TLatex latex;
    latex.SetTextSize(0.05);
    //latex.DrawLatex(0,0.1,"E_{#gamma}=1809 keV");
    //latex.DrawLatex(300000000,0.1,"E_{#gamma}=1809 keV"); 
    //latex.DrawLatex(0,0.125,"#gamma processes");
}
