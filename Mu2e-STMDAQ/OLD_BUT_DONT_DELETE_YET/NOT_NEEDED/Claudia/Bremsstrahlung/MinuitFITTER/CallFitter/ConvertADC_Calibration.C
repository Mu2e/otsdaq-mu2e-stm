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

//This code reads the root file containing a histogram with the Energy spectrum of the signal and the background for the 347 keV converts the spectrum linearly ( 1ADC=-E/0.57 ) to an ADC spectrum and then convert it back to an energy spectrum following a different dependency (sigmoid and inverse sigmoid)

double FuncShape1(double x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A*log((B/(x+C))-1)+D;
  return value;
};

double FuncShape2(double x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];
  double E = par[4];

  double value = A/(exp(x/B + C)+D)+E;
  return value;
};

double FuncShape3(double x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A*log10((B-x)/(C+x))+D;
  return value;
};

double FuncShape4(double x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = (-1+A*pow(10, (x+B)/C))/(1+pow(10, (x+B)/C))+D;
  return value;
};




void ConvertADC_Calibration( std::string rootname , std::string outpath , int shape ) {

  gROOT->SetStyle("ATLAS");
  gStyle->SetTitleFillColor(kWhite);
  gStyle->SetTitleBorderSize(5);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetPadTopMargin(0.08);
  gStyle->SetTitleX(0.14);


  bool store_dataROOT = false;
  
  TCanvas* c = new TCanvas("c");
 
  //***********READ DATA***********// 
  std::cout<<rootname<<std::endl;
  
  TFile *infile = new TFile(rootname.c_str());
  double pos1 = rootname.find("TimeSim_") + 8;
  double pos2 = rootname.find("s_seed");
  double diff = pos2-pos1;
  std::string time_str = rootname.substr(pos1, diff);
  double TimeSim = std::stod(time_str);

  pos1 = rootname.find("NOIntegral_") + 11;
  pos2 = rootname.find(".00kHz");
  diff = pos2-pos1;
  std::string rate_str = rootname.substr(pos1, diff);
  double RateSim = std::stod(rate_str);

  //pos1 = rootname.find("seed_0_") + 7;
  pos1 = rootname.find("seed_1_") + 7;
  pos2 = rootname.find("MeV_1");
  diff = pos2-pos1;
  std::string resolution_str = rootname.substr(pos1, diff);
  double ResolSim = std::stod(resolution_str);


  std::cout<<"Time simulation: "<<TimeSim<<" Rate  simulation: "<<RateSim<<" Resolution simulation: "<<ResolSim<<std::endl;

  TFile*output;
  std::string rootoutfile;
  if(store_dataROOT==true) {

    rootoutfile = outpath+"/NewCalibrationHistoShape"+std::to_string(shape)+"_Timesim_"+std::to_string(int(TimeSim))+"s_"+rate_str+"kHz_"+resolution_str+"MeV.root";
    output = new TFile(rootoutfile.c_str(),"recreate");
  }
 
  
  std::cout<<"Check time simulation: "<<TimeSim<<" s"<<std::endl;

  
  //***************GET HISTOGRAM AND FIT*****************//
  std::vector<double> *PeaksSignal=0, *PeaksBackground=0;

  std::vector<double> Peaks;
  
  TH1D *histo = (TH1D*)infile->Get("histo");
  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix1");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars1");
  TMatrixD *SeedFitPars = (TMatrixD*)infile->Get("SeedFitPars1");
  TTree *TreePeaksGen = (TTree*)infile->Get("AllPeaksGen");
  TreePeaksGen->SetBranchAddress("PeaksSignal", &PeaksSignal);
  TreePeaksGen->SetBranchAddress("PeaksBackground", &PeaksBackground);

  unsigned long int peaksvector = TreePeaksGen->GetEntry(0);
  
  for(unsigned long int i=0; i < PeaksSignal->size(); i++){
    Peaks.push_back(PeaksSignal->at(i));
  }
  for(unsigned long int i=0; i < PeaksBackground->size(); i++){
    Peaks.push_back(PeaksBackground->at(i));
  }
  
  //Covmatrix->Print();
  //BestFitPars->Print();

  std::string yaxis = "Counts / run time "+time_str+" s";
  char* yaxischar = const_cast<char*>(yaxis.c_str());

  //histo->GetXaxis()->SetRangeUser(0.3, 0.4);
  //histo->GetYaxis()->SetRangeUser(0,12000);
  histo->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
  histo->GetYaxis()->SetTitle(yaxischar);
  histo->SetFillColor(kOrange-3);
  histo->SetLineColor(kOrange-3);
  //histo->Draw("");  
  
  std::string str_latex1 = rate_str+" kHz, "+resolution_str+" MeV";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  
  int Nentries = histo->GetEntries();
  int Nbins = histo->GetNbinsX();
  double bin_width = histo->GetBinWidth(0);

  std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;
  //***********Fill new histogram linear calibration***********//
  double xminE = histo->GetXaxis()->GetBinCenter(0);
  double xmaxE = histo->GetXaxis()->GetBinCenter(Nbins);
  std::cout<<"Energy range: "<<xminE<<","<<xmaxE<<" MeV, binning: "<<bin_width<<std::endl;

  //In MeV
  double EtoADCcalib = -0.00057;
  
  double xminADC = xmaxE/EtoADCcalib;
  double xmaxADC = xminE/EtoADCcalib;

  TH1D *hADC = new TH1D("hADC", "", Nbins, xminADC, xmaxADC);
  TH1D *hEnewCalib = new TH1D("hEnewCalib", "", Nbins, xminE, xmaxE); //don't change this, change setrangeuser
  
  for(int i = 0 ; i < Nbins ; i++){

    double bincontent = histo->GetBinContent(Nbins-i);
    
    hADC->SetBinContent(i, bincontent);

  }
  
  double bin_widthADC = hADC->GetBinWidth(0);
  std::cout<<"ADC range: "<<xminADC<<","<<xmaxADC<<" binning: "<<bin_widthADC<<std::endl;

  hADC->GetXaxis()->SetRangeUser(-700, -525);
  hADC->GetYaxis()->SetRangeUser(0,12000);
  hADC->GetXaxis()->SetTitle("ADC Counts");
  hADC->GetYaxis()->SetTitle(yaxischar);
  hADC->SetLineColor(kPink-3);
  hADC->SetFillColor(kPink-3);
  hADC->SetFillStyle(3001);
  //hADC->Draw("");
  
  latex1.DrawLatexNDC(.55,.82,char_latex1);


  //Define new ADC to E calibrations    
  double* par_calib_1 = new double[4];
  double* par_calib_2 = new double[5];
  double* par_calib_3 = new double[4];
  double* par_calib_4 = new double[4];

  //Shape 1
  par_calib_1[0]= -190;
  par_calib_1[1]= -3601;
  par_calib_1[2]= -52.6;
  par_calib_1[3]= 800.192;
  //Shape 2
  par_calib_2[0]= -2000;
  par_calib_2[1]= -180;
  par_calib_2[2]= -7.2;
  par_calib_2[3]= 1;
  par_calib_2[4]= 1998.508;
  //Shape 3
  par_calib_3[0]= 833;
  par_calib_3[1]= 199.31;
  par_calib_3[2]= 4000;
  par_calib_3[3]= 1085;
  //Shape 4
  par_calib_4[0]= -2000;
  par_calib_4[1]= 1770;
  par_calib_4[2]= 1050;
  par_calib_4[3]= 1959.61;

  unsigned long int treeentries = Peaks.size();
  std::cout<<"Entries in the Tree is the number of peaks: "<<treeentries<<std::endl;

  for(unsigned long int i = 0 ; i < treeentries ; i++){
 
    double peak_E = Peaks.at(i); 
    double ADC_linear = peak_E/EtoADCcalib;
    double EnergykeV;

    if (shape==1){
      EnergykeV = FuncShape1( ADC_linear, par_calib_1 );
    }
    else if (shape==2){  
      EnergykeV = FuncShape2( ADC_linear, par_calib_2 );
    }
    else if (shape==3){
      EnergykeV = FuncShape3( ADC_linear, par_calib_3 );
    }
    else{
      EnergykeV = FuncShape4( ADC_linear, par_calib_4 );
    }
    
    double Energy = EnergykeV/1000;

    if(i<100){
      std::cout<<"ADC value: "<<ADC_linear<<std::endl;
      std::cout<<"Energy input: "<<peak_E<<" after new calibration: "<<Energy<<std::endl;
    }

    hEnewCalib->Fill(Energy);
    
  }

  
  hEnewCalib->GetYaxis()->SetTitle(yaxischar);
  hEnewCalib->SetFillStyle(3001);
  hEnewCalib->GetXaxis()->SetRangeUser(0.52, 0.62);
    
  if (shape==1){
    hEnewCalib->GetXaxis()->SetTitle("E_{#gamma, Calibration 1} (STM) [MeV]");
    hEnewCalib->SetFillColor(kViolet-6);
    hEnewCalib->SetLineColor(kViolet-6);
  }
  else if (shape==2){
    hEnewCalib->GetXaxis()->SetTitle("E_{#gamma, Calibration 2} (STM) [MeV]");
    hEnewCalib->SetFillColor(kViolet+1);
    hEnewCalib->SetLineColor(kViolet+1);
  }   
  else if (shape==3){
    hEnewCalib->GetXaxis()->SetTitle("E_{#gamma, Calibration 3} (STM) [MeV]");
    hEnewCalib->SetFillColor(kOrange-6);
    hEnewCalib->SetLineColor(kOrange-6);
  }   
  else{
    hEnewCalib->GetXaxis()->SetTitle("E_{#gamma, Calibration 4} (STM) [MeV]");
    hEnewCalib->SetFillColor(kOrange+1);
    hEnewCalib->SetLineColor(kOrange+1);
  }
  
  hEnewCalib->Draw("");
  
  //Save the new histogram to a root file

  if(store_dataROOT==true) {

    output->WriteObject(histo, "histo");
    output->WriteObject(hADC, "hADC");
    output->WriteObject(hEnewCalib, "hEnewCalib");
    
    std::string str_SeedFitPars = "SeedFitPars";
    char* char_SeedFitPars = const_cast<char*>(str_SeedFitPars.c_str());
    output->WriteObject(SeedFitPars, char_SeedFitPars);
        
    std::string str_BestFitPars = "BestFitPars";
    char* char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());    
    output->WriteObject(BestFitPars, char_BestFitPars);

    output->Close();
  }
  
  

  std::string nameplot_png = "BackgroundShape"+std::to_string(shape)+"_"+rate_str+"kHz_"+resolution_str+"MeV_zoom.png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());  
  
  std::cout<<"Generated plot in: "<<nameplot_png<<std::endl;
  //c->Print(char_nameplot_png);
  //c->Print("50kHz_2keVADClinspectrum_zoom.png"); 
}

