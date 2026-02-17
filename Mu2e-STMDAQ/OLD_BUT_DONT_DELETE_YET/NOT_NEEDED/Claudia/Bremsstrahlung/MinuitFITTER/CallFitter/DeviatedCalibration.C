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

void DeviatedCalibration() {
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
  double yy1[2]={-613,-600};
  
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
  Double_t x[13]={661.7,40.1186,121.78,244.7,344.28,411.1165,443.965,778.91,867.380,964.08,1085.837,1112.076,1408.013};
  Double_t ex[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

  //ADC
  Double_t y[13]={-1161.64,-70.01,-211.75,-426.91,-602.16,-720.7,-778.7,-1368.78,-1525.7,-1695.5,-1910.53,-1956.92,-2472.68};
  Double_t ey[13]={0.03,0.03,0.03,0.11,0.04,0.3,0.3,0.1,0.4,0.11,0.23,0.11,0.13};
  
  //Calibration Fit
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  ge->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  ge->SetMarkerStyle(20);
  ge->Print();
  
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", -100,2000);
  FitCalibrationErrors->SetParameters(0.05726,-1.736,-4.265E-5,1.967E-8,0,0);
  ge->Fit(FitCalibrationErrors,"0","",-100,2000);

  TF1*FitCalibrationErrorsPrint = new TF1("FitCalibrationErrorsPrint", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", xx1[0]-20, xx1[1]+10);
  double p0=FitCalibrationErrors->GetParameter(0);
  double p1=FitCalibrationErrors->GetParameter(1);
  double p2=FitCalibrationErrors->GetParameter(2);
  double p3=FitCalibrationErrors->GetParameter(3);
  double p4=FitCalibrationErrors->GetParameter(4);
  double p5=FitCalibrationErrors->GetParameter(5);
  FitCalibrationErrorsPrint->SetParameters(p0,p1,p2,p3,p4,p5);
  FitCalibrationErrorsPrint->SetLineColor(kRed);
  //FitCalibrationErrorsPrint->Draw("same,l"); 
  
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
  fitfuncEval[0]=FitCalibrationErrors->Eval(Eeval[0]);
  fitfuncEval[1]=FitCalibrationErrors->Eval(Eeval[1]);
  fitfuncEval[2]=FitCalibrationErrors->Eval(Eeval[2]);
  fitfuncEval[3]=FitCalibrationErrors->Eval(Eeval[3]);
  fitfuncEval[4]=FitCalibrationErrors->Eval(Eeval[4]);
  fitfuncEval[5]=FitCalibrationErrors->Eval(Eeval[5]);
  fitfuncEval[6]=FitCalibrationErrors->Eval(Eeval[6]);
  fitfuncEval[7]=FitCalibrationErrors->Eval(Eeval[7]);
  fitfuncEval[8]=FitCalibrationErrors->Eval(Eeval[8]);
  
  std::cout<<"E= "<<Eeval[0]<<"keV, fit ADC= "<<fitfuncEval[0]<<std::endl;
  std::cout<<"E= "<<Eeval[1]<<"keV, fit ADC= "<<fitfuncEval[1]<<std::endl;
  std::cout<<"E= "<<Eeval[2]<<"keV, fit ADC= "<<fitfuncEval[2]<<std::endl;
  std::cout<<"E= "<<Eeval[3]<<"keV, fit ADC= "<<fitfuncEval[3]<<std::endl;
  std::cout<<"E= "<<Eeval[4]<<"keV, fit ADC= "<<fitfuncEval[4]<<std::endl;
  std::cout<<"E= "<<Eeval[5]<<"keV, fit ADC= "<<fitfuncEval[5]<<std::endl;
  std::cout<<"E= "<<Eeval[6]<<"keV, fit ADC= "<<fitfuncEval[6]<<std::endl;
  std::cout<<"E= "<<Eeval[7]<<"keV, fit ADC= "<<fitfuncEval[7]<<std::endl;
  std::cout<<"E= "<<Eeval[8]<<"keV, fit ADC= "<<fitfuncEval[8]<<std::endl;

  
  double Chi2errors;
  Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;

  int nnew=22;
  
  double xnew[22];
  double ynew[22];
  double exnew[22];
  double eynew[22];
  int newp=0;
  double yold[22];
  
  for(int i =0; i<nnew; i++){
    if(i<n){
      xnew[i]=x[i];
      ynew[i]=y[i];
      exnew[i]=ex[i];
      eynew[i]=ey[i];
    }
    else{
       xnew[i]=Eeval[newp];
       ynew[i]=fitfuncEval[newp];
       exnew[i]=0;
       eynew[i]=0.04;
       newp++;
    }
  }


  auto ge_addpoints= new TGraphErrors(nnew,xnew,ynew,exnew,eynew);
  ge_addpoints->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  ge_addpoints->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  ge_addpoints->SetMarkerStyle(20);
  //ge_addpoints->Draw("same,p");

  //Move the last added points
  
  double diff;

  double factor = 0.001;
  
  for(int i =0; i<nnew; i++){
    if(xnew[i]<347){
      yold[i]=ynew[i];
      ynew[i]=ynew[i]+(factor*xnew[i]);
      diff = ynew[i]-yold[i];
      std::cout<<"Energy: "<<xnew[i]<<" Difference in data gen new-old= "<<diff<<std::endl;
    }
    else if(xnew[i]==347){
      yold[i]=ynew[i];
      ynew[i]=ynew[i];
      diff = ynew[i]+yold[i];
      std::cout<<"Energy: "<<xnew[i]<<" Difference in data gen new-old= "<<diff<<std::endl;

    }
    else{
      yold[i]=ynew[i];
      ynew[i]=ynew[i]-(factor*xnew[i]);
      diff = ynew[i]-yold[i];
      std::cout<<"Energy: "<<xnew[i]<<" Difference in data gen new-old= "<<diff<<std::endl;
    }
  }


  auto genew= new TGraphErrors(nnew,xnew,ynew,exnew,eynew);
  genew->SetMarkerStyle(20);
  genew->Print();
  //genew->Draw("same,p");


  TF1*FitCalibrationErrorsnew = new TF1("FitCalibrationErrorsnew", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", xx1[0]-20, xx1[1]+10);
  //FitCalibrationErrorsnew->SetParameters(-2649.38, -0.0947618, 0.0179267, 5.12351e-05, 3.48999e-08, -5.42572e-10  );
  FitCalibrationErrorsnew->SetParameters(0.05726,-1.736,-4.265E-5,1.967E-8,0,0);
  genew->Fit(FitCalibrationErrorsnew,"0","",320,370);
  FitCalibrationErrorsnew->SetLineColor(kBlue);
  FitCalibrationErrorsnew->SetLineStyle(2);
  FitCalibrationErrorsnew->Draw("same,l");


   TF1*Lineardependence = new TF1("Lineardependence", "x/(-0.57)", xx1[0]-20, xx1[1]+10);
  Lineardependence->SetLineColor(kBlue);
  Lineardependence->Draw("same,l");
  
  auto legend = new TLegend(0.4,0.65,0.9,0.9);
  //legend->AddEntry("FitCalibrationErrorsPrint","Calibration","l");
  legend->AddEntry("Lineardependence","ADC=E/(-0.57)","l");
  legend->AddEntry("FitCalibrationErrorsnew","New Signal Calibration","l");
  legend->Draw("same");
   
  //Print everything but the probability
  //gStyle->SetOptFit(01111);

  
  pad2->cd();

  double xx2[2]={xx1[0],xx1[1]};
  double yy2[2]={-5, 5};
  TGraph *graph2 = new TGraph (2,xx2,yy2);
  graph2->GetXaxis()->SetRangeUser(xx2[0],xx2[1]);
  graph2->GetYaxis()->SetRangeUser(yy2[0],yy2[1]);
  graph2->SetTitle("");
  //graph2->GetXaxis()->SetTitle("E [keV]"); 
  graph2->GetYaxis()->SetTitle("#DeltaADC");
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
    //diffy[i]= FitCalibrationErrorsnew->Eval(xnew[i]) - FitCalibrationErrors->Eval(xnew[i]);
    diffy[i]= FitCalibrationErrorsnew->Eval(xnew[i]) - Lineardependence->Eval(xnew[i]);
    ediffy[i]=0;

    std::cout<<"Energy: "<<xnew[i]<<"keV----"<<std::endl;
    std::cout<<"    ADC best calibration: "<<FitCalibrationErrors->Eval(xnew[i])<<std::endl;
    std::cout<<"    ADC New calibration: "<<FitCalibrationErrorsnew->Eval(xnew[i])<<std::endl;
    std::cout<<"    Difference between the 2 fits: "<<diffy[i]<<std::endl;
  }

  
  auto gdiffcalib= new TGraphErrors(nnew,xnew,diffy,exnew,ediffy); 
 
  gdiffcalib->SetMarkerStyle(20);
  gdiffcalib->SetMarkerColor(kViolet+3);
  gdiffcalib->SetTitle("");
  gdiffcalib->Draw("same,p");
 

  c1->cd(); 
  
  
  //c1->Print("CalibrationLiverpoolM1000L500_goodrootfile_grade5poly.pdf","pdf");
  c1->Print("NewCalibrationAround347keV_factor0.001_.png","png");

}
