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

void Calculate_MeanPOTpermicro_RunI(){

  gROOT->SetStyle("ATLAS");
  
  TCanvas *c = new TCanvas();

  //Low intensity
  double low_intensity_POT = 1.6e+7;
  double low_intensity_runtime_s = 9.5e+6;
  double low_intensity_nMicro = 1.8e+12;
  
  //High intensity
  double high_intensity_POT = 3.9e+7;
  double high_intensity_runtime_s = 1.6e+6;
  double high_intensity_nMicro = 2.3e+11;
  
  TH1D *histoPOT = new TH1D("histoPOT", "", 100, 1e+7, 5e+7);

  histoPOT->GetXaxis()->SetTitle("Run-I POT");
  histoPOT->GetYaxis()->SetTitle("Entries weighted by run time");
  
  //histoPOT->Fill(low_intensity_POT, low_intensity_runtime_s);
  //histoPOT->Fill(high_intensity_POT, high_intensity_runtime_s);

  histoPOT->Fill(low_intensity_POT, low_intensity_nMicro);
  histoPOT->Fill(high_intensity_POT, high_intensity_runtime_s);
  
  double mean_weighted_POT = histoPOT->GetMean();
  double mean_error = histoPOT->GetMeanError();
  
  auto l = new TLine(mean_weighted_POT,0,mean_weighted_POT,histoPOT->GetMaximum()+500000);
  
  histoPOT->Draw("HIST");
  l->SetLineColor(kBlue);
  l->SetLineStyle(2);
  l->Draw("same");

  std::string str_latex1 = "#color[4]{#splitline{Weighted mean POT=}{"+std::to_string(mean_weighted_POT)+" +- "+std::to_string(mean_error)+"}}";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.35,.85,char_latex1);

  
  std::cout<<"Mean POT: "<<mean_weighted_POT<<"+-"<<mean_error<<std::endl;
}
