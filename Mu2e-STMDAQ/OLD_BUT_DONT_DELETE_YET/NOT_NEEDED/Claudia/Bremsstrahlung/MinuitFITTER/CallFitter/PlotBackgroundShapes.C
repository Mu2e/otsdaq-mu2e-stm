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

double Bremsfunc_1(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  
  double value = A/(exp(B*x[0])+C)+D;
  return value;
};


void PlotBackgroundShapes(){

  gROOT->SetStyle("ATLAS");
 
  gStyle->SetOptStat(111110);

  TCanvas* c1 = new TCanvas();
  double range[2] = {0, 2};
  int NPbrems = 4;

  int numbackshapes = 7;
  TF1* fbrems[7];
    
  //Initialise the function to generate the background
  for(int i = 0 ; i < numbackshapes ; i++){

    fbrems[i] = new TF1("BremsShape",Bremsfunc_1,range[0],range[1],NPbrems);
    fbrems[i]->SetNpx(300000);

    double p0,p1,p2,p3;

    //Read from: /work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/SystematicsBackgroundShapes/backgroundshapes
    if(i==0){p0=0.00158168; p1=2.36728; p2=-0.974756; p3=0.00011748;}
    else if(i==1){p0=0.5; p1=7.6; p2=-0.99; p3=0.01;}
    else if(i==2){p0=0.5; p1=30.6; p2=-0.99; p3=0.0009;}
    else if(i==3){p0=8.5; p1=30.6; p2=-0.99; p3=0.0009;}
    else if(i==4){p0=4000; p1=0.9; p2=-0.00045; p3=13;}
    else if(i==5){p0=2; p1=2.36728; p2=-0.974756; p3=0.00011748;}
    else {p0=50; p1=10; p2=-0.01; p3=13;}

    double par[4] = {p0, p1, p2, p3};
    
    for(int j = 0 ; j < NPbrems; j++) {
      fbrems[i]->SetParameter(j,par[j]);
    }

    double integral = fbrems[i]->Integral(0.04,2);
    std::cout<<"Integral: "<<integral<<std::endl;
    
    p0 = p0 / integral;
    p3 = p3 / integral;
  
    par[0] = p0;
    par[1] = p1;
    par[2] = p2;
    par[3] = p3;

    for(int j = 0 ; j < NPbrems; j++) {
      fbrems[i]->SetParameter(j,par[j]);
    }

    fbrems[i]->SetLineColor(i+1);
    fbrems[i]->GetYaxis()->SetRangeUser(0,10);
    fbrems[i]->GetXaxis()->SetRangeUser(0,0.5);
    fbrems[i]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    fbrems[i]->GetYaxis()->SetTitle("Normalised Background Shape");
    if(i==0){
      fbrems[i]->SetLineStyle(2);
      fbrems[i]->Draw("");
    }
    else{ fbrems[i]->Draw("same"); }
  }
 
  std::string nameplot_png = "BackgroundShapes.png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());

  auto legend = new TLegend(0.55,0.55,0.93,0.9);
  legend->AddEntry(fbrems[0],"Nominal Shape","l");
  legend->AddEntry(fbrems[1],"Shape 1","l");
  legend->AddEntry(fbrems[2],"Shape 2","l");
  legend->AddEntry(fbrems[3],"Shape 3","l");
  legend->AddEntry(fbrems[4],"Shape 4","l");
  legend->AddEntry(fbrems[5],"Shape 5","l");
  legend->AddEntry(fbrems[6],"Shape 6","l");
  legend->Draw("same");
  
  gPad->RedrawAxis();
  

  //c1->Print(char_nameplot_png);
}
