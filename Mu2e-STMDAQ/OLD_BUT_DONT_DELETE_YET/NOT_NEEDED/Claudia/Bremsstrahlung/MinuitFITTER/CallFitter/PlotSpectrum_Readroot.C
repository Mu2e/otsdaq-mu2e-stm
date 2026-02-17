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
#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"

//1- This code reads the Data (a histogam with the energy spectrum) and plot the data and best fit function

void PlotSpectrum_Readroot(std::string rootname, double x_low, double x_max, std::string rate_str, std::string resolution_str, std::string time_str){

  gROOT->SetStyle("ATLAS");
  TCanvas* c = new TCanvas("c");
 
  //***********READ DATA***********// 
  std::cout<<rootname<<std::endl;
  
  TFile *infile = new TFile(rootname.c_str());
  double TimeSim = std::stod(time_str);
  double RateSim = std::stod(rate_str);
  double ResolSim = std::stod(resolution_str);
  int NP = 7;
  std::cout<<"Time simulation: "<<TimeSim<<"s Rate  simulation: "<<RateSim<<" Resolution simulation: "<<ResolSim<<std::endl;
  
  //***************GET HISTOGRAM AND FIT*****************//
  
  TH1D *hSTM = (TH1D*)infile->Get("hSTM1");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars1");
  TMatrixD *SeedFitPars = (TMatrixD*)infile->Get("SeedFitPars1");

  int Nentries = hSTM->GetEntries();
  int Nbins = hSTM->GetNbinsX();
  double bin_width = hSTM->GetBinWidth(0);

  std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;
  //***********Initialise Minuit Class***********//
  
  //Calculate sqrt(cov matrix)
  TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
  TMatrixD _SeedFitPars(1,NP,SeedFitPars->GetMatrixArray(),"");

  _BestFitPars.Print();
  _SeedFitPars.Print();
  
   //Recover best fit parameters 
  double best_fitpar[NP];
  
  for( int i = 0 ; i < NP ; i++ ){
    best_fitpar[i] = _BestFitPars[0][i];
  }

  double seed_fitpar[NP];

  for( int i = 0 ; i < NP ; i++ ){
    seed_fitpar[i] = _SeedFitPars[0][i];
  }

  TF1* ffit;

  double reco_peaks;
  double reco_sigma;

  double range[2];
  range[0]=0.04;
  range[1]=2;
  
  //Initialise S+B function
  double FitOption = 2000; //not used
  double seed = 1; //not used
  MinuitFitter *fitter = new MinuitFitter(FitOption, seed);
  std::string  genfunc_name = "fgenSB";
  char* char_genfunc_name = const_cast<char*>(genfunc_name.c_str());
  fitter->Init_SBfunc(char_genfunc_name, range);
  fitter->SetPar_SBfunc(best_fitpar);
   
  //***********PLOT DATA***********//
  //Draw Minuit fit

  ffit = fitter->return_fSB();

  double ymax;
  ymax = hSTM->GetMaximum();
  //double plot_y_range[2]={0,ymax};
  double plot_y_range[2]={0,1500};
  //double plot_x_range[2]={0.04,2};                         
  //double plot_y_range[2]={0,1000};
  //double plot_x_range[2]={0.32,0.37};
  //double plot_x_range[2]={0.04,1};
  //double plot_x_range[2]={0.05,0.5};
  double plot_x_range[2]={x_low, x_max};   
  TGraph *graph1 = new TGraph (2, plot_x_range, plot_y_range);
  graph1->GetXaxis()->SetRangeUser(plot_x_range[0], plot_x_range[1]);
  graph1->GetYaxis()->SetRangeUser(plot_y_range[0], plot_y_range[1]);
  graph1->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
  string yaxis_str;
  char*  yaxis_char;
  yaxis_str = "Counts / run time "+std::to_string(int(TimeSim))+" s";
  yaxis_char = const_cast<char*>(yaxis_str.c_str());
  graph1->GetYaxis()->SetTitle(yaxis_char);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  
  hSTM->SetFillColor(kOrange-3);
  hSTM->SetLineColor(kOrange-3);
    
  hSTM->SetFillStyle(3001);
  hSTM->Draw("same");

  ffit->SetLineColor(kBlack);
  ffit->SetLineStyle(2);
  ffit->Draw("same");
    
  
  c->Print("ReadRootFile_Fitrange_0.04_2.000_Timesim_2000s_1kHz_0.0010MeV.png");
  
}
