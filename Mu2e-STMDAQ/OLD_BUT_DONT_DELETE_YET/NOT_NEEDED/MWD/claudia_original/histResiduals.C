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

void histResiduals() {
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",200,10,700,600);
  c1->SetGrid();

 


  const Int_t n=13;
  Double_t xdata[n],ydata[n],x[n],y[n],ex[n],ey[n];
  TH1F*h1 = new TH1F("TH1F","", 105, -30, 30);//0.57kev bin
  //  TH1F*h1 = new TH1F("TH1F","", 60, -30, 30);  
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

  x[10]=411.1165;
  x[11]=443.965;
  x[12]=867.380;


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

  ex[10]=0;
  ex[11]=0;
  ex[12]=0;


 
  //ADC Counts  CALIBRATION WITH NEW PROGRAM
  y[0]=-1160.05;                                                                                                                              
  y[1]=-71.4;                                                                                                                                    
  y[2]=-214.74;                                                                                                                                     
  y[3]=-426.08 ;                                                                                                                                     
  y[4]=-601.77;                                                                                                                                  
  y[5]=-1368.7;                                                                                                                                  
  y[6]=-1693.55;                                                                                                                                  
  y[7]=-1910.6;                                                                                                                                  
  y[8]=-1954.36 ;                                                                                                                                  
  y[9]=-2475.0;                                                                                                                                  
  y[10]=-720.4;
  y[11]=-777.2;
  y[12]=-1523.7;                                                                                                                                 
  //ADC Count error                                                                                                                              
  ey[0]=0.05 ;                                                                                                                                    
  ey[1]=0.1;                                                                                                                                    
  ey[2]=0.06;                                                                                                                                     
  ey[3]=0.17 ;                                                                                                                                     
  ey[4]=0.08;                                                                                                                                    
  ey[5]=0.2;                                                                                                                                     
  ey[6]=0.15;                                                                                                                                     
  ey[7]=0.3;                                                                                                                                     
  ey[8]=0.18;                                                                                                                                     
  ey[9]=0.3;

  ey[10]=0.6;
  ey[11]=0.3;
  ey[12]=0.3;

  gStyle->SetOptFit(1111);

  //Calibration Fit                                                                                                                                      
  auto ge= new TGraphErrors(n,x,y,ex,ey);

  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", 0, 1500);
  FitCalibrationErrors->SetParameters(-2.81467e-01,-1.74204e+00);
  ge->Fit("FitCalibrationErrors");
 
  Double_t resx[n], resy[n], eresx[n], eresy[n];
     
  for (Int_t i=0;i<n;i++){
    resx[i]=x[i];
    resy[i]=(y[i]-FitCalibrationErrors->Eval(x[i]))/ey[i];
    h1->Fill(resy[i]);
    eresx[i]=0;
    eresy[i]=resy[i]*(ey[i]/y[i]);
    std::cout<<"Data: "<<y[i]<<" x: " <<x[i]<<" Fit: "<<FitCalibrationErrors->Eval(x[i])<<std::endl;
   
 }
  h1->SetTitle("");
  h1->GetXaxis()->SetTitle("Residuals");
  h1->GetYaxis()->SetTitle("");  
  h1->Draw("same");
  c1->cd(); 
 
}
