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

void MWDEfficiencyRates() {
  gROOT->SetStyle("ATLAS");

  double xx1[2]={0,200};
  double yy1[2]={0.8,1.05};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("MWD Efficiency");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  //Energy: 675 keV, 1836 keV or 511 keV
  bool E675=false;
  bool E1836=false;
  bool E511=false;
  bool E_allEnergies=false;
  bool Noise_E675=false;
  bool GausNoise_E675=true;


  if(E675==true){
  const Int_t n=13;
  Double_t effMWDrawM1000L500[n],effMWDrawM500L200[n],effMWDrawM400L200[n];
  Double_t effMWDrawM1000L500_error[n], effMWDrawM500L200_error[n],effMWDrawM400L200_error[n];
  Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
  Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
  //Double_t Npeaksoriginal[13]={1000,5000,10000,20000,30000,40000,49999,80000,99999,120000,150000,180000,199999};
  Double_t Npeaksoriginal[13]={1000,4996,9984,19997,29978,39987,49999,79973,99953,120000,149999,180000,199634};

  //Using Poisson distribution for the simulated data
  Double_t NpeaksrawMWDM1000L500[13]={1000,4996,9984,19996,29978,39987,49998,79416, 95998,106269,105755,90986,76205};
  Double_t NpeaksrawMWDM500L200[13]={1000,4996,9984,19997,29978,39987,49999, 79938, 99607, 118441, 142831, 160450, 167340};
  Double_t NpeaksrawMWDM400L200[13]={1000,4996,9984,19997, 29978, 39987, 49999, 79961, 99845, 119436, 147070, 170955, 184105};

 
  for(int i=0;i<n;i++){
    effMWDrawM1000L500[i]=NpeaksrawMWDM1000L500[i]/Npeaksoriginal[i];
    effMWDrawM500L200[i]=NpeaksrawMWDM500L200[i]/Npeaksoriginal[i];
    effMWDrawM400L200[i]=NpeaksrawMWDM400L200[i]/Npeaksoriginal[i];
    std::cout<<effMWDrawM1000L500[i]<<" "<<effMWDrawM500L200[i]<<" "<<effMWDrawM400L200[i]<<std::endl;

    effMWDrawM1000L500_error[i]=effMWDrawM1000L500[i]/sqrt(NpeaksrawMWDM1000L500[i]);
    effMWDrawM500L200_error[i]=effMWDrawM500L200[i]/sqrt(NpeaksrawMWDM500L200[i]);
    effMWDrawM400L200_error[i]=effMWDrawM400L200[i]/sqrt(NpeaksrawMWDM400L200[i]);    
  }




  TGraphErrors *greffrawM1000L500 = new TGraphErrors(n,&rate[0],&effMWDrawM1000L500[0],&rate_error[0],&effMWDrawM1000L500_error[0]);
  greffrawM1000L500->SetMarkerColor(kBlack);
  greffrawM1000L500->Draw("same,p");

  TGraphErrors *greffrawM500L200 = new TGraphErrors(n,&rate[0],&effMWDrawM500L200[0],&rate_error[0],&effMWDrawM500L200_error[0]);
  greffrawM500L200->SetMarkerColor(kCyan-7);
  greffrawM500L200->Draw("same,p");

  TGraphErrors *greffrawM400L200 = new TGraphErrors(n,&rate[0],&effMWDrawM400L200[0],&rate_error[0],&effMWDrawM400L200_error[0]);
  greffrawM400L200->SetMarkerColor(kViolet+2);
  greffrawM400L200->Draw("same,p");

  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(greffrawM1000L500,"M=1000, L=500","p"); 
  legend->AddEntry(greffrawM500L200,"M=500, L=200","p");
  legend->AddEntry(greffrawM400L200,"M=400, L=200","p");
  legend->Draw("same");
  }





  if(E1836==true){

    const Int_t n=13;
    Double_t effMWDrawM400L200[n];
    Double_t effMWDrawM400L200_error[n];
    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
    //Double_t Npeaksoriginal[13]={1000,5000,10000,20000,30000,40000,49999,80000,99999,120000,150000,180000,199999};
    Double_t Npeaksoriginal[13]={999,5000,9991,20000,29979,39950,49999,80000,99942,120000,149776,179788,199882};

    //Using Poisson distribution for the simulated data
    Double_t NpeaksrawMWDM400L200[13]={999,4951,9907,20000,29978,39924,49852,79968,99605,118316,142676,160000,167346};

    for(int i=0;i<n;i++){
      effMWDrawM400L200[i]=NpeaksrawMWDM400L200[i]/Npeaksoriginal[i];
      std::cout<<effMWDrawM400L200[i]<<std::endl;

      effMWDrawM400L200_error[i]=effMWDrawM400L200[i]/sqrt(NpeaksrawMWDM400L200[i]);
    }


    TGraphErrors *greffrawM400L200 = new TGraphErrors(n,&rate[0],&effMWDrawM400L200[0],&rate_error[0],&effMWDrawM400L200_error[0]);
    greffrawM400L200->SetMarkerColor(kViolet+2);
    greffrawM400L200->Draw("same,p");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(greffrawM400L200,"M=400, L=200","p");
    legend->Draw("same");

}




  if(E511==true){

    const Int_t n=13;
    Double_t effMWDrawM400L200[n];
    Double_t effMWDrawM400L200_error[n];
    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
    //Double_t Npeaksoriginal[13]={1000,5000,10000,20000,30000,40000,49999,80000,99999,120000,150000,180000,199999};
    Double_t Npeaksoriginal[13]={999,4997,9990,20000,29995,40000,49999,79880,99875,120000,150000,179922,199999};

    //Using Poisson distribution for the simulated data
    Double_t NpeaksrawMWDM400L200[13]={999,4993,9989,20000,29992,40000,49999,79864,99774,119423,147097,170739,183942};

    for(int i=0;i<n;i++){
      effMWDrawM400L200[i]=NpeaksrawMWDM400L200[i]/Npeaksoriginal[i];
      std::cout<<effMWDrawM400L200[i]<<std::endl;

      effMWDrawM400L200_error[i]=effMWDrawM400L200[i]/sqrt(NpeaksrawMWDM400L200[i]);
    }


    TGraphErrors *greffrawM400L200 = new TGraphErrors(n,&rate[0],&effMWDrawM400L200[0],&rate_error[0],&effMWDrawM400L200_error[0]);
    greffrawM400L200->SetMarkerColor(kViolet+2);
    greffrawM400L200->Draw("same,p");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(greffrawM400L200,"M=400, L=200","p");
    legend->Draw("same");

  }





  if(E_allEnergies==true){

    const Int_t n=13;

    Double_t effMWDrawM400L200_675[n],effMWDrawM400L200_1836[n],effMWDrawM400L200_511[n];
    Double_t effMWDrawM400L200_error_675[n],effMWDrawM400L200_error_1836[n],effMWDrawM400L200_error_511[n];

    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    //E=675 keV
    Double_t Npeaksoriginal_675[13]={1000,4996,9984,19997,29978,39987,49999,79973,99953,120000,149999,180000,199634};
    Double_t NpeaksrawMWDM400L200_675[13]={1000,4996,9984,19997, 29978, 39987, 49999, 79961, 99845, 119436, 147070, 170955, 184105};

    //E=1836 keV
    Double_t Npeaksoriginal_1836[13]={999,5000,9991,20000,29979,39950,49999,80000,99942,120000,149776,179788,199882};
    Double_t NpeaksrawMWDM400L200_1836[13]={999,4951,9907,20000,29978,39924,49852,79968,99605,118316,142676,160000,167346};

    //E=511 keV
    Double_t Npeaksoriginal_511[13]={999,4997,9990,20000,29995,40000,49999,79880,99875,120000,150000,179922,199999};
    Double_t NpeaksrawMWDM400L200_511[13]={999,4993,9989,20000,29992,40000,49999,79864,99774,119423,147097,170739,183942};




    for(int i=0;i<n;i++){
      effMWDrawM400L200_675[i]=NpeaksrawMWDM400L200_675[i]/Npeaksoriginal_675[i];
      effMWDrawM400L200_1836[i]=NpeaksrawMWDM400L200_1836[i]/Npeaksoriginal_1836[i];
      effMWDrawM400L200_511[i]=NpeaksrawMWDM400L200_511[i]/Npeaksoriginal_511[i];

      effMWDrawM400L200_error_675[i]=effMWDrawM400L200_675[i]/sqrt(NpeaksrawMWDM400L200_675[i]);
      effMWDrawM400L200_error_1836[i]=effMWDrawM400L200_1836[i]/sqrt(NpeaksrawMWDM400L200_1836[i]);
      effMWDrawM400L200_error_511[i]=effMWDrawM400L200_511[i]/sqrt(NpeaksrawMWDM400L200_511[i]);
    }




    TGraphErrors *greff_675 = new TGraphErrors(n,&rate[0],&effMWDrawM400L200_675[0],&rate_error[0],&effMWDrawM400L200_error_675[0]);
    greff_675->SetMarkerColor(kGreen-5);
    greff_675->SetMarkerStyle(21);
    greff_675->SetMarkerSize(1.3);
    greff_675->Draw("same,p");

    TGraphErrors *greff_1836 = new TGraphErrors(n,&rate[0],&effMWDrawM400L200_1836[0],&rate_error[0],&effMWDrawM400L200_error_1836[0]);
    greff_1836->SetMarkerColor(kPink+10);
    greff_1836->SetMarkerStyle(20);
    greff_1836->SetMarkerSize(1.3);
    greff_1836->Draw("same,p");

    TGraphErrors *greff_511 = new TGraphErrors(n,&rate[0],&effMWDrawM400L200_511[0],&rate_error[0],&effMWDrawM400L200_error_511[0]);
    greff_511->SetMarkerColor(kBlue);
    greff_511->SetMarkerStyle(22);
    greff_511->SetMarkerSize(1.3);
    greff_511->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(.2,.9,"M=400, L=200");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(greff_511,"E = 511 keV","p");
    legend->AddEntry(greff_675,"E = 675 keV","p");
    legend->AddEntry(greff_1836,"E = 1836 keV","p");
    legend->Draw("same");


}





  //noiseSD=10, sigma_noise=0.17
  //noiseSD=13, sigma_noise=0.29
  //noiseSD=20, sigma_noise=0.50
  //noiseSD=50, sigma_noise=1.10

  if(Noise_E675==true){

    const Int_t n=13;

    Double_t effADC10_675[n],effADC13_675[n],effADC20_675[n],effADC50_675[n];
    Double_t effADC10_error_675[n],effADC13_error_675[n],effADC20_error_675[n],effADC50_error_675[n];

    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    //E=675 keV noiseSD=10
    Double_t NpeaksoriginalADC10_675[13]={1000,4996,9984,19997,29978,39987,49999,79973,99953,120000,149999,180000,199634};
    Double_t NpeaksrawADC10_675[13]={1000,4996,9984,19997,29978,39987,49999,79961,99845,119436,147070,170955,184105};

    //E=675 keV noiseSD=13
    Double_t NpeaksoriginalADC13_675[13]={1000,5000,9998,20000,29979,40000,49966,80000,99887,119768,149883,179981,199999};
    Double_t NpeaksrawADC13_675[13]={1000,5000,9997,20000,29976,40000,49961,79988,99785,119209,146968,170839,183954};

    //E=675 keV noiseSD=20
    Double_t NpeaksoriginalADC20_675[13]={1000,4999,10000,20000,30000,39971,49999,80000,99902,120000,149987,180000,199957};
    Double_t NpeaksrawADC20_675[13]={999,4998,10000,19999,30000,39966,49999,79996,99785,119440,147208,171020,184267};

    //E=675 keV noiseSD=50  
    Double_t NpeaksoriginalADC50_675[13]={1000,4994,10000,20000,29983,39976,49999,79949,99999,120000,149912,180000,199999};
    Double_t NpeaksrawADC50_675[13]={1000,4993,10000,20000,29980,39972,49999,79931,99905,119414,147051,170910,183911};



    for(int i=0;i<n;i++){
      effADC10_675[i]=NpeaksrawADC10_675[i]/NpeaksoriginalADC10_675[i];
      effADC13_675[i]=NpeaksrawADC13_675[i]/NpeaksoriginalADC13_675[i];
      effADC20_675[i]=NpeaksrawADC20_675[i]/NpeaksoriginalADC20_675[i];
      effADC50_675[i]=NpeaksrawADC50_675[i]/NpeaksoriginalADC50_675[i];

      effADC10_error_675[i]=effADC10_675[i]/sqrt(NpeaksrawADC10_675[i]);
      effADC13_error_675[i]=effADC13_675[i]/sqrt(NpeaksrawADC13_675[i]);
      effADC20_error_675[i]=effADC20_675[i]/sqrt(NpeaksrawADC20_675[i]);
      effADC50_error_675[i]=effADC50_675[i]/sqrt(NpeaksrawADC50_675[i]);
    }



    TGraphErrors *grnoise10 = new TGraphErrors(n,&rate[0],&effADC10_675[0],&rate_error[0],&effADC10_error_675[0]);
    grnoise10->SetMarkerColor(kAzure-7);
    grnoise10->SetMarkerStyle(20);
    grnoise10->SetMarkerSize(1.3);
    grnoise10->Draw("same,p");

    TGraphErrors *grnoise13 = new TGraphErrors(n,&rate[0],&effADC13_675[0],&rate_error[0],&effADC13_error_675[0]);
    grnoise13->SetMarkerColor(kTeal-7);
    grnoise13->SetMarkerStyle(20);
    grnoise13->SetMarkerSize(1.3);
    grnoise13->Draw("same,p");

    TGraphErrors *grnoise20 = new TGraphErrors(n,&rate[0],&effADC20_675[0],&rate_error[0],&effADC20_error_675[0]);
    grnoise20->SetMarkerColor(kOrange-2);
    grnoise20->SetMarkerStyle(20);
    grnoise20->SetMarkerSize(1.3);
    grnoise20->Draw("same,p");

    TGraphErrors *grnoise50 = new TGraphErrors(n,&rate[0],&effADC50_675[0],&rate_error[0],&effADC50_error_675[0]);
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

    Double_t effADC10_675[n],effADC13_675[n],effADC20_675[n],effADC50_675[n];
    Double_t effADC10_error_675[n],effADC13_error_675[n],effADC20_error_675[n],effADC50_error_675[n];

    Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};

    //E=675 keV noiseSD=0.17 mV
    Double_t NpeaksoriginalADC10_675[13]={1000,5000,9992,19986,29967,39986,49999,79993,99999,120000,150000,180000,199934};
    Double_t NpeaksrawADC10_675[13]={1000,5000,9991,19984,29965,39983,49999,79977,99899,119458,146953,171268,184330};

    //E=675 keV noiseSD=0.29 mV
    Double_t NpeaksoriginalADC13_675[13]={1000,5000,9995,19995,30000,39995,49999,79927,99999,119951,150000,179899,199999};
    Double_t NpeaksrawADC13_675[13]={1000,5000,9994,19992,29997,39991,49999,79916,99911,119400,147163,170826,183875};

    //E=675 keV noiseSD=0.50
    Double_t NpeaksoriginalADC20_675[13]={999,5000,10000,20000,29991,39942,49999,80000,99999,119886,150000,180000,199999};
    Double_t NpeaksrawADC20_675[13]={999,4999,10000,20000,29989,39938,49998,79994,99893,119326,147160,171042,184095};

    //E=675 keV noiseSD=1.10
    Double_t NpeaksoriginalADC50_675[13]={1000,4998,9994,20000,30000,40000,49996,80000,99855,119832,150000,180000,199570};
    Double_t NpeaksrawADC50_675[13]={1000,4994,9993,19996,29997,40000,49982,79920,99747,119288,147197,170753,183990};



    for(int i=0;i<n;i++){
      effADC10_675[i]=NpeaksrawADC10_675[i]/NpeaksoriginalADC10_675[i];
      effADC13_675[i]=NpeaksrawADC13_675[i]/NpeaksoriginalADC13_675[i];
      effADC20_675[i]=NpeaksrawADC20_675[i]/NpeaksoriginalADC20_675[i];
      effADC50_675[i]=NpeaksrawADC50_675[i]/NpeaksoriginalADC50_675[i];

      effADC10_error_675[i]=effADC10_675[i]/sqrt(NpeaksrawADC10_675[i]);
      effADC13_error_675[i]=effADC13_675[i]/sqrt(NpeaksrawADC13_675[i]);
      effADC20_error_675[i]=effADC20_675[i]/sqrt(NpeaksrawADC20_675[i]);
      effADC50_error_675[i]=effADC50_675[i]/sqrt(NpeaksrawADC50_675[i]);
    }



    TGraphErrors *grnoise10 = new TGraphErrors(n,&rate[0],&effADC10_675[0],&rate_error[0],&effADC10_error_675[0]);
    grnoise10->SetMarkerColor(kAzure-7);
    grnoise10->SetMarkerStyle(20);
    grnoise10->SetMarkerSize(1.3);
    grnoise10->Draw("same,p");

    TGraphErrors *grnoise13 = new TGraphErrors(n,&rate[0],&effADC13_675[0],&rate_error[0],&effADC13_error_675[0]);
    grnoise13->SetMarkerColor(kTeal-7);
    grnoise13->SetMarkerStyle(20);
    grnoise13->SetMarkerSize(1.3);
    grnoise13->Draw("same,p");

    TGraphErrors *grnoise20 = new TGraphErrors(n,&rate[0],&effADC20_675[0],&rate_error[0],&effADC20_error_675[0]);
    grnoise20->SetMarkerColor(kOrange-2);
    grnoise20->SetMarkerStyle(20);
    grnoise20->SetMarkerSize(1.3);
    grnoise20->Draw("same,p");

    TGraphErrors *grnoise50 = new TGraphErrors(n,&rate[0],&effADC50_675[0],&rate_error[0],&effADC50_error_675[0]);
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
