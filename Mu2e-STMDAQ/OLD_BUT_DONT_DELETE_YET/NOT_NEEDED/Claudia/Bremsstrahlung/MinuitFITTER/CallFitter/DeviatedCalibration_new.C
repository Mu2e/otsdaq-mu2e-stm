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

void DeviatedCalibration_new() {
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

  
  //double xx1[2]={320,370};
  //double yy1[2]={-850, -400};

  double xx1[2]={344.5, 350};
  double yy1[2]={-616,-600};
  
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
  
  TF1*Lineardependence = new TF1("Lineardependence", "x/(-0.57)", xx1[0]-20, xx1[1]+10);
  Lineardependence->SetLineColor(kBlue);
  Lineardependence->Draw("same,l");
  
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
  fitfuncEval[0]=Lineardependence->Eval(Eeval[0]);
  fitfuncEval[1]=Lineardependence->Eval(Eeval[1]);
  fitfuncEval[2]=Lineardependence->Eval(Eeval[2]);
  fitfuncEval[3]=Lineardependence->Eval(Eeval[3]);
  fitfuncEval[4]=Lineardependence->Eval(Eeval[4]);
  fitfuncEval[5]=Lineardependence->Eval(Eeval[5]);
  fitfuncEval[6]=Lineardependence->Eval(Eeval[6]);
  fitfuncEval[7]=Lineardependence->Eval(Eeval[7]);
  fitfuncEval[8]=Lineardependence->Eval(Eeval[8]);
  
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
  double factor = 0.005;
  double ynew[9],y[9],x[9],ex[9],ey[9];
  
  for(int i =0; i<nnew; i++){
    x[i]=Eeval[i];
    y[i]=fitfuncEval[i];
    ex[i]=0;
    ey[i]=0.004;
    
    if(x[i]<347){
      ynew[i]=y[i]+(factor*x[i]);
      diff = ynew[i]-y[i];
      std::cout<<"Energy: "<<x[i]<<" Difference in data gen new-old= "<<diff<<std::endl;
    }
    else if(x[i]==347){
      ynew[i]=y[i];
      diff = ynew[i]-y[i];
      std::cout<<"Energy: "<<x[i]<<" Difference in data gen new-old= "<<diff<<std::endl;

    }
    else{
      ynew[i]=y[i]-(factor*x[i]);
      diff = ynew[i]-y[i];
      std::cout<<"Energy: "<<x[i]<<" Difference in data gen new-old= "<<diff<<std::endl;
    }
  }


  auto genew= new TGraphErrors(nnew,x,ynew,ex,ey);
  genew->SetMarkerStyle(20);
  genew->Print();
  //genew->Draw("same,p");


  TF1*FitCalibrationErrorsnew = new TF1("FitCalibrationErrorsnew", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", xx1[0]-20, xx1[1]+10);
  FitCalibrationErrorsnew->SetParameters(0.05726,-1.736,-4.265E-5,1.967E-8,0,0);
  genew->Fit(FitCalibrationErrorsnew,"0","",320,370);
  FitCalibrationErrorsnew->SetLineColor(kBlue);
  FitCalibrationErrorsnew->SetLineStyle(2);
  FitCalibrationErrorsnew->Draw("same,l");


  auto legend = new TLegend(0.4,0.65,0.9,0.9);
  legend->AddEntry("Lineardependence","ADC=E/(-0.57)","l");
  legend->AddEntry("FitCalibrationErrorsnew","New Calibration around 347keV","l"); 
  legend->Draw("same");
 
  
  pad2->cd();
 
  double xx2[2]={xx1[0],xx1[1]};
  double yy2[2]={-5, 5};
  TGraph *graph2 = new TGraph (2,xx2,yy2);
  graph2->GetXaxis()->SetRangeUser(xx2[0],xx2[1]);
  graph2->GetYaxis()->SetRangeUser(yy2[0],yy2[1]);
  graph2->SetTitle("");
  graph2->GetYaxis()->SetTitle("ADC New-Old");
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

  Double_t diffy[nnew], ediffy[nnew];
     
  for (Int_t i=0;i<nnew;i++){
    diffy[i]= FitCalibrationErrorsnew->Eval(x[i]) - Lineardependence->Eval(x[i]);
    ediffy[i]=0;

    std::cout<<"Energy: "<<x[i]<<"keV----"<<std::endl;
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
  //c1->Print("NewCalibrationAround347keV_factor-0.005.png","png");

}
