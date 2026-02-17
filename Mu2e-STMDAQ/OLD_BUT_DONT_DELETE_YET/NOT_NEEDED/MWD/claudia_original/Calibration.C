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

  TCanvas* c1= new TCanvas("c1");   
 
 
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


  double xx1[2]={-100,2000};
  double yy1[2]={-3500, 0};
  
  //double xx1[2]={0,200};
  //double yy1[2]={-300, 0};
  
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->SetMaximum(yy1[1]);
  graph1->SetMinimum(yy1[0]);
  graph1->Draw("ap");
  
  const Int_t n=13;

  
  //Energies
  Double_t x[13]={661.659,40.1186,121.7817,244.6974,344.2785,411.1165,443.965,778.91,867.380,964.08,1085.837,1112.076,1408.013};
  Double_t ex[13]={0.003,0.0001,0.0003,0.0008,0.0012,0.0012,0.003,0.0024,0.003,0.018,0.010,0.003,0.003};
  //Double_t ex[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  /*//M8000L1000 Liverpool calibration (old cali bad root file generated with test_new) 
  //ADC
  Double_t y[13]={-1160.05,-71.4,-214.74,-426.08,-601.77,-720.4,-777.2,-1368.7,-1523.7,-1693.55,-1910.6,-1954.36,-2475.0};
  Double_t ey[13]={0.05,0.1,0.06,0.17,0.08,0.6,0.3,0.2,0.3,0.15,0.3,0.18,0.3};
  */

  /*//M400L200 Liverpool calibration  
  //ADC
  Double_t y[13]={-1167.24,-70.96,-212.92,-432.1,-605.36,-725.7,-782.64,-1375.17,-1530.8,-1703.22,-1919.6,-1965.64,-2484.25};
  Double_t ey[13]={0.04,0.06,0.04,1.8,0.06,0.4,0.22,0.15,0.9,0.17,0.3,0.15,0.17};
  */

  /* //M8000L1000 Liverpool calibration (root file generated with MWDROOT)
  //ADC
  Double_t y[13]={-1161.98,-75.56,-217.05,-432.0,-604.65,-722.6,-780.4,-1370.42,-1526.4,-1696.05,-1911.62,-1956.32,-2475.78};
  Double_t ey[13]={0.04,0.06,0.05,2.5,0.07,0.6,0.3,0.15,1.3,0.16,0.23,0.17,0.21};
  */

  //M1000L500 Liverpool calibration (root file generated with MWDROOT) 
  //ADC
  Double_t y[13]={-1161.64,-70.01,-211.75,-426.91,-602.16,-720.7,-778.7,-1368.78,-1525.7,-1695.5,-1910.53,-1956.92,-2472.68};
  Double_t ey[13]={0.03,0.03,0.03,0.11,0.04,0.3,0.3,0.1,0.4,0.11,0.23,0.11,0.13};
  
  //Calibration Fit
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->Draw("same,p");
  
   
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", -100, 2000);
  FitCalibrationErrors->SetParameters(-2.81467e-01,-1.74204e+00);
  ge->Fit(FitCalibrationErrors,"0","",-100,2000);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");

  /*
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[3]*x*x*x+[2]*x*x+[1]*x+[0]", -100, 2000);
  FitCalibrationErrors->SetParameters(0.05726,-1.736,-4.265E-5,1.967E-8);
  ge->Fit(FitCalibrationErrors,"0","",-100,2000);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");
*/  
  /*
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[0]/(1+exp((x/[1])+[2]))+[3]", -100, 2000);
  FitCalibrationErrors->SetParameters(-19239.2, -2721.81, 0.265479, 8350.2); 
  //FitCalibrationErrors->SetParameters(-19239.2, -2721.81, 3, 500);
  ge->Fit(FitCalibrationErrors,"0","",-100,2000);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same, l");
  */
  /*
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", -100, 2000);
  //FitCalibrationErrors->SetParameters(0.05726,-1.736,-4.265E-5,1.967E-8,0,0);
  FitCalibrationErrors->SetParameters(-0.230653, -1.73361, -4.80243e-05, 2.27423e-08,-1e-10,-1e-11 );
  ge->Fit(FitCalibrationErrors,"0","",-100,2000);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");
  */
  /* 
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[0]*sin([1]*x)*sin([1]*x)*([2]*x+[3])+[4]*x+[5]", -100, 2000);
  FitCalibrationErrors->SetParameters(-5,0.005,0.1,1,-1.5,2);
  ge->Fit(FitCalibrationErrors,"0","",-100,2000);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");
  */

  double Chi2errors;
  Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;
 
  //Print everything but the probability
  gStyle->SetOptFit(01111);
  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(ge,"","p");
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  //legend->Draw("same");


  gPad->Update();
  TPaveStats* ps = (TPaveStats *)ge->FindObject("stats");
  ps->SetY1NDC( 0.55 );
  ps->SetY2NDC( 0.91 );
  ps->SetX1NDC( 0.58 ); //new x start position
  ps->SetX2NDC( 0.91 );
  ps->SetLineWidth( 0 );
  //ps->SetLineColor( kWhite );
  
  c1->Modified();
  c1->Update();
    
  ge->Draw("same,p");
  FitCalibrationErrors->Draw("same, l"); 
 
  pad2->cd();

  double xx2[2]={-100,2000};
  double yy2[2]={-50, 50};
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

  TAxis *Y = graph2->GetYaxis();
  Y->SetNdivisions(5,3,0);

  graph2->Draw("ap");
  c1->Modified();
  c1->Update();
  double chi2 = 0;

  Double_t resx[n], resy[n], eresx[n], eresy[n];
     
  for (Int_t i=0;i<n;i++){
    resx[i]=x[i];
    resy[i]=(y[i]-FitCalibrationErrors->Eval(x[i]))/ey[i];
    eresx[i]=0;
    eresy[i]=resy[i]*(ey[i]/y[i]);
    std::cout<<"Data: "<<y[i]<<" x: " <<x[i]<<" Fit: "<<FitCalibrationErrors->Eval(x[i])<<std::endl;
    
    chi2 = chi2 + resy[i]*resy[i];  
 }
  std::cout<<"check chi2: "<<chi2<<std::endl;
    
  auto gresid= new TGraphErrors(n,resx,resy,eresx,eresy); 
 
  gresid->SetMarkerStyle(20);
  gresid->SetMarkerColor(kGreen+3);
  gresid->GetYaxis()->SetTitle("Residuals");
  gresid->SetTitle("");
  gresid->Draw("same,p");
 

  c1->cd();

  c1->Print("CalibrationLiverpoolM1000L500_goodrootfile_newbinning_Eerrors.pdf","pdf");
  c1->Print("CalibrationLiverpoolM1000L500_goodrootfile_newbinning_Eerrors.png","png");

}
