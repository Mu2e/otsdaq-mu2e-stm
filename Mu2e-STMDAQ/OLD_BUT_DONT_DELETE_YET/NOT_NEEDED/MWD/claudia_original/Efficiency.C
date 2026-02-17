//Efficiency reconstructing energy peaks
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

void Efficiency() {
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  //auto c1= new TCanvas("c1","Title",200,10,700,600);
  //c1->SetGrid();
  gROOT->SetStyle("ATLAS");
  auto c1= new TCanvas("c1");     
 
  double xx1[2]={0,1600};
  double yy1[2]={0,1.05};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("Efficiency");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  const Int_t n=12;
  double x[n],y[n],ex[n],ey[n];

  double counts[n], ecounts[n], countstheory[n], ecountstheory[n];
    
  //Energies
  x[0]=40.1186;
  x[1]=121.78;
  x[2]=244.7;
  x[3]=344.28;
  x[4]=411.11;
  x[5]=443.97;
  x[6]=778.91;
  x[7]=867.38;
  x[8]=964.08;
  x[9]=1085.837;
  x[10]=1112.076;
  x[11]=1408.013;

   
  ex[0]=0;
  ex[1]=0;
  ex[2]=0;
  ex[3]=0;
  ex[4]=0;
  ex[5]=0;
  ex[6]=0;
  ex[7]=0;
  ex[8]=0;
  ex[9]=0;
  ex[10]=0;
  ex[11]=0;
  
  //Counts
  counts[0]=12453.4;
  counts[1]=5747.65;
  counts[2]=1139.55;
  counts[3]=3265.35;
  counts[4]=241.217;
  counts[5]=306.191;
  counts[6]=957.832;
  counts[7]=344.388;
  counts[8]=1011.66;
  counts[9]=746.029;
  counts[10]=890.824;
  counts[11]=1032.52;

  for(int i = 0; i<n; i++){
    ecounts[i]=sqrt(counts[i]);
  }

  countstheory[0]=37.7;
  countstheory[1]=28.41;
  countstheory[2]=7.55;
  countstheory[3]=26.59;
  countstheory[4]=2.238;
  countstheory[5]=2.8;
  countstheory[6]=12.97;
  countstheory[7]=4.243;
  countstheory[8]=14.5;
  countstheory[9]=10.13;
  countstheory[10]=13.41;
  countstheory[11]=20.85;

  ecountstheory[0]=0.5;
  ecountstheory[1]=0.13;
  ecountstheory[2]=0.04;
  ecountstheory[3]=0.12;
  ecountstheory[4]=0.01;
  ecountstheory[5]=0.2;
  ecountstheory[6]=0.06;
  ecountstheory[7]=0.023;
  ecountstheory[8]=0.06;
  ecountstheory[9]=0.06;
  ecountstheory[10]=0.06;
  ecountstheory[11]=0.08;
  
  double Nref = counts[0];
  double Nthref = countstheory[0];
  double Nthref_error = ecountstheory[0];
  
  for(int i=0; i<n; i++){
    std::cout<<counts[i]<<"/"<<Nref<<" / "<<countstheory[i]<<"/"<<Nthref<<std::endl;
    y[i] = (counts[i]/Nref) / (countstheory[i]/Nthref);
    ey[i] = y[i]*(sqrt((1/counts[i])+(1/Nref)+ (Nthref_error/Nthref)*(Nthref_error/Nthref) + (ecountstheory[i]/countstheory[i])*(ecountstheory[i]/countstheory[i])));
  }
  
  //Efficiency
  auto ge= new TGraphErrors(n,x,y,ex,ey);
  ge->SetMarkerStyle(20);
  ge->SetMarkerColor(kBlue);
  ge->Draw("same,p");

  TF1*FitEff_data = new TF1("FitEff_data", "[0]+[1]*exp(x/[2])", 0, 2000);
  FitEff_data->SetParameters(1.018,16.17,-163.78);
  FitEff_data->SetLineColor(kBlue);
  FitEff_data->SetLineStyle(2);
  //ge->Fit(FitEff_data);
  //FitEff_data->Draw("same");
 
 
  //GEANT
  const Int_t ng=11;
  long int photonslaunched;
  photonslaunched=50000;

  Double_t efficiency[ng],efficiencyerror[ng],resolEtrue[ng],resolEreco[ng],resolEreco_error[ng];

  double energy[11]={40,100,300,500,662,800,1000,2000,4000,8000,10000};
  double energy_error[11]={0,0,0,0,0,0,0,0,0,0,0};

  double resolution[11]={31.5,56.03,98.9,128.5,147.38,162.4,182,255.13,365.7,519.14,567.34};
  double resolution_error[11]={0.1,0.18,0.3,0.5,0.6,0.7,0.9,1.32,2.9,10.5,18.5};

  double countspeak[11]={49780,48849,43910,38189,35063,32371,29707,19383,8435,1639,741};

  double ehtrue[11]={9729,29931,97271,164611,219157,265621,332961,669662,1343062,2689864,3363264};
  double ehreco[11]={9730,29932,97271,164613,219158,265622,332962,669661,1343060,2689850,3363240};
  double ehreco_error[11]={0.14,0.3,0.5,0.7,0.8,0.9,1.1,1.84,4.04,13.37,22.41};

  double energyrec[11]={28.8957,88.902,288.955,488.89,650.891,788.897,988.89,1988.9,3988.9,7988.89,9988.90};
  double energyrec_error[11]={0.00003,0.002,0.005,0.00023,0.00009,0.021,0.00004,0.00004,0.00004,0.0014,0.00006};


  for(int i=0;i<ng;i++){
    efficiency[i]=countspeak[i]/photonslaunched;
    efficiencyerror[i]=efficiency[i]/sqrt(countspeak[i]);

    resolEtrue[i]=resolution[i]/energy[i];
    //cout<<i<<" "<<resolEtrue[i]<<endl;                                                                                               
    resolEreco[i]=resolution[i]/energyrec[i];
    resolEreco_error[i]=(resolution[i]/energyrec[i])*sqrt(((resolution_error[i]/resolution[i])*(resolution_error[i]/resolution[i]))+((energyrec_error[i]/energyrec[i])*(energyrec_error[i]/energyrec[i])));
  }

  //Energy-peak absolute Efficiency
  auto g2= new TGraphErrors(ng,energy,efficiency,energy_error,efficiencyerror);

  g2->SetMarkerStyle(20);
  g2->SetMarkerColor(kRed-4);
  g2->SetTitle("");
  //g2->Draw("same,p");


  TF1*FitEff = new TF1("FitEff", "[0]+[1]*exp(x/[2])", 0, 10100);
  FitEff->SetParameters(1.018,16.17,-163.78);
  FitEff->SetLineColor(kRed);
  FitEff->SetLineStyle(2);
  //g2->Fit(FitEff);
  //FitEff->Draw("same");
  
  ge->Print();
  g2->Print();

  auto leg1 = new TLegend(0.6,0.75,0.85,0.9);
  leg1->AddEntry(ge, "Data","p");
  leg1->AddEntry(g2, "Simulation","p");
  //leg1->Draw("same");

  c1->Print("DataSimulationEfficiencyData.png");

}
