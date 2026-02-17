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

void Resolution() {
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",200,10,700,600);
  c1->SetGrid();


  double xx1[2]={0,1500};
  double yy1[2]={0, 6};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,1500);
  graph1->GetYaxis()->SetRangeUser(0, 6);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("Resolution (keV)");
  graph1->Draw("ap");

  const Int_t n=10;
  Double_t xdata[n],ydata[n],x[n],y[n],ex[n],ey[n];

  //Energies
  x[0]=661.7;
 
  x[1]=40.1186;
  x[2]=121.78;
  x[3]=244.7;
  x[4]=344.28;
  x[5]=778.91;
  x[6]=964.08;
  x[7]=1085.837;
  x[8]=1112.076;
  x[9]=1408.013;

  //  x[10]=411.1165;
  //x[11]=443.965;
  //x[12]=867.380;

  
  ex[0]=0;
  ex[1]=0;
  ex[2]=0;
  ex[3]=0;
  ex[4]=0;
  ex[5]=0;
  ex[6]=0;
  ex[7]=0;
  ex[8]=0;
  ex[9]=0;

  //ex[10]=0;
  //ex[11]=0;
  //ex[12]=0;

  //Resolution  CALIBRATION WITH NEW PROGRAM
  y[0]=1.97;                                                                                                                              
  y[1]=5.28;                                                                                                                                    
  y[2]=2.35;                                                                                                                                     
  y[3]=3.6;                                                                                                                                     
  y[4]=2.4;                                                                                                                                  
  y[5]=2.92;                                                                                                                                  
  y[6]=2.3;
  y[7]=2.8;
  y[8]=2.3;
  y[9]=2.20;
  //Resolution  error                                                                                                                              
  ey[0]=0.022 ;                                                                                                                                    
  ey[1]=0.07;                                                                                                                                    
  ey[2]=0.03;                                                                                                                                     
  ey[3]=0.3 ;                                                                                                                                     
  ey[4]=0.05;                                                                                                                                    
  ey[5]=0.12;                                                                                                                                     
  ey[6]=0.07;
  ey[7]=0.1;
  ey[8]=0.1;
  ey[9]=0.06 ;
  /*  TGraph* gr = new TGraph();
  for(int i=0;i<n;i++){
    gr->SetPoint(i,x[i],y[i]) ;
  }

  //gr->SetLineColor(kBlack);
  gr->SetMarkerStyle(20);
  gr->Draw("p");
  */
  //Calibration Fit
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->Draw("p");
  /*TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", 0, 1500);
  FitCalibrationErrors->SetParameters(-2.81467e-01,-1.74204e+00);
  ge->Fit("FitCalibrationErrors");
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");

  double Chi2errors;
  Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;
  */

  gStyle->SetOptFit(1111);

 


  c1->cd(); 


}
