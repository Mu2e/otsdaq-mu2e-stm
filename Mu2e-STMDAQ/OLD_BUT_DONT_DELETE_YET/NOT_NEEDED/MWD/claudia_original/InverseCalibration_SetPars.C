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

void InverseCalibration_SetPars() {
  gROOT->SetStyle("ATLAS");

  bool saveCalibtoROOT = true;
  
  TCanvas* c1 = new TCanvas("c1");   
 
  TPad *pad1 = new TPad("pad1", "pad1",0,0.33,1,1);
  TPad *pad2 = new TPad("pad2", "pad2",0,0,1,0.38);
    
  pad1->SetBottomMargin(0.1);
  pad1->SetGrid();
  pad1->SetBorderMode(0);
  pad2->SetTopMargin(0.1);
  pad2->SetBottomMargin(0.3);
  pad2->SetBorderMode(0);
  pad2->SetGrid();
  pad1->Draw();
  pad2->Draw();
 
  pad1->cd();


  double yy1[2] = {-100,2000};
  double xx1[2] = {-3000, 0};
    
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetYaxis()->SetTitle("E [keV]");
  graph1->GetXaxis()->SetTitle("");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->GetXaxis()->SetLabelSize(0);
  graph1->SetMarkerStyle(1);
  graph1->SetMaximum(yy1[1]);
  graph1->SetMinimum(yy1[0]);
  graph1->GetYaxis()->SetLabelSize(0.05);
  graph1->GetYaxis()->SetTitleSize(0.05);
  graph1->Draw("ap");
  
  const Int_t n = 13;

  
  //Energies
  Double_t y[13]={661.659,40.1186,121.7817,244.6974,344.2785,411.1165,443.965,778.91,867.380,964.08,1085.837,1112.076,1408.013};
  Double_t ey[13]={0.003,0.0001,0.0003,0.0008,0.0012,0.0012,0.003,0.0024,0.003,0.018,0.010,0.003,0.003};
  
  
  //M1000L500 Liverpool calibration (root file generated with MWDROOT) 
  //ADC
  Double_t x[13] = {-1161.64,-70.01,-211.75,-426.91,-602.16,-720.7,-778.7,-1368.78,-1525.7,-1695.5,-1910.53,-1956.92,-2472.68};
  Double_t ex[13] = {0.03,0.03,0.03,0.11,0.04,0.3,0.3,0.1,0.4,0.11,0.23,0.11,0.13};

  double ML_fitpars[6] = {-5.37452884e-01, -5.82585531e-01, -2.82211678e-05, -2.55100017e-08, -1.12628116e-11, -1.90315894e-15};

  double ML_residuals[13] = {-0.54551735,  0.02663266, -0.04853033, -0.22860641,  0.01333023,  1.26978691, 1.43051437,  1.21138457,  1.18599237, -0.33833164, -0.2051254, 0.04583907, 0.01220134};
  
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->Draw("same,p");
  
   
  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[5]*x*x*x*x*x+[4]*x*x*x*x+[3]*x*x*x+[2]*x*x+[1]*x+[0]", -3500, 0);
  FitCalibrationErrors->SetParameters(ML_fitpars[0], ML_fitpars[1], ML_fitpars[2], ML_fitpars[3], ML_fitpars[4], ML_fitpars[5]);
  FitCalibrationErrors->SetLineColor(kRed);
  FitCalibrationErrors->Draw("same");


  std::string string_[6];
  char* char_p[6];
  TString text[6];
     
  for(int i = 0; i < 6; i++){
    text[i] = Form("%5.4g", ML_fitpars[i]);
    text[i].ReplaceAll("e-0","x10^{-");
    text[i].ReplaceAll("e-","x10^{-");
    text[i].Append("}");

    if((i==0)||(i==1)){
      std::stringstream stream_;
      stream_ <<std::setprecision(4)<< (ML_fitpars[i]*10);
      text[i] = stream_.str()+"x10^{-1}"; }

    std::cout<<text[i]<<std::endl;
    string_[i] = "#bf{#color[2]{p_{"+std::to_string(i)+"} = "+text[i]+"}}";
    char_p[i] = const_cast<char*>(string_[i].c_str());

    
  }

  const char *chi2ndf = "#bf{#color[1]{#chi^{2}/ndf = 7.04 / 7}}";
  
  TLatex latex1;
  latex1.SetTextSize(0.08);
  latex1.DrawLatex(-1000,1600,chi2ndf);
  TLatex latex;
  latex1.SetTextSize(0.075);
  latex.DrawLatex(-2850,850,char_p[0]);
  latex.DrawLatex(-2850,700,char_p[1]);
  latex.DrawLatex(-2850,550,char_p[2]);
  latex.DrawLatex(-2850,400,char_p[3]);
  latex.DrawLatex(-2850,250,char_p[4]);
  latex.DrawLatex(-2850,100,char_p[5]);
  
  pad2->cd();

  double xx2[2]={-3000, 0};
  double yy2[2]={-2, 2};
  TGraph *graph2 = new TGraph (2,xx2,yy2);
  graph2->GetXaxis()->SetRangeUser(xx2[0],xx2[1]);
  graph2->GetYaxis()->SetRangeUser(yy2[0],yy2[1]);
  graph2->SetTitle("");
  graph2->GetXaxis()->SetTitle("ADC Counts");
  graph2->GetYaxis()->SetTitle("#splitline{Residual = }{(Fit-Value)/#sigma}");
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
  double chi2 = 0;

  Double_t resx[n], resy[n], eresx[n], eresy[n];
     
  for (Int_t i=0;i<n;i++){
    
    resx[i]=x[i];
    resy[i]=ML_residuals[i];
    eresx[i]=0;
    eresy[i]=0;
}
    
  auto gresid= new TGraphErrors(n,resx,resy,eresx,eresy); 
 
  gresid->SetMarkerStyle(20);
  gresid->SetMarkerColor(kGreen+3);
  gresid->SetTitle("");
  gresid->Draw("same,p");
 

  c1->cd();

  //c1->Print("InverseCalibrationLiverpoolM1000L500_goodrootfileGrade1poly_newbinning_Eerrors.pdf","pdf");
  c1->Print("InverseCalibrationLiverpoolM1000L500_grade5poly_fixedvalues.png","png");

}
