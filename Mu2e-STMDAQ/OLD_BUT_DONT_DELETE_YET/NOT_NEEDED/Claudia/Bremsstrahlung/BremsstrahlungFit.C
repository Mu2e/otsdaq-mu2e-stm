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

#define LogLike
//#define LeastSquares

//Run Config
int nexperiments = 1;
int nbins = 20000;
//Control couts
bool debugout = false;
bool debugout2 = false;
//Different dra options
bool drawplot = true; //signal+background (for just 1 experiment)
bool draw_just_signal = false; //signal (for just 1 experiment)
bool drawplotdivide = false; //Reco and true: fot background and signal (for more than 1 experiment)
//Scale plots by N POT
bool doscale = false;
//Logfile
bool save_to_logfile = false;
bool debugoutlog = true;

bool reject;
double xreject[2];

double Bremsfunc(Double_t *x, Double_t *p)
{
  double A = p[0];
  double B = p[1];
  double C = p[2];
  double D = p[3];

  if (reject && x[0] > xreject[0] && x[0] < xreject[1]) {
    TF1::RejectPoint();
    return 0;
  }
  double f = A/(exp(B*x[0])+C)+D;
  return f;
}


//Brems rate: kHz, TimeSim: s, mean and sigma: MeV
std::vector<std::vector<double>>Signal_over_Background(double Bremsrate_kHz, double TimeSim, double meanE, double sigma, double sigmacut, double seed){

  //Result
  std::vector<std::vector<double>> SignalBackground;
  std::vector<double> SignalBackgroundHisto;
  std::vector<double> SignalBackgroundFit;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);

  int dimpeaks;
  if (meanE==0){dimpeaks=4;}
  else {dimpeaks=1;}

  //****************************
  //-----SET THESE VARIABLES----
  //****************************
  //Plot range in x
  double xx1[2]={0.3,0.5};
  //double xx1[2]={0.05,1.9};
  //Function and histogram range
  double frange[2] = {0.04,2};
  double photopeak_range[dimpeaks][2];
  double fit_range[dimpeaks][2];
  double bck_histo_signalrange[dimpeaks];
  double mean_E[dimpeaks];
  if (meanE==0){
    mean_E[0]=0.066;
    mean_E[1]=0.347;
    mean_E[2]=0.844;
    mean_E[3]=1.809;
  }
  else {
    mean_E[0]=meanE;
  }
  //Best fit parameters for Bremsstrahlung
  double p0 = 0.00157448;
  double p1 = 2.37638;
  double p2 = -0.975819;
  double p3 = 0.000122886;
  //Number of fit parameters
  int NP = 4;
  //Number of histo bins
  double binning = (frange[1]-frange[0])/nbins;
  double fitbrems_binning = 0.00196;
  //Assume these parameters
  double prot_per_maininjectorcycle = 8e12;
  double maininjectortime = 1.4;
  double STM_accept = 8.03e-10;
  double stoppedmuons_proton = 0.0016;
  double percentage_66keV = 0.62;
  double percentage_347keV = 0.8;
  double percentage_844keV = 0.05704;
  double percentage_1809keV = 0.3162;
  //*************************************

  //Rate Hz
  double rateHz = Bremsrate_kHz*1000;

  //Number of bremsstrahlung photons in the sample
double pulseNumBrems = rateHz*TimeSim;

double pulseNumXrays[dimpeaks];

double rate_prot_s = prot_per_maininjectorcycle/maininjectortime;
double POT = rate_prot_s*TimeSim;
double scalePOT = 1./POT;
double rate_stoppedmuons_s = rate_prot_s*stoppedmuons_proton;
double rate_66keV_s = rate_stoppedmuons_s*percentage_66keV;
double rate_347keV_s = rate_stoppedmuons_s*percentage_347keV;
double rate_844keV_s = rate_stoppedmuons_s*percentage_844keV;
double rate_1809keV_s = rate_stoppedmuons_s*percentage_1809keV;
//Rate Hz from X-rays at the STM
double rate_66keV_s_STM = rate_66keV_s*STM_accept;
double rate_347keV_s_STM = rate_347keV_s*STM_accept;
double rate_844keV_s_STM = rate_844keV_s*STM_accept;
double rate_1809keV_s_STM = rate_1809keV_s*STM_accept;

TCanvas* c1;
if(drawplot==true){
  //Plot just brems
  //c1 = new TCanvas("");
  //c1->Divide(1,1);
  //Plot signal and brems
  c1 = new TCanvas("c1","c1",0,0,1200,500);
  c1->Divide(2,1);
 }

if(debugout==true){
  std::cout<<"Theoretical pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl;
  std::cout<<"--Sample number of pulses with a Poisson distribution---"<<std::endl;
 }
pulseNumBrems = gRandom->Poisson(pulseNumBrems);
if(debugout==true){std::cout<<"Number of pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl;}

//Bremsstrahlung function
reject=false;
TF1* fbrems = new TF1("fbrems",Bremsfunc,frange[0],frange[1],NP);
//Take into account the binning in fit
double fitpar[4] = {p0, p1, p2,p3};
fbrems->SetParameters(fitpar[0],fitpar[1],fitpar[2],fitpar[3]);
//Get the background integral for 1 brems pulse
double bckbef_function_fullrange = fbrems->Integral(frange[0],frange[1]);
if(debugout==true){std::cout<<"Integral Background function full range (1 pulse) assuming a binning of "<<fitbrems_binning<<"MeV for the brems fit: "<<bckbef_function_fullrange<<std::endl;}
//If we have a different binning we need to redefine the height of the function
//Correct parameters by binning used
double height_brems = (binning/fitbrems_binning)*p0;
double height_brems2 = (binning/fitbrems_binning)*p3;
fitpar[0] = height_brems;
fitpar[3] = height_brems2;
fbrems->SetParameters(fitpar[0],fitpar[1],fitpar[2],fitpar[3]);
double bck = fbrems->Integral(frange[0],frange[1]);
//bck is just (binning/fitbrems_binning)*bckbef_function_fullrange
if(debugout==true){std::cout<<"Integral Background function full range (1 pulse) assuming a binning of "<<binning<<"MeV for the brems fit: "<<bck<<std::endl;}

TH1F *hBrems = new TH1F("hBrems", "", nbins,frange[0],frange[1]);
hBrems->FillRandom("fbrems",pulseNumBrems);

if(doscale==true){
  hBrems->Scale(scalePOT);
 }

//Generate weights for 1kHz
//Number of pulses for a rate of 1kHz during 100,000s bremsstrahlung
/*  double pulseNumBrems_aux, weight_aux;
  double weight[nbins];
  pulseNumBrems_aux = 1000*100000;
  //Fill the histogram for 1kHz
  TH1F *hBrems = new TH1F("hBrems", "", nbins,frange[0],frange[1]);
  hBrems->FillRandom("fbrems",pulseNumBrems_aux);

  for(int i=0; i<nbins;i++){
    weight_aux = hBrems->GetBinContent(i);
    weight[i] = pulseNumBrems * weight_aux / pulseNumBrems_aux;
    //bin number and weight to increment the bin height
    hBrems->SetBinContent(i,weight[i]);
}

  double try1 = mean_E[0]-sigmacut*sigma;
  double try2 = mean_E[0]+sigmacut*sigma;
  std::cout<<"CHECK Integral to histo: "<<hBrems->Integral(hBrems->FindFixBin(try1), hBrems->FindFixBin(try2))<<std::endl;
*/

double bck_histo =  hBrems->Integral(hBrems->FindFixBin(frange[0]), hBrems->FindFixBin(frange[1]), "");

//Define signal region
for(int i =0; i < dimpeaks; i++){
  //Signal range
  photopeak_range[i][0] = mean_E[i]-sigmacut*sigma;
  photopeak_range[i][1] = mean_E[i]+sigmacut*sigma;
  bck_histo_signalrange[i] =  hBrems->Integral(hBrems->FindFixBin(photopeak_range[i][0]), hBrems->FindFixBin(photopeak_range[i][1]), "");
  if(debugout==true){
    std::cout<<"Integral for brems photons in signal range (histogram) E="<<mean_E[i]<<" MeV: "<<bck_histo_signalrange[i]<<std::endl;
  }
 }


if(debugout==true){std::cout<<"Integral for brems photons (function): "<<bck<<std::endl;
  std::cout<<"Integral for brems photons (histogram): "<<bck_histo<<std::endl;
  std::cout<<"Simulation time: "<<TimeSim<<" s, Total POT: "<<POT<<std::endl;
  std::cout<<"Mean energy: "<<meanE<<" MeV"<<std::endl;
  std::cout<<"Detector resolution: "<<sigma<<" MeV"<<std::endl;
  std::cout<<"Bremsstrahlung Rate: "<<rateHz<<" Hz"<<std::endl;
  std::cout<<"Bremsstrahlung pulses: "<<pulseNumBrems<<std::endl;

  std::cout<<"Protons per main injector cycle: "<<prot_per_maininjectorcycle<<std::endl;
  //std::cout<<""<<std::endl;
  std::cout<<"STM acceptance: "<<STM_accept<<" prot per s: "<<rate_prot_s<<" stopped muons per second: "<<rate_stoppedmuons_s<<std::endl;
  //std::cout<<""<<std::endl;
  std::cout<<"rate_66keV_s: "<<rate_66keV_s<<" rate_347keV_s: "<<rate_347keV_s<<" rate_844keV_s: "<<rate_844keV_s<<" rate_1809keV_s: "<<rate_1809keV_s<<std::endl;
  //std::cout<<""<<std::endl;
  std::cout<<"rate_66keV_s_STM: "<<rate_66keV_s_STM<<" rate_347keV_s_STM: "<<rate_347keV_s_STM<<" rate_844keV_s_STM: "<<rate_844keV_s_STM<<" rate_1809keV_s_STM: "<<rate_1809keV_s_STM<<std::endl;
 }

if(meanE==0.066){pulseNumXrays[0] = rate_66keV_s_STM*TimeSim;
  if(debugout==true){std::cout<<"66keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
 }
 else if(meanE==0.347){pulseNumXrays[0] = rate_347keV_s_STM*TimeSim;
   if(debugout==true){std::cout<<"347keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
 }
 else if(meanE==0.844){pulseNumXrays[0] = rate_844keV_s_STM*TimeSim;
   if(debugout==true){std::cout<<"844keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
 }
 else if(meanE==1.809){pulseNumXrays[0] = rate_1809keV_s_STM*TimeSim;
   if(debugout==true){std::cout<<"1809keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
 }
 else if(meanE==0){
   pulseNumXrays[0]=rate_66keV_s_STM*TimeSim;
   pulseNumXrays[1]=rate_347keV_s_STM*TimeSim;
   pulseNumXrays[2]=rate_844keV_s_STM*TimeSim;
   pulseNumXrays[3]=rate_1809keV_s_STM*TimeSim;
 }
 else{std::cout<<"X-Ray energy not valid"<<std::endl; exit(0);}

//Generate signal
TF1* fgaus[dimpeaks];
TF1* fgausXray[dimpeaks];
TH1F* hsignal[dimpeaks];
double integral_gaus;
double s[dimpeaks];
double sum_integral[dimpeaks];
double back_integral1[dimpeaks];
double back_integral2[dimpeaks];
double backsignal_integral[dimpeaks];
double signal_integral[dimpeaks];
double bcksig_histo_signalrange[dimpeaks];

for(int i =0; i < dimpeaks; i++){
  if(debugout==true){
    std::cout<<"Number of pulses in X-Ray peak: "<<pulseNumXrays[i]<<std::endl;
  }
  pulseNumXrays[i] = gRandom->Poisson(pulseNumXrays[i]);
  //Number of pulses in X-Ray peak
  if(debugout==true){
    std::cout<<"--Sample number of pulses with a Poisson distribution---"<<"MeanE: "<<mean_E[i]<<std::endl;
    std::cout<<"Number of pulses in X-Ray peak: "<<pulseNumXrays[i]<<std::endl;
  }


  //Get the integral of a Gaussian with the sigma given
  /*fgaus[i]= new TF1("fgaus","gaus",-10,10);
    fgaus[i]->SetNpx(300000);
    double height_gaus = 1;
    fgaus[i]->SetParameters(height_gaus,0,sigma);
    integral_gaus = fgaus[i]->Integral(-100,100);
    if(debugout==true){std::cout<<"Integral for gaus: "<<integral_gaus<<std::endl;}
    //Normalise the signal to the integral
    height_gaus = 1/integral_gaus;
  */
  //Generate the signal as a Gaussian
  fgausXray[i] = new TF1("fgausXray","gaus",frange[0],frange[1]);
  fgausXray[i]->SetParameters(1,mean_E[i],sigma);

  //Fill the Xray signal histogram
  string hname = "hsignal"+std::to_string(i);
  char*  hname_char = const_cast<char*>(hname.c_str());
  hsignal[i] = new TH1F(hname_char,"",nbins,frange[0],frange[1]);
  hsignal[i]->FillRandom("fgausXray",pulseNumXrays[i]);

  s[i] = hsignal[i]->Integral(hsignal[i]->FindFixBin(photopeak_range[i][0]), hsignal[i]->FindFixBin(photopeak_range[i][1]), "");
  if(debugout==true){std::cout<<"Integral for signal (histogram) - depends on the photopeak range defined (sigmacut): "<<s[i]<<std::endl;}

  if(doscale==true){
    hsignal[i]->Scale(scalePOT);
  }

  hBrems->Add(hsignal[i]);

  bcksig_histo_signalrange[i] =  hBrems->Integral(hBrems->FindFixBin(photopeak_range[i][0]), hBrems->FindFixBin(photopeak_range[i][1]), "");
  if(debugout==true){std::cout<<"Integral for signal+back (histogram): "<<bcksig_histo_signalrange[i]<<std::endl;}
 }


//****************************
//-----PLOTTING---------------
//****************************
double ymax, ymax1, ymax2;
if(meanE==0){
  ymax1 = hBrems->GetMaximum();
  ymax2 = hsignal[0]->GetMaximum();}
 else{
   ymax1 = hBrems->GetMaximum()/2;
   ymax2 = hsignal[0]->GetMaximum()/2;}
if(ymax2>ymax1){ymax=ymax2;}
 else{ymax=ymax1;}
double yy1[2]={0,ymax};
TGraph *graph1 = new TGraph (2,xx1,yy1);
graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
graph1->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
string yaxis_str;
char*  yaxis_char;
if(doscale==true){
  yaxis_str = "Counts / POT";
  yaxis_char = const_cast<char*>(yaxis_str.c_str());
 }
 else{
   yaxis_str = "Counts / run time "+std::to_string(int(TimeSim))+" s";
   yaxis_char = const_cast<char*>(yaxis_str.c_str());
 }
graph1->GetYaxis()->SetTitle(yaxis_char);
graph1->SetTitle("");
graph1->SetMarkerStyle(1);

std::string str_latex = "~ 6#times10^{12} POT/s, 10^{10} stopped #mu/s";
char* char_latex = const_cast<char*>(str_latex.c_str());
TLatex latex;

for(int i =0; i < dimpeaks; i++){
  hsignal[i]->SetFillColor(kCyan-3);
  hsignal[i]->SetLineColor(kCyan-3);
  hsignal[i]->SetFillStyle(3001);
 }


hBrems->SetFillColor(kOrange-3);
hBrems->SetLineColor(kOrange-3);
hBrems->SetFillStyle(3001);

//Fit function to background
TF1* fx[2];
TF1 *fleft;
TF1 *fright;

//Fit background around peaks
if(meanE!=0){
  for(int i =0; i < dimpeaks; i++){

    xreject[0] = photopeak_range[i][0];
    xreject[1] = photopeak_range[i][1];

    fit_range[i][0] = mean_E[i]-100*sigmacut*sigma;
    fit_range[i][1] = mean_E[i]+100*sigmacut*sigma;

    reject = true;
    fx[0] = new TF1("fx0",Bremsfunc,fit_range[i][0],fit_range[i][1],NP);
    fx[0]->SetNpx(300000);
    fx[0]->SetLineColor(kRed);
    double height_histo1 = bck_histo*fitpar[0];
    double height_histo2 = bck_histo*fitpar[3];
    fx[0]->SetParameters(height_histo1,fitpar[1],fitpar[2],height_histo2);
    TFitResultPtr fp;
      #ifdef LogLike
    fp = hBrems->Fit(fx[0],"S0QL","", fit_range[i][0],fit_range[i][1]);
      #endif
      #ifdef LeastSquares
    fp = hBrems->Fit(fx[0],"S0Q","", fit_range[i][0],fit_range[i][1]);
      #endif
    int status = fp->Status();
    double chi2reject = fx[0]->GetChisquare();
    double ndf = fx[0]->GetNDF();
    double chi2ndf = chi2reject/ndf;

    if(status!=0){std::cout<<"Fit FAILED... stop"<<std::endl; exit(0);}
    if(debugout==true){std::cout<<"Chi2 reject signal: "<<chi2reject<<" ndf: "<<ndf<<std::endl;
      std::cout<<""<<std::endl;}

    //store 2 separate functions for visualization
    reject = false;
    fx[1] = new TF1("fx1",Bremsfunc,fit_range[i][0],fit_range[i][1],NP);
    fx[1]->SetParameters(fx[0]->GetParameters());
    /*hBrems->Fit(fx[1],"0","", fit_range[i][0],fit_range[i][1]);
    double chi2noreject = fx[1]->GetChisquare();
    std::cout<<"Chi2 full histogram: "<<chi2noreject<<std::endl;*/

    fleft = new TF1("fleft",Bremsfunc,fit_range[i][0],photopeak_range[i][0],NP);
    fleft->SetParameters(fx[1]->GetParameters());

    fright = new TF1("fright",Bremsfunc,photopeak_range[i][1],fit_range[i][1],NP);
    fright->SetParameters(fx[1]->GetParameters());

    //Get integral of this function (background):
    double back_integral_fit = fx[1]->Integral(photopeak_range[i][0], photopeak_range[i][1]);

    //Get integral of the background+signal
    backsignal_integral[i] = hBrems->Integral(hBrems->FindFixBin(photopeak_range[i][0]), hBrems->FindFixBin(photopeak_range[i][1]));

    double backsignal_integral_width = hBrems->Integral(hBrems->FindFixBin(photopeak_range[i][0]), hBrems->FindFixBin(photopeak_range[i][1]),"width");

    //Get the signal
    signal_integral[i] = backsignal_integral[i] - back_integral_fit/binning;

    double signalplusback = bck_histo_signalrange[i]+s[i];

    if(debugout==true){
      std::cout<<"binning: "<<binning<<" Nbins: "<<nbins<<std::endl;
      std::cout<<"THEORETICAL RESULT: background: "<<bck_histo_signalrange[i]<<" signal: "<<s[i]<<" signal+back: "<<signalplusback<<std::endl;
      std::cout<<" "<<std::endl;
      std::cout<<"EXPERIMENTAL RESULT: "<<std::endl;
      std::cout<<"Integral background fit function: "<<back_integral_fit<<std::endl;
      std::cout<<"Integral background fit/binning: "<<back_integral_fit/binning<<std::endl;
      std::cout<<"Integral signal+background width: "<<backsignal_integral_width<<std::endl;
      std::cout<<"Integral signal+background: "<<backsignal_integral[i]<<std::endl;
      std::cout<<"Integral signal(back-fit): "<<signal_integral[i]<<std::endl;


    }

    SignalBackgroundHisto.push_back(signalplusback);
    SignalBackgroundHisto.push_back(bck_histo_signalrange[i]);
    SignalBackgroundHisto.push_back(s[i]);

    SignalBackgroundFit.push_back(signalplusback);
    SignalBackgroundFit.push_back(back_integral_fit/binning);
    SignalBackgroundFit.push_back(signal_integral[i]);

  }
 }// if meanE=0

if(drawplot==true){
  double sigmakeV = sigma*1000;
  std::stringstream sigmastream;
  sigmastream << std::fixed << std::setprecision(1) << sigmakeV;


  c1->cd(1);
  graph1->Draw("ap");
  latex.DrawLatexNDC(.35,.85,char_latex);
  for(int i =0; i < dimpeaks; i++){
    hsignal[i]->Draw("same,HIST");
  }

  string Signal_str = "Signal (#sigma_{HPGe}= "+sigmastream.str()+" keV)";
  char*  Signal_char = const_cast<char*>(Signal_str.c_str());

  auto legend1 = new TLegend(0.35,0.6,0.7,0.8);
  legend1->AddEntry(hsignal[0],Signal_char,"f");
  legend1->Draw("same");
  gPad->RedrawAxis();

  c1->cd(2);
  graph1->Draw("ap");
  latex.DrawLatexNDC(.35,.85,char_latex);
  hBrems->SetFillStyle(3001);
  hBrems->SetLineColor(kOrange-3);
  hBrems->SetFillColor(kOrange-3);
  hBrems->Draw("same, HIST");
  if(meanE!=0){
    fleft->SetLineColor(kRed);
    fright->SetLineColor(kRed);
    fleft->Draw("same");
    fright->Draw("same");
  }

  string Bremsrate1_kHz_str = "#splitline{Signal (#sigma_{HPGe}= "+sigmastream.str()+" keV) + }{\n"+std::to_string(int(Bremsrate_kHz))+" kHz Bremsstrahlung}";
  char*  Bremsrate1_kHz_char = const_cast<char*>(Bremsrate1_kHz_str.c_str());

  auto legend2 = new TLegend(0.35,0.6,0.7,0.8);
  legend2->AddEntry(hBrems,Bremsrate1_kHz_char,"f");
  legend2->Draw("same");
  gPad->RedrawAxis();
 }

//Delete objects
if(drawplot==false){
  delete gROOT->FindObject("c1");
  delete gROOT->FindObject("hBrems");
  delete gROOT->FindObject("fbrems");
  delete gROOT->FindObject("fx1");
  delete gROOT->FindObject("fx0");
  delete gROOT->FindObject("fleft");
  delete gROOT->FindObject("fright");
  delete gROOT->FindObject("fgausXray");
  delete gROOT->FindObject("fgaus");
  for(int i = 0; i < dimpeaks; i++){
    string hname = "hsignal"+std::to_string(i);
    char*  hname_char = const_cast<char*>(hname.c_str());
    delete gROOT->FindObject(hname_char);
  }
 }

SignalBackground.push_back(SignalBackgroundHisto);
SignalBackground.push_back(SignalBackgroundFit);
return SignalBackground;
}

void BremsstrahlungFit(double Bremsrate_kHz, double TimeSim, double meanE, double sigma, double sigmacut){

  TStopwatch t;
  t.Start();

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);
  TCanvas* c1;

  if(save_to_logfile==true){
    std::stringstream stream_meanE, stream_sigma, stream_sigmacut;
    stream_meanE << std::fixed << std::setprecision(3) << meanE;
    stream_sigma << std::fixed << std::setprecision(4) << sigma;
    stream_sigmacut << std::fixed << std::setprecision(1) << sigmacut;

    std::string logname = "datalog/100s_0.347keV_3cut_20000bins_1000experiments/data-FitHisto"+std::to_string(int(Bremsrate_kHz))+"kHz_"+std::to_string(int(TimeSim))+"s_"+stream_meanE.str()+"keV_sigma_"+stream_sigma.str()+"keV_cut_"+stream_sigmacut.str()+".log";
    char* lognames = const_cast<char*>(logname.c_str());

    gSystem->RedirectOutput(lognames);
  }
  if(TimeSim==0){
    exit(0);
    /*TimeSim = 10;
    double accuracy = 10;
    double acc_limit = 0.01;
    while(accuracy > acc_limit){
      accuracy = Signal_over_Background(Bremsrate_kHz,TimeSim,meanE,sigma,sigmacut);
      std::cout<<"Check accuracy: "<<accuracy<<" time: "<<TimeSim<<std::endl;
      std::cout<<"-----------------------------------------------"<<std::endl;
      TimeSim = TimeSim+1000;

  }*/
  }
  else{

    std::cout<<"Nexperiments: "<<nexperiments<<std::endl;

    TH1F *hbackground_histo, *hbackground_fit, *hsignal_histo, *hsignal_fit, *hdeltasignal, *hdeltabackground;
    TGraph *graph[4];

    std::vector<std::vector<double>> SignalBackground;
    std::vector<double> SignalBackgroundHisto;
    std::vector<double> SignalBackgroundFit;
    std::vector<double> seedvector;
    double rangemin_b, rangemax_b, rangemin_s, rangemax_s;

    double range_histo = 500;
    double Nbins = 200;
    double range_plot = 300;
    double range_plot_delta = 0.2;

    for(int i = 0; i < nexperiments; i++){
      //This seed is for fill random fixed seed, same sequence everytime we run it
      //double seed = i+1;
      //gRandom->SetSeed(seed);

      //if seed = 0 random seed, different sequence everytime we run it
      double seed = 0;
      gRandom->SetSeed(seed);
      //std::cout<<"seed: "<<gRandom->GetSeed()<<std::endl;
      seedvector.push_back(gRandom->GetSeed());
      //std::cout<<"---------------------------------------"<<std::endl;
      SignalBackground = Signal_over_Background(Bremsrate_kHz,TimeSim,meanE,sigma,sigmacut,seed);
      double signalbackgroundhisto = SignalBackground[0].at(0);
      double backgroundhisto = SignalBackground[0].at(1);
      double signalhisto = SignalBackground[0].at(2);

      double signalbackgroundfit = SignalBackground[1].at(0);
      double backgroundfit = SignalBackground[1].at(1);
      double signalfit = SignalBackground[1].at(2);

      if(i==0){
        rangemin_b = backgroundhisto - range_histo;
        rangemax_b = backgroundhisto + range_histo;

        rangemin_s = signalhisto - range_histo;
        rangemax_s = signalhisto + range_histo;

        hbackground_histo = new TH1F("Bhistogram true", "", Nbins,rangemin_b,rangemax_b);
        hbackground_fit = new TH1F("BFit reco", "", Nbins,rangemin_b,rangemax_b);

        hsignal_histo = new TH1F("Shistogram true", "", Nbins,rangemin_s,rangemax_s);
        hsignal_fit = new TH1F("SFit reco", "", Nbins,rangemin_s,rangemax_s);

        hdeltasignal = new TH1F("Signal", "", Nbins,(-1)*range_plot_delta,range_plot_delta);
        hdeltabackground = new TH1F("Background", "", Nbins,(-1)*range_plot_delta,range_plot_delta);
      }

      double delta_signal = signalhisto-signalfit;
      double delta_back = backgroundhisto-backgroundfit;

      if((backgroundhisto<rangemin_b)||(backgroundhisto>rangemax_b)){
	std::cout<<"ERROR: Incomplete histogram--Background counts out of histogram range, make histogram range bigger"<<std::endl;
      }
      if((signalhisto<rangemin_s)||(signalhisto>rangemax_s)){
	std::cout<<"ERROR: Incomplete histogram--Background counts out of histogram range, make histogram range bigger"<<std::endl;
      }
      hsignal_histo->Fill(signalhisto);
      hbackground_histo->Fill(backgroundhisto);

      hsignal_fit->Fill(signalfit);
      hbackground_fit->Fill(backgroundfit);

      hdeltasignal->Fill(delta_signal/signalhisto);
      hdeltabackground->Fill(delta_back/backgroundhisto);
    }

    if((hsignal_histo->GetEntries()<nexperiments)||(hsignal_fit->GetEntries()<nexperiments)||(hdeltasignal->GetEntries()<nexperiments)){
      std::cout<<"ERROR: Incomplete histogram--Signal counts out of histogram range, make histogram range bigger"<<std::endl;
    }
    if((hbackground_histo->GetEntries()<nexperiments)||(hbackground_fit->GetEntries()<nexperiments)||(hdeltabackground->GetEntries()<nexperiments)){
      std::cout<<"ERROR: Incomplete histogram--Background counts out of histogram range, make histogram range bigger"<<std::endl;
    }
    double Sh = hsignal_histo->GetMean();
    double RMS_Sh = hsignal_histo->GetRMS();

    double Bh = hbackground_histo->GetMean();
    double RMS_Bh = hbackground_histo->GetRMS();

    double Sf = hsignal_fit->GetMean();
    double RMS_Sf = hsignal_fit->GetRMS();

    double Bf = hbackground_fit->GetMean();
    double RMS_Bf = hbackground_fit->GetRMS();

    double accuracy_histo = Sh/sqrt(Bh);
    double accuracy_RMS_histo = Sh/RMS_Bh;

    double accuracy_fit = Sf/sqrt(Bf);
    double accuracy_RMS_fit = Sf/RMS_Bf;


    if(drawplotdivide==true){
      //Plot just brems
      //c1 = new TCanvas("");
      //c1->Divide(1,1);
      //Plot signal and brems
      c1 = new TCanvas("c1","c1",0,0,1200,1200);
      c1->Divide(2,2);
      double xx1[2],yy1[2];
      xx1[0]= hbackground_fit->GetMean()-range_plot;
      xx1[1]= hbackground_fit->GetMean()+range_plot;
      yy1[0]=0;
      yy1[1]= hbackground_fit->GetMaximum()+50;
      graph[0] = new TGraph (2,xx1,yy1);
      graph[0]->SetMarkerStyle(1);

      xx1[0]= hsignal_histo->GetMean()-range_plot;
      xx1[1]= hsignal_histo->GetMean()+range_plot;
      yy1[0]=0;
      yy1[1]= hsignal_histo->GetMaximum()+50;
      graph[1] = new TGraph (2,xx1,yy1);
      graph[1]->SetMarkerStyle(1);

      xx1[0]= hdeltabackground->GetMean()-range_plot_delta;
      xx1[1]= hdeltabackground->GetMean()+range_plot_delta;
      yy1[0]=0;
      yy1[1]= hdeltabackground->GetMaximum()+50;
      graph[2] = new TGraph (2,xx1,yy1);
      graph[2]->SetMarkerStyle(1);

      xx1[0]= hdeltasignal->GetMean()-range_plot_delta;
      xx1[1]= hdeltasignal->GetMean()+range_plot_delta;
      yy1[0]=0;
      yy1[1]= hdeltasignal->GetMaximum()+50;
      graph[3] = new TGraph (2,xx1,yy1);
      graph[3]->SetMarkerStyle(1);

      TPaveStats *stat[2];

      c1->cd(1);
      hbackground_fit->SetFillColor(kGreen-3);
      hbackground_fit->SetLineColor(kBlack);
      hbackground_fit->SetFillStyle(3001);
      hbackground_fit->Draw("HIST");

      gPad->Update();
      stat[0] = (TPaveStats*)hbackground_fit->FindObject("stats");
      stat[0]->SetY1NDC(.74);
      stat[0]->SetY2NDC(.91);
      stat[0]->SetX1NDC(0.2); //new x start position
      stat[0]->SetX2NDC(0.45);
      stat[0]->SetTextSize(0.043);
      stat[0]->SetTextColor(kGreen-3);

      hbackground_histo->SetFillColor(kViolet);
      hbackground_histo->SetLineColor(kBlack);
      hbackground_histo->SetFillStyle(3001);
      hbackground_histo->Draw("HIST,sames");
      gPad->Update();
      stat[1] = (TPaveStats*)hbackground_histo->FindObject("stats");
      stat[1]->SetY1NDC(.74);
      stat[1]->SetY2NDC(.91);
      stat[1]->SetX1NDC(0.2); //new x start position
      stat[1]->SetX2NDC(0.45);
      stat[1]->SetTextSize(0.043);
      stat[1]->SetTextColor(kViolet);

      graph[0]->Draw("ap");
      graph[0]->GetXaxis()->SetTitle("Counts in Background");
      graph[0]->GetYaxis()->SetTitle("Experiments");
      hbackground_fit->Draw("HIST,same");
      stat[0]->Draw("same");
      hbackground_histo->Draw("HIST,same");
      stat[1]->Draw("same");

      c1->cd(2);
      gPad->RedrawAxis();
      hsignal_histo->SetFillColor(kViolet);
      hsignal_histo->SetLineColor(kBlack);
      hsignal_histo->SetFillStyle(3001);
      hsignal_histo->Draw("HIST");

      gPad->Update();
      stat[1] = (TPaveStats*)hsignal_histo->FindObject("stats");
      stat[1]->SetY1NDC(.74);
      stat[1]->SetY2NDC(.91);
      stat[1]->SetX1NDC(0.2); //new x start position
      stat[1]->SetX2NDC(0.45);
      stat[1]->SetTextSize(0.043);
      stat[1]->SetTextColor(kViolet);

      hsignal_fit->SetFillColor(kGreen-3);
      hsignal_fit->SetLineColor(kBlack);
      hsignal_fit->SetFillStyle(3001);
      hsignal_fit->Draw("HIST,sames");

      gPad->Update();
      stat[0] = (TPaveStats*)hsignal_fit->FindObject("stats");
      stat[0]->SetY1NDC(.74);
      stat[0]->SetY2NDC(.91);
      stat[0]->SetX1NDC(0.2); //new x start position
      stat[0]->SetX2NDC(0.45);
      stat[0]->SetTextSize(0.043);
      stat[0]->SetTextColor(kGreen-3);

      graph[1]->Draw("ap");
      graph[1]->GetXaxis()->SetTitle("Counts in Signal");
      graph[1]->GetYaxis()->SetTitle("Experiments");
      hsignal_histo->Draw("HIST,same");
      stat[1]->Draw("same");
      hsignal_fit->Draw("HIST,same");
      stat[0]->Draw("same");

      c1->cd(3);
      gPad->RedrawAxis();
      hdeltabackground->SetFillColor(kBlue);
      hdeltabackground->SetLineColor(kBlack);
      hdeltabackground->SetFillStyle(3001);
      hdeltabackground->Draw("HIST");

      gPad->Update();
      stat[0] = (TPaveStats*)hdeltabackground->FindObject("stats");
      stat[0]->SetY1NDC(.74);
      stat[0]->SetY2NDC(.91);
      stat[0]->SetX1NDC(0.2); //new x start position
      stat[0]->SetX2NDC(0.45);
      stat[0]->SetTextSize(0.043);
      stat[0]->SetTextColor(kBlue);

      graph[2]->Draw("ap");
      graph[2]->GetXaxis()->SetTitle("#DeltaBackground: (true-reco)/true");
      graph[2]->GetYaxis()->SetTitle("Experiments");
      hdeltabackground->Draw("HIST,same");
      stat[0]->Draw("same");

      c1->cd(4);
      gPad->RedrawAxis();
      hdeltasignal->SetFillColor(kBlue);
      hdeltasignal->SetLineColor(kBlack);
      hdeltasignal->SetFillStyle(3001);
      hdeltasignal->Draw("HIST");

      gPad->Update();
      stat[0] = (TPaveStats*)hdeltasignal->FindObject("stats");
      stat[0]->SetY1NDC(.74);
      stat[0]->SetY2NDC(.91);
      stat[0]->SetX1NDC(0.2); //new x start position
      stat[0]->SetX2NDC(0.45);
      stat[0]->SetTextSize(0.043);
      stat[0]->SetTextColor(kBlue);

      graph[3]->Draw("ap");
      graph[3]->GetXaxis()->SetTitle("#DeltaSignal:  (true-reco)/true");
      graph[3]->GetYaxis()->SetTitle("Experiments");
      hdeltasignal->Draw("HIST,same");
      stat[0]->Draw("same");
    }

    if(debugoutlog==true){
      std::cout<<"Rate(kHz): "<<Bremsrate_kHz<<std::endl;
      std::cout<<"Time(s): "<<TimeSim<<std::endl;
      std::cout<<"Resol(MeV): "<<sigma<<std::endl;
      std::cout<<"sigmacut: "<<sigmacut<<std::endl;
      std::cout<<"Energy: "<<meanE<<std::endl;
      std::cout<<"signalmean(counts): "<<Sf<<std::endl;
      std::cout<<"RMSsignal(counts): "<<RMS_Sf<<std::endl;
      //std::cout<<"Real mean Xrays simulated in the peak: "<<pulseNumXrays[0]<<std::endl;
      std::cout<<"backmean(counts): "<<Bf<<std::endl;
      std::cout<<"RMSback(counts): "<<RMS_Bf<<std::endl;
      std::cout<<"UncertaintyRMS/meansignal: "<<accuracy_fit<<std::endl;
      std::cout<<""<<std::endl;
      std::cout<<"RESULT Histogram(true): Signal: "<<Sh<<" RMS Signal: "<<RMS_Sh<<" Background: "<<Bh<<" RMS Background: "<<RMS_Bh<<" accuracy: "<<accuracy_histo<<std::endl;
      std::cout<<"RESULT Fit(reco): Signal: "<<Sf<<" RMS Signal: "<<RMS_Sf<<" Background: "<<Bf<<" RMS Background: "<<RMS_Bf<<" accuracy: "<<accuracy_fit<<std::endl;
      std::cout<<"SEEDARRAY: "<<std::endl;
      for(int i = 0 ; i<nexperiments; i++){
	std::cout<<i<<"- "<<seedvector.at(i)<<std::endl;
      }
    }//debugoutlog==true

  }//else


  t.Stop();
  t.Print();

  if(save_to_logfile==true){
    exit(0);
  }
}
