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

void plotTree_RMS() {

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);  
  auto c1= new TCanvas("");

  int PDGID, EventID;
  double Xpos, Ypos, Zpos;
  
  TFile *input=new TFile("/work/cgarcia/DATA/geant4/FixedPointEvent/STM100cmaway_E1809keV.root");
  TTree* tree=(TTree*)input->Get("Steps");


  tree->SetBranchAddress("PDGID",&PDGID);
  tree->SetBranchAddress("EventID",&EventID);
  tree->SetBranchAddress("Xpos",&Xpos);
  tree->SetBranchAddress("Ypos",&Ypos);
  tree->SetBranchAddress("Zpos",&Zpos);



    unsigned long entries=tree->GetEntries();
    cout<<"entries: "<<entries<<endl;

    //Initialise arrays of histograms
    TH1F *hx[1000];
    TH1F *hy[1000];
    TH1F *hz[1000];
    TH1F *hr[1000];

    TH1F *hxRMS = new TH1F("hxRMS","",100, -8,8);
    TH1F *hyRMS = new TH1F("hyRMS","",100, -8,8);;
    TH1F *hzRMS = new TH1F("hzRMS","",100, -8,8);
    TH1F *hrRMS = new TH1F("hrRMS","",100, -8,8);

    for(int i=0;i<1000;i++){
      hx[i]=new TH1F("","",1000, -10,10);
      hy[i]=new TH1F("","",1000, -10,10);
      hz[i]=new TH1F("","",1000, 100,110);
      hr[i]=new TH1F("","",1000, -10,10);
    }

    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);

      //if particle is an electron or a positron
      //Fill with positions of electrons for each event get 1000 histograms, one per event


      if((PDGID==11)||(PDGID==-11)){
	hx[EventID]->Fill(Xpos);
	hy[EventID]->Fill(Ypos);
	hz[EventID]->Fill(Zpos);
	double r = sqrt(Xpos*Xpos+Ypos*Ypos);
	hr[EventID]->Fill(r);
	
	std::cout<<"EventID: "<<EventID<<" xpos: "<<Xpos<<" ypos: "<<Ypos<<" zpos: "<<Zpos<<std::endl;
	std::cout<<"Radial distance: "<<r<<std::endl;
	std::cout<<"PDGID: "<<PDGID<<std::endl;
	}
    }
    


    double meanx[1000],meany[1000],meanz[1000],meanr[1000];
    double RMSx[1000],RMSy[1000],RMSz[1000],RMSr[1000];

    
    // Get the RMS in x,y and z for each of the 1000 events
    for(int i=0;i<1000;i++){
      //Plot the x, y or z positions for each event 
      //hx[i]->Draw();
      //hy[i]->Draw();
      //hz[i]->Draw(); 

      //c1->Update();
      //c1->Modified();
      //c1->WaitPrimitive();

      //Get mean and RMS and fill the RMS histogram for x, y and z
      meanx[i]=hx[i]->GetMean();
      RMSx[i]=hx[i]->GetRMS();      

      meany[i]=hy[i]->GetMean();
      RMSy[i]=hy[i]->GetRMS();
        
      meanz[i]=hz[i]->GetMean();
      RMSz[i]=hz[i]->GetRMS();

      meanr[i]=hr[i]->GetMean();
      RMSr[i]=hr[i]->GetRMS();

      //If x,y,z RMS are 0 don't count the event, this means that the event has no electrons, one electron or that they are created in the same point and we don't include them in the analysis
      if((RMSx[i]!=0)&&(RMSy[i]!=0)&&(RMSz[i]!=0)){
      hxRMS->Fill(RMSx[i]);
      std::cout<<"Event: "<<i<<" mean in x positions: "<< meanx[i]<<" RMS: "<< RMSx[i]<<std::endl;
      hyRMS->Fill(RMSy[i]);
      std::cout<<"Event: "<<i<<" mean in y positions: "<< meany[i]<<" RMS: "<< RMSy[i]<<std::endl;
      hzRMS->Fill(RMSz[i]);
      std::cout<<"Event: "<<i<<" mean in z positions: "<< meanz[i]<<" RMS: "<< RMSz[i]<<std::endl;

      //Uncomment just for the gaussian fit
      hxRMS->Fill((-1)*RMSx[i]);
      hyRMS->Fill((-1)*RMSy[i]);
      hzRMS->Fill((-1)*RMSz[i]);
      }

      if(RMSr[i]!=0){
	hrRMS->Fill(RMSr[i]);
	std::cout<<"Event: "<<i<<" mean in radial positions: "<< meanr[i]<<" RMS: "<< RMSr[i]<<std::endl;

	//Uncomment just for the gaussian fit
	//hrRMS->Fill((-1)*RMSr[i]);
      }

      }

    //Fit with a gaussian
    TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])",-8,8);
    Fit1->SetParameters(1,0,1);


    double yminnorm=0;
    //double ymaxnorm=0.1;
    //double ymaxnorm=0.33;
    //double ymaxnorm=0.18;   
    //double ymaxnorm=0.25;
    double ymaxnorm=0.2;
    //double ymaxnorm=0.15;

    //X RMS Plot
    hxRMS->GetXaxis()->SetTitle("RMS in x of each event [cm]");
    hxRMS->GetYaxis()->SetTitle("");
    hxRMS->SetFillColor(kGreen-3);    
    hxRMS->SetLineColor(kGreen-3);
    //hxRMS->Draw();
    //X RMS Plot normalised 
    TH1F*hnormx = (TH1F*)(hxRMS->Clone("histRMSx"));
    hnormx->Scale(1./hnormx->Integral());
    hnormx->GetYaxis()->SetRangeUser(yminnorm,ymaxnorm);
    //hnormx->Draw("HIST");
    //hnormx->Fit(Fit1,"0","",-8,8);  

    //Y RMS plot
    hyRMS->GetXaxis()->SetTitle("RMS in y of each event [cm]");
    hyRMS->GetYaxis()->SetTitle("");
    hyRMS->SetFillColor(kBlue-3);
    hyRMS->SetLineColor(kBlue-3);
    //hyRMS->Draw();
    //Y RMS Plot normalised
    TH1F*hnormy = (TH1F*)(hyRMS->Clone("histRMSy"));
    hnormy->Scale(1./hnormy->Integral());
    hnormy->GetYaxis()->SetRangeUser(yminnorm,ymaxnorm);
    //hnormy->Draw("HIST");
    //hnormy->Fit(Fit1,"0","",-8,8);  

    //Z plot
    hzRMS->GetXaxis()->SetTitle("RMS in z of each event [cm]");
    hzRMS->GetYaxis()->SetTitle("");
    hzRMS->SetFillColor(kOrange-3);
    hzRMS->SetLineColor(kOrange-3);
    //hzRMS->Draw();
    //X RMS Plot normalised
    TH1F*hnormz = (TH1F*)(hzRMS->Clone("histRMSz"));
    hnormz->Scale(1./hnormz->Integral());
    hnormz->GetYaxis()->SetRangeUser(yminnorm,ymaxnorm);
    //hnormz->Draw("HIST");
    //hnormz->Fit(Fit1,"0","",-8,8);




    //r plot
    hrRMS->GetXaxis()->SetTitle("RMS in r of each event [cm]");
    hrRMS->GetYaxis()->SetTitle("");
    hrRMS->SetFillColor(kCyan-3);
    hrRMS->SetLineColor(kCyan-3);
    //hrRMS->Draw();
    //r RMS Plot normalised
    TH1F*hnormr = (TH1F*)(hrRMS->Clone("histRMSr"));
    hnormr->Scale(1./hnormr->Integral());
    hnormr->GetYaxis()->SetRangeUser(yminnorm,ymaxnorm);
    hnormr->Draw("HIST");
    hnormr->Fit(Fit1,"0","",-8,8);

    Fit1->SetLineColor(kBlue);
    Fit1->SetLineStyle(2);
    //Fit1->Draw("same");
    


    TLatex latex;
    latex.SetTextSize(0.05);
    //latex.DrawLatex(-7,0.26,"E_{#gamma}=347 keV");
    //latex.DrawLatex(-7,0.23,"e^{-} or e^{+} processes");
    //latex.DrawLatex(-7,0.14,"E_{#gamma}=844 keV");
    //latex.DrawLatex(-7,0.125,"e^{-} or e^{+} processes");
    //latex.DrawLatex(-7,0.2,"E_{#gamma}=1809 keV");
    //latex.DrawLatex(-7,0.18,"e^{-} or e^{+} processes");
    
    latex.DrawLatex(-7,0.08,"E_{#gamma}=1809 keV");
    latex.DrawLatex(-7,0.065,"e^{-} or e^{+} processes");
}
