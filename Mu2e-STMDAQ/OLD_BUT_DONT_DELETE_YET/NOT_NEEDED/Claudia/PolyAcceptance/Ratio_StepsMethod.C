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
#include <iomanip>


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "TFitResult.h"
#include "TMatrixD.h"


//================================================================
void PlotAcceptance_E(double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest){

  gROOT->SetStyle("ATLAS");

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  //gPad->SetLogx();

  double p_1step[4] = {4.149e-7, 1.261e6, 1.519e12, 1.589e12};
  double p_morestep[4] = {6.748e-7, 5.923e5, 4.455e11, 3.506e11};
  
  //TF1* f_1step = new TF1("f_1step", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  TF1 f_1step("f_1step", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);  
  f_1step.SetParameters(p_1step[0],p_1step[1],p_1step[2],p_1step[3]);

  //TF1* f_morestep = new TF1("f_morestep", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  TF1 f_morestep("f_morestep", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax); 
  f_morestep.SetParameters(p_morestep[0],p_morestep[1],p_morestep[2],p_morestep[3]);

  TF1 fratio("fratio","f_1step/f_morestep", xmin, xmax);
  fratio.SetParameters(p_1step[0],p_1step[1],p_1step[2],p_1step[3],p_morestep[0],p_morestep[1],p_morestep[2],p_morestep[3]);


  TF1* fratio_ptr = new TF1("fratio_ptr", "([0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) ))/([4] + (1./([5]*x)) - sqrt( (1./([6]*x)) + (1./([7]*x*x)) ))", xmin, xmax);
  fratio_ptr->SetParameters(p_1step[0],p_1step[1],p_1step[2],p_1step[3],p_morestep[0],p_morestep[1],p_morestep[2],p_morestep[3]);
 
  fratio.SetLineColor(kBlue);
  fratio.SetLineStyle(2);
  fratio.SetLineWidth(3);

  fratio_ptr->SetLineColor(kBlue-7);
  fratio_ptr->SetLineStyle(2);
  fratio_ptr->SetLineWidth(3);
  fratio_ptr->Draw("same,l");
  
  double n =0.05;
  
  for(int i =1; i<50;i++){
    std::cout<<"E= "<<n<<std::endl;
    std::cout<<f_1step.Eval(n)<<std::endl;
    std::cout<<f_morestep.Eval(n)<<std::endl;
    std::cout<<fratio.Eval(n)<<std::endl;
    std::cout<<fratio_ptr->Eval(n)<<std::endl;
    std::cout<<" "<<std::endl;
    n=n+0.05;
  }
    
  c1->Print("AcceptanceVD10_VD8990_RatioMethods.png");
  c1->Print("AcceptanceVD10_VD8990_RatioMethods.pdf");
}


//================================================================

void Ratio_StepsMethod(){
    
    PlotAcceptance_E( 0.1, 6, 0.3, 1, "E_{#gamma} [MeV]", "Acceptance Ratio");

}
