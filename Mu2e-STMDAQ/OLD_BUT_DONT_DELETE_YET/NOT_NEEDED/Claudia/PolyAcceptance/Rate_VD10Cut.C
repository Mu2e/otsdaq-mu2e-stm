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


//================================================================
void PlotRate_Cut(double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest){

  gROOT->SetStyle("ATLAS");

  gStyle->SetOptStat(1110);
  gStyle->SetOptFit(01111);

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
  //gPad->SetLogy();

  int N = 8;

  double Cut[8] = {2.5, 3.5, 4.5, 5, 5.5, 7.50224, 7.86, 57.7};
  double Cut_error[8];

  double Nphotons_nocut[8] = {200474, 417873, 665589, 794536, 922836, 1414626, 1493267, 8914495};
  
  double acceptance_phot_5MeV[8] = {4.09E-08, 7.95E-08, 7.86E-08, 7.71E-08, 7.23E-08, 5.36E-08, 5.11E-08, 8.86E-09};
  double acceptance_error_5MeV[8];

  double acceptance_phot_50MeV[8] = {5.24E-08, 9.25E-08, 9.19E-08, 9.18E-08, 8.83E-08, 6.95E-08, 6.62E-08, 1.09E-08};
  double acceptance_error_50MeV[8];

  double rate_5MeV_avintensity[8], rate_5MeV_avintensity_error[8];
  double rate_50MeV_avintensity[8], rate_50MeV_avintensity_error[8];

  double rate_5MeV_highintensity[8], rate_5MeV_highintensity_error[8];
  double rate_50MeV_highintensity[8], rate_50MeV_highintensity_error[8];

  double time_avintensity_us = 16.95;
  double time_highintensity_us = 10;
  
  double time_avintensity_ms = time_avintensity_us / 1000;
  double time_highintensity_ms = time_highintensity_us / 1000 ;
  
  for(int i = 0; i < 8; i++){

    Cut_error[i] = 0;

    /*  
    rate_5MeV_avintensity[i] = Nphotons_nocut[i] * acceptance_phot_5MeV[i] / time_avintensity_ms;
    rate_50MeV_avintensity[i] = Nphotons_nocut[i] * acceptance_phot_50MeV[i] / time_avintensity_ms;
    rate_5MeV_highintensity[i] = Nphotons_nocut[i] * acceptance_phot_5MeV[i] / time_highintensity_ms;
    rate_50MeV_highintensity[i] = Nphotons_nocut[i] * acceptance_phot_50MeV[i] / time_highintensity_ms;
    */
    
    rate_5MeV_avintensity[i] = Nphotons_nocut[i] * acceptance_phot_5MeV[i];
    rate_50MeV_avintensity[i] = Nphotons_nocut[i] * acceptance_phot_50MeV[i];
    rate_5MeV_highintensity[i] = Nphotons_nocut[i] * acceptance_phot_5MeV[i];
    rate_50MeV_highintensity[i] = Nphotons_nocut[i] * acceptance_phot_50MeV[i];
    
    rate_5MeV_avintensity_error[i] = 0;
    rate_50MeV_avintensity_error[i] = 0;
    rate_5MeV_highintensity_error[i] = 0;
    rate_50MeV_highintensity_error[i] = 0;

    //std::cout<<" "<<std::endl;
  }


  //Normalise:
  for(int i = 0 ; i < 8 ; i++){
    rate_5MeV_avintensity[i] = rate_5MeV_avintensity[i]/rate_5MeV_avintensity[7];
    rate_50MeV_avintensity[i] = rate_50MeV_avintensity[i]/rate_50MeV_avintensity[7];
  }
  
  graph1->Draw("ap");
  
  TGraphErrors *gracc[4];

  gracc[0] = new TGraphErrors( N, Cut, rate_5MeV_avintensity, Cut_error, rate_5MeV_avintensity_error );
  gracc[1] = new TGraphErrors( N, Cut, rate_50MeV_avintensity, Cut_error, rate_50MeV_avintensity_error );

  //gracc[2] = new TGraphErrors( N, Cut, rate_5MeV_highintensity, Cut_error, rate_5MeV_highintensity_error );
  //gracc[3] = new TGraphErrors( N, Cut, rate_50MeV_highintensity, Cut_error, rate_50MeV_highintensity_error );
  
  for( int i = 0; i<2; i++){
  gracc[i]->Print("");
  gracc[i]->SetMarkerStyle(21);

  gracc[0]->SetMarkerColor(kAzure-9);
  gracc[1]->SetMarkerColor(kAzure+1);
  //gracc[2]->SetMarkerColor(kOrange-9);
  //gracc[3]->SetMarkerColor(kOrange+1);

  gracc[0]->SetLineColor(kAzure-9);
  gracc[1]->SetLineColor(kAzure+1);
  //gracc[2]->SetLineColor(kOrange-3);
  //gracc[3]->SetLineColor(kOrange-3);

  gracc[i]->SetLineStyle(2);
  
  gracc[i]->SetLineWidth(2);
  //gracc[i]->Draw("same,pl");

  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);

  c1->Modified();
  c1->Update();
 
  }

  gracc[0]->Draw("same,pl"); 
  gracc[1]->Draw("same,pl"); 


  auto leg = new TLegend(0.38,0.2,0.9,0.52);

  //leg->AddEntry(gracc[0], "Average intensity Run-I, E_{#gamma}<5 MeV","pl");
  //leg->AddEntry(gracc[1], "Average intensity Run-I, E_{#gamma}<50 MeV","pl");
  //leg->AddEntry(gracc[2], "High intensity Run-I, E_{#gamma}<5 MeV","pl");
  //leg->AddEntry(gracc[3], "High intensity Run-I, E_{#gamma}<50 MeV","pl");

  leg->AddEntry(gracc[0], "Counts VD10#timesMean Acceptance: E_{#gamma}<5 MeV","pl");
  leg->AddEntry(gracc[1], "Counts VD10#timesMean Acceptance: E_{#gamma}<50 MeV","pl");
  leg->Draw("same");
   
  c1->Print("CountstimesmeanAcceptance_STM_VD10cut_nozoom_norm.png");
  c1->Print("CountstimesmeanAcceptance_STM_VD10cut_nozoom_norm.pdf");
}


//================================================================

void Rate_VD10Cut(){
  
  //PlotRate_Cut( 0, 60, 0, 20, "VD10 radius cut [cm]", "#gamma rate at STM [kHz]");

  //PlotRate_Cut( 2, 10, 0, 20, "VD10 radius cut [cm]", "#gamma rate at STM [kHz]");

  //PlotRate_Cut( 2, 10, 0, 1.2, "VD10 radius cut [cm]", "Normalised # #gamma at STM MDC2020 "); 
  PlotRate_Cut( 2, 60, 0, 1.2, "VD10 radius cut [cm]", "Normalised # #gamma at STM MDC2020");
}
