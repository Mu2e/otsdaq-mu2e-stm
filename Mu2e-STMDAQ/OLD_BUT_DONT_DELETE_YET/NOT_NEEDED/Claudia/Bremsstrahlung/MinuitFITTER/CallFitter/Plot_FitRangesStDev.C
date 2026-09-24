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

#include <iomanip>


void Plot_FitRangesStDev(){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  
  TCanvas *c = new TCanvas("");
  double Strue[9], Sreco[9][4], StdevSreco[9][4];

  //1kHz 1keV
  Strue[0]= 24091;
  Sreco[0][0]=24178.3;
  StdevSreco[0][0]=182.2;
  Sreco[0][1]=24175.8;
  StdevSreco[0][1]=182.3;
  Sreco[0][2]=24126.7;
  StdevSreco[0][2]=182.8;
  Sreco[0][3]=24098.1;
  StdevSreco[0][3]=186.1;

  //1kHz 2keV 
  Strue[1]=23791;
  Sreco[1][0]=23839.5;
  StdevSreco[1][0]=204.3;
  Sreco[1][1]=23831.1;
  StdevSreco[1][1]=204.8;
  Sreco[1][2]=23801.5;
  StdevSreco[1][2]=206.7;
  Sreco[1][3]=23860.2;
  StdevSreco[1][3]=229.5;

  //1kHz 3keV 
  Strue[2]=24001;
  Sreco[2][0]=23917.2;
  StdevSreco[2][0]=222.7;
  Sreco[2][1]=23899.8;
  StdevSreco[2][1]=223.7;
  Sreco[2][2]=23949.7;
  StdevSreco[2][2]=227.8;
  Sreco[2][3]=24128.3;
  StdevSreco[2][3]=280.2;

  //50kHz 1keV 
  Strue[3]=24271;
  Sreco[3][0]=24356.9;
  StdevSreco[3][0]=661.6;
  Sreco[3][1]= 24260.5;
  StdevSreco[3][1]=662.3;
  Sreco[3][2]=24144.2;
  StdevSreco[3][2]=662.1;
  Sreco[3][3]=23827.1;
  StdevSreco[3][3]=684.9;
  
  //50kHz 2keV 
  Strue[4]=23929;
  Sreco[4][0]=22807.3;
  StdevSreco[4][0]=912.3;
  Sreco[4][1]=22620.4;
  StdevSreco[4][1]=913.8;
  Sreco[4][2]=23064.3;
  StdevSreco[4][2]=945.2;
  Sreco[4][3]=23251.5;
  StdevSreco[4][3]=1022.6;

  //50kHz 3keV 
  /*Strue[5]= 23833;
  Sreco[5][0]=20591.6;
  StdevSreco[5][0]=1054.1;
  Sreco[5][1]=20514.8;
  StdevSreco[5][1]=1060.4;
  Sreco[5][2]=20822.1;
  StdevSreco[5][2]=1102.0;
  Sreco[5][3]=20942.8;
  StdevSreco[5][3]=1231.6;
  */

  //50kHz 3keV NEW RUN I
  /*Strue[5]=24136;
  Sreco[5][0]=25703.5;
  StdevSreco[5][0]=1200.46;
  Sreco[5][1]=25731.9;
  StdevSreco[5][1]=1213.5;
  Sreco[5][2]=25987.3;
  StdevSreco[5][2]=1239.36;
  Sreco[5][3]=23614.8;
  StdevSreco[5][3]=1396.76;
  */
  
  //50kHz 3keV NEW RUN II
  Strue[5]=24268;
  Sreco[5][0]=23914.7;
  StdevSreco[5][0]=1115.1;
  Sreco[5][1]=24127.7;
  StdevSreco[5][1]=1130.15;
  Sreco[5][2]=23940.1;
  StdevSreco[5][2]=1164.54;
  Sreco[5][3]=25405.5;
  StdevSreco[5][3]=1419.09;

  
  //200kHz 1keV 
  Strue[6]=23899;
  Sreco[6][0]=22947.7;
  StdevSreco[6][0]=1297.7;
  Sreco[6][1]=22993;
  StdevSreco[6][1]=1303.4;
  Sreco[6][2]=23312.6;
  StdevSreco[6][2]=1328.8;
  Sreco[6][3]=23483.9;
  StdevSreco[6][3]=1381.5;

  //200kHz 2keV 
  Strue[7]=23871;
  Sreco[7][0]=23329.7;
  StdevSreco[7][0]=1831.5;
  Sreco[7][1]=22854.6;
  StdevSreco[7][1]=1826.0;
  Sreco[7][2]=23104.3;
  StdevSreco[7][2]=1851.6;
  Sreco[7][3]=22594.4;
  StdevSreco[7][3]=2000.8;

  //200kHz 3keV
  Strue[8]=23928;
  Sreco[8][0]=25760.9;
  StdevSreco[8][0]=2230.5;
  Sreco[8][1]=26233.7;
  StdevSreco[8][1]=2269.0;
  Sreco[8][2]=25606.2;
  StdevSreco[8][2]=2328.2;
  Sreco[8][3]=28375.4;
  StdevSreco[8][3]=2782.2;

  TGraphErrors* gr[36];
  int Npoints = 1;
  int Nfits = 4;
  int Nresol = 9;

  double y[1];
  double x[1];
  double ex[1];
  double ey[1];
  int n_graphs =0;
  int sumx, aux;
  x[0]=0;
 
    //9 combination of resols
    for(int k =0 ; k <Nresol ; k++){
      //4 fits
      for(int j =0 ; j <Nfits ; j++){
	if(n_graphs<12){sumx=1; std::cout<<""<<std::endl; std::cout<<"RATE: 1 kHz"<<std::endl;}
	else if(n_graphs<24){sumx=12; std::cout<<""<<std::endl; std::cout<<"RATE: 50 kHz"<<std::endl;}
	else{sumx=23; std::cout<<""<<std::endl; std::cout<<"RATE: 200 kHz"<<std::endl;}
	x[0] = 2.5*j+sumx;
 
	if((k==1)||(k==4)||(k==7)){x[0] = x[0]+1;}
	if((k==2)||(k==5)||(k==8)){x[0] = x[0]+2;}
		
	y[0] = (Strue[k] - Sreco[k][j])/StdevSreco[k][j];
	ex[0] = 0;
	ey[0] = 1;
	
	std::cout<<"Resol index: "<<k<<" fit index: "<<j<<std::endl;
	std::cout<<"Strue: "<<Strue[k]<<" Sreco: "<<Sreco[k][j]<<" stddev reco: "<<StdevSreco[k][j]<<std::endl;
	std::cout<<"Graph value: "<<y[0]<<std::endl;
	gr[n_graphs] = new TGraphErrors(Npoints,x,y,ex,ey);

	if((k==0)||(k==3)||(k==6)){gr[n_graphs]->SetMarkerStyle(20); std::cout<<"Resolution: 1 keV CIRCLE"<<std::endl;}
	if((k==1)||(k==4)||(k==7)){gr[n_graphs]->SetMarkerStyle(21); std::cout<<"Resolution: 2 keV SQUARE"<<std::endl;}
	if((k==2)||(k==5)||(k==8)){gr[n_graphs]->SetMarkerStyle(22); std::cout<<"Resolution: 3 keV TRIANGLE"<<std::endl;}

	if(j==0){gr[n_graphs]->SetMarkerColor(kRed); std::cout<<"Fit range 1 RED"<<std::endl;}
	if(j==1){gr[n_graphs]->SetMarkerColor(kGreen-3); std::cout<<"Fit range 2 GREEN"<<std::endl;}
	if(j==2){gr[n_graphs]->SetMarkerColor(kBlue-3); std::cout<<"Fit range 3 BLUE"<<std::endl;}
	if(j==3){gr[n_graphs]->SetMarkerColor(kPink+9); std::cout<<"Fit range 4 PINK"<<std::endl;}

	n_graphs++;
      }
    }
 

 
  TGraph* gr_template;
  double xx1[2] = {0, 30};
  double yy1[2] = {-2, 4};
  gr_template = new TGraph (2,xx1,yy1);
  gr_template->SetMarkerStyle(1);

  gr_template->Draw("ap");
   
  for( int i = 0; i < n_graphs; i++ ) {    
    gr[i]->Draw("same,p");
  }

  
  gr_template->GetYaxis()->SetTitle("(S_{true}- S_{reco})/#sigma_{S_{reco}}");
  gr_template->GetHistogram()->GetXaxis()->SetLabelSize(0);
  gr_template->GetHistogram()->GetXaxis()->SetTickSize(0);

  //TLatex
  TLatex latex1;
  latex1.SetTextSize(0.04);
  latex1.DrawLatex(2,yy1[0]-0.25,"1 kHz");

  TLatex latex2;
  latex2.SetTextSize(0.04);
  latex2.DrawLatex(13,yy1[0]-0.25,"50 kHz");

  TLatex latex3;
  latex3.SetTextSize(0.04);
  latex3.DrawLatex(24,yy1[0]-0.25,"200 kHz");

  TLine* l1 = new TLine(11,yy1[0],11,yy1[1]);
  l1->SetLineStyle(2);
  l1->SetLineColor(kBlack);
  l1->Draw("same");

  TLine* l2 = new TLine(22,yy1[0],22,yy1[1]);
  l2->SetLineStyle(2);
  l2->SetLineColor(kBlack);
  l2->Draw("same");


  TGraph* grlegend1 = new TGraph();
  grlegend1->SetMarkerStyle(20);
  TGraph* grlegend2 = new TGraph();
  grlegend2->SetMarkerStyle(21);
  TGraph* grlegend3 = new TGraph();
  grlegend3->SetMarkerStyle(22);
  
  auto legend = new TLegend(0.7,0.7,0.9,0.9);
  legend->AddEntry(grlegend1,"1 keV", "p");
  legend->AddEntry(grlegend2,"2 keV","p");
  legend->AddEntry(grlegend3,"3 keV","p");
  legend->Draw("same");


  TLatex latex_fit1;
  latex_fit1.SetTextSize(0.04);
  latex_fit1.DrawLatex(0,yy1[1]-7,"#bf{#color[2]{fit [0.04, 2] MeV}}");

  TLatex latex_fit2;
  latex_fit2.SetTextSize(0.04);
  latex_fit2.DrawLatex(10,yy1[1]-7,"#bf{#color[8]{fit [0.1, 1.2] MeV}}");

  TLatex latex_fit3;
  latex_fit3.SetTextSize(0.04);
  latex_fit3.DrawLatex(0,yy1[1]-7.5,"#bf{#color[9]{fit [0.2, 0.5] MeV}}");

  TLatex latex_fit4;
  latex_fit4.SetTextSize(0.04);
  latex_fit4.DrawLatex(10,yy1[1]-7.5,"#bf{#color[6]{fit [0.32, 0.37] MeV}}");

  //c->Print("Minuit_FitRanges_Resol_Rates_newrun_.png");
}
