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

double Calib_Line(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  
  double value = A + B*x[0];
  return value;
};

double Calib_Grade3Poly(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A + B*x[0] + C*x[0]*x[0] + D*x[0]*x[0]*x[0];
  return value;
};

double Calib_Sigmoid(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A/(1+exp(x[0]/B + C))+D;
  return value;
};

double Calib_InverseSigmoid(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];
  double E = par[4];
  
  double value = A*log((B/(x[0]+C))+D)+E;
  return value;
};





void PlotCalibrationShapes(){

  gROOT->SetStyle("ATLAS");
 
  gStyle->SetOptStat(111110);

  TCanvas* c1 = new TCanvas();
  double range[2] = {0, 2000};

  TF1* fitfunc[3];
  
  TF1* extremefunc[4];


  fitfunc[2] = new TF1("FitFunc0",Calib_Line,range[0],range[1],2);
  fitfunc[2]->SetNpx(300000);
  fitfunc[2]->SetParameter(0,2.731);
  fitfunc[2]->SetParameter(1,-1.76);
  
  fitfunc[0] = new TF1("FitFunc1",Calib_Grade3Poly,range[0],range[1],4);
  fitfunc[0]->SetNpx(300000);
  fitfunc[0]->SetParameter(0,0.05726);
  fitfunc[0]->SetParameter(1,-1.736);
  fitfunc[0]->SetParameter(2,-4.265E-5);
  fitfunc[0]->SetParameter(3,1.9667E-8);
  
  fitfunc[1] = new TF1("FitFunc2",Calib_Sigmoid,range[0],range[1],4);
  fitfunc[1]->SetNpx(300000);
  fitfunc[1]->SetParameter(0,-19240);
  fitfunc[1]->SetParameter(1,-2722);
  fitfunc[1]->SetParameter(2,0.2655);
  fitfunc[1]->SetParameter(3,8350);

  //////////
  
  extremefunc[0] = new TF1("ExtremeFunc0",Calib_Sigmoid,range[0],range[1],4);
  extremefunc[0]->SetNpx(300000);
  extremefunc[0]->SetParameter(0,-3900);
  extremefunc[0]->SetParameter(1,-500);
  extremefunc[0]->SetParameter(2,2.2);
  extremefunc[0]->SetParameter(3,100);
  
  extremefunc[1] = new TF1("ExtremeFunc1",Calib_Sigmoid,range[0],range[1],4);
  extremefunc[1]->SetNpx(300000);
  //extremefunc[1]->SetParameter(0,-3000);
  //extremefunc[1]->SetParameter(1,-75);
  //extremefunc[1]->SetParameter(2,6);
  //extremefunc[1]->SetParameter(3,-1);

  extremefunc[1]->SetParameter(0,-3502);
  extremefunc[1]->SetParameter(1,-120);
  extremefunc[1]->SetParameter(2,4.6375);
  extremefunc[1]->SetParameter(3,1);
  
  extremefunc[2] = new TF1("ExtremeFunc2",Calib_InverseSigmoid,range[0],range[1],5);
  extremefunc[2]->SetNpx(300000);
  extremefunc[2]->SetParameter(0,-800);
  extremefunc[2]->SetParameter(1,-2999);
  extremefunc[2]->SetParameter(2,-2001);
  extremefunc[2]->SetParameter(3,-1.2);
  extremefunc[2]->SetParameter(4,-980);

  extremefunc[3] = new TF1("ExtremeFunc3",Calib_InverseSigmoid,range[0],range[1],5);
  extremefunc[3]->SetNpx(300000);
  extremefunc[3]->SetParameter(0,-200);
  extremefunc[3]->SetParameter(1,-1500);
  extremefunc[3]->SetParameter(2,-20000);
  extremefunc[3]->SetParameter(3,-0.07495);
  extremefunc[3]->SetParameter(4,-1930);

  

  /*
  for(int i = 0 ; i < 3; i++) {
    fitfunc[i]->GetYaxis()->SetRangeUser(-1000,0);
    fitfunc[i]->GetXaxis()->SetRangeUser(0,500);
    fitfunc[i]->GetXaxis()->SetTitle("E [keV]");
    fitfunc[i]->GetYaxis()->SetTitle("ADC Counts");
    if(i==0){fitfunc[i]->Draw(""); fitfunc[i]->SetLineColor(kRed);}
    else {fitfunc[i]->Draw("same"); fitfunc[i]->SetLineColor(i+2); fitfunc[i]->SetLineStyle(2);}
  
  }

  auto legend = new TLegend(0.55,0.65,0.93,0.9);
  legend->AddEntry(fitfunc[0],"Grade-3 Polynomial Fit","l");
  legend->AddEntry(fitfunc[1],"Sigmoid Fit","l");
  legend->AddEntry(fitfunc[2],"Line Fit","l");
  legend->Draw("same");
  */
  
 
  for(int i = 0 ; i < 4; i++) {
    extremefunc[i]->GetYaxis()->SetRangeUser(-3500,1000);
    extremefunc[i]->GetXaxis()->SetRangeUser(0,2000);
    extremefunc[i]->GetXaxis()->SetTitle("E [keV]");
    extremefunc[i]->GetYaxis()->SetTitle("ADC Counts");
    extremefunc[i]->SetLineStyle(2);
    if(i==0){extremefunc[i]->SetLineColor(kOrange); extremefunc[i]->Draw("");}
    else {extremefunc[i]->SetLineColor(i+6); extremefunc[i]->Draw("same");}
    }
  fitfunc[1]->SetLineColor(kRed);
  fitfunc[1]->Draw("same");

  auto legend = new TLegend(0.45,0.7,0.93,0.9);
  legend->AddEntry(extremefunc[0],"Sigmoid Shape 1","l");
  legend->AddEntry(extremefunc[1],"Sigmoid Shape 2","l");
  legend->AddEntry(extremefunc[2],"Inverse Sigmoid Shape 1","l");
  legend->AddEntry(extremefunc[3],"Inverse Sigmoid Shape 2","l");
  legend->AddEntry(fitfunc[1],"Best Fit","l");
  legend->Draw("same");

  gPad->RedrawAxis();

  std::string nameplot_png = "DifferentFunctionsCalibration_E_ADC.png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());

  c1->Print(char_nameplot_png);
}
