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


void CalculateIntegral_back( int backfit , double p0, double p1, double p2, double p3){

  gROOT->SetStyle("ATLAS");
 
  gStyle->SetOptStat(111110);

  TCanvas* c1 = new TCanvas();
  double range[2] = {0 , 2};
  int NPbrems = 4;
  
  //Initialise the function to generate the background
  TF1* fbrems = new TF1("BremsShape",Bremsfunc_1,range[0],range[1],NPbrems);

  fbrems->SetNpx(300000);

  double par[4] = {p0, p1, p2, p3};  
    
  
  for(int i = 0 ; i < NPbrems; i++) {
    fbrems->SetParameter(i,par[i]);
  }

  double integral = fbrems->Integral(0.04,2);
  std::cout<<"Integral: "<<integral<<std::endl;

  p0 = p0 / integral;
  p3 = p3 / integral;

  par[0] = p0;
  par[1] = p1;
  par[2] = p2;
  par[3] = p3;
  
  for(int i = 0 ; i < NPbrems; i++) {
    fbrems->SetParameter(i,par[i]);
  }

  fbrems->GetYaxis()->SetRangeUser(0,10);
  fbrems->GetXaxis()->SetRangeUser(0,0.5);
 
  fbrems->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
  
  std::string str_ = "Background Shape "+std::to_string(backfit);
  char* char_ = const_cast<char*>(str_.c_str());
   
  if( backfit==0 ){
    fbrems->GetYaxis()->SetTitle("Nominal Background Shape");
  }
  else{
    fbrems->GetYaxis()->SetTitle(char_);
  }

  fbrems->Draw("");

  std::string nameplot_png = "BackgroundShape_"+std::to_string(backfit)+".png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());
  std::cout<<"Name plot: "<<nameplot_png<<std::endl;
  c1->Print(char_nameplot_png);

}
