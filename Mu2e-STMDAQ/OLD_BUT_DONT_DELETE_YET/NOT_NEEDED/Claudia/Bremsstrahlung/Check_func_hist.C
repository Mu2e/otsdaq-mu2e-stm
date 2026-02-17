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

double Bremsfunc(double* t, double* p){
  double A = p[0];
  double B = p[1];
  double C = p[2];
  double D = p[3];
  double f = A/(exp(B*t[0])+C)+D;
  return f;
};

void Check_func_hist(){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  //gStyle->SetOptTitle(1);

  TCanvas* c1 = new TCanvas("c1","c1",0,0,1200,500);
  c1->Divide(2,1);

  gRandom->SetSeed (0);
  double frange[2] = {0.04,2};
  double frange_signal[2] = {0.3,0.4};
  int Nbins = 5000;
  double binning = (frange[1]-frange[0])/Nbins;
  std::cout<<"New binning: "<<binning<<std::endl;
  double fitbrems_binning = 0.00196;

  double pulseNumBrems = 500000;
  double pulseNumXrays = 50000;

  double mean_E = 0.347;
  double sigma = 0.001;

  //sample
  pulseNumBrems = gRandom->Poisson(pulseNumBrems);
  pulseNumXrays = gRandom->Poisson(pulseNumXrays);

  std::cout<<"Number of pulses gen in background: "<<pulseNumBrems<<std::endl;
  std::cout<<"Number of pulses gen in signal: "<<pulseNumXrays<<std::endl;

  //Background CHECK
  //Best fit parameters for Bremsstrahlung
  double p0 = 0.00157448;
  double p1 = 2.37638;
  double p2 = -0.975819;
  double p3 = 0.000122886;
  //Number of fit parameters
  int NP = 4;

  TPaveStats *stat[2];

  //This is Bremsstrahlung//////////////////////////////
  c1->cd(1);
  TF1* fbrems = new TF1("fbrems",Bremsfunc,frange[0],frange[1],NP);
  fbrems->SetNpx(300000);
  double fitpar[4] = {p0, p1, p2,p3};
  //Correct function height by binning
  fbrems->SetParameters(fitpar[0],fitpar[1],fitpar[2],fitpar[3]);
  double bckbef_function_fullrange = fbrems->Integral(frange[0],frange[1]);
  std::cout<<"Integral Background function full range (1 pulse) assuming a binning of "<<fitbrems_binning<<"MeV for the brems fit: "<<bckbef_function_fullrange<<std::endl;

  //Correct by binning if new binning is chosen
  double height_brems = (binning/fitbrems_binning)*p0;
  double height_brems2 = (binning/fitbrems_binning)*p3;
  fitpar[0] = height_brems;
  fitpar[3] = height_brems2;
  fbrems->SetParameters(fitpar[0],fitpar[1],fitpar[2],fitpar[3]);

  double bckbef_function_fullrange_newbin = fbrems->Integral(frange[0],frange[1]);
  std::cout<<"Integral Background function full range (1 pulse) assuming a binning of "<<binning<<"MeV for the brems fit: "<<bckbef_function_fullrange_newbin<<std::endl;

  TH1F *hBrems = new TH1F("hBrems", "", Nbins,frange[0],frange[1]);
  //this multiplies the height
  hBrems->FillRandom("fbrems",pulseNumBrems);

  double bck_histo =  hBrems->Integral(hBrems->FindFixBin(frange[0]), hBrems->FindFixBin(frange[1]), "");
  std::cout<<"Counts background histo: "<<bck_histo<<std::endl;

  double bck_histo_width =  hBrems->Integral(hBrems->FindFixBin(frange[0]), hBrems->FindFixBin(frange[1]), "WIDTH");
  std::cout<<"Counts background histo width: "<<bck_histo_width<<std::endl;

  double xx1[2]={0.04,2};
  double yy1[2]={0,1400};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->SetMarkerStyle(1);
  graph1->GetXaxis()->SetTitle("E_{#gamma} (STM) Bremsstrahlung [MeV]");
  graph1->GetYaxis()->SetTitle("Counts");
  graph1->GetYaxis()->SetTitleOffset(1.65);

  hBrems->Draw("HIST");

  //redefine height for the brems function
  fitpar[0]=pulseNumBrems*fitpar[0];
  fitpar[3]=pulseNumBrems*fitpar[3];
  fbrems->SetParameters(fitpar[0],fitpar[1],fitpar[2],fitpar[3]);
  double integral_brems = fbrems->Integral(frange[0],frange[1]);
  std::cout<<"Integral for brems function: "<<integral_brems<<std::endl;
  std::cout<<"Integral for brems function/binning: "<<integral_brems/binning<<std::endl;

  gPad->Update();
  stat[0] = (TPaveStats*)hBrems->FindObject("stats");
  stat[0]->SetY1NDC(.68);
  stat[0]->SetY2NDC(.85);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.47);
  stat[0]->SetTextSize(0.043);
  stat[0]->SetTextColor(kOrange-3);
  graph1->Draw("ap");

  hBrems->SetLineColor(kOrange-3);
  hBrems->SetFillColor(kOrange-3);
  hBrems->SetFillStyle(3001);
  hBrems->Draw("HIST,same");
  stat[0]->Draw("same");
  fbrems->SetLineColor(kGreen-3);
  fbrems->Draw("same");
  std::string str_latex1 = "#scale[0.8]{Height corrected: (new binning/0.00196) #times fit height}";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.2,.86,char_latex1);

  std::string str_latex2 = "#scale[1]{f(E) = #frac{p_{0}}{{}_{e^{p_{1}E}} - p_{2}} + p_{3}}";
  char* char_latex2 = const_cast<char*>(str_latex2.c_str());
  TLatex latex2;
  latex2.DrawLatexNDC(.5,.75,char_latex2);
  gPad->RedrawAxis();

  double sigmakeV = sigma*1000;
  std::stringstream sigmastream;
  sigmastream << std::fixed << std::setprecision(1) << sigmakeV;
  string Bremshisto_str = "Gen Bremsstrahlung Histogram (#sigma_{HPGe}= "+sigmastream.str()+" keV)";
  char*  Bremshisto_char = const_cast<char*>(Bremshisto_str.c_str());

  string Bremsfunct_str = "Gen Bremsstrahlung Function (#sigma_{HPGe}= "+sigmastream.str()+" keV)";
  char*  Bremsfunct_char = const_cast<char*>(Bremsfunct_str.c_str());

  auto legend1 = new TLegend(0.35,0.6,0.7,0.8);
  legend1->AddEntry(hBrems,Bremshisto_char,"f");
  legend1->AddEntry(fbrems,Bremsfunct_char,"l");
  legend1->Draw("same");



  //This is Signal//////////////////////////////
  c1->cd(2);
  //Signal CHECK normalised gaus function
  TF1* fgaus= new TF1("fgaus","gaus",frange[0],frange[1]);
  fgaus->SetNpx(300000);
  double height_gaus = 1;
  fgaus->SetParameters(height_gaus,mean_E,sigma);
  double integral_gaus = fgaus->Integral(-10,10);
  std::cout<<"Integral for gaus integral: "<<integral_gaus<<std::endl;
  std::cout<<"The Integral for gaus has to be the same as: "<<height_gaus*sigma*sqrt(3.14159*2)<<std::endl;

  //Fill the Xray signal histogram
  TH1F *hsignal = new TH1F("hsignal","",Nbins,frange[0],frange[1]);
  hsignal->FillRandom("fgaus",pulseNumXrays);

  double s = hsignal->Integral(hsignal->FindFixBin(frange[0]), hsignal->FindFixBin(frange[1]), "");
  std::cout<<"Integral for gaus-signal histo: "<<s<<std::endl;
  double swidth = hsignal->Integral(hsignal->FindFixBin(frange[0]), hsignal->FindFixBin(frange[1]), "WIDTH");
  std::cout<<"Integral for gaus-signal histo width: "<<swidth<<std::endl;

  fgaus->SetParameters(pulseNumXrays*binning/(sigma*sqrt(3.14159*2)),mean_E,sigma);
  double integral_gaus2 = fgaus->Integral(-10,10);
  std::cout<<"Integral for gaus function: "<<integral_gaus2<<std::endl;

  std::cout<<"Integral for gaus function/binning: "<<integral_gaus2/binning<<std::endl;

  double xx2[2]={0.2,0.36};
  double yy2[2]={0,15000};
  TGraph *graph2 = new TGraph (2,xx2,yy2);
  graph2->GetXaxis()->SetRangeUser(xx2[0], xx2[1]);
  graph2->GetYaxis()->SetRangeUser(yy2[0], yy2[1]);
  graph2->SetMarkerStyle(1);
  graph2->GetXaxis()->SetTitle("E_{#gamma} (STM) Signal [MeV]");
  graph2->GetYaxis()->SetTitle("Counts");
  graph2->GetYaxis()->SetTitleOffset(1.65);

  hsignal->SetFillStyle(3001);
  hsignal->SetLineColor(kCyan-3);
  hsignal->SetFillColor(kCyan-3);
  hsignal->Draw("HIST");

  gPad->Update();
  stat[1] = (TPaveStats*)hsignal->FindObject("stats");
  stat[1]->SetY1NDC(.68);
  stat[1]->SetY2NDC(.85);
  stat[1]->SetX1NDC(0.2); //new x start position
  stat[1]->SetX2NDC(0.47);
  stat[1]->SetTextSize(0.043);
  stat[1]->SetTextColor(kCyan-3);
  graph2->Draw("ap");
  hsignal->Draw("HIST,same");
  stat[1]->Draw("same");
  fgaus->SetLineColor(kGreen-3);
  fgaus->Draw("same");

  std::string str_latex3 = "f(E) = #scale[1]{A#timese^{- #frac{1}{2} ( #frac{E-mean}{#sigma} )^{2}}}";
  char* char_latex3 = const_cast<char*>(str_latex3.c_str());
  TLatex latex3;
  latex3.DrawLatexNDC(.5,.75,char_latex3);
  gPad->RedrawAxis();

  string Signalhisto_str = "Gen Signal Histogram (#sigma_{HPGe}= "+sigmastream.str()+" keV)";
  char*  Signalhisto_char = const_cast<char*>(Signalhisto_str.c_str());

  string Signalfunct_str = "#splitline{Gen Signal Function (#sigma_{HPGe}= "+sigmastream.str()+" keV,}{A=(NXrays#timesbin width)/(#sigma_{HPGe}#sqrt{2#pi}))}";
  char*  Signalfunct_char = const_cast<char*>(Signalfunct_str.c_str());

  auto legend2 = new TLegend(0.35,0.6,0.7,0.8);
  legend2->AddEntry(hsignal,Signalhisto_char,"f");
  legend2->AddEntry(fgaus,Signalfunct_char,"l");
  legend2->Draw("same");

}
