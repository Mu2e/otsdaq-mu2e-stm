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

void Check_Integrals(){

  gRandom->SetSeed (0);
  double frange[2] = {0.04,2};
  double frange_signal[2] = {0.3,0.4};
  int Nbins = 1000;
  double binning = (frange[1]-frange[0])/Nbins;
  double fitbrems_binning = 0.00196;

  double pulseNumBrems = 50000;
  double pulseNumXrays = 10000;

  double mean_E = 0.347;
  double sigma = 0.003;

  //sample
  pulseNumBrems = gRandom->Poisson(pulseNumBrems);
  pulseNumXrays = gRandom->Poisson(pulseNumXrays);

  std::cout<<"Number of pulses gen in background: "<<pulseNumBrems<<std::endl;
  std::cout<<"Number of pulses gen in signal: "<<pulseNumXrays<<std::endl;

  //Background
  //Best fit parameters for Bremsstrahlung
  double p0 = 0.001574;
  double p1 = 2.376;
  double p2 = -0.9758;
  double p3 = 0.0001229;
  //Number of fit parameters
  int NP = 4;

  std::cout<<"-------------FUNCTIONS:"<<std::endl;
  ///////************FUNCTIONS*******************///////
  //This is using functions
  TF1* fbrems = new TF1("fbrems",Bremsfunc,frange[0],frange[1],NP);
  fbrems->SetNpx(300000);
  double fitpar_check[4] = {p0, p1, p2,p3};
  fbrems->SetParameters(fitpar_check[0],fitpar_check[1],fitpar_check[2],fitpar_check[3]);
  //Get the background integral for 1 brems pulse
  double bckbef_function_fullrange = fbrems->Integral(frange[0],frange[1]);
  double bckbef_function_signalrange = fbrems->Integral(frange_signal[0],frange_signal[1]);

  std::cout<<"Integral Background function full range (1 pulse) assuming a binning of "<<fitbrems_binning<<"MeV for the brems fit: "<<bckbef_function_fullrange<<std::endl;
  //Correct integral by binning used
  bckbef_function_fullrange = (binning/fitbrems_binning)*bckbef_function_fullrange;
  std::cout<<"Integral Background function full range (1 pulse) assuming a binning of "<<binning<<"MeV for the brems fit: "<<bckbef_function_fullrange<<std::endl;


  //Multiply the function by pulseNumBrems
  double height_brems = (pulseNumBrems/bckbef_function_fullrange)*p0;
  double height_brems2 = (pulseNumBrems/bckbef_function_fullrange)*p3;
  double fitpar[4] = {height_brems, p1, p2,height_brems2};
  fbrems->SetParameters(fitpar[0],fitpar[1],fitpar[2],fitpar[3]);
  fbrems->Draw("");
  //Get the background integral
  double bck_function_fullrange = fbrems->Integral(frange[0],frange[1]);
  double bck_function_signalrange = fbrems->Integral(frange_signal[0],frange_signal[1]);

  std::cout<<"Integral Background function full range: "<<bck_function_fullrange<<std::endl;
  std::cout<<"Integral Background function signal range: "<<bck_function_signalrange<<std::endl;

  //Signal
  TF1* fgaus;
  TF1* fsignal;
  //Get the integral of a Gaussian with the sigma given
  fgaus= new TF1("fgaus","gaus",-10,10);
  fgaus->SetNpx(300000);
  fgaus->SetParameters(1,0,sigma);
  double integral_gaus = fgaus->Integral(-100,100);
  std::cout<<"Integral of theoretical gaussian: "<<integral_gaus<<std::endl;
  //Generate the signal as a Gaussian
  fsignal = new TF1("fsignal","gaus",frange[0],frange[1]);
  fsignal->SetNpx(300000);
  double height_gaus = pulseNumXrays/integral_gaus;
  fsignal->SetParameters(height_gaus,mean_E,sigma);
  //Get the signal integral
  double signal_function_fullrange = fsignal->Integral(frange[0],frange[1]);
  double signal_function_signalrange = fsignal->Integral(frange_signal[0],frange_signal[1]);

  std::cout<<"Integral Signal function full range: "<<signal_function_fullrange<<std::endl;
  std::cout<<"Integral Signal function signal range: "<<signal_function_signalrange<<std::endl;

  std::cout<<"-------------HISTOGRAMS:"<<std::endl;
  ///////************HISTOGRAMS*******************///////
  //This is using histograms
  std::cout<<"nbins: "<<Nbins<<std::endl;
  //Bremsstrahlung
  TH1F *hBrems = new TH1F("hBrems", "", Nbins,frange[0],frange[1]);
  hBrems->FillRandom("fbrems",pulseNumBrems);
  //hBrems->Draw("");
  double bck_histogram_fullrange = hBrems->Integral(hBrems->FindFixBin(frange[0]), hBrems->FindFixBin(frange[1]), "WIDTH");
  std::cout<<"Integral Background histogram full range (width - multiplies by bin width): "<<bck_histogram_fullrange<<std::endl;
  double bck_histogram_fullrange_now = hBrems->Integral(hBrems->FindFixBin(frange[0]), hBrems->FindFixBin(frange[1]), "");
  std::cout<<"Integral Background histogram full range (no width): "<<bck_histogram_fullrange_now<<std::endl;
  std::cout<<""<<std::endl;

  //Signal
  TH1F* hsignal = new TH1F("hsignal","",Nbins,frange[0],frange[1]);
  hsignal->FillRandom("fsignal",pulseNumXrays);
  //Get the signal integral
  double signal_histogram_fullrange = hsignal->Integral(hsignal->FindFixBin(frange[0]), hsignal->FindFixBin(frange[1]), "WIDTH");
  std::cout<<"Integral Signal histogram full range (width - multiplies by bin width): "<<signal_histogram_fullrange<<std::endl;
  double signal_histogram_fullrange_now = hsignal->Integral(hsignal->FindFixBin(frange[0]), hsignal->FindFixBin(frange[1]), "");
  std::cout<<"Integral Signal histogram full range (no width): "<<signal_histogram_fullrange_now<<std::endl;




}
