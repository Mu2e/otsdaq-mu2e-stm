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

void HeightShifting() {
  gROOT->SetStyle("ATLAS");

  bool simulationenergy=false;
  bool simulationLiverpool=true;


  if(simulationenergy==true){
  double xx1[2]={0,200};
  double yy1[2]={0.03,0.1};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("Height Shifting [(E_{true}-E_{reco})/E_{true}]");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  
    const Int_t n=13;

    Double_t shift_675[n],shift_1836[n],shift_511[n];
    Double_t shift_error_675[n],shift_error_1836[n],shift_error_511[n];

    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    //E=675 keV
    Double_t Etrue_675 = 675.36;
    Double_t Ereco_675[13]={627.748,627.692,627.083,626.119,625.718,625.488,625.301,624.845,624.606,624.388,624.079,623.749,623.529};
    Double_t Ereco_error_675[13]={0.01,0.006,0.004,0.003,0.003,0.004,0.004,0.006,0.006,0.007,0.007,0.008,0.008};

    //E=1836 keV
    Double_t Etrue_1836 = 1836;
    Double_t Ereco_1836[13]={1706.52,1706.37,1704.69,1702.03,1700.97,1700.37,1699.9,1698.69,1698.04,1697.47,1696.68,1695.8,1695.14};
    Double_t Ereco_error_1836[13]={0.01,0.005,0.005,0.004,0.007,0.009,0.01,0.014,0.016,0.017,0.019,0.021,0.023};

    //E=511 keV
    Double_t Etrue_511 = 511;
    Double_t Ereco_511[13]={475,474.972,474.491,473.769,473.463,473.275,473.145,472.859,472.712,472.573,472.376,472.16,471.998};
    Double_t Ereco_error_511[13]={0.01,0.005,0.004,0.003,0.003,0.003,0.004,0.004,0.005,0.005,0.006,0.006,0.006};




    for(int i=0;i<n;i++){
      shift_675[i]=(Etrue_675-Ereco_675[i])/Etrue_675;
      shift_1836[i]=(Etrue_1836-Ereco_1836[i])/Etrue_1836;
      shift_511[i]=(Etrue_511-Ereco_511[i])/Etrue_511;

      shift_error_675[i]=shift_675[i]*Ereco_error_675[i]/Ereco_675[i];
      shift_error_1836[i]=shift_1836[i]*Ereco_error_1836[i]/Ereco_1836[i];
      shift_error_511[i]=shift_511[i]*Ereco_error_511[i]/Ereco_511[i];
    }




    TGraphErrors *grshift_675 = new TGraphErrors(n,&rate[0],&shift_675[0],&rate_error[0],&shift_error_675[0]);
    grshift_675->SetMarkerColor(kGreen-5);
    grshift_675->SetMarkerStyle(21);
    grshift_675->SetMarkerSize(1.3);
    grshift_675->Draw("same,p");

    TGraphErrors *grshift_1836 = new TGraphErrors(n,&rate[0],&shift_1836[0],&rate_error[0],&shift_error_1836[0]);
    grshift_1836->SetMarkerColor(kPink+10);
    grshift_1836->SetMarkerStyle(20);
    grshift_1836->SetMarkerSize(1.3);
    grshift_1836->Draw("same,p");

    TGraphErrors *grshift_511 = new TGraphErrors(n,&rate[0],&shift_511[0],&rate_error[0],&shift_error_511[0]);
    grshift_511->SetMarkerColor(kBlue);
    grshift_511->SetMarkerStyle(22);
    grshift_511->SetMarkerSize(1.3);
    grshift_511->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.025);
    latex.DrawLatex(10,0.04,"M=400, L=200");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grshift_511,"E = 511 keV","p");
    legend->AddEntry(grshift_675,"E = 675 keV","p");
    legend->AddEntry(grshift_1836,"E = 1836 keV","p");
    legend->Draw("same");

  }





  if(simulationLiverpool==true){


    double xx1[2]={0,1600};
    //double yy1[2]={0,0.1};
    //double yy1[2]={0,110};
    double yy1[2]={-110,110};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
    graph1->SetTitle("");
    graph1->GetXaxis()->SetTitle("E_{reco} [keV]");
    //graph1->GetYaxis()->SetTitle("Height Shifting [|(E_{true}-E_{reco})|/E_{true}]");
    graph1->GetYaxis()->SetTitle("Height Shifting [(E_{true}-E_{reco})] [keV]"); 
    graph1->GetXaxis()->SetTitleOffset(0.9);
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");


    const Int_t n=10;

    Double_t shift_error_M400[n],shift_error_M8000[n],shift_M400[n],shift_M8000[n];

    //Liverpool Calibration M=400,L=200
    Double_t Etrue_M400[10] = {40.032,100.021,299.795,510.887,661.990,800.078,899.682,1100.021,1299.795,1399.965};
    Double_t Etrue_error_M400[10] = {0,0,0,0,0,0,0,0,0,0};
    Double_t Ereco_M400[10]={37.481,93.232,278.817,475.029,615.383,743.775,836.39,1022.53,1208.25,1301.32};
    Double_t Ereco_error_M400[10]={0.021,0.021,0.021,0.022,0.022,0.022,0.022,0.021,0.021,0.022};

    //Liverpool Calibration M=8000,L=1000 
    Double_t Etrue_M8000[10] = {39.515,99.909,301.033,513.552,665.676,804.696,904.973,1106.667,1307.790,1408.637};
    Double_t Etrue_error_M8000[10] = {0,0,0,0,0,0,0,0,0,0};
    Double_t Ereco_M8000[10]={42.396,107.205,323.339,551.772,715.299,864.729,972.548,1189.4,1405.61,1514};
    Double_t Ereco_error_M8000[10]={0.008,0.009,0.01,0.009,0.01,0.009,0.009,0.01,0.01,0.01};



    for(int i=0;i<n;i++){
      //shift_M400[i]=abs((Etrue_M400[i]-Ereco_M400[i])/Etrue_M400[i]);
      //shift_M8000[i]=abs((Etrue_M8000[i]-Ereco_M8000[i])/Etrue_M8000[i]);
      
      shift_M400[i]=(Etrue_M400[i]-Ereco_M400[i]);                                                                             
      shift_M8000[i]=(Etrue_M8000[i]-Ereco_M8000[i]); 

      shift_error_M400[i]=shift_M400[i]*Ereco_error_M400[i]/Ereco_M400[i];
      shift_error_M8000[i]=shift_M8000[i]*Ereco_error_M8000[i]/Ereco_M8000[i];

      std::cout<<"M=400 "<<shift_M400[i]<<"+-"<<shift_error_M400[i]<<std::endl;
      std::cout<<"M=8000 "<<shift_M8000[i]<<"+-"<<shift_error_M8000[i]<<std::endl;
    }




    TGraphErrors *grshift_M400 = new TGraphErrors(n,&Etrue_M400[0],&shift_M400[0],&Etrue_error_M400[0],&shift_error_M400[0]);
    grshift_M400->SetMarkerColor(kBlue-5);
    grshift_M400->SetMarkerStyle(20);
    grshift_M400->Draw("same,p");

    TGraphErrors *grshift_M8000 = new TGraphErrors(n,&Etrue_M8000[0],&shift_M8000[0],&Etrue_error_M8000[0],&shift_error_M8000[0]);
    grshift_M8000->SetMarkerColor(kBlue+10);
    grshift_M8000->SetMarkerStyle(20);
    grshift_M8000->Draw("same,p");

    TLatex latex1;
    latex1.SetTextSize(0.04);
    latex1.DrawLatex(0,0.05,"Simulation 1kHz");

    auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
    leg1->AddEntry(grshift_M8000, "M=8000, L=1000","p");
    leg1->AddEntry(grshift_M400, "M=400, L=200","p");
    leg1->Draw("same");

   
  }










}
