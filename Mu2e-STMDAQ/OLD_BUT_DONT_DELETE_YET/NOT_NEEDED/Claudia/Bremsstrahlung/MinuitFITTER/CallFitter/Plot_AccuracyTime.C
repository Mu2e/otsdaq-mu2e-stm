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


void Plot_AccuracyTime( std::string input_path , std::string rate, std::string resolution) {

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
  double rate_doub = std::stod(rate);
  int Nfiles = 10;
  std::string TimeSim[10] = { "60", "90", "120", "500", "900", "1800", "3600", "5400", "9000", "10000" };
  double timesimulation[10];
  double sigmaS[10];
  double SReco[10];
  double accuracy[10];
  double timesimulation_error[10] = { 0,0,0,0,0,0,0,0,0,0 }; 
  double accuracy_error[10];
  double sigma_sigmaS[10];

  double factor_acc;   
  /*
  factor_acc = 0.01;
  int setdec;
  if(factor_acc == 0.001){setdec = 3;}
  else if(factor_acc == 0.01){setdec = 2;}
  else if(factor_acc == 0.1){setdec = 1;}
  else{setdec = 2;}

  std::stringstream accuracy_err_str;
  accuracy_err_str << std::fixed << std::setprecision(setdec) << factor_acc;
  std::string accuracy_err_str_ = "Error ="+accuracy_err_str.str()+"#timesAccuracy";
  std::string PlotLabel = "#splitline{"+title_rate_resol+"}{"+accuracy_err_str_+"}";*/
  std::string title_rate_resol = "Bremsstrahlung rate: "+rate+" kHz, Resolution: "+resolution+" MeV";     
  std::string PlotLabel = title_rate_resol;
  char* PlotLabel_char = const_cast<char*>(PlotLabel.c_str());
  std::string PlotLabel_png = "Accuracy_Bremsstrahlungrate"+rate+"kHzResolution"+resolution+"MeV_new.png";
  std::string PlotLabel_pdf = "Accuracy_Bremsstrahlungrate"+rate+"kHzResolution"+resolution+"MeV_new.pdf";
  
  std::string PlotLabel_log = "Accuracy_Bremsstrahlungrate"+rate+"kHzResolution"+resolution+"MeV_new.log";
  char* char_PlotLabel_log = const_cast<char*>(PlotLabel_log.c_str()); 
  std::cout<<"Generated log file: "<<PlotLabel_log<<std::endl;
  //gSystem->RedirectOutput(char_PlotLabel_log);

  /* double mean_E = 0.347;
  double sigma = 0.001;
  double frange[2] = { 0.04, 2 };*/

  //Input File
  std::string rootname;
  std::vector<double> *TrueSignal=0, *RecoSignal=0;
  TH1D *hSTM;
  TMatrixD *BestFitPars;
  //double xx1[2] = { 55, 5405 };
  //double yy1[2] = { 0.001 , 0.5 };
  //TGraph *graph1 = new TGraph(2,xx1,yy1);



  for(int i = 0 ; i < Nfiles ; i++) {

    rootname = input_path+"/BinnedLoglike_NOIntegral_"+rate+"kHz_TimeSim_"+TimeSim[i]+"s_seed_0_"+resolution+"keV_1Runs_Job_0.root";

    std::cout<<"Input file: "<<rootname<<std::endl;

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
  

    timesimulation[i] = std::stod(TimeSim[i]);
    sigmaS[i] = sqrt(_Covmatrix[0][0]) / bin_width;
    SReco[i] = RecoSignal->at(0);
    accuracy[i] =  sigmaS[i] / SReco[i];    
    sigma_sigmaS[i] = 1./sqrt(2);
    
    double factor1 = sigma_sigmaS[i]/sigmaS[i];
    double factor2 = sigmaS[i]/SReco[i];
    factor_acc = sqrt((factor1 * factor1) + (factor2*factor2));

    accuracy_error[i]= factor_acc*accuracy[i];

    std::cout<<"TimeSim: "<<timesimulation[i]<<"s, Signal reco:  "<<SReco[i]<<"peaks, uncertainty in Signal reco: "<<sigmaS[i]<<" accuracy: uncertainty/Sreco= "<<accuracy[i]<<" error in accuracy= factorxaccuracy= "<<factor_acc<<"x"<<accuracy[i]<<"= "<<accuracy_error[i]<<std::endl;


    infile->Close();

  }//Nfiles


  TGraphErrors *gracc = new TGraphErrors( Nfiles, timesimulation, accuracy, timesimulation_error, accuracy_error );
    
  gracc->SetMarkerStyle(20);
  gracc->GetXaxis()->SetTitle("Time [s]");
  gracc->GetYaxis()->SetTitle("Accuracy (#sigma_{S}/S)");
  gracc->SetTitle(PlotLabel_char);
  gracc->SetMarkerStyle(20);
  gracc->SetMarkerColor(kMagenta+3);
  TAxis *X = gracc->GetXaxis();
  X->SetNdivisions(5,3,0);
  gracc->Draw("ap");


  //Fit the graph
  //TF1* faccuracy = new TF1("faccuracy","[0]*sqrt([1]*x)+[2]", 0, 5450);
  //faccuracy->SetParameters(-1.94026e-03, 3.69361e-02, 3.30388e-02);
  //TF1* faccuracy = new TF1("faccuracy","[0]/(x*x+[1])+[2]", 0, 5450);
  //faccuracy->SetParameters(8.19543e+03, 1.70474e+05, 8.09284e-03);
  //TF1* faccuracy = new TF1("faccuracy","[0]*exp([1]*x+[2])+[3]", 0, 5450);
  //faccuracy->SetParameters(3.33698e-02, -1.96422e-03, 4.61574e-01, 8.47392e-03);
  //TF1* faccuracy = new TF1("faccuracy","[0]/(exp([1]*x+[2]))+[3]", 0, 5450);
  //faccuracy->SetParameters(3.33698, 1.96422e-03, 4.61574e-01, 8.47392e-03);
  TF1* faccuracy = new TF1("faccuracy","[0]/sqrt(x)+[1]", 0, 10000);
  faccuracy->SetParameters(0.52, 4.21370e-04); 

  faccuracy->SetNpx(300000);
  gracc->Fit(faccuracy, "0", "", 0, 10000);
  faccuracy->SetLineColor(kBlack);
  faccuracy->SetLineStyle(2);
  faccuracy->Draw("same");
  //faccuracy->Draw(""); 


  double p0_val = faccuracy->GetParameter(0);
  double p1_val = faccuracy->GetParameter(1);
  double p0_val_error = faccuracy->GetParError(0);
  double p1_val_error  = faccuracy->GetParError(1);
  double chi2_val = gracc->GetFunction("faccuracy")->GetChisquare();


  //Calculate the time for which the accuracy is 0.1                                                                                
  double limit_acc = 0.1;
  double min_time = (p0_val*p0_val) / ((limit_acc-p1_val)*(limit_acc-p1_val));

  double A = (2*p0_val)/((limit_acc-p1_val)*(limit_acc-p1_val));
  double B = (2*p0_val*p0_val)/((limit_acc-p1_val)*(limit_acc-p1_val)*(limit_acc-p1_val));
  double min_time_error = sqrt(A*A*p0_val_error*p0_val_error+B*B*p1_val_error*p1_val_error);

  std::cout<<"Time for reaching an accuracy=0.1: "<<min_time<<" +- "<<min_time_error<<std::endl;


  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.62);
  ps->SetY2NDC(.87);
  ps->SetX1NDC(0.5);
  ps->SetX2NDC(0.86);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  c->Modified();
  c->Update();
  //c->WaitPrimitive();

  //TGaxis::SetMaxDigits(2);
  gracc->Draw("ap");
  faccuracy->Draw("same");

  TLatex latex1;
  latex1.SetTextSize(0.04);
  double ylatex = TMath::MaxElement(10,gracc->GetY()) / 2;
  std::cout<<"ylatex: "<<ylatex<<std::endl;
  //latex1.DrawLatex(1000,ylatex,PlotLabel_char);

  char* PlotLabel_char_png = const_cast<char*>(PlotLabel_png.c_str());
  char* PlotLabel_char_pdf = const_cast<char*>(PlotLabel_pdf.c_str());
  //c->Print(PlotLabel_char_png);
  //c->Print(PlotLabel_char_pdf);


  //exit(0);
}
