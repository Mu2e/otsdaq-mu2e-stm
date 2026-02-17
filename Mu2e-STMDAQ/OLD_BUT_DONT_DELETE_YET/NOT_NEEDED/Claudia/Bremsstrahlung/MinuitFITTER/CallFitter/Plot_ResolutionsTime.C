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


void Plot_ResolutionsTime( std::string input_path , std::string rate_kHz ){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1);
  gStyle->SetOptTitle(1);

  gStyle->SetTitleFillColor(kWhite);
  gStyle->SetTitleBorderSize(5);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetPadTopMargin(0.08);
  gStyle->SetTitleX(0.14);


  TCanvas* c = new TCanvas("c");

  std::string PlotLabel_log = "RatesAndTimes_Rate"+rate_kHz+"kHz.log";
  char* char_PlotLabel_log = const_cast<char*>(PlotLabel_log.c_str()); 
  std::cout<<"Generated log file: "<<PlotLabel_log<<std::endl;
  //gSystem->RedirectOutput(char_PlotLabel_log);


  //Input File
  string root_path;
  vector<string> file_name;
  file_name.clear();

  fstream readfile_txt;
  readfile_txt.open( input_path, ios::in );
  std::string rootname;

  //Inside root files
  std::vector<double> *TrueSignal=0, *RecoSignal=0;
  TH1D *hSTM;
  TMatrixD *BestFitPars;

  //Number of files
  int Nfiles = 0;

  while(1){
    readfile_txt >> root_path;
    file_name.push_back(root_path);
    if(readfile_txt.eof()) break;
    Nfiles++;
  }



  double timesimulation[Nfiles], timesimulation_error[Nfiles], sigmaS[Nfiles], SReco[Nfiles], accuracy[Nfiles], sigma_sigmaS[Nfiles], time_acc[Nfiles], time_acc_error[Nfiles], resolution[Nfiles], resolution_error[Nfiles], accuracy_error[Nfiles];

  std::cout<< "Number of files to analyse: " << Nfiles <<std::endl;

  double pos1, pos2, diff;

  for(int i = 0 ; i < Nfiles ; i++) {

    rootname = file_name.at(i);

    std::cout<<" "<<std::endl;
    std::cout<<"********* Input file: "<<rootname<<std::endl;

    TFile *infile = new TFile(rootname.c_str());

    //Read Tree
    TTree *treeread=(TTree*)infile->Get("Signaltree");
    treeread->SetBranchAddress("TrueSignal",&TrueSignal);
    treeread->SetBranchAddress("RecoSignal",&RecoSignal);

    treeread->GetEntry(0);
    int nexperiments = TrueSignal->size();
    std::cout<<"Reading TTree..."<<std::endl;
    std::cout<<"1 vector: "<<treeread->GetEntries()<<" of entries: "<<nexperiments<<std::endl;
    
    //Read Covariance Matrix, just 1 covmatrix per root file
    TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix1");
    int NP = Covmatrix->GetNcols();
    TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");
    std::cout<<"Reading Covmatrix of dim: "<<NP<<"x"<<NP<<"..."<<std::endl;

    //Get S+B histogram, just 1 histogram per root file
    std::string str_hSTM = "hSTM1";
    char* char_hSTM = const_cast<char*>(str_hSTM.c_str());
    hSTM = (TH1D*)infile->Get(char_hSTM);
    double bin_width = hSTM->GetBinWidth(0);
    std::cout<<"Reading Data generated... with binning: "<<bin_width<<std::endl;

    //Best Fit Parameters, just 1 matrix of best fit parameters per root file
    /*std::string str_BestFitPars = "BestFitPars1";
    char* char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());
    BestFitPars = (TMatrixD*)infile->Get(char_BestFitPars);
    TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
    std::cout<<"Reading Matrix of best fit parameters..."<<std::endl;

    double *best_fitpar = new double[NPbrems];

    for( int k = 0 ; k < NPbrems ; k++ ){
      best_fitpar[k] = _BestFitPars[0][k+NPsignal];
      //std::cout<<k<<" "<<k+NPsignal<<" "<<_BestFitPars[0][k+NPsignal]<<std::endl;
    }
    */


    //Get TimeSim, resolution, rate from root file name
    //Get the time of each file
    pos1 = rootname.find("TimeSim_") + 8;
    pos2 = rootname.find("_seed");
    diff = pos2-pos1;
    std::string time_sim_str = rootname.substr(pos1, diff);
    timesimulation[i] = std::stod(time_sim_str);
    std::cout<<"Check simulation time: "<<timesimulation[i]<<" s"<<std::endl;

    pos1 = rootname.find("NoInt_") + 6; // To skip "NoInt_" characters
    pos2 = rootname.find("kHz_");
    diff = pos2-pos1;
    std::string rate_sim_str = rootname.substr(pos1, diff);
    double rate_sim = std::stod(rate_sim_str);
    std::cout<<"Check rate: "<<rate_sim<<" kHz"<<std::endl;
    
    //For 1kHz - 100kHz
    /*
    std::string find_string = "NoInt_"+rate_kHz+"kHz_";
    //Rate with 2 cifra: 100kHz 
    //pos1 = rootname.find(find_string) + 13;
    //Rate with 1 cifra: 1kHz
    //pos1 = rootname.find(find_string) + 11; 
    pos2 = rootname.find("MeV_");
    */

    //For 1000kHz 
    pos1 = rootname.find("_0") + 2;
    pos2 = rootname.find("MeV_");

    diff= pos2-pos1;
    std::string res_sim_str = rootname.substr(pos1, diff);
    double res_sim = std::stod(res_sim_str);
    std::cout<<"Check resolution: "<<res_sim<<" MeV"<<std::endl;


    sigmaS[i] = sqrt(_Covmatrix[0][0]) / bin_width;
    SReco[i] = RecoSignal->at(0);
    accuracy[i] =  sigmaS[i] / SReco[i];    
    sigma_sigmaS[i] = 1./sqrt(2);
    
    double factor1 = sigma_sigmaS[i]/sigmaS[i];
    double factor2 = sigmaS[i]/SReco[i];
    double factor_acc = sqrt((factor1 * factor1) + (factor2*factor2));

    accuracy_error[i] = factor_acc*accuracy[i];

    time_acc[i] = (accuracy[i]/0.1)*(accuracy[i]/0.1)*timesimulation[i];
    time_acc_error[i] = 2*accuracy[i]*timesimulation[i]*accuracy_error[i]/(0.1*0.1);

    resolution[i] = res_sim;
    resolution_error[i] = 0;

    std::cout<<"TimeSim: "<<timesimulation[i]<<"s, Signal reco:  "<<SReco[i]<<"peaks, uncertainty in Signal reco: "<<sigmaS[i]<<" accuracy: uncertainty/Sreco= "<<accuracy[i]<<" error in accuracy= factorxaccuracy= "<<factor_acc<<"x"<<accuracy[i]<<"= "<<accuracy_error[i]<<std::endl;

    std::cout<<"Time for accuracy=0.1 = "<<time_acc[i]<<" +- "<<time_acc_error[i]<<std::endl;

    infile->Close();

  }//Nfiles


  TGraphErrors *grres = new TGraphErrors( Nfiles, resolution, time_acc, resolution_error, time_acc_error );

  grres->SetMarkerStyle(20);
  grres->GetYaxis()->SetTitle("Time (#sigma_{S}/S=0.1) [s]");
  grres->GetXaxis()->SetTitle("Resolution [MeV]");
  std::string PlotLabel_title = "Bremsstrahlung rate: "+rate_kHz+" kHz";
  char* PlotLabel_char_title = const_cast<char*>(PlotLabel_title.c_str());
  grres->SetTitle(PlotLabel_char_title);
  grres->SetMarkerStyle(20);
  grres->SetMarkerColor(kGreen+3);
  grres->GetXaxis()->SetRangeUser(0, 0.03);
  grres->GetYaxis()->SetRangeUser(0, 100);
  TAxis *X = grres->GetXaxis();
  X->SetNdivisions(5,3,0);
  grres->Draw("ap");


  //Fit the graph
  TF1* fres = new TF1("fres","[0]+[1]*x+[2]*x*x", 0, 0.03);
  fres->SetParameters(10000,10000,1); 

  fres->SetNpx(300000);
  grres->Fit(fres, "0", "", 0, 0.03);
  fres->SetLineColor(kBlack);
  fres->SetLineStyle(2);
  fres->Draw("same");
  


  double p0_val = fres->GetParameter(0);
  double p1_val = fres->GetParameter(1);
  double p0_val_error = fres->GetParError(0);
  double p1_val_error  = fres->GetParError(1);
  double chi2_val = grres->GetFunction("fres")->GetChisquare();


  gPad->Update();
  TPaveStats* ps = (TPaveStats *)grres->FindObject("stats");
  ps->SetY1NDC(.62);
  ps->SetY2NDC(.85);
  ps->SetX1NDC(0.2);
  ps->SetX2NDC(0.52);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  c->Modified();
  c->Update();
  //c->WaitPrimitive();
  
  //TGaxis::SetMaxDigits(2);
  grres->Draw("ap");
  fres->Draw("same");

  TLatex latex1;
  latex1.SetTextSize(0.04);
  double ylatex = TMath::MaxElement(10,grres->GetY()) / 2;
  std::cout<<"ylatex: "<<ylatex<<std::endl;
  //latex1.DrawLatex(1000,ylatex,PlotLabel_char);

  std::string PlotLabel_png = "ResolutionsAndTimes_Rate"+rate_kHz+"kHz_invaxis_zoom.png";
  std::string PlotLabel_pdf = "ResolutionsAndTimes_Rate"+rate_kHz+"kHz_invaxis_zoom.pdf";
  char* PlotLabel_char_png = const_cast<char*>(PlotLabel_png.c_str());
  char* PlotLabel_char_pdf = const_cast<char*>(PlotLabel_pdf.c_str());
  c->Print(PlotLabel_char_png);
  c->Print(PlotLabel_char_pdf);


  //exit(0);
}
