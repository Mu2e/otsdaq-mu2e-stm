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
#include "TGraphErrors.h"
#include "TGaxis.h"

#include <iomanip>
#include <string>
#include <stdlib.h>
#include <vector>
#include <utility>
#include <sstream>
#include <fstream>

void Plot_AccuracyTime_points( std::string txtpaths_file ) {

  //std::string txtpaths_file = "/data1/cgarcia/SignaltoBackground/directories.txt";

  //Generated root file in current dir
  std::string out_rootfilename = "p0_p1_rate_resolution_time_accuracy.root";
  TFile* output  = new TFile(out_rootfilename.c_str(),"recreate");

  //Generated log file in current dir
  std::string Label_log = "Accuracy_Time_FittingsOutputsCheck.log";
  char* char_Label_log = const_cast<char*>(Label_log.c_str());
  std::cout<<"Generated log file: "<<Label_log<<std::endl;
  //gSystem->RedirectOutput(char_Label_log); 

  //TreeFit
  std::vector<double> p0_fit, p1_fit, p0error_fit, p1error_fit, chi2_fit, rate_fit, resolution_fit, time_10percent_fit, Nparams_fit;
  //TreeRoot
  std::vector<double> rate_root, resolution_root, time_root, accuracy_root;

  TTree* TreeFit = new TTree("TreeFit", "TreeFit");
  TreeFit->Branch("p0_fit", &p0_fit);
  TreeFit->Branch("p1_fit", &p1_fit);
  TreeFit->Branch("p0error_fit", &p0error_fit);
  TreeFit->Branch("p1error_fit", &p1error_fit);
  TreeFit->Branch("chi2_fit", &chi2_fit);
  TreeFit->Branch("rate_fit", &rate_fit);
  TreeFit->Branch("resolution_fit", &resolution_fit);
  TreeFit->Branch("time_10percent_fit", &time_10percent_fit);
  TreeFit->Branch("Nparams_fit", &Nparams_fit);

  TTree* TreeRoot = new TTree("TreeRoot", "TreeRoot");
  TreeRoot->Branch("rate_root", &rate_root);
  TreeRoot->Branch("resolution_root", &resolution_root);
  TreeRoot->Branch("time_root", &time_root);
  TreeRoot->Branch("accuracy_root", &accuracy_root);

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

  std::vector<double> timesimulation, sigmaS, SReco, accuracy, timesimulation_error, accuracy_error, sigma_sigmaS, TimeSim, resolution, rate;
  double factor_acc;

  string dir_path, root_path;
  vector<string> file_name, rootpath_name;
  file_name.clear();

  fstream readfile_txt;
  readfile_txt.open( txtpaths_file, ios::in );

  // Read directories from txt file
  while(1){
    
    readfile_txt >> dir_path;
    file_name.push_back(dir_path);
    if(readfile_txt.eof()) break;
  
  }

  unsigned long int num_dir = file_name.size() - 1 ; 

  double pos1, pos2, diff;
  
  std::cout<<"Number of directories: "<<num_dir<<std::endl;

  for ( unsigned long int i = 0; i < num_dir; i++ ) {
  //for ( unsigned long int i = 0; i < 1; i++ ) {  
    // Read the log file in each directory wuth the path to the root files
    std::string log_file_with_root_paths = file_name.at(i)+"/RootFiles.log";

    std::cout<<"Path to root files: "<<log_file_with_root_paths<<std::endl;
    
    rootpath_name.clear();
    timesimulation.clear();
    resolution.clear();
    rate.clear();
    sigmaS.clear();
    SReco.clear();
    accuracy.clear(); 
    timesimulation_error.clear();
    accuracy_error.clear();
    sigma_sigmaS.clear();
    TimeSim.clear();

    //Get TimeSim, resolution, rate from root file name 
    pos1 = log_file_with_root_paths.find("NoInt_") + 6; // To skip "NoInt_" characters
    pos2 = log_file_with_root_paths.find("kHz_");
    diff = pos2-pos1; 
    std::string rate_sim_str = log_file_with_root_paths.substr(pos1, diff);
    double rate_sim = std::stod(rate_sim_str);
    std::cout<<"Check rate: "<<rate_sim<<" kHz"<<std::endl;
    rate.push_back(rate_sim);
    
    pos1 = log_file_with_root_paths.find("kHz_") + 4;
    pos2 = log_file_with_root_paths.find("MeV_");
    diff= pos2-pos1;
    std::string res_sim_str = log_file_with_root_paths.substr(pos1, diff);
    double res_sim = std::stod(res_sim_str);
    std::cout<<"Check resolution: "<<res_sim<<" MeV"<<std::endl;

    resolution.push_back(res_sim);
    rate.push_back(rate_sim);
      
    fstream readfile_log;
      
    std::vector<double> *TrueSignal=0, *RecoSignal=0;
    TH1D *hSTM;
    TMatrixD *BestFitPars;

    readfile_log.open( log_file_with_root_paths, ios::in );

    while(1){
      
      readfile_log >> root_path;
      rootpath_name.push_back(root_path);
      if(readfile_log.eof()) break;
    }
    

    int Nfiles = 0;

    // Analyse Nfiles number of root files in log file
    unsigned long int numfiles = rootpath_name.size() - 1;
    std::cout<<"Number of root files: "<<numfiles<<std::endl;

    for ( unsigned long int j = 0 ; j < numfiles; j++ ) {

      std::cout<<" "<<std::endl;
      std::cout<<"Input file: "<<rootpath_name.at(j)<<std::endl;

      TFile *infile = new TFile(rootpath_name.at(j).c_str());


      pos1 = rootpath_name.at(j).find("TimeSim_") + 8;
      pos2 = rootpath_name.at(j).find("_seed");
      diff= pos2-pos1;
      std::string time_sim_str = rootpath_name.at(j).substr(pos1, diff);
      double time_sim = std::stod(time_sim_str);
      std::cout<<"Check simulation time: "<<time_sim<<" s"<<std::endl;

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
      std::cout<<"Reading Data generated..."<<std::endl;

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
	}*/
      

      timesimulation.push_back(time_sim);
      sigmaS.push_back(sqrt(_Covmatrix[0][0]) / bin_width);
      SReco.push_back(RecoSignal->at(0));
      accuracy.push_back(sigmaS.at(j) / SReco.at(j));    
      sigma_sigmaS.push_back(1./sqrt(2));
      
      double factor1 = sigma_sigmaS.at(j)/sigmaS.at(j);
      double factor2 = sigmaS.at(j)/SReco.at(j);
      factor_acc = sqrt((factor1 * factor1) + (factor2*factor2));
      
      accuracy_error.push_back(factor_acc*accuracy.at(j));
    
      timesimulation_error.push_back(0);


      std::cout<<"TimeSim: "<<timesimulation.at(j)<<"s, Signal reco:  "<<SReco.at(j)<<"peaks, uncertainty in Signal reco: "<<sigmaS.at(j)<<" accuracy: uncertainty/Sreco= "<<accuracy.at(j)<<" error in accuracy= factorxaccuracy= "<<factor_acc<<"x"<<accuracy.at(j)<<"= "<<accuracy_error.at(j)<<std::endl;

      Nfiles++;


      rate_root.push_back(rate_sim); 
      resolution_root.push_back(res_sim); 
      time_root.push_back(timesimulation.at(j)); 
      accuracy_root.push_back(accuracy.at(j));
    
      infile->Close();

    }//j - number of root files in directory



    std::string title_rate_resol = "Bremsstrahlung rate: "+rate_sim_str+" kHz, Resolution: "+res_sim_str+" MeV";
    std::string PlotLabel = title_rate_resol;
    std::string PlotLabel_png = file_name.at(i)+"/Accuracy_Bremsstrahlungrate"+rate_sim_str+"kHzResolution"+res_sim_str+"MeV_fit.png";
    std::string PlotLabel_pdf = file_name.at(i)+"/Accuracy_Bremsstrahlungrate"+rate_sim_str+"kHzResolution"+res_sim_str+"MeV_fit.pdf";
    char* PlotLabel_char = const_cast<char*>(PlotLabel.c_str());

    TGraphErrors *gracc = new TGraphErrors( Nfiles, &timesimulation[0], &accuracy[0], &timesimulation_error[0], &accuracy_error[0] );
   
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

    if(p0_val<0){std::cout<<"Wrong fit, the funtion grows with time for... "<<rate_sim<<"kHz and "<<res_sim<<"keV"<<std::endl;
      std::cout<<"Don't fill the tree..."<<std::endl;
    }
      else{
	p0_fit.push_back(p0_val); 
	p1_fit.push_back(p1_val); 
	p0error_fit.push_back(p0_val_error);
	p1error_fit.push_back(p1_val_error); 
	chi2_fit.push_back(chi2_val); 
	rate_fit.push_back(rate_sim); 
	resolution_fit.push_back(res_sim);
	Nparams_fit.push_back(Nfiles);
      }
    //Calculate the time for which the accuracy is 0.1
    double limit_acc = 0.1;
    double min_time = (p0_val*p0_val) / ((limit_acc-p1_val)*(limit_acc-p1_val));
    time_10percent_fit.push_back(min_time);

    double A = (2*p0_val)/((limit_acc-p1_val)*(limit_acc-p1_val));
    double B = (2*p0_val*p0_val)/((limit_acc-p1_val)*(limit_acc-p1_val)*(limit_acc-p1_val));
    double min_time_error = sqrt(A*A*p0_val_error*p0_val_error+B*B*p1_val_error*p1_val_error);

    std::cout<<"Time for reaching an accuracy=0.1: "<<min_time<<" +- "<<min_time_error<<std::endl;

    gPad->Update();
    TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats"); 
    ps->SetY1NDC(.62);
    ps->SetY2NDC(.87);
    ps->SetX1NDC(0.5); //new x start position
    ps->SetX2NDC(0.86);
    ps->SetLineWidth(6);
    ps->SetLineColor(kWhite);

    c->Modified();
    c->Update();
    //c->WaitPrimitive();
 
    //TGaxis::SetMaxDigits(2);
    gracc->Draw("ap");
    faccuracy->Draw("same");

    //gSystem->Sleep(1000);

    TLatex latex1;
    latex1.SetTextSize(0.04);
    double ylatex = TMath::MaxElement( Nfiles,gracc->GetY() ) / 2;
    std::cout<<"ylatex: "<<ylatex<<std::endl;
    double xlatex = timesimulation.at(0) + 200;
    std::cout<<"xlatex: "<<xlatex<<std::endl;
    //latex1.DrawLatex(xlatex, ylatex, PlotLabel_char);

    char* PlotLabel_char_png = const_cast<char*>(PlotLabel_png.c_str());
    char* PlotLabel_char_pdf = const_cast<char*>(PlotLabel_pdf.c_str());
    std::cout<<"PNG generated: "<<PlotLabel_char_png<<std::endl;
    c->Print(PlotLabel_char_png);
    //c->Print(PlotLabel_char_pdf);


 }//i

  TreeRoot->Fill();
  TreeFit->Fill();
  output->Write();
  output->Close();

  readfile_txt.close();
  exit(0);

}
