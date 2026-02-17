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

//This code reads the energy data generated and leave the spectrum as it is except in the region 320-370keV that apply the new deviated calibration to the signal

double FuncShape1(double x, double* par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];
  double E = par[4];
  double F = par[5];
  
  double value = A + B*x + C*x*x + D*x*x*x + E*x*x*x*x + F*x*x*x*x*x;
  return value;
};



void Signal_Calibration( std::string rootname , std::string outpath , double factor) {

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


  std::stringstream streamfactor;
  streamfactor << std::fixed << std::setprecision(4) << factor;
  
  TFile*output;
  std::string rootoutfile;
  if(store_dataROOT==true) {

    rootoutfile = outpath+"/SignalRegionFactorShape"+streamfactor.str()+"_Timesim_"+std::to_string(int(TimeSim))+"s_"+rate_str+"kHz_"+resolution_str+"MeV.root";
    output = new TFile(rootoutfile.c_str(),"recreate");
  }
 
  std::cout<<"Check time simulation: "<<TimeSim<<" s"<<std::endl;
 
  //***************GET HISTOGRAM AND FIT*****************//
  std::vector<double> *PeaksSignal=0, *PeaksBackground=0;

  TH1D *histo = (TH1D*)infile->Get("histo");
  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix1");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars1");
  TMatrixD *SeedFitPars = (TMatrixD*)infile->Get("SeedFitPars1");
  TTree *TreePeaksGen = (TTree*)infile->Get("AllPeaksGen");
  TreePeaksGen->SetBranchAddress("PeaksSignal", &PeaksSignal);
  TreePeaksGen->SetBranchAddress("PeaksBackground", &PeaksBackground);

    
  //Covmatrix->Print();
  //BestFitPars->Print();

  std::string yaxis = "Counts / run time "+time_str+" s";
  char* yaxischar = const_cast<char*>(yaxis.c_str());

  //histo->GetXaxis()->SetRangeUser(0.3, 0.4);
  histo->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
  histo->GetYaxis()->SetTitle(yaxischar);
  histo->SetFillColor(kOrange-3);
  histo->SetLineColor(kOrange-3);
 
    
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
  
  TH1D *hEnewCalib = new TH1D("hEnewCalib", "", Nbins, xminE, xmaxE); //don't change this, change setrangeuser  

  //Define new ADC to E calibrations    
  double* par_calib_1 = new double[6];
  double* par_calib_2 = new double[6];
  double* par_calib_3 = new double[6];
  
  //Shape 1, factor 0.005
  par_calib_1[0]= -16117.1 ;
  par_calib_1[1]= 64.5696 ;
  par_calib_1[2]= 0.105626 ;
  par_calib_1[3]= -0.00032044 ;
  par_calib_1[4]= -1.55268e-06 ;
  par_calib_1[5]= 3.23718e-09 ;

  //Shape 1, factor 0.001
  par_calib_2[0]= -3208.64 ;
  par_calib_2[1]= 11.4639 ;
  par_calib_2[2]= 0.0210026 ;
  par_calib_2[3]= -6.37768e-05 ;
  par_calib_2[4]= -3.09301e-07 ;
  par_calib_2[5]= 6.44789e-10 ;

  //Shape 1, factor -0.005
  par_calib_3[0]= 16150.8;
  par_calib_3[1]= -68.167;
  par_calib_3[2]= -0.105982;
  par_calib_3[3]= 0.000321037;
  par_calib_3[4]= 1.5565e-06;
  par_calib_3[5]= -3.24449e-09;
  
  unsigned long int peaksvector = TreePeaksGen->GetEntry(0);

  for(unsigned long int i = 0 ; i < PeaksSignal->size() ; i++){
 
    double EnergyMeV = PeaksSignal->at(i); //MeV
    double EnergykeV = EnergyMeV*1000;
    double ADC_linear;

    if (factor==0.005){
	ADC_linear = FuncShape1( EnergykeV, par_calib_1 );
	if(i==0){std::cout<<"calibration Factors: "<<par_calib_1[0]<<" "<<par_calib_1[1]<<" "<<par_calib_1[2]<<" "<<par_calib_1[3]<<" "<<par_calib_1[4]<<" "<<par_calib_1[5]<<" "<<std::endl;}
    }
    else if (factor==0.001){  
	ADC_linear = FuncShape1( EnergykeV, par_calib_2 );
	if(i==0){std::cout<<"calibration Factors: "<<par_calib_2[0]<<" "<<par_calib_2[1]<<" "<<par_calib_2[2]<<" "<<par_calib_2[3]<<" "<<par_calib_2[4]<<" "<<par_calib_2[5]<<" "<<std::endl;}
    }
    else if (factor==-0.005){
        ADC_linear = FuncShape1( EnergykeV, par_calib_3 );
	if(i==0){std::cout<<"calibration Factors: "<<par_calib_3[0]<<" "<<par_calib_3[1]<<" "<<par_calib_3[2]<<" "<<par_calib_3[3]<<" "<<par_calib_3[4]<<" "<<par_calib_3[5]<<" "<<std::endl;}
    }
    else {std::cout<<"Something is wrong with the factor"<<std::endl; exit(0);}

    double Energy_after_cali = ADC_linear*EtoADCcalib;
    //std::cout<<"energy: "<<Energy<<std::endl;
    
    if(i<100){
      std::cout<<"Energy input: "<<EnergyMeV<<"-->ADC: "<<ADC_linear<<" E after new calibration: "<<Energy_after_cali<<std::endl;
    }

    hEnewCalib->Fill(Energy_after_cali);
    
  }

  for(unsigned long int i = 0 ; i < PeaksBackground->size() ; i++){
    hEnewCalib->Fill(PeaksBackground->at(i));
  }
  
  hEnewCalib->GetYaxis()->SetTitle(yaxischar);
  hEnewCalib->SetFillStyle(3001);
  //hEnewCalib->GetXaxis()->SetRangeUser(0.3, 0.4);

  hEnewCalib->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
  hEnewCalib->SetFillColor(kBlue+1);
  hEnewCalib->SetLineColor(kBlue+1);
  //double ymax = hEnewCalib->GetMaximum()+5000;
  //double ymax = hEnewCalib->GetMaximum()+5;
  //histo->GetYaxis()->SetRangeUser(0,ymax); 

  histo->Draw("");
  hEnewCalib->Draw("same");
  
  std::string sim_legend_str = "#splitline{Nominal Signal Shape }{(rate= "+ rate_str+" kHz, #sigma= "+resolution_str+" MeV)}";
  char* sim_legend = const_cast<char*>(sim_legend_str.c_str());
  
  auto legend = new TLegend(0.57,0.6,0.9,0.9);
  legend->AddEntry("histo",sim_legend,"l");
  legend->AddEntry("hEnewCalib","Non-Linear Signal Calibration","l");
  legend->Draw("same");


   
  //Save the new histogram to a root file 
  if(store_dataROOT==true) {

    output->WriteObject(histo, "histo");
    output->WriteObject(hEnewCalib, "hEnewCalib");
    
    std::string str_SeedFitPars = "SeedFitPars";
    char* char_SeedFitPars = const_cast<char*>(str_SeedFitPars.c_str());
    output->WriteObject(SeedFitPars, char_SeedFitPars);
        
    std::string str_BestFitPars = "BestFitPars";
    char* char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());    
    output->WriteObject(BestFitPars, char_BestFitPars);

    output->Close();
  }
  
  
  
  std::string nameplot_png = "SignalRegionFactorShape"+streamfactor.str()+"_"+rate_str+"kHz_"+resolution_str+"MeV_zoom.png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());  
  
  std::cout<<"Generated plot in: "<<nameplot_png<<std::endl;
  //c->Print(char_nameplot_png);
  //c->Print("50kHz_2keVADClinspectrum_zoom.png"); 
}

