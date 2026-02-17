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

void InverseCalibration() {
  gROOT->SetStyle("ATLAS");

  bool saveCalibtoROOT = true;
  
  TCanvas* c1 = new TCanvas("c1");   
 
  TPad *pad1 = new TPad("pad1", "pad1",0,0.33,1,1);
  TPad *pad2 = new TPad("pad2", "pad2",0,0,1,0.33);
    
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


  double yy1[2] = {-100,2000};
  double xx1[2] = {-3500, 0};
    
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetYaxis()->SetTitle("E [keV]");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->SetMaximum(yy1[1]);
  graph1->SetMinimum(yy1[0]);
  graph1->Draw("ap");
  
  const Int_t n = 13;

  
  //Energies
  Double_t y[13]={661.659,40.1186,121.7817,244.6974,344.2785,411.1165,443.965,778.91,867.380,964.08,1085.837,1112.076,1408.013};
  Double_t ey[13]={0.003,0.0001,0.0003,0.0008,0.0012,0.0012,0.003,0.0024,0.003,0.018,0.010,0.003,0.003};
  
  
  //M1000L500 Liverpool calibration (root file generated with MWDROOT) 
  //ADC
  Double_t x[13]={-1161.64,-70.01,-211.75,-426.91,-602.16,-720.7,-778.7,-1368.78,-1525.7,-1695.5,-1910.53,-1956.92,-2472.68};
  Double_t ex[13]={0.03,0.03,0.03,0.11,0.04,0.3,0.3,0.1,0.4,0.11,0.23,0.11,0.13};
  //Double_t ex[13]={0,0,0,0,0,0,0,0,0,0,0,0,0}; 
  
  //Calibration Fit
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->Draw("same,p");

  /*   
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", -3500, 0);
  FitCalibrationErrors->SetParameters(-2.81467e-01,-1.74204e+00);
  ge->Fit(FitCalibrationErrors,"0","",-3500, 0);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");
  */

  /*
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[3]*x*x*x+[2]*x*x+[1]*x+[0]", -3500, 0);
  FitCalibrationErrors->SetParameters(-0.122565, -0.576702, -8.83658e-06, -2.38309e-09);
  ge->Fit(FitCalibrationErrors,"0","", -3500, 0);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");
  */
  
   
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", -3500, 0);
  //FitCalibrationErrors->FixParameter(0,-5.37452884e-01);
  //FitCalibrationErrors->FixParameter(1,-5.82585531e-01);
  //FitCalibrationErrors->FixParameter(2,-2.82211678e-05);
  //FitCalibrationErrors->FixParameter(3,-2.55100017e-08);
  //FitCalibrationErrors->FixParameter(4,-1.12628116e-11);
  //FitCalibrationErrors->FixParameter(5,-1.90315894e-15);
  FitCalibrationErrors->SetParameters( -0.535101, -0.58256, -2.81612e-05, -2.54579e-08, -1.12446e-11, -1.90103e-15);
  //ge->Fit(FitCalibrationErrors,"0","", -3500, 0);
  TFitResultPtr fitptr = ge->Fit(FitCalibrationErrors,"0S","", -3500, 0);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");
  
  double Chi2errors;
  Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;

  
  TMatrixD _cov = fitptr->GetCovarianceMatrix();
  //Store best fit paramters and errors (p0 +- p0_error...) and the covariance matrix 
  double NP = 6; //Number fit parameters
  TMatrixD _BestFitPars(1,NP,FitCalibrationErrors->GetParameters(),"");
  TMatrixD _BestFitParsErrors(1,NP,FitCalibrationErrors->GetParErrors(),"");

  TMatrixD* cov = new TMatrixD(NP,NP,_cov.GetMatrixArray(),"");
  TMatrixD* BestFitPars = new TMatrixD(NP,1,_BestFitPars.GetMatrixArray(),"");
  TMatrixD* BestFitParsErrors = new TMatrixD(NP,1,_BestFitParsErrors.GetMatrixArray(),"");

  std::cout<<"Cov matrix..."<<std::endl;
  cov->Print();
  std::cout<<"Best fit parameters..."<<std::endl;
  BestFitPars->Print();
  std::cout<<"Best fit parameters errors..."<<std::endl;
  BestFitParsErrors->Print();


  TFile *output;

  if(saveCalibtoROOT==true){
    
    std::string rootfile = "InverseCalibration_2DErrors_CovMatrix_Bestfits.root";
    output = new TFile(rootfile.c_str(),"recreate");

    output->WriteObject(cov, "Covmatrix");
    output->WriteObject(BestFitPars, "BestFitPars");
    output->WriteObject(BestFitParsErrors, "BestFitParsErrors");

    output->Close();
  }
  
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

  double xx2[2]={-3500, 0};
  double yy2[2]={-400, 40};
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
    std::cout<<"Data y: "<<y[i]<<"+_"<<ey[i]<<" x: " <<x[i]<<" Fit: "<<FitCalibrationErrors->Eval(x[i])<<" res: "<<resy[i]<<std::endl;
    
    chi2 = chi2 + resy[i]*resy[i];
    

    //this method just useful for a grade 1 poly fit
    /*double par0 = FitCalibrationErrors->GetParameter(0);
    double par1	= FitCalibrationErrors->GetParameter(1);
    std::cout<<"par0: "<<par0<<" par1: "<<par1<<std::endl;
    double d = (par1*x[i]-y[i]+par0) / sqrt(par1*par1 + (-1)*(-1));
    double dx = (par1/sqrt(par1*par1 + (-1)*(-1)))*ex[i];
    double dy = (-1/sqrt(par1*par1 + (-1)*(-1)))*ey[i];
    double sigma = sqrt( dx*dx + dy*dy );
    chi2 = chi2 + (d*d / (sigma*sigma));
   
    resx[i]=x[i];
    resy[i]=d/sigma;
    eresx[i]=0;
    eresy[i]=0; //?
    std::cout<<"Data y: "<<y[i]<<"+_"<<ey[i]<<" x: " <<x[i]<<" Fit: "<<FitCalibrationErrors->Eval(x[i])<<" res: "<<resy[i]<<std::endl;
    */  
}
  std::cout<<"check chi2: "<<chi2<<std::endl;
    
  auto gresid= new TGraphErrors(n,resx,resy,eresx,eresy); 
 
  gresid->SetMarkerStyle(20);
  gresid->SetMarkerColor(kGreen+3);
  gresid->GetYaxis()->SetTitle("Residuals");
  gresid->SetTitle("");
  gresid->Draw("same,p");
 

  c1->cd();

  //c1->Print("InverseCalibrationLiverpoolM1000L500_goodrootfileGrade1poly_newbinning_Eerrors.pdf","pdf");
  //c1->Print("InverseCalibrationLiverpoolM1000L500_goodrootfileGrade5poly_Eerrors.png","png");

}
