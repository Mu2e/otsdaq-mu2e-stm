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

#define MeV_to_keV 1000


//================================================================
void ReadtxtFile(std::string Efficiency_file, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, double det_angle_dregrees, double branch){

  gROOT->SetStyle("ATLAS");

  gStyle->SetOptStat(1110);
  gStyle->SetOptFit(01111);

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle= const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  unsigned long int gammas_gen = 1000000;
  
  double E_error[31];
  double gamma_detected[31];
  double gamma_detected_eff[31];
  double gamma_detected_eff_error[31];
  double gamma_detected_error[31];

  double E[31]={0.05, 0.06,0.07,0.08,0.09,0.1,0.2, 0.3, 0.347,0.38,0.4,0.45,0.5, 0.51,0.52,0.53,0.54,0.55,0.58,0.6,0.65,0.7,0.8,0.9,1,1.5, 2, 2.5, 3, 4, 5};
  
  //Read file
  fstream readfile;
  readfile.open(Efficiency_file, ios::in);
  int N=0;
  
  while(1){

    readfile>>gamma_detected_eff[N];

    if(readfile.eof())break;
    N++;
  }


  for( int i = 0 ; i < N ; i++ ){

    E_error[i] = 0;

    gamma_detected[i] = gamma_detected_eff[i]*gammas_gen;
    
    gamma_detected_error[i] = sqrt(gamma_detected[i]);

    gamma_detected_eff_error[i] = gamma_detected_eff[i]/gamma_detected[i];

    if(gamma_detected[i]==0){gamma_detected_eff_error[i] = 0;}
      
    std::cout<<"Photons detected:  "<<gamma_detected[i]<<"+-"<<gamma_detected_error[i]<<std::endl;  
  }

  
  TGraphErrors *greff = new TGraphErrors( N, E, gamma_detected_eff, E_error,  gamma_detected_eff_error);
  greff->Print("");

  if(branch==0){
    greff->SetMarkerStyle(21);
    greff->SetMarkerColor(kBlue-9);
    greff->SetLineColor(kBlue-9);
    greff->SetLineWidth(2);
    greff->Draw("same,p");
  }
  
  if(branch==1){
    greff->SetMarkerStyle(21);
    greff->SetMarkerColor(kBlue-9);
    greff->SetLineColor(kBlue-9);
    greff->SetLineWidth(2);
    greff->Draw("same,p");

    /*
      TF1 *Fit = new TF1("Fit", "[0]*exp(-0.5*((x-[1])/[2])^2) + [3]*exp([4]*x-[5])+[6]", 0.4,5);
      Fit->SetParameters(-0.0266075, 0.391095, -0.0649706, 0.234963, -5.6454, 0.5, 0.002);
      Fit->SetParameters(-0.0320602, 0.39, -0.005, 0.331591, -6.80915, -0.269724, 0.000667923);
    */
    /*
    TF1 *Fit = new TF1("Fit", "[0]/(1+exp((x-[1])/[2])) * (1-( [3]/(1+exp((x-[1])/[2]) ) ) )", 0.4,5);
    Fit->SetParameters(1,1,1,1);
  
  
    
    greff->Fit(Fit,"0","", xmin, xmax);
    Fit->SetLineColor(kMagenta+3);
    Fit->SetLineStyle(2);
    Fit->SetLineWidth(3);
    Fit->SetNpx(3000);
    Fit->Draw("same");

    gPad->Update();
    TPaveStats* ps = (TPaveStats *)greff->FindObject("stats");
    ps->SetY1NDC(.22);
    ps->SetY2NDC(.55);
    ps->SetX1NDC(0.52);
    ps->SetX2NDC(0.9);
    ps->SetLineWidth(6);
    ps->SetLineColor(kWhite);
    */
  /*
    TAxis *Y = graph1->GetYaxis();
    Y->SetNdivisions(6,6,1);
    
    c1->Modified();
    c1->Update();
  */
    
    graph1->Draw("ap");
    greff->Draw("same,p");
    //Fit->Draw("same");

  }

  if(branch==2){
    
    greff->SetMarkerStyle(21);
    greff->SetMarkerColor(kGreen-9);
    greff->SetLineColor(kGreen-9);
    greff->SetLineWidth(2);
    greff->Draw("same,p");

  }
  c1->Print("347contribution_noAl_Geomv08.png");
 
}



//================================================================
void plot_Efficiency(std::string Efficiency_file){

  //ReadtxtFile(Efficiency_file, 0, 5, 0, 1, "E_{#gamma} [MeV]", "Photopeak fraction at STM (+0.5mm Al)", 0, 0);

  ReadtxtFile(Efficiency_file, -1,5, 0, 0.015, "E_{#gamma} [MeV]", "Fraction contributing to 347keV", 0, 1);

  //ReadtxtFile(Efficiency_file, 0, 5, 0, 0.5, "E_{#gamma} [MeV]", "Fraction not depositing E at STM (+0.5mm Al)", 0, 2);


}

//================================================================
