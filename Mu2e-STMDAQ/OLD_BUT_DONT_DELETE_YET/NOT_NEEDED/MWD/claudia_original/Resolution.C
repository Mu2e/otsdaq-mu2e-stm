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
  gROOT->SetStyle("ATLAS");
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  //auto c1= new TCanvas("c1","Title",200,10,700,600);
  //c1->SetGrid();

  auto c1= new TCanvas("c1");

  double M=8000;
  bool simulation=false;

  //Using Liverpool data results with M=8000, L=1000 (same values for the calibration) old .root files (generated with test_new.cc)
  /*  if(M==8000&&simulation==false){
  double xx1[2]={0.1,1500};
  double yy1[2]={0, 6};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->GetXaxis()->SetTitle("E_{reco} [keV]");
  graph1->GetYaxis()->SetTitle("#sigma_{TOT} [keV]");
  graph1->Draw("ap");

  const Int_t n=10;
  double resol[n]={1.92,2.44,2.88,2.36,3.16,2.24,3.2,2.26,2.22,4.67};
  double eresol[n]={0.022,0.04,0.15,0.05,0.15,0.08,0.14,0.08,0.07,0.06};
  //Energies reco
  double x[n]={661.15,122.53,243.19,343.02,780.0,965.17,1088.53,1113.66,1410.65,40.91};
  //Error in energy reco
  double ex[n]={0.03,0.03,0.11,0.05,0.1,0.08,0.14,0.09,0.08,0.06};

  //Calibration Fit
  auto ge= new TGraphErrors(n,x,resol,ex,eresol);
  ge->SetMarkerStyle(20);
  ge->Draw("p");
  }*/



  //Using Liverpool data results with M=8000, L=1000 (same values for the calibration) together with Liverpool data results with M=400, L=200 (same values for the calibration) and M=1000 L=500 (same values for the calibration)
  if(M==8000400&&simulation==false){
    double xx1[2]={0.1,1500};
    double yy1[2]={0,6};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);
    graph1->GetXaxis()->SetTitle("E_{reco} [keV]");
    graph1->GetYaxis()->SetTitle("#sigma_{TOT} [keV]");
    graph1->Draw("ap");

    const Int_t n=10;
    double resolM8000[n]={1.574,3.86,1.92,3.03,1.82,1.95,1.96,2.67,2.05,2.04};
    double error_resolM8000[n]={0.018,0.04,0.03,0.18,0.04,0.09,0.07,0.12,0.08,0.07};
    //Energy reco
    double EM8000[n]={660.910,41.16,121.89,242.80,343.00,779.83,965.41,1088.57,1114.01,1410.65};
    //Error in energy reco
    double error_EM8000[n]={0.024,0.04,0.03,0.13,0.04,0.08,0.07,0.13,0.09,0.08};

    //Calibration Fit
    auto gM8000= new TGraphErrors(n,EM8000,resolM8000,error_EM8000,error_resolM8000);
    gM8000->SetMarkerStyle(20);
    gM8000->Draw("p");



    double resolM400[n]={1.806,2.29,1.63,2.29,1.77,2.58,2.40,3.52,2.44,2.98};
    double error_resolM400[n]={0.018,0.04,0.03,0.09,0.03,0.09,0.06,0.12,0.06,0.07};
    //Energy reco
    double EM400[n]={661.56,40.77,121.477,244.12,343.57,779.30,964.85,1087.54,1113.34,1406.79};
    //Error in energy reco
    double error_EM400[n]={0.03,0.02,0.022,0.07,0.03,0.08,0.08,0.13,0.08,0.10};

    //Calibration Fit
    auto gM400= new TGraphErrors(n,EM400,resolM400,error_EM400,error_resolM400);
    gM400->SetMarkerStyle(20);
    gM400->SetMarkerColor(kSpring-9);
    gM400->Draw("p");


    double resolM1000[n]={1.309,1.77,1.266,2.05,1.25,1.83,1.76,2.59,1.63,1.94};
    double error_resolM1000[n]={0.014,0.04,0.021,0.08,0.03,0.08,0.05,0.10,0.05,0.05};
    //Energy reco
    double EM1000[n]={661.589,41.294,121.892,244.34,343.70,779.42,964.97,1087.50,1113.35,1406.64};
    //Error in energy reco
    double error_EM1000[n]={0.019,0.018,0.018,0.06,0.03,0.06,0.06,0.11,0.06,0.07};

    //Calibration Fit
    auto gM1000= new TGraphErrors(n,EM1000,resolM1000,error_EM1000,error_resolM1000);
    gM1000->SetMarkerStyle(20);
    gM1000->SetMarkerColor(kBlue);
    gM1000->Draw("p");




    auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
    leg1->AddEntry(gM8000, "M=8000, L=1000","p");
    leg1->AddEntry(gM1000, "M=1000, L=500","p");
    leg1->AddEntry(gM400, "M=400, L=200","p");
    leg1->Draw("same");
  }






  //Using simulated liverpool data using  with M=8000, L=1000 (same values for the calibration old cali for M=8000, L=1000) together with simulation results with M=400, L=200 (same values for the calibration)  
  if(M==8000400&&simulation==true){
    double xx1[2]={0.1,1600};
    double yy1[2]={0, 1.5};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);
    graph1->GetXaxis()->SetTitle("E_{reco} [keV]");
    graph1->GetYaxis()->SetTitle("#sigma_{algorithm} [keV]");
    graph1->Draw("ap");

    const Int_t n=10;
    double resolM8000[n]={0.237,0.266,0.3,0.284,0.313,0.286,0.293,0.3,0.295,0.295};
    double error_resolM8000[n]={0.005,0.006,0.007,0.006,0.007,0.006,0.007,0.007,0.007,0.007};
    //Energy reco
    double EM8000[n]={42.3955,107.205,323.339,551.772,715.299,864.729,972.548,1189.4,1405.61,1514};
    //Error in energy reco
    double error_EM8000[n]={0,0,0,0,0,0,0,0,0,0};

    //Calibration Fit
    auto gM8000= new TGraphErrors(n,EM8000,resolM8000,error_EM8000,error_resolM8000);
    gM8000->SetMarkerStyle(20);
    gM8000->SetMarkerColor(kGray+1);
    gM8000->Draw("p");



    double resolM400[n]={0.663,0.633,0.658,0.655,0.657,0.648,0.665,0.642,0.653,0.661};
    double error_resolM400[n]={0.015,0.014,0.015,0.015,0.015,0.015,0.015,0.014,0.015,0.015};
    //Energy reco
    double EM400[n]={37.4806,93.2319,278.817,475.029,615.383,743.775,836.39,1022.53,1208.25,1301.32};
    //Error in energy reco
    double error_EM400[n]={0,0,0,0,0,0,0,0,0,0};

    //Calibration Fit
    auto gM400= new TGraphErrors(n,EM400,resolM400,error_EM400,error_resolM400);
    gM400->SetMarkerStyle(20);
    gM400->SetMarkerColor(kSpring+9);
    gM400->Draw("p");

    TLatex latex1;
    latex1.SetTextSize(0.04);
    latex1.DrawLatex(0,0.05,"Simulation 1kHz");  

    auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
    leg1->AddEntry(gM8000, "M=8000, L=1000","p");
    leg1->AddEntry(gM400, "M=400, L=200","p");
    leg1->Draw("same");
  }






  //Using Liverpool data results with M=1000,L=500 and calibrations using M=1000, L=500/ M=8000, L=1000(new) and M=400, L=200                                                                                                                                             
    if(M==1000&&simulation==false){

      double xx1[2]={0.1,1500};
      double yy1[2]={0,6};
      TGraph *graph1 = new TGraph (2,xx1,yy1);
      graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
      graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
      graph1->SetTitle("");
      graph1->SetMarkerStyle(1);
      graph1->GetXaxis()->SetTitle("E_{reco} [keV]");
      graph1->GetYaxis()->SetTitle("#sigma_{TOT} [keV]");
      graph1->Draw("ap");

      const Int_t n=10;
      double resolM8000[n]={1.310,4.77,1.24,4.6,1.27,1.83,1.76,2.5,1.59,2.00};
      double error_resolM8000[n]={0.014,0.08,0.02,0.3,0.03,0.08,0.05,0.1,0.05,0.06};
      //Energy reco
      double EM8000[n]={660.739,37.17,118.872,241.57,341.59,779.03,965.29,1088.27,1114.26,1408.77};
      //Error in energy reco
      double error_EM8000[n]={0.019,0.11,0.017,0.15,0.03,0.06,0.06,0.11,0.06,0.07};

      //Calibration Fit
      auto gM8000= new TGraphErrors(n,EM8000,resolM8000,error_EM8000,error_resolM8000);
      gM8000->SetMarkerStyle(20);
      gM8000->Draw("p");



      double resolM400[n]={1.301,1.7,1.254,5.4,1.25,1.88,1.86,2.5,1.62,2.7};
      double error_resolM400[n]={0.014,0.03,0.021,0.4,0.03,0.07,0.06,0.1,0.05,0.3};
      //Energy reco
      double EM400[n]={658.403,40.578,120.842,242.30,341.78,775.79,960.60,1082.56,1108.37,1399.2};
      //Error in energy reco
      double error_EM400[n]={0.019,0.017,0.018,0.18,0.03,0.07,0.06,0.11,0.06,0.7};

      //Calibration Fit
      auto gM400= new TGraphErrors(n,EM400,resolM400,error_EM400,error_resolM400);
      gM400->SetMarkerStyle(20);
      gM400->SetMarkerColor(kSpring-9);
      gM400->Draw("p");


      double resolM1000[n]={1.309,1.77,1.266,2.05,1.25,1.83,1.76,2.59,1.63,1.94};
      double error_resolM1000[n]={0.014,0.04,0.021,0.08,0.03,0.08,0.05,0.10,0.05,0.05};
      //Energy reco
      double EM1000[n]={661.589,41.294,121.892,244.34,343.70,779.42,964.97,1087.50,1113.35,1406.64};
      //Error in energy reco
      double error_EM1000[n]={0.019,0.018,0.018,0.06,0.03,0.06,0.06,0.11,0.06,0.07};


      TLatex latex1;
      latex1.SetTextSize(0.04);
      latex1.DrawLatex(0,0.05,"M=1000, L=500");


      //Calibration Fit
      auto gM1000= new TGraphErrors(n,EM1000,resolM1000,error_EM1000,error_resolM1000);
      gM1000->SetMarkerStyle(20);
      gM1000->SetMarkerColor(kBlue);
      gM1000->Draw("p");


      auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
      leg1->AddEntry(gM8000, "Calibration: M=8000, L=1000","p");
      leg1->AddEntry(gM1000, "Calibration: M=1000, L=500","p");
      leg1->AddEntry(gM400, "Calibration: M=400, L=200","p");
      leg1->Draw("same");

    }





    //Using Liverpool data results with M=8000, L=1000 and using old calibration with M=8000, L=1000 and new calibration with M=8000 L=1000 (comparing old and new calibrations)
       if(M==8000&&simulation==false){
	 double xx1[2]={0.1,1500};
	 double yy1[2]={0,4};
	 TGraph *graph1 = new TGraph (2,xx1,yy1);
	 graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
	 graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
	 graph1->SetTitle("");
	 graph1->SetMarkerStyle(1);
	 graph1->GetXaxis()->SetTitle("E_{reco} [keV]");
	 graph1->GetYaxis()->SetTitle("#sigma_{TOT} [keV]");
	 graph1->Draw("ap");

	 const Int_t n=10;
	 double resolM8000_newcali[n]={1.574,3.86,1.92,3.03,1.82,1.95,1.96,2.67,2.05,2.04};
	 double error_resolM8000_newcali[n]={0.018,0.04,0.03,0.18,0.04,0.09,0.07,0.12,0.08,0.07};
	 //Energy reco
	 double EM8000_newcali[n]={660.910,41.16,121.89,242.80,343.00,779.83,965.41,1088.57,1114.01,1410.65};
	 //Error in energy reco
	 double error_EM8000_newcali[n]={0.024,0.04,0.03,0.13,0.04,0.08,0.07,0.13,0.09,0.08};

	 //Calibration Fit
	 auto gM8000_newcali= new TGraphErrors(n,EM8000_newcali,resolM8000_newcali,error_EM8000_newcali,error_resolM8000_newcali);
	 gM8000_newcali->SetMarkerStyle(20);
	 gM8000_newcali->SetMarkerColor(kBlack);
	 gM8000_newcali->Draw("p");



	 double resolM8000_oldcali[n]={1.560,3.68,1.90,3.0,1.78,1.96,1.84,2.63,2.01,2.07};
	 double error_resolM8000_oldcali[n]={0.018,0.04,0.03,0.2,0.04,0.10,0.07,0.11,0.08,0.07};
	 //Energy reco
	 double EM8000_oldcali[n]={662.245,43.14,123.87,244.71,344.68,781.02,966.31,1089.33,1114.71,1411.04};
	 //Error in energy reco
	 double error_EM8000_oldcali[n]={0.024,0.03,0.03,0.09,0.04,0.08,0.08,0.12,0.09,0.08};

	 //Calibration Fit
	 auto gM8000_oldcali= new TGraphErrors(n,EM8000_oldcali,resolM8000_oldcali,error_EM8000_oldcali,error_resolM8000_oldcali);
	 gM8000_oldcali->SetMarkerStyle(20);
	 gM8000_oldcali->SetMarkerColor(kViolet);
	 gM8000_oldcali->Draw("p");


	 TLatex latex1;
	 latex1.SetTextSize(0.04);
	 latex1.DrawLatex(0,0.05,"M=8000, L=1000");

	 auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
	 leg1->AddEntry(gM8000_oldcali, "Old Caibration: M=8000, L=1000","p");
	 leg1->AddEntry(gM8000_newcali, "New Calibration: M=8000, L=1000","p");
	 leg1->Draw("same");

       }

}
