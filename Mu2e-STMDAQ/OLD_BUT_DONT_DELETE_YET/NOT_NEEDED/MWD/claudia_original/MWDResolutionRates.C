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

void MWDResolutionRates() {
  gROOT->SetStyle("ATLAS");

  double xx1[2]={0,200};
  double yy1[2]={0,5};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("Resolution [keV]");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  //Energy: 675 keV, 1836 keV or 511 keV
  bool E675=false;
  bool E1836=false;
  bool E511=false;
  bool E_allEnergies=false;
  bool E_allEnergies_resolE=false;
  bool Noise_E675=false;
  bool GausNoise_E675=true;


  if(E675==true){
  const Int_t n=13;
  Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
  Double_t ResolrawMWDM1000L500[13]={0.186, 0.248, 0.41, 0.470, 1.062, 1.620, 2.08, 3.283, 4.023, 4.71, 5.605, 6.346, 6.873};
  Double_t ResolrawMWDM1000L500_error[13]={0.004, 0.003, 0.003, 0.003, 0.006, 0.007, 0.007, 0.008, 0.009, 0.01, 0.012, 0.015, 0.018};
 
  Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  Double_t ResolrawMWDM500L200[13]={0.284, 0.377, 0.414, 0.4373, 0.643, 0.885, 1.148, 1.836, 2.252, 2.658, 3.224, 3.771, 4.149};
  Double_t ResolrawMWDM500L200_error[13]={0.007, 0.004, 0.003, 0.0021, 0.003, 0.004, 0.005, 0.005, 0.005, 0.006, 0.006, 0.007, 0.007};
  
  Double_t ResolrawMWDM400L200[13]={0.292, 0.375, 0.395, 0.4226, 0.565, 0.747, 0.952, 1.548, 1.92, 2.252, 2.718, 3.152, 3.459};
  Double_t ResolrawMWDM400L200_error[13]={0.007, 0.004, 0.003, 0.0021, 0.003, 0.003, 0.004, 0.005, 0.005, 0.005, 0.005, 0.006, 0.006};


  TGraphErrors *grResrawM1000L500 = new TGraphErrors(n,&rate[0],&ResolrawMWDM1000L500[0],&rate_error[0],&ResolrawMWDM1000L500_error[0]); 
  grResrawM1000L500->SetMarkerColor(kBlack);
  grResrawM1000L500->Draw("same,p");

  TGraphErrors *grResrawM500L200 = new TGraphErrors(n,&rate[0],&ResolrawMWDM500L200[0], &rate_error[0],&ResolrawMWDM500L200_error[0]);
  grResrawM500L200->SetMarkerColor(kCyan-7);
  grResrawM500L200->Draw("same,p");

  TGraphErrors *grResrawM400L200 = new TGraphErrors(n,&rate[0],&ResolrawMWDM400L200[0], &rate_error[0],&ResolrawMWDM400L200_error[0]);
  grResrawM400L200->SetMarkerColor(kViolet+2);
  grResrawM400L200->Draw("same,p");

  
  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(grResrawM1000L500,"M=1000, L=500","p"); 
  legend->AddEntry(grResrawM500L200,"M=500, L=200","p"); 
  legend->AddEntry(grResrawM400L200,"M=400, L=200","p");
  legend->Draw("same");
  }




  if(E1836==true){
    const Int_t n=13;
    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    Double_t ResolrawMWDM400L200[13]={0.313,0.343,0.502,0.544,1.007,1.556,2.146,3.879,4.878,5.794,7.053,8.252,9.004};
    Double_t ResolrawMWDM400L200_error[13]={0.008,0.003,0.003,0.003,0.005,0.007,0.008,0.012,0.013,0.014,0.014,0.015,0.016};


    TGraphErrors *grResrawM400L200 = new TGraphErrors(n,&rate[0],&ResolrawMWDM400L200[0], &rate_error[0],&ResolrawMWDM400L200_error[0]);
    grResrawM400L200->SetMarkerColor(kViolet+2);
    grResrawM400L200->Draw("same,p");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grResrawM400L200,"M=400, L=200","p");
    legend->Draw("same");
  }



  if(E511==true){
    const Int_t n=13;
    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    Double_t ResolrawMWDM400L200[13]={0.301,0.355,0.379,0.4129,0.542,0.681,0.831,1.217,1.469,1.734,2.078,2.399,2.626};
    Double_t ResolrawMWDM400L200_error[13]={0.007,0.004,0.003,0.0022,0.003,0.003,0.003,0.003,0.004,0.004,0.004,0.004,0.004};


    TGraphErrors *grResrawM400L200 = new TGraphErrors(n,&rate[0],&ResolrawMWDM400L200[0], &rate_error[0],&ResolrawMWDM400L200_error[0]);
    grResrawM400L200->SetMarkerColor(kViolet+2);
    grResrawM400L200->Draw("same,p");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grResrawM400L200,"M=400, L=200","p");
    legend->Draw("same");
  }




  if(E_allEnergies==true){
  
    const Int_t n=13;
    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};


    //675 keV
    Double_t ResolrawMWDM400L200_675[13]={0.292, 0.375, 0.395, 0.4226, 0.565, 0.747, 0.952, 1.548, 1.92, 2.252, 2.718, 3.152, 3.459};
    Double_t ResolrawMWDM400L200_error_675[13]={0.007, 0.004, 0.003, 0.0021, 0.003, 0.003, 0.004, 0.005, 0.005, 0.005, 0.005, 0.006, 0.006};

    //1836 keV
    Double_t ResolrawMWDM400L200_1836[13]={0.313,0.343,0.502,0.544,1.007,1.556,2.146,3.879,4.878,5.794,7.053,8.252,9.004};
    Double_t ResolrawMWDM400L200_error_1836[13]={0.008,0.003,0.003,0.003,0.005,0.007,0.008,0.012,0.013,0.014,0.014,0.015,0.016};

    //511 keV
    Double_t ResolrawMWDM400L200_511[13]={0.301,0.355,0.379,0.4129,0.542,0.681,0.831,1.217,1.469,1.734,2.078,2.399,2.626};
    Double_t ResolrawMWDM400L200_error_511[13]={0.007,0.004,0.003,0.0022,0.003,0.003,0.003,0.003,0.004,0.004,0.004,0.004,0.004};



    TGraphErrors *grRes_675 = new TGraphErrors(n,&rate[0],&ResolrawMWDM400L200_675[0], &rate_error[0],&ResolrawMWDM400L200_error_675[0]);
    grRes_675->SetMarkerColor(kGreen-5);
    grRes_675->SetMarkerStyle(21);
    grRes_675->SetMarkerSize(1.3);
    grRes_675->Draw("same,p");

    TGraphErrors *grRes_1836 = new TGraphErrors(n,&rate[0],&ResolrawMWDM400L200_1836[0], &rate_error[0],&ResolrawMWDM400L200_error_1836[0]);
    grRes_1836->SetMarkerColor(kPink+10);
    grRes_1836->SetMarkerStyle(20);
    grRes_1836->SetMarkerSize(1.3);
    grRes_1836->Draw("same,p");

    TGraphErrors *grRes_511 = new TGraphErrors(n,&rate[0],&ResolrawMWDM400L200_511[0], &rate_error[0],&ResolrawMWDM400L200_error_511[0]);
    grRes_511->SetMarkerColor(kBlue);
    grRes_511->SetMarkerStyle(22);
    grRes_511->SetMarkerSize(1.3);
    grRes_511->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.045);
    latex.DrawLatex(.2,.9,"M=400, L=200");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grRes_511,"E = 511 keV","p");
    legend->AddEntry(grRes_675,"E = 675 keV","p");
    legend->AddEntry(grRes_1836,"E = 1836 keV","p");
    legend->Draw("same");

}





  if(E_allEnergies_resolE==true){

    graph1->GetYaxis()->SetTitle("#sigma_{algorithm}/E");

    const Int_t n=13;
    Double_t resE_675[n], resE_error_675[n],resE_511[n], resE_error_511[n],resE_1836[n], resE_error_1836[n];
    
    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    //True energies (keV)
    Double_t TrueEnergy[3]={511,675,1836};

    //675 keV
    Double_t ResolrawMWDM400L200_675[13]={0.292, 0.375, 0.395, 0.4226, 0.565, 0.747, 0.952, 1.548, 1.92, 2.252, 2.718, 3.152, 3.459};
    Double_t ResolrawMWDM400L200_error_675[13]={0.007, 0.004, 0.003, 0.0021, 0.003, 0.003, 0.004, 0.005, 0.005, 0.005, 0.005, 0.006, 0.006};

    //1836 keV
    Double_t ResolrawMWDM400L200_1836[13]={0.313,0.343,0.502,0.544,1.007,1.556,2.146,3.879,4.878,5.794,7.053,8.252,9.004};
    Double_t ResolrawMWDM400L200_error_1836[13]={0.008,0.003,0.003,0.003,0.005,0.007,0.008,0.012,0.013,0.014,0.014,0.015,0.016};

    //511 keV
    Double_t ResolrawMWDM400L200_511[13]={0.301,0.355,0.379,0.4129,0.542,0.681,0.831,1.217,1.469,1.734,2.078,2.399,2.626};
    Double_t ResolrawMWDM400L200_error_511[13]={0.007,0.004,0.003,0.0022,0.003,0.003,0.003,0.003,0.004,0.004,0.004,0.004,0.004};


    for(int i=0;i<n;i++){
      resE_511[i]=ResolrawMWDM400L200_511[i]/TrueEnergy[0];
      resE_675[i]=ResolrawMWDM400L200_675[i]/TrueEnergy[1];
      resE_1836[i]=ResolrawMWDM400L200_1836[i]/TrueEnergy[2];

      resE_error_511[i]=ResolrawMWDM400L200_error_511[i]/TrueEnergy[0];
      resE_error_675[i]=ResolrawMWDM400L200_error_675[i]/TrueEnergy[1];
      resE_error_1836[i]=ResolrawMWDM400L200_error_1836[i]/TrueEnergy[2];

      std::cout<<"511: "<<ResolrawMWDM400L200_511[i]<<" "<<resE_511[i]<<std::endl;
      std::cout<<"675: "<<ResolrawMWDM400L200_675[i]<<" "<<resE_675[i]<<std::endl;
      std::cout<<"1836: "<<ResolrawMWDM400L200_1836[i]<<" "<<resE_1836[i]<<std::endl;
    }


    TGraphErrors *grRes_675 = new TGraphErrors(n,&rate[0],&resE_675[0], &rate_error[0],&resE_error_675[0]);
    grRes_675->SetMarkerColor(kGreen-5);
    grRes_675->SetMarkerStyle(21);
    grRes_675->SetMarkerSize(1.3);
    grRes_675->Draw("same,p");

    TGraphErrors *grRes_1836 = new TGraphErrors(n,&rate[0],&resE_1836[0], &rate_error[0],&resE_error_1836[0]);
    grRes_1836->SetMarkerColor(kPink+10);
    grRes_1836->SetMarkerStyle(20);
    grRes_1836->SetMarkerSize(1.3);
    grRes_1836->Draw("same,p");

    TGraphErrors *grRes_511 = new TGraphErrors(n,&rate[0],&resE_511[0], &rate_error[0],&resE_error_511[0]);
    grRes_511->SetMarkerColor(kBlue);
    grRes_511->SetMarkerStyle(22);
    grRes_511->SetMarkerSize(1.3);
    grRes_511->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.045);
    latex.DrawLatex(.2,.005,"M=400, L=200");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grRes_511,"E = 511 keV","p");
    legend->AddEntry(grRes_675,"E = 675 keV","p");
    legend->AddEntry(grRes_1836,"E = 1836 keV","p");
    legend->Draw("same");

  }




  //noiseSD=10, sigma_noise=0.17
  //noiseSD=13, sigma_noise=0.29
  //noiseSD=20, sigma_noise=0.50
  //noiseSD=50, sigma_noise=1.10                                                                                                                                              

  if(Noise_E675==true){

    const Int_t n=13;
    
    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    //E=675 keV noiseSD=10
    Double_t resADC10_675[13]={0.292,0.375,0.395,0.4226,0.565,0.747,0.952,1.548,1.920,2.252,2.718,3.152,3.459};
    Double_t resADC10_error_675[13]={0.007,0.004,0.003,0.0021,0.003,0.003,0.004,0.005,0.005,0.005,0.005,0.006,0.006};

    //E=675 keV noiseSD=13 
    Double_t resADC13_675[13]={0.3923,0.454,0.472,0.494,0.629,0.803,0.992,1.581,1.937,2.275,2.725,3.199,3.480};
    Double_t resADC13_error_675[13]={0.011,0.004,0.003,0.003,0.003,0.003,0.004,0.005,0.005,0.005,0.005,0.006,0.006};

    //E=675 keV noiseSD=20
    Double_t resADC20_675[13]={0.627,0.673,0.680,0.695,0.811,0.955,1.128,1.654,1.988,2.306,2.773,3.232,3.504};
    Double_t resADC20_error_675[13]={0.016,0.007,0.005,0.004,0.004,0.004,0.004,0.005,0.005,0.005,0.005,0.006,0.006};

    //E=675 keV noiseSD=50
    Double_t resADC50_675[13]={1.45,1.607,1.628,1.665,1.735,1.806,1.910,2.241,2.488,2.756,3.146,3.533,3.830};
    Double_t resADC50_error_675[13]={0.04,0.016,0.011,0.008,0.007,0.007,0.006,0.006,0.006,0.006,0.006,0.006,0.006};



    TGraphErrors *grnoise10 = new TGraphErrors(n,&rate[0],&resADC10_675[0],&rate_error[0],&resADC10_error_675[0]);
    grnoise10->SetMarkerColor(kAzure-7);
    grnoise10->SetMarkerStyle(20);
    grnoise10->SetMarkerSize(1.3);
    grnoise10->Draw("same,p");

    TGraphErrors *grnoise13 = new TGraphErrors(n,&rate[0],&resADC13_675[0],&rate_error[0],&resADC13_error_675[0]);
    grnoise13->SetMarkerColor(kTeal-7);
    grnoise13->SetMarkerStyle(20);
    grnoise13->SetMarkerSize(1.3);
    grnoise13->Draw("same,p");

    TGraphErrors *grnoise20 = new TGraphErrors(n,&rate[0],&resADC20_675[0],&rate_error[0],&resADC20_error_675[0]);
    grnoise20->SetMarkerColor(kOrange-2);
    grnoise20->SetMarkerStyle(20);
    grnoise20->SetMarkerSize(1.3);
    grnoise20->Draw("same,p");

    TGraphErrors *grnoise50 = new TGraphErrors(n,&rate[0],&resADC50_675[0],&rate_error[0],&resADC50_error_675[0]);
    grnoise50->SetMarkerColor(kOrange-7);
    grnoise50->SetMarkerStyle(20);
    grnoise50->SetMarkerSize(1.3);
    grnoise50->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(.2,.9,"M=400, L=200");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grnoise10,"#sigma_{noise} = 0.17 mV","p");
    legend->AddEntry(grnoise13,"#sigma_{noise} = 0.29 mV","p");
    legend->AddEntry(grnoise20,"#sigma_{noise} = 0.50 mV","p");
    legend->AddEntry(grnoise50,"#sigma_{noise} = 1.10 mV","p");
    legend->Draw("same");


  }




  if(GausNoise_E675==true){

    const Int_t n=13;

    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    //E=675 keV noiseSD=0.17 mV
    Double_t resADC10_675[13]={0.336,0.403,0.433,0.4579,0.593,0.773,0.968,1.571,1.924,2.254,2.737,3.165,3.445};
    Double_t resADC10_error_675[13]={0.008,0.004,0.003,0.0022,0.003,0.003,0.004,0.005,0.005,0.005,0.005,0.006,0.006};

    //E=675 keV noiseSD=0.29 mV
    Double_t resADC13_675[13]={0.580,0.640,0.656,0.685,0.788,0.942,1.104,1.642,1.989,2.317,2.767,3.204,3.512};
    Double_t resADC13_error_675[13]={0.014,0.006,0.005,0.004,0.004,0.004,0.004,0.005,0.005,0.005,0.005,0.006,0.006};

    //E=675 keV noiseSD=0.50 mV
    Double_t resADC20_675[13]={1.06,1.089,1.120,1.136,1.211,1.322,1.455,1.891,2.180,2.466,2.912,3.317,3.625};
    Double_t resADC20_error_675[13]={0.03,0.011,0.008,0.006,0.006,0.005,0.005,0.005,0.005,0.005,0.006,0.006,0.006};

    //E=675 keV noiseSD=1.10 mV
    Double_t resADC50_675[13]={2.27,2.344,2.371,2.394,2.45,2.500,2.566,2.822,3.037,3.258,3.612,3.967,4.191};
    Double_t resADC50_error_675[13]={0.06,0.024,0.016,0.012,0.01,0.009,0.008,0.007,0.007,0.007,0.007,0.007,0.007};



    TGraphErrors *grnoise10 = new TGraphErrors(n,&rate[0],&resADC10_675[0],&rate_error[0],&resADC10_error_675[0]);
    grnoise10->SetMarkerColor(kAzure-7);
    grnoise10->SetMarkerStyle(20);
    grnoise10->SetMarkerSize(1.3);
    grnoise10->Draw("same,p");

    TGraphErrors *grnoise13 = new TGraphErrors(n,&rate[0],&resADC13_675[0],&rate_error[0],&resADC13_error_675[0]);
    grnoise13->SetMarkerColor(kTeal-7);
    grnoise13->SetMarkerStyle(20);
    grnoise13->SetMarkerSize(1.3);
    grnoise13->Draw("same,p");

    TGraphErrors *grnoise20 = new TGraphErrors(n,&rate[0],&resADC20_675[0],&rate_error[0],&resADC20_error_675[0]);
    grnoise20->SetMarkerColor(kOrange-2);
    grnoise20->SetMarkerStyle(20);
    grnoise20->SetMarkerSize(1.3);
    grnoise20->Draw("same,p");

    TGraphErrors *grnoise50 = new TGraphErrors(n,&rate[0],&resADC50_675[0],&rate_error[0],&resADC50_error_675[0]);
    grnoise50->SetMarkerColor(kOrange-7);
    grnoise50->SetMarkerStyle(20);
    grnoise50->SetMarkerSize(1.3);
    grnoise50->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(.2,.9,"M=400, L=200");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grnoise10,"#sigma_{noise} = 0.17 mV","p");
    legend->AddEntry(grnoise13,"#sigma_{noise} = 0.29 mV","p");
    legend->AddEntry(grnoise20,"#sigma_{noise} = 0.50 mV","p");
    legend->AddEntry(grnoise50,"#sigma_{noise} = 1.10 mV","p");
    legend->Draw("same");


  }



}
