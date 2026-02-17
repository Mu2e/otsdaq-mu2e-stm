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
void ReadtxtFile(std::string file_inputVD1, std::string file_inputVD2, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int VD1, int VD2, bool both){

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
  //gPad->SetLogx();
  //graph1->SetMinimum(0.00000001);
  //graph1->SetMaximum(1);


  unsigned long int gammas_gen_VD15 = 100000;
  double E[21] = {0.05, 0.06,0.07, 0.08, 0.09, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.5, 2, 2.5, 3, 4, 5}; 
  double E_error[21];
  double gamma_detectedVD1[21], gamma_detected_errorVD1[21];
  double gamma_detectedVD2[21], gamma_detected_errorVD2[21];
  double gamma_detectedVD2_89[21], gamma_detectedVD2_90[21];
  double acceptance[21], acceptance_error[21];
  
  //Read file
  fstream readfileVD1;
  readfileVD1.open(file_inputVD1,ios::in);
  int NVD1=0;
  
  fstream readfileVD2;
  readfileVD2.open(file_inputVD2,ios::in);
  int NVD2=0;

  int N;
  
  while(1){

    readfileVD1>>gamma_detectedVD1[NVD1];

    if(readfileVD1.eof())break;
    NVD1++;
  }


  while(1){

    /*if(VD2==8990){
      readfileVD2>>gamma_detectedVD2_89[NVD2];
      readfileVD2>>gamma_detectedVD2_90[NVD2];
    }
    else { readfileVD2>>gamma_detectedVD2[NVD2]; }
    */
    readfileVD2>>gamma_detectedVD2[NVD2];
    if(readfileVD2.eof())break;
    NVD2++;
  }

  if(NVD1!=NVD2) { std::cout<<"File sizes are different...Check this..."<<std::endl; exit(0); }
  else{ N = NVD1; }
    
  for( int i = 0 ; i < N ; i++ ){

    E_error[i] = 0;

    /*if(VD2==8990){
      gamma_detectedVD2[i] = (gamma_detectedVD2_89[i] + gamma_detectedVD2_90[i])/2;
      gamma_detected_errorVD2[i] = sqrt(gamma_detectedVD2[i]) /2;
    }
    else{ gamma_detected_errorVD2[i] = sqrt(gamma_detectedVD2[i]); }
    */
    //gamma_detectedVD2[i]= 2*gamma_detectedVD2[i];
    
    gamma_detected_errorVD2[i] = sqrt(gamma_detectedVD2[i]);
    if(VD2==8990){
      //error on the mean
      gamma_detected_errorVD2[i]=sqrt(2*gamma_detectedVD2[i])/2;
    }
    gamma_detected_errorVD1[i] = sqrt(gamma_detectedVD1[i]);
    
    acceptance[i] = gamma_detectedVD2[i] / gamma_detectedVD1[i];

    //old error (assuming no correlation)
    //acceptance_error[i] = acceptance[i] * sqrt( (gamma_detected_errorVD1[i]/gamma_detectedVD1[i])*(gamma_detected_errorVD1[i]/gamma_detectedVD1[i]) + (gamma_detected_errorVD2[i]/gamma_detectedVD2[i])*(gamma_detected_errorVD2[i]/gamma_detectedVD2[i]) );

    //new error (assuming correlation - binomial)
    acceptance_error[i] = sqrt( acceptance[i]*(1-acceptance[i]) / gamma_detectedVD1[i]);
    
    std::cout<<i<<", E: "<<E[i]<<"MeV, Photons detected at VD"<<VD1<<": "<<gamma_detectedVD1[i]<<"+-"<<gamma_detected_errorVD1[i]<<" and at VD"<<VD2<<": "<<gamma_detectedVD2[i]<<"+-"<<gamma_detected_errorVD2[i]<<std::endl;
    std::cout<<"Acceptance VD"<<VD1<<"-"<<VD2<<": "<<acceptance[i]<<"+-"<<acceptance_error[i]<<std::endl;
  }

  
  TGraphErrors *gracc = new TGraphErrors( N, E, acceptance, E_error, acceptance_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kRed-9);
  gracc->SetLineColor(kRed-9);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  TF1*Fit;
  
  if((VD1==100)&&(VD2==101)){
    Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
    Fit->SetParameters(1.337,0.5822,0.3353,0.3359);
  
    //Fit->SetParameters(0.93,1.23,0.69,1.52);    
    //Fit->SetParameters( 1.14443, 0.343692, 0.152397, 0.118056);
  }
  
  if((VD1==15)&&(VD2==100)){
    Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
    Fit->SetParameters(0.9339, 1.216, 0.6772, 1.473);

  }
  
  if((VD1==15)&&(VD2==101)){
    Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
    Fit->SetParameters(1.148, 0.3404, 0.1504, 0.1158);

  }
  if((VD1==101)&&(VD2==8990)){
    Fit = new TF1("Fit", "[0]", xmin, xmax);
    Fit->SetParameter(1,0.004);
  }
  
  if((VD1==101)&&(VD2==88)){
    Fit = new TF1("Fit", "[0]", xmin, xmax);
    Fit->SetParameter(1,0.007);
  }
  
 if((VD1==101)&&(VD2==88)){
    Fit = new TF1("Fit", "[0]", xmin, xmax);
    Fit->SetParameter(1,0.007);
  }
 
 if((VD1==88)&&(VD2==8990)&&(both==false)){
    Fit = new TF1("Fit", "[0]", xmin, xmax);
    Fit->SetParameter(1,0.5);
 }

 if((VD1==88)&&(VD2==8990)&&(both==true)){
    Fit = new TF1("Fit", "[0]", xmin, xmax);
    Fit->SetParameter(1,1);
 }
 
  
  gracc->Fit(Fit,"0","", xmin, xmax);
  Fit->SetLineColor(kMagenta+3);
  Fit->SetLineStyle(2);
  Fit->SetLineWidth(3);
  Fit->Draw("same");

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.22);
  ps->SetY2NDC(.55);
  ps->SetX1NDC(0.52);
  ps->SetX2NDC(0.9);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);
  
  c1->Modified();
  c1->Update();
 
  graph1->Draw("ap");
  gracc->Draw("same,p");
  Fit->Draw("same");

  //c1->Print("Acceptance_E_VD88_VD8990sum_binomial.png");
}



//================================================================
void Poly_Acceptance(int VD1, int VD2){

  bool pzht0 = false;
  // The acceptance is VD2 / VD1
  std::string  E_detparticles_txt_VD1 = "PhotonsDetectedVD"+std::to_string(VD1)+".txt";
  std::string  E_detparticles_txt_VD2 = "PhotonsDetectedVD"+std::to_string(VD2)+".txt";
  std::string ylabel;
  if((VD2==8990)&&(pzht0==false)){ ylabel = "Acceptance (VD=89,90)/(VD="+std::to_string(VD1)+")"; }
  if((VD2==8990)&&(pzht0==true)){
    E_detparticles_txt_VD1 = "PhotonsDetectedVD88_pzht0.txt";
    E_detparticles_txt_VD2 = "PhotonsDetectedVD8990_pzht0.txt";
    ylabel = "Acceptance (VD=89,90)/(VD="+std::to_string(VD1)+"), p_{z}> 0"; }

  else{ ylabel = "Acceptance (VD="+std::to_string(VD2)+")/(VD="+std::to_string(VD1)+")"; }
  
  ReadtxtFile(E_detparticles_txt_VD1, E_detparticles_txt_VD2, 0, 6, 0, 0.006, "E_{#gamma} [MeV]", ylabel, VD1, VD2, pzht0);

}

//================================================================
