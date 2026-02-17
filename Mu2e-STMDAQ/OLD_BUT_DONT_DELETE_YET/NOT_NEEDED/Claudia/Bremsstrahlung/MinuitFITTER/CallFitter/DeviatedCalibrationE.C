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
#include "TGaxis.h"

void DeviatedCalibrationE() {
  gROOT->SetStyle("ATLAS");

  TCanvas* c1= new TCanvas("c1");   
 
 
  TPad *pad1= new TPad("pad1", "pad1",0,0.33,1,1);
  TPad *pad2= new TPad("pad2", "pad2",0,0,1,0.38);
    
  pad1->SetBottomMargin(0.1);
  pad1->SetGrid();
  pad1->SetBorderMode(0);
  pad2->SetTopMargin(0.1);
  pad2->SetBottomMargin(0.1);
  pad2->SetBottomMargin(0.3);
  pad2->SetBorderMode(0);
  pad2->SetGrid();
  pad1->Draw();
  pad2->Draw();
  
  pad1->cd();


  double yy1[2]={344, 350};
  double xx1[2]={-614,-604};
  
  //double xx1[2]={-3500,0};
  //double yy1[2]={40,2000};
  
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetYaxis()->SetTitle("E [keV]");
  graph1->GetXaxis()->SetTitle("");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->SetMaximum(yy1[1]);
  graph1->SetMinimum(yy1[0]);
  graph1->GetXaxis()->SetLabelSize(0);
  graph1->GetYaxis()->SetLabelSize(0.05);
  graph1->GetYaxis()->SetTitleSize(0.05);
  
  TAxis *Yax = graph1->GetYaxis();
  Yax->SetNdivisions(5,3,0);
  graph1->Draw("ap");
  c1->Modified();
  c1->Update();
  
  TF1*Lineardependence = new TF1("Lineardependence", "(-0.57)*x", xx1[0]-20, xx1[1]+10);
  Lineardependence->SetLineColor(kBlue);
  Lineardependence->Draw("same,l");

  TF1*Lineardependence_getADC = new TF1("Lineardependence_getADC", "x/(-0.57)", yy1[0]-20, yy1[1]+10);
  
  double Eeval[9];
  Eeval[0]=320;
  Eeval[1]=345;
  Eeval[2]=345.5;
  Eeval[3]=346;
  Eeval[4]=347;
  Eeval[5]=348;
  Eeval[6]=348.5;
  Eeval[7]=349;
  Eeval[8]=370;
  
  double fitfuncEval[9];
  fitfuncEval[0]=Lineardependence_getADC->Eval(Eeval[0]);
  fitfuncEval[1]=Lineardependence_getADC->Eval(Eeval[1]);
  fitfuncEval[2]=Lineardependence_getADC->Eval(Eeval[2]);
  fitfuncEval[3]=Lineardependence_getADC->Eval(Eeval[3]);
  fitfuncEval[4]=Lineardependence_getADC->Eval(Eeval[4]);
  fitfuncEval[5]=Lineardependence_getADC->Eval(Eeval[5]);
  fitfuncEval[6]=Lineardependence_getADC->Eval(Eeval[6]);
  fitfuncEval[7]=Lineardependence_getADC->Eval(Eeval[7]);
  fitfuncEval[8]=Lineardependence_getADC->Eval(Eeval[8]);
  
  std::cout<<"E= "<<Eeval[0]<<"keV, fit ADC= "<<fitfuncEval[0]<<std::endl;
  std::cout<<"E= "<<Eeval[1]<<"keV, fit ADC= "<<fitfuncEval[1]<<std::endl;
  std::cout<<"E= "<<Eeval[2]<<"keV, fit ADC= "<<fitfuncEval[2]<<std::endl;
  std::cout<<"E= "<<Eeval[3]<<"keV, fit ADC= "<<fitfuncEval[3]<<std::endl;
  std::cout<<"E= "<<Eeval[4]<<"keV, fit ADC= "<<fitfuncEval[4]<<std::endl;
  std::cout<<"E= "<<Eeval[5]<<"keV, fit ADC= "<<fitfuncEval[5]<<std::endl;
  std::cout<<"E= "<<Eeval[6]<<"keV, fit ADC= "<<fitfuncEval[6]<<std::endl;
  std::cout<<"E= "<<Eeval[7]<<"keV, fit ADC= "<<fitfuncEval[7]<<std::endl;
  std::cout<<"E= "<<Eeval[8]<<"keV, fit ADC= "<<fitfuncEval[8]<<std::endl;
  
  double diff;
  int nnew = 9;
  double factor = 200;
  double ynew[9],y[9],x[9],ex[9],ey[9];

  //double fixedpoint = 347;
  //double fixedpoint = 349;
  double fixedpoint = 345;
  //double fixedpoint = 370;
  //double fixedpoint = 320;
  
  for(int i =0; i<nnew; i++){
    y[i]=Eeval[i];
    x[i]=fitfuncEval[i];
    ex[i]=0;
    ey[i]=0.004;
    
    if(y[i]<fixedpoint){
      ynew[i]=y[i]-(factor/y[i]);
      diff = ynew[i]-y[i];
      std::cout<<"Energy: "<<y[i]<<" Difference in data gen new-old= "<<diff<<std::endl;
    }
    else if(y[i]==fixedpoint){
      ynew[i]=y[i];
      diff = ynew[i]-y[i];
      std::cout<<"Energy: "<<y[i]<<" Difference in data gen new-old= "<<diff<<std::endl;

    }
    else{
      ynew[i]=y[i]+(factor/y[i]);
      diff = ynew[i]-y[i];
      std::cout<<"Energy: "<<y[i]<<" Difference in data gen new-old= "<<diff<<std::endl;
    }
  }


  auto genew= new TGraphErrors(nnew,x,ynew,ex,ey);
  genew->SetMarkerStyle(20);
  genew->Print();
  //genew->Draw("same,p");


  double xfit_low =-614;
  double xfit_max =-604;
  
  TF1*FitCalibrationErrorsnew = new TF1("FitCalibrationErrorsnew", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", xx1[0]-20, xx1[1]+10);
  FitCalibrationErrorsnew->SetParameters(0.05726,-1.736,-4.265E-5,1.967E-8,0,0);
  genew->Fit(FitCalibrationErrorsnew,"0","",xfit_low, xfit_max);
  FitCalibrationErrorsnew->SetLineColor(kRed-3);
  FitCalibrationErrorsnew->SetLineStyle(2);
  FitCalibrationErrorsnew->Draw("same,l");


  auto legend = new TLegend(0.5,0.7,0.9,0.9);
  legend->AddEntry("Lineardependence","E=-0.57ADC","l");
  legend->AddEntry("FitCalibrationErrorsnew","New Signal Calibration","l"); 
  legend->Draw("same");
 
  
  pad2->cd();
 
  double xx2[2]={xx1[0],xx1[1]};
  double yy2[2]={-2, 2};
  TGraph *graph2 = new TGraph (2,xx2,yy2);
  graph2->GetXaxis()->SetRangeUser(xx2[0],xx2[1]);
  graph2->GetYaxis()->SetRangeUser(yy2[0],yy2[1]);
  graph2->SetTitle("");
  graph2->GetYaxis()->SetTitle("#DeltaE [keV]");
  graph2->GetXaxis()->SetTitle("ADC Counts");
  graph2->GetXaxis()->SetTitleOffset(0.9);
  graph2->SetMarkerStyle(1);
  graph2->GetXaxis()->SetTitleOffset(1.2);
  graph2->GetXaxis()->SetLabelOffset(0.035);
  graph2->SetMarkerStyle(1);
  graph2->GetYaxis()->SetTitleOffset(0.7);
  graph2->GetYaxis()->SetLabelSize(0.1);
  graph2->GetXaxis()->SetLabelSize(0.1);
  graph2->GetYaxis()->SetTitleSize(0.1);
  graph2->GetXaxis()->SetTitleSize(0.1);
  graph2->GetXaxis()->SetTickLength(0.06);
  
  graph2->Draw("ap");

  TAxis *Y = graph2->GetYaxis();
  Y->SetNdivisions(5,3,0);

  graph2->Draw("ap");
  c1->Modified();
  c1->Update();

  Double_t diffy[nnew], ediffy[nnew];
     
  for (Int_t i=0;i<nnew;i++){
    diffy[i]= FitCalibrationErrorsnew->Eval(x[i]) - Lineardependence->Eval(x[i]);
    ediffy[i]=0;

    std::cout<<"Energy: "<<y[i]<<"keV----ADC: "<<x[i]<<std::endl;
    std::cout<<"    ADC Linear calibration: "<<Lineardependence->Eval(x[i])<<std::endl;
    std::cout<<"    ADC New calibration: "<<FitCalibrationErrorsnew->Eval(x[i])<<std::endl;
    std::cout<<"    Difference between the 2 fits: "<<diffy[i]<<std::endl;
  }

  
  auto gdiffcalib= new TGraphErrors(nnew,x,diffy,ex,ediffy); 
 
  gdiffcalib->SetMarkerStyle(20);
  gdiffcalib->SetMarkerColor(kViolet+3);
  gdiffcalib->SetTitle("");
  gdiffcalib->Draw("same,p");
 

  c1->cd(); 
 
  //c1->Print("CalibrationLiverpoolM1000L500_goodrootfile_grade5poly.pdf","pdf");
  c1->Print("NewCalibrationAround345keV_facto200.png","png");

}
