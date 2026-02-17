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


void Plot_CalibDeviationsStDev(std::string filename){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  
  TCanvas *c = new TCanvas("");
  //[combination rate,res][number of deviations]
  double Sreco[6][6], StdevSreco[6][6];
  double Strue,rate,resol;
  //Read csv
  std::ifstream myFile(filename);

  double val;
  std::string line, title;
  int Nrow = 0;
  

  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

   while(std::getline(myFile, line))
     {
       std::stringstream ss(line);
       if(Nrow != 0){
	 ss >> rate;
	 ss >> resol;
	 ss >> Strue;
	 std::cout<<"Nrow: "<<Nrow<<" rate "<<rate<<" resol "<<resol<<" Strue "<< Strue<<std::endl;
	 for (int i =0; i<6 ; i++){
	   ss >> Sreco[Nrow-1][i];
	   ss >> StdevSreco[Nrow-1][i];
	   std::cout<<"Sreco: "<<Sreco[Nrow-1][i]<<" StDeV: "<<StdevSreco[Nrow-1][i]<<std::endl;
	 }
       }
       Nrow++;
     }
   
   myFile.close();

  TGraphErrors* gr[36];
  int Npoints = 1;
  int Nfits = 6;
  int Nresol = 6;
  
  double y[1];
  double x[1];
  double ex[1];
  double ey[1];
  int n_graphs =0;
  double sumx;
  x[0]=0;
 
    //6 combination of resols
    for(int k =0 ; k <Nresol ; k++){
      //6 calib
      for(int j =0 ; j <Nfits ; j++){

	if((k==0)||(k==3)){sumx=1;}
	if((k==1)||(k==4)){sumx=3;}
	if((k==2)||(k==5)){sumx=5;}
	
	x[0] = 1.5*j+sumx;
	
	if((k==3)||(k==4)||(k==5)){x[0] = x[0]+15; std::cout<<"50kHz"<<std::endl;}
	else{std::cout<<"1kHz"<<std::endl;}
	
	y[0] = (Strue - Sreco[k][j])/StdevSreco[k][j];
	ex[0] = 0;
	ey[0] = 1;

	std::cout<<"Resol index: "<<k<<" fit index: "<<j<<std::endl;
	std::cout<<"Strue: "<<Strue<<" Sreco: "<<Sreco[k][j]<<" stddev reco: "<<StdevSreco[k][j]<<std::endl;
	std::cout<<"Graph value: "<<y[0]<<std::endl;
	gr[n_graphs] = new TGraphErrors(Npoints,x,y,ex,ey);

	if((k==0)||(k==3)){gr[n_graphs]->SetMarkerStyle(20); std::cout<<"Resolution: 1 keV CIRCLE"<<std::endl;}
	if((k==1)||(k==4)){gr[n_graphs]->SetMarkerStyle(21); std::cout<<"Resolution: 2 keV SQUARE"<<std::endl;}
	if((k==2)||(k==5)){gr[n_graphs]->SetMarkerStyle(22); std::cout<<"Resolution: 3 keV TRIANGLE"<<std::endl;}

	if(j==0){gr[n_graphs]->SetMarkerColor(kRed); std::cout<<"Calib nominal 0 RED"<<std::endl;}
	if(j==1){gr[n_graphs]->SetMarkerColor(kGreen-3); std::cout<<"Dev 1 GREEN"<<std::endl;}
	if(j==2){gr[n_graphs]->SetMarkerColor(kBlue-3); std::cout<<"Dev 2 BLUE"<<std::endl;}
	if(j==3){gr[n_graphs]->SetMarkerColor(kPink+9); std::cout<<"Dev 3 PINK"<<std::endl;}
	if(j==4){gr[n_graphs]->SetMarkerColor(kOrange-8); std::cout<<"Dev 4 ORANGE"<<std::endl;}
	if(j==5){gr[n_graphs]->SetMarkerColor(kAzure-7); std::cout<<"Dev 5 AZURE"<<std::endl;}

	n_graphs++;
      }
    }
 
    std::cout<<"n_graphs "<<n_graphs<<std::endl;
    
    TGraph* gr_template;
    double xx1[2] = {0, 30};
    double yy1[2] = {-2, 6};
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
  latex2.DrawLatex(16,yy1[0]-0.25,"50 kHz");


  TLine* l1 = new TLine(15,yy1[0],15,yy1[1]);
  l1->SetLineStyle(2);
  l1->SetLineColor(kBlack);
  l1->Draw("same");


  TGraph* grlegend1 = new TGraph();
  grlegend1->SetMarkerStyle(20);
  TGraph* grlegend2 = new TGraph();
  grlegend2->SetMarkerStyle(21);
  TGraph* grlegend3 = new TGraph();
  grlegend3->SetMarkerStyle(22);
  
  auto legend = new TLegend(0.75,0.2,0.9,0.4);
  legend->AddEntry(grlegend1,"1 keV", "p");
  legend->AddEntry(grlegend2,"2 keV","p");
  legend->AddEntry(grlegend3,"3 keV","p");
  legend->Draw("same");

 
  TLatex latex_fit1;
  latex_fit1.SetTextSize(0.04);
  latex_fit1.DrawLatex(0,yy1[1]-9.3,"#bf{#color[2]{Nominal Calibration}}");

  TLatex latex_fit2;
  latex_fit2.SetTextSize(0.04);
  latex_fit2.DrawLatex(11,yy1[1]-9.3,"#bf{#color[8]{Dev. Calibration 1}}");

  TLatex latex_fit3;
  latex_fit3.SetTextSize(0.04);
  latex_fit3.DrawLatex(21,yy1[1]-9.3,"#bf{#color[9]{Dev. Calibration 2}}");

  TLatex latex_fit4;
  latex_fit4.SetTextSize(0.04);
  latex_fit4.DrawLatex(0,yy1[1]-9.8,"#bf{#color[6]{Dev. Calibration 3}}");

  TLatex latex_fit5;
  latex_fit5.SetTextSize(0.04);
  latex_fit5.DrawLatex(11,yy1[1]-9.8,"#bf{#color[42]{Dev. Calibration 4}}");

  TLatex latex_fit6;
  latex_fit6.SetTextSize(0.04);
  latex_fit6.DrawLatex(21,yy1[1]-9.8,"#bf{#color[36]{Dev. Calibration 5}}");

  c->Print("Minuit_DevCalib_Resol_Rates.png");
}
