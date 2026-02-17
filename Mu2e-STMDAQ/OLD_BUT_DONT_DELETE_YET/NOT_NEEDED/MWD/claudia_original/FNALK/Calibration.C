//Energy ADC counts calibration
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

void Calibration() {
  gROOT->SetStyle("ATLAS");

  auto c1= new TCanvas("c1");   
  c1->SetGrid();

  TPad *pad1= new TPad("pad1", "pad1",0,0.33,1,1);
  TPad *pad2= new TPad("pad2", "pad2",0,0,1,0.33);

  pad1->SetBottomMargin(0.1);
  pad1->SetGrid();
  pad1->SetBorderMode(0);
  pad2->SetTopMargin(0.1);
  pad2->SetBottomMargin(0.1);
  pad2->SetBorderMode(0);
  pad2->SetGrid();
  pad1->Draw();
  pad2->Draw();


  pad1->cd();
  double xx1[2]={-100,3000};
  double yy1[2]={-7500, 0};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  const Int_t n=3;

  
  //Energies
  Double_t x[3]={511, 1460.8, 2614.51};
  Double_t ex[3]={0,0,0};

  //M1000L500 Liverpool calibration (root file generated with MWDROOT) 
  //ADC
  Double_t y[3]={-1624.92, -3892.31, -6965.78};
  Double_t ey[3]={10,10,10};
  
  
  //Calibration Fit
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->Draw("p");

  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", -100, 3000);
  FitCalibrationErrors->SetParameters(-2.54344e+00,-2.72671e+02);
  ge->Fit(FitCalibrationErrors,"0","",-100,3000);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");

  double Chi2errors;
  Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;


  //Print everything but the probability
  //gStyle->SetOptFit(01111);
  //Print just Fit parameters
  gStyle->SetOptFit(00111);
  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(ge,"","p");
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  //legend->Draw("same");
  
  pad2->cd();

  double xx2[2]={-100,3000};
  double yy2[2]={-30, 30};
  TGraph *graph2 = new TGraph (2,xx2,yy2);
  graph2->GetXaxis()->SetRangeUser(xx2[0],xx2[1]);
  graph2->GetYaxis()->SetRangeUser(yy2[0],yy2[1]);
  graph2->SetTitle("");
  //graph2->GetXaxis()->SetTitle("E [keV]"); 
  graph2->GetYaxis()->SetTitle("Residuals");
  graph2->GetXaxis()->SetTitleOffset(0.9);
  graph2->SetMarkerStyle(1);
  graph2->GetYaxis()->SetTitleOffset(0.71);
  graph2->GetYaxis()->SetLabelSize(0.1);
  graph2->GetYaxis()->SetTitleSize(0.1);
  graph2->GetXaxis()->SetLabelSize(0.1);


  graph2->Draw("ap");

  double total_chi2 = 0;

  Double_t resx[n], resy[n], eresx[n], eresy[n];
     
  for (Int_t i=0;i<n;i++){
    resx[i]=x[i];
    resy[i]=(y[i]-FitCalibrationErrors->Eval(x[i]))/ey[i];
    //resy[i]=(y[i]-FitCalibrationErrors->Eval(x[i]));
    eresx[i]=0;
    eresy[i]=resy[i]*(ey[i]/y[i]);
    total_chi2=total_chi2+resy[i]*resy[i];
    std::cout<<"Data: "<<y[i]<<" x: " <<x[i]<<" Fit: "<<FitCalibrationErrors->Eval(x[i])<<" chi2: "<<resy[i]<<std::endl;
    std::cout<<"Total chi2 calculated: "<<total_chi2<<std::endl;
 }
  auto gresid= new TGraphErrors(n,resx,resy,eresx,eresy); 
 
  gresid->SetMarkerStyle(20);
  gresid->SetMarkerColor(kGreen+3);
  gresid->GetYaxis()->SetTitle("Residuals");
  gresid->SetTitle("");
  gresid->Draw("p");
 


  c1->cd(); 
  //c1->Print("CalibrationE-ADCFitError-newprogram-residuals_newmidas.pdf","pdf");
  //c1->Print("CalibrationE-ADCFitError-newprogram-residuals_newmidas.png","png");

}
