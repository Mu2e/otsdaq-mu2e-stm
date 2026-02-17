#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <iomanip>

#include "TGraph.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"
#include "TF1.h"
#include "TH1F.h"
#include "TSpectrum.h"
#include "TLegend.h"
#include "TRandom3.h"
#include "TLine.h"
#include "TPaveStats.h"


void CheckSignalHeight(){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  //gStyle->SetOptTitle(1);

  TCanvas* c1 = new TCanvas("c1","c1",0,0,1200,500);
  c1->Divide(3,1);

  unsigned long int dimpeaks = 3;
  double nbins = 100;
  double pulseNumXrays = 50000;
  double frange[2] = {-2,4};
  double xx1[2]={-2,4};
  double yy1[2]={0,2000};
  double sigma = 1;
  double mean_E = 1;

  TF1* fgausXray[dimpeaks];
  TH1F* hsignal[dimpeaks];
  TPaveStats *stat[dimpeaks];

  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->SetMarkerStyle(1);
  graph1->GetXaxis()->SetTitle("E_{#gamma} (STM) Signal [MeV]");
  graph1->GetYaxis()->SetTitle("Counts");
  graph1->GetYaxis()->SetTitleOffset(1.65);

  //method 1
  c1->cd(1);
  gPad->RedrawAxis();
  fgausXray[0] = new TF1("fgausXray0","gaus",frange[0],frange[1]);
  fgausXray[0]->SetParameters(1,mean_E,sigma);
  hsignal[0] = new TH1F("signal0","",nbins,frange[0],frange[1]);
  hsignal[0]->FillRandom("fgausXray0",pulseNumXrays);
  hsignal[0]->SetFillColor(kCyan-3);
  hsignal[0]->SetLineColor(kCyan-3);
  hsignal[0]->SetFillStyle(3001);
  hsignal[0]->Draw("HIST");

  gPad->Update();
  stat[0] = (TPaveStats*)hsignal[0]->FindObject("stats");
  stat[0]->SetY1NDC(.68);
  stat[0]->SetY2NDC(.85);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.65);
  stat[0]->SetTextSize(0.043);
  stat[0]->SetTextColor(kCyan-3);
  graph1->Draw("ap");
  hsignal[0]->Draw("HIST,same");
  stat[0]->Draw("same");
  fgausXray[0]->SetLineColor(kRed);
  //fgausXray[0]->Draw("same");
  std::string str_latex1 = "Gaus Function, amplitude=1";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.2,.86,char_latex1);

  //method 2 - different amplitude for funtion
  c1->cd(2);
  gPad->RedrawAxis();
  fgausXray[1] = new TF1("fgausXray1","gaus",frange[0],frange[1]);
  fgausXray[1]->SetParameters(1000,mean_E,sigma);
  hsignal[1] = new TH1F("signal1","",nbins,frange[0],frange[1]);
  hsignal[1]->FillRandom("fgausXray1",pulseNumXrays);
  hsignal[1]->SetFillColor(kRed-3);
  hsignal[1]->SetLineColor(kRed-3);
  hsignal[1]->SetFillStyle(3001);
  hsignal[1]->Draw("HIST");

  gPad->Update();
  stat[1] = (TPaveStats*)hsignal[1]->FindObject("stats");
  stat[1]->SetY1NDC(.68);
  stat[1]->SetY2NDC(.85);
  stat[1]->SetX1NDC(0.2); //new x start position
  stat[1]->SetX2NDC(0.65);
  stat[1]->SetTextSize(0.043);
  stat[1]->SetTextColor(kRed-3);
  graph1->Draw("ap");
  hsignal[1]->Draw("HIST,same");
  stat[1]->Draw("same");
  fgausXray[1]->SetLineColor(kRed);
  fgausXray[1]->Draw("same");
  std::string str_latex2 = "Gaus Function, amplitude=1000";
  char* char_latex2 = const_cast<char*>(str_latex2.c_str());
  TLatex latex2;
  latex2.DrawLatexNDC(.2,.86,char_latex2);

  //method 3
  c1->cd(3);
  gPad->RedrawAxis();
  hsignal[2] = new TH1F("signal2","",nbins,frange[0],frange[1]);
  for(int i = 0; i<pulseNumXrays; i++){
    //smearing
    hsignal[2]->Fill(gRandom->Gaus(mean_E,sigma));
  }
  hsignal[2]->SetFillColor(kOrange-3);
  hsignal[2]->SetLineColor(kOrange-3);
  hsignal[2]->SetFillStyle(3001);
  hsignal[2]->Draw("HIST");

  gPad->Update();
  stat[2] = (TPaveStats*)hsignal[2]->FindObject("stats");
  stat[2]->SetY1NDC(.68);
  stat[2]->SetY2NDC(.85);
  stat[2]->SetX1NDC(0.2); //new x start position
  stat[2]->SetX2NDC(0.65);
  stat[2]->SetTextSize(0.043);
  stat[2]->SetTextColor(kOrange-3);
  graph1->Draw("ap");
  hsignal[2]->Draw("HIST,same");
  stat[2]->Draw("same");
  std::string str_latex3 = "Smearing mean signal";
  char* char_latex3 = const_cast<char*>(str_latex3.c_str());
  TLatex latex3;
  latex3.DrawLatexNDC(.2,.86,char_latex3);


}
