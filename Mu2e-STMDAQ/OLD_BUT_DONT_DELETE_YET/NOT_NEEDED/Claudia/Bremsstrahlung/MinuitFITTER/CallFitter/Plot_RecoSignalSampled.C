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


void Plot_RecoSignalSampled(std::string filename){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  TCanvas *c = new TCanvas("");
  double Strue[9], Sreco[9], StdevSreco[9];

  //Read csv
  std::ifstream myFile(filename);

  double val;
  std::string line, title;

  int Nrows = 0;
  int Ncol = 0;
  
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

   while(std::getline(myFile, line))
     {
       
        std::stringstream ss(line);

	if(Nrows==0){
	  Ncol = 0;
	  while(ss >> title){
	    if(Ncol != 0){
	      std::cout<<"title: "<<title<<std::endl;
	    }
	    Ncol++;
	  }
	}
	else{
	  Ncol = 0;
	  while(ss >> val){
	    if(Ncol != 0){

	    if(Ncol==1){Strue[Nrows-1] = val;}
	    if(Ncol==2){Sreco[Nrows-1] = val;}
	    if(Ncol==3){StdevSreco[Nrows-1] = val;}
	    }
	    Ncol++;
	  }
	  std::cout<<"N: "<<(Nrows-1)<<" Strue: "<<Strue[Nrows-1]<<" Sreco: "<<Sreco[Nrows-1]<<" StdevSreco: "<<StdevSreco[Nrows-1]<<std::endl;
	}
	Nrows++;
     }

   myFile.close();

    /*
  //1kHz 1keV
  Strue[0]=24091;
  Sreco[0]=24054.1;
  StdevSreco[0]=95.5;
  
  //1kHz 2keV 
  Strue[1]=23791;
  Sreco[1]=23778.2;
  StdevSreco[1]=128.2;
  
  //1kHz 3keV 
  Strue[2]=24001;
  Sreco[2]=24028.5;
  StdevSreco[2]=163.3;
    
  //50kHz 1keV 
  Strue[3]=24271;
  Sreco[3]=24219.1;
  StdevSreco[3]=628.5;
  
  //50kHz 2keV 
  Strue[4]=23929;
  Sreco[4]=23893.3;
  StdevSreco[4]=910.9;
    
  //50kHz 3keV 
  Strue[5]=23833;
  Sreco[5]=23906.2;
  StdevSreco[5]=1134.2;
  
  //200kHz 1keV 
  Strue[6]=23899;
  Sreco[6]=23968;
  StdevSreco[6]=1268.2;
  
  //200kHz 2keV 
  Strue[7]=23871;
  Sreco[7]=24007.2;
  StdevSreco[7]=1809.9;
  
  //200kHz 3keV
  Strue[8]=23928;
  Sreco[8]=23965.5;
  StdevSreco[8]=2349.5;
    */
   
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
 			
      y[0] = (Strue[k] - Sreco[k])/StdevSreco[k];
      ex[0] = 0;
      ey[0] = 1;
	
      std::cout<<"Resol index: "<<k<<std::endl;
      std::cout<<"Strue: "<<Strue[k]<<" Sreco: "<<Sreco[k]<<" stddev reco: "<<StdevSreco[k]<<std::endl;
      std::cout<<"Graph value: "<<y[0]<<std::endl;
      gr[n_graphs] = new TGraphErrors(Npoints,x,y,ex,ey);

      if((k==0)||(k==3)||(k==6)){gr[n_graphs]->SetMarkerStyle(20); std::cout<<"Resolution: 1 keV CIRCLE"<<std::endl;}
      if((k==1)||(k==4)||(k==7)){gr[n_graphs]->SetMarkerStyle(21); std::cout<<"Resolution: 2 keV SQUARE"<<std::endl;}
      if((k==2)||(k==5)||(k==8)){gr[n_graphs]->SetMarkerStyle(22); std::cout<<"Resolution: 3 keV TRIANGLE"<<std::endl;}

      //gr[n_graphs]->SetMarkerColor(kRed-4);
      //gr[n_graphs]->SetMarkerColor(kGreen-7);
      //gr[n_graphs]->SetMarkerColor(kBlue-7);
      //gr[n_graphs]->SetMarkerColor(kYellow-7);
      //gr[n_graphs]->SetMarkerColor(kMagenta-3);
      gr[n_graphs]->SetMarkerColor(kCyan-7);
      
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


  //c->Print("Minuit_SampledShapes_Resol_Rates.png");

  c->Print("Minuit_bckgShape6.png");
}
