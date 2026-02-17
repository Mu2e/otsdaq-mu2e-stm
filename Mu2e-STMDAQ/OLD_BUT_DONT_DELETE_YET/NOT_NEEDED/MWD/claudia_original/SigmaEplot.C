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

void SigmaEplot() {
  gROOT->SetStyle("ATLAS");
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  //auto c1= new TCanvas("c1","Title",200,10,700,600);
  //c1->SetGrid();

  auto c1= new TCanvas("c1");


  double xx1[2]={0.1,1500};
  double yy1[2]={-0.06, 0.06};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->GetXaxis()->SetTitle("E_{reco} [keV]");
  graph1->GetYaxis()->SetTitle("#sigma_{TOT}/E_{reco} [keV]");
  graph1->Draw("ap");

  /*//OLD VALUES
  const Int_t n=10;
  Double_t y[n],ey[n];
  double resol[n]={1.92,2.44,2.88,2.36,3.16,2.24,3.2,2.26,2.22,4.67};
  double eresol[n]={0.022,0.04,0.15,0.05,0.15,0.08,0.14,0.08,0.07,0.06};
  //Energies reco
  double x[n]={661.15,122.53,243.19,343.02,780.0,965.17,1088.53,1113.66,1410.65,40.91};
  //Error in energy reco
  double ex[n]={0.03,0.03,0.11,0.05,0.1,0.08,0.14,0.09,0.08,0.06};
  */

  const Int_t n=13;
  Double_t y[n],ey[n];
  double resol[n]={1.304, 1.515, 1.27, 1.76, 1.26, 2.14, 1.84, 1.93, 1.88, 1.66, 2.5, 1.60, 2.00};
  double eresol[n]={0.08, 0.021, 0.14, 0.07, 0.19, 0.14, 0.08, 0.09, 0.05, 0.1, 0.05, 0.06};
  //Energies reco
  double x[n]={662.156, 39.89, 120.724, 243.33, 343.24, 411.04, 443.93, 780.39, 869.81, 966.39, 1089.35, 1115.29, 1409.57};
  //Error in energy reco
  double ex[n]={0.019, 0.015, 0.018, 0.05, 0.03, 0.14, 0.11, 0.07, 0.10, 0.06, 0.11, 0.056, 0.07};
 
 
    for(int i=0;i<n;i++){
      y[i]=resol[i]/x[i];}
 
 
    for (int i=0;i<n;i++){
      ey[i]=y[i]*sqrt(((eresol[i]/resol[i])*(eresol[i]/resol[i]))+((ex[i]/x[i])*(ex[i]/x[i])));
    }



  //Calibration Fit
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->Draw("p");

  TF1*FitCalibrationErrors1 = new TF1("FitCalibrationErrors1", "[0]/sqrt(x)",0, 1500);
  TF1*FitCalibrationErrors2 = new TF1("FitCalibrationErrors2", "sqrt(([0]/x)+([1]/(x*x))+[2])",0, 1500);

  FitCalibrationErrors1->SetParameters(-3.13173e-02, 1.68184e+01, 1.77995e-05 );
  FitCalibrationErrors1->SetLineColor(kMagenta+1);
  FitCalibrationErrors1->SetLineStyle(kDashed);
  ge->Fit("FitCalibrationErrors1","S","");
  FitCalibrationErrors1->Draw("same");

  auto leg1 = new TLegend(0.18,0.2,0.55,0.5);
  leg1->AddEntry(FitCalibrationErrors1, "fit k_{E}/#sqrt{E_{reco}}","l");
  
  FitCalibrationErrors2->SetParameters(-3.13173e-02, 1.68184e+01, 1.77995e-05 );
  FitCalibrationErrors2->SetLineColor(kRed);
  FitCalibrationErrors2->SetLineStyle(kDashed);
  ge->Fit("FitCalibrationErrors2","S","");
  FitCalibrationErrors2->Draw("same");

  /* double Chi2errors1, Chi2errors2;
  Chi2errors1=ge->GetFunction("FitCalibrationErrors1")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors1<<endl;
  Chi2errors2=ge->GetFunction("FitCalibrationErrors2")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors2<<endl;
  */

  leg1->AddEntry(FitCalibrationErrors2, "fit #sqrt{D+k_{E}^{2}/E_{reco}+C/E_{reco}^{2}}","l");
  leg1->Draw("same");
  //gStyle->SetOptFit(1111); 

  gStyle->SetOptFit(0000);


  c1->cd();
  c1->Print("FinalFitLiverpoolData_Edependence_addpoints.pdf");
  c1->Print("FinalFitLiverpoolData_Edependence_addpoints.png");
}
