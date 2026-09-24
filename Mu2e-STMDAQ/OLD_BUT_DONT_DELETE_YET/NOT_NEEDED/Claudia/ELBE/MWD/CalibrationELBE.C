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

void CalibrationELBE() {
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
  double xx1[2]={-100,2000};
  double yy1[2]={-3500, 0};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  const Int_t n=5;
  double E[5]={511,661.7,1173.24,1332.492,1836.07};
  double ADC[5]={-926.3,-1181.31,-2095.93,-2380.15,-3278.7};
  double error_E[5]={0,0,0,0,0};
  double error_ADC[5]={0.6,0.18,0.21,0.24,0.7};

 
  //Calibration Fit
  auto ge= new TGraphErrors(n,E,ADC,error_E,error_ADC);
  ge->SetMarkerStyle(20);
  ge->Draw("p");

  TF1*Fit = new TF1("Fit", "[1]*x+[0]", -100, 2000); 
  Fit->SetParameters(-2.81467e-01,-1.74204e+00);
  ge->Fit(Fit,"0","",-100, 2000);
  Fit->SetLineColor(kRed);
  Fit->Draw("same");

  double Chi2errors;
  Chi2errors=ge->GetFunction("Fit")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;


  //Print everything but the probability
  gStyle->SetOptFit(01111);
  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(ge,"","p");
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  //legend->Draw("same");
  
  pad2->cd();

  double xx2[2]={-100,2000};
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


  Double_t resx[n], resy[n], eresx[n], eresy[n];
     
  for (Int_t i=0;i<n;i++){
    resx[i]=E[i];
    resy[i]=(ADC[i]-Fit->Eval(E[i]))/error_ADC[i];
    //resy[i]=(y[i]-Fit->Eval(x[i]));
    eresx[i]=0;
    eresy[i]=resy[i]*(error_ADC[i]/ADC[i]);
    std::cout<<"ADC: "<<ADC[i]<<" E: " <<E[i]<<" Fit ADC: "<<Fit->Eval(E[i])<<" Res: "<<resy[i]<<" Res error: "<<eresy[i]<<std::endl;
   
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
