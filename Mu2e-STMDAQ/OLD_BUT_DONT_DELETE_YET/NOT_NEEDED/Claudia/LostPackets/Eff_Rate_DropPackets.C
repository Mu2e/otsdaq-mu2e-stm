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

void Eff_Rate_DropPackets() {
  gROOT->SetStyle("ATLAS");

  double xx1[2]={0,100};
  double yy1[2]={0,1};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Dropped Packets [%]");
  graph1->GetYaxis()->SetTitle("MWD Efficiency");
  //graph1->GetYaxis()->SetTitle("Resolution [keV]");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  //Prob
  /*const Int_t n=10;
  Double_t eff[n],eff_error[n];
  Double_t dropPackets[10]={0,9.98338,19.8205,30.2613,40.0972,49.9406,60.1102,70.0386,79.9345,89.9566};
  Double_t dropPackets_error[10]={0,0,0,0,0,0,0,0,0,0};
  Double_t Npeaksoriginal=1000;
  Double_t Npeaksreco[10]={1000,882,759,666,552,502,379,274,173,91};
  Double_t resolution[10]={0.284,0.295,0.301, 0.31, 0.30, 0.31, 0.295,0.280,0.297,0.38 };
  Double_t resolution_error[10]={0.006 ,0.006 ,0.009,0.01,0.01,0.01,0.012, 0.014,0.019,0.04};
  */

  //Frac
  const Int_t n=11;
  Double_t eff[n],eff_error[n];
  Double_t dropPackets[11]={0,9.99963,19.9993,24.9997,33.3333,39.9985 ,49.9994,59.9978,69.9974,79.997,89.9966};
  Double_t dropPackets_error[11]={0,0,0,0,0,0,0,0,0,0,0};
  Double_t Npeaksoriginal=1000;
  Double_t Npeaksreco[11]={1000,885,779,735,637, 552,439,354,264,170,79};
  Double_t resolution[11]={0.284,0.290,0.293,0.291,0.304,0.300,0.308,0.299,0.278,0.328,0.33};
  Double_t resolution_error[11]={0.006,0.007, 0.007,0.008,0.009,0.009,0.011,0.011,0.016,0.024,0.05};
 



   for(int i=0;i<n;i++){
    eff[i]=Npeaksreco[i]/Npeaksoriginal;
    eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
    std::cout<<eff[i]<<" "<<eff_error[i]<<std::endl;
   }


   TLatex latex;
   latex.SetTextSize(0.06);
   latex.DrawLatex(5,0.6,"1 kHz");


  TGraphErrors *greff = new TGraphErrors(n,&dropPackets[0],&eff[0],&dropPackets_error[0],&eff_error[0]);
  greff->SetMarkerColor(kViolet+1);
  greff->Draw("same,p");
   
   /*TGraphErrors *grres = new TGraphErrors(n,&dropPackets[0],&resolution[0],&dropPackets_error[0],&resolution_error[0]);
  grres->SetMarkerColor(kTeal+1);
  grres->Draw("same,p");*/
  



  /*auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(,"","p");
  legend->Draw("same");*/
}
