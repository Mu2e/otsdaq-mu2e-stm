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


double FuncShape1(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A*log((B/(x[0]+C))-1)+D;
  return value;
};

double FuncShape2(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];
  double E = par[4];
  
  double value = A/(exp(x[0]/B + C)+D)+E;
  return value;
};

double FuncShape3(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A*log10((B-x[0])/(C+x[0]))+D;
  return value;
};

double FuncShape4(double* x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];
  
  double value = (-1+A*pow(10, (x[0]+B)/C))/(1+pow(10, (x[0]+B)/C))+D;
  return value;
};

void PlotEtoADCFunctions(){

  gROOT->SetStyle("ATLAS");
 
  gStyle->SetOptStat(111110);

  TCanvas* c1 = new TCanvas();
  double range[2] = {-3501, 0};

  TF1* fitfunc[1];
  
  TF1* extremefunc[4];
 
  fitfunc[0] = new TF1("FitFunc0","[0]*x", range[0],range[1]);
  fitfunc[0]->SetNpx(300000);
  fitfunc[0]->SetParameter(0,-0.57);
  
  //////////
  ///*****Crossig 347keV line as in best calibration******///
  /*
  extremefunc[0] = new TF1("ExtremeFunc0",FuncShape1,range[0],range[1],4);
  extremefunc[0]->SetNpx(300000);
  extremefunc[0]->SetParameter(0,-500);
  extremefunc[0]->SetParameter(1,-3900);
  extremefunc[0]->SetParameter(2,-100);
  extremefunc[0]->SetParameter(3,1100);

  extremefunc[1] = new TF1("ExtremeFunc1",FuncShape1,range[0],range[1],4);
  extremefunc[1]->SetNpx(300000);
  extremefunc[1]->SetParameter(0,-120);
  extremefunc[1]->SetParameter(1,-3502);
  extremefunc[1]->SetParameter(2,-1);
  extremefunc[1]->SetParameter(3,556.5);
  
  extremefunc[2] = new TF1("ExtremeFunc2",FuncShape2,range[0],range[1],5);
  extremefunc[2]->SetNpx(300000);
  extremefunc[2]->SetParameter(0,-2999);
  extremefunc[2]->SetParameter(1,-800);
  extremefunc[2]->SetParameter(2,-1.225);
  extremefunc[2]->SetParameter(3,1.2);
  extremefunc[2]->SetParameter(4,2001);

  extremefunc[3] = new TF1("ExtremeFunc3",FuncShape2,range[0],range[1],5);
  extremefunc[3]->SetNpx(300000);
  extremefunc[3]->SetParameter(0,-1500);
  extremefunc[3]->SetParameter(1,-200);
  extremefunc[3]->SetParameter(2,-9.65);
  extremefunc[3]->SetParameter(3,0.07495);
  extremefunc[3]->SetParameter(4,20000);
*/
  ///*****Random calibration for 347keV line******///
  extremefunc[0] = new TF1("ExtremeFunc1",FuncShape1,range[0],range[1],4);
  extremefunc[0]->SetNpx(300000);
  extremefunc[0]->SetParameter(0,-190);
  extremefunc[0]->SetParameter(1,-3601);
  extremefunc[0]->SetParameter(2,-52.6);
  extremefunc[0]->SetParameter(3,800.192);

  extremefunc[1] = new TF1("ExtremeFunc2",FuncShape2,range[0],range[1],5);
  extremefunc[1]->SetNpx(300000);
  extremefunc[1]->SetParameter(0,-2000);
  extremefunc[1]->SetParameter(1,-180);
  extremefunc[1]->SetParameter(2,-7.2);
  extremefunc[1]->SetParameter(3,1);
  extremefunc[1]->SetParameter(4,1998.508);

  extremefunc[2] = new TF1("ExtremeFunc3",FuncShape3,range[0],range[1],4);
  extremefunc[2]->SetNpx(300000);
  extremefunc[2]->SetParameter(0,833);
  extremefunc[2]->SetParameter(1,199.31);
  extremefunc[2]->SetParameter(2,4000);
  extremefunc[2]->SetParameter(3,1085);

  extremefunc[3] = new TF1("ExtremeFunc4",FuncShape4,range[0],range[1],4);
  extremefunc[3]->SetNpx(300000);
  extremefunc[3]->SetParameter(0,-2000);
  extremefunc[3]->SetParameter(1,1770);
  extremefunc[3]->SetParameter(2,1050);
  extremefunc[3]->SetParameter(3,1959.61);
  

  for(int i = 0 ; i < 4; i++) {

    extremefunc[i]->GetYaxis()->SetRangeUser(0,3000);
    extremefunc[i]->GetXaxis()->SetRangeUser(-3501,0);
    extremefunc[i]->GetXaxis()->SetTitle("ADC Counts");
    extremefunc[i]->GetYaxis()->SetTitle("E [keV]");
    extremefunc[i]->SetLineStyle(i+2);
    
    if(i==0){extremefunc[i]->SetLineColor(kViolet-6); extremefunc[i]->Draw("");}
    else if(i==1){extremefunc[i]->SetLineColor(kViolet+1); extremefunc[i]->Draw("same");}
    else if(i==2){extremefunc[i]->SetLineColor(kOrange-6); extremefunc[i]->Draw("same");}
    else {extremefunc[i]->SetLineColor(kOrange+1); extremefunc[i]->Draw("same");}

  }

  fitfunc[0]->SetLineColor(kBlue);
  fitfunc[0]->Draw("same");

  auto legend = new TLegend(0.45,0.7,0.9,0.9);
  legend->AddEntry(extremefunc[0],"Shape 1","l");
  legend->AddEntry(extremefunc[1],"Shape 2","l");
  legend->AddEntry(extremefunc[2],"Shape 3","l");
  legend->AddEntry(extremefunc[3],"Shape 4","l");
  legend->AddEntry(fitfunc[0],"E=-0.57ADC","l");
  legend->Draw("same");


  gPad->RedrawAxis();


  std::string nameplot_png = "EADC_DifferentShapesNOTcalibratedto347keV_new.png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());

  c1->Print(char_nameplot_png);

}
