#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"
#include "TMinuit.h"
#include "TRandom3.h"
#include "TRandom.h"
#include "TMatrixD.h"
#include "Math/Util.h"
#include "TStyle.h"
#include "TPad.h"
#include "TPaveStats.h"
#include "TTree.h"
#include "TBranch.h"
#include "TSystem.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include <iomanip>


void Plot_RecoSignalSampled_FittedHisto(){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  TCanvas *c = new TCanvas("");
  double DeltaS[9], StdevSreco[9];


  //1kHz 1keV
  DeltaS[0]= 0.203;
  StdevSreco[0]= 0.517;
  
  //1kHz 2keV 
  DeltaS[1]= 0.082;
  StdevSreco[1]= 0.611;
  
  //1kHz 3keV
  DeltaS[2]= -0.125;
  StdevSreco[2]= 0.713;
    
  //50kHz 1keV
  DeltaS[3]= 0.074;
  StdevSreco[3]= 0.934;
  
  //50kHz 2keV
  DeltaS[4]= 0.066;
  StdevSreco[4]= 0.942;
    
  //50kHz 3keV
  DeltaS[5]= -0.016;
  StdevSreco[5]= 0.972;
  
  //200kHz 1keV
  DeltaS[6]= -0.023;
  StdevSreco[6]= 0.961; 

  //200kHz 2keV 
  DeltaS[7]= -0.123;
  StdevSreco[7]= 0.911;

  //200kHz 3keV
  DeltaS[8]= -0.026;
  StdevSreco[8]= 1.009;

   
  TGraphErrors* gr[9];
  int Npoints = 1;
  int Nresol = 9;

  double y[1];
  double x[1];
  double ex[1];
  double ey[1];
  int n_graphs = 0;
  
    //9 combination of resols
    for(int k =0 ; k <Nresol ; k++){
      if(n_graphs<3){ x[0] = k+1; std::cout<<""<<std::endl; std::cout<<"RATE: 1 kHz"<<std::endl;}
      else if(n_graphs<6){x[0] = k+2; std::cout<<""<<std::endl; std::cout<<"RATE: 50 kHz"<<std::endl;}
      else{x[0] = k+3; std::cout<<""<<std::endl; std::cout<<"RATE: 200 kHz"<<std::endl;}
 			
      y[0] = DeltaS[k];
      ex[0] = 0;
      ey[0] = StdevSreco[k];
	
      std::cout<<"Resol index: "<<k<<std::endl;
      std::cout<<"DeltaS: "<<DeltaS[k]<<" stddev reco: "<<StdevSreco[k]<<std::endl;
      std::cout<<"Graph value: "<<y[0]<<std::endl;
      gr[n_graphs] = new TGraphErrors(Npoints,x,y,ex,ey);

      if((k==0)||(k==3)||(k==6)){gr[n_graphs]->SetMarkerStyle(20); std::cout<<"Resolution: 1 keV CIRCLE"<<std::endl;}
      if((k==1)||(k==4)||(k==7)){gr[n_graphs]->SetMarkerStyle(21); std::cout<<"Resolution: 2 keV SQUARE"<<std::endl;}
      if((k==2)||(k==5)||(k==8)){gr[n_graphs]->SetMarkerStyle(22); std::cout<<"Resolution: 3 keV TRIANGLE"<<std::endl;}

      gr[n_graphs]->SetMarkerColor(kRed);

      n_graphs++;
    }

 
double xx1[2] = {0, 12};
double yy1[2] = {-4, 6};

  auto gtemp = new TGraph(2,xx1,yy1);
  gtemp->SetMarkerStyle(1);

  gtemp->GetYaxis()->SetTitle("(S_{true}- S_{reco})/#sigma_{S_{reco}}");
  gtemp->GetHistogram()->GetXaxis()->SetLabelSize(0);
  gtemp->GetHistogram()->GetXaxis()->SetTickSize(0);
  
  gtemp->Draw("ap");
 
  for( int i = 0; i < n_graphs; i++ ) {    
    gr[i]->Draw("same,p");
  }


  //TLatex
  TLatex latex1;
  latex1.SetTextSize(0.04);
  latex1.DrawLatex(2,yy1[0]-0.25,"1 kHz");

  TLatex latex2;
  latex2.SetTextSize(0.04);
  latex2.DrawLatex(5,yy1[0]-0.25,"50 kHz");

  TLatex latex3;
  latex3.SetTextSize(0.04);
  latex3.DrawLatex(9,yy1[0]-0.25,"200 kHz");

  TLine* l1 = new TLine(4,yy1[0],4,yy1[1]);
  l1->SetLineStyle(2);
  l1->SetLineColor(kBlack);
  l1->Draw("same");

  TLine* l2 = new TLine(8,yy1[0],8,yy1[1]);
  l2->SetLineStyle(2);
  l2->SetLineColor(kBlack);
  l2->Draw("same");
  
  auto legend = new TLegend(0.65,0.7,0.81,0.9);
  legend->AddEntry(gr[0],"1 keV", "p");
  legend->AddEntry(gr[1],"2 keV","p");
  legend->AddEntry(gr[2],"3 keV","p");
  legend->Draw("same");


  c->Print("Minuit_SampledShapes_FitResults.png");

}
