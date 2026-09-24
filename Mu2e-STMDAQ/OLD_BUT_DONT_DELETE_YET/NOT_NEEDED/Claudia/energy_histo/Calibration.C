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
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",200,10,700,600);
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
  double xx1[2]={-100,1500};
  double yy1[2]={-1200, 0};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(-100,1500);
  graph1->GetYaxis()->SetRangeUser(-3000, 0);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->Draw("ap");

  const Int_t n=13;
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
  //ADC Counts  CALIBRATION WITHOUT EU BACKGROUND SUPPRESSION
  /*y[0]=-1097.31;
  
   y[1]=-61.9;
  y[2]=-194.6;
  y[3]=-392.7;
  y[4]=-561.63;

  y[5]=-1288.0;
  y[6]=-1596.1;
  y[7]=-1801.5;
  y[8]=-1841.9;
  y[9]=-2337.7;
  
  //ADC Count error
  ey[0]=0.09;
  ey[1]=0.2;
  ey[2]=0.3;
  ey[3]=0.6;
  ey[4]=0.24;
  ey[5]=0.6;
  ey[6]=0.4;
  ey[7]=0.6;
  ey[8]=0.9;
  ey[9]=0.3;*/



  //ADC Counts  CALIBRATION WITH EU BACKGROUND SUPPRESSION                                                                        
  /* y[0]=-1097.31;                                                                                                                   
  y[1]=-60.4;                                                                                                                       
  y[2]=-195;                                                                                                                      
  y[3]=-395;                                                                                                                      
  y[4]=-561.12;                                                                                                                  
  y[5]=-1287.6;                                                                                                                     
  y[6]=-1596.4;                                                                                                                     
  y[7]=-1802.3;                                                                                                                     
  y[8]=-1846.2;                                                                                                                     
  y[9]=-2337.2;                                                                                                                      
                                                                                                                                    
  //ADC Count error                                                                                                                 
  ey[0]=0.09;                                                                                                                       
  ey[1]=0.15;                                                                                                                       
  ey[2]=0.2;                                                                                                                        
  ey[3]=0.4;                                                                                                                        
  ey[4]=0.18;                                                                                                                       
  ey[5]=0.4;                                                                                                                        
  ey[6]=0.3;                                                                                                                        
  ey[7]=0.5;
  ey[8]=0.3;
  ey[9]=0.3;*/ 

  //ADC Counts  CALIBRATION WITH NEW PROGRAM
  y[0]=-1151.50;                                                                                                                              
  y[1]=-73.17;                                                                                                                                    
  y[2]=-213.46;                                                                                                                                     
  y[3]=-423.28 ;                                                                                                                                     
  y[4]=-597.02;                                                                                                                                  
  y[5]=-1357.99;                                                                                                                                  
  y[6]=-1681.9;                                                                                                                                  
  y[7]=-1896.45;                                                                                                                                  
  y[8]=-1939.9 ;                                                                                                                                  
  y[9]=-2457.13;                                                                                                                                  
  y[10]=-715.9;
  y[11]=-772.3;
  y[12]=-1514.3;                                                                                                                                 
  //ADC Count error                                                                                                                              
  ey[0]=0.05 ;                                                                                                                                    
  ey[1]=0.11;                                                                                                                                    
  ey[2]=0.05;                                                                                                                                     
  ey[3]=0.19 ;                                                                                                                                     
  ey[4]=0.09;                                                                                                                                    
  ey[5]=0.18;                                                                                                                                     
  ey[6]=0.2;                                                                                                                                     
  ey[7]=0.23;                                                                                                                                     
  ey[8]=0.3;                                                                                                                                     
  ey[9]=0.14;

  ey[10]=0.7;
  ey[11]=0.4;
  ey[12]=0.7;
  TGraph* gr = new TGraph();
  for(int i=0;i<n;i++){
    gr->SetPoint(i,x[i],y[i]) ;
  }

  //gr->SetLineColor(kBlack);
  gr->SetMarkerStyle(20);
  gr->Draw("p");

  //Calibration Fit
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->Draw("p");
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", 0, 1500);
  FitCalibrationErrors->SetParameters(-2.81467e-01,-1.74204e+00);
  ge->Fit("FitCalibrationErrors");
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");

  double Chi2errors;
  Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;


  gStyle->SetOptFit(1111);

 


  pad2->cd();
  Double_t resx[n], resy[n], eresx[n], eresy[n];
     
  for (Int_t i=0;i<n;i++){
    resx[i]=x[i];
    resy[i]=(y[i]-FitCalibrationErrors->Eval(x[i]))/ey[i];
    //resy[i]=(y[i]-FitCalibrationErrors->Eval(x[i]));
    eresx[i]=0;
    eresy[i]=resy[i]*(ey[i]/y[i]);
    std::cout<<"Data: "<<y[i]<<" x: " <<x[i]<<" Fit: "<<FitCalibrationErrors->Eval(x[i])<<std::endl;
   
 }
  auto gresid= new TGraphErrors(n,resx,resy,eresx,eresy); 
 
  gresid->SetMarkerStyle(5);
  gresid->GetYaxis()->SetTitle("Residuals");
  gresid->SetTitle("");
  gresid->GetXaxis()->SetLabelSize(0.07);
   gresid->GetYaxis()->SetLabelSize(0.07);
  //gresid->GetXaxis()->SetLabelFont(80);                                                                                                              
  gresid->GetYaxis()->SetTitleSize(0.065);  
   gresid->Draw("ap");
 

  c1->cd(); 
  //c1->Print("CalibrationE-ADCFitError-newprogram-residuals1.pdf","pdf");
  //c1->Print("CalibrationE-ADCFitError-newprogram-residuals1.png","png");

}
