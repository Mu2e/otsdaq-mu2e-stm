#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"


//1- This code reads the Data (a histogam with the energy spectrum with a given calibration), Seeds for fit and Best fit values from a root file
//2- Set the fit range and initial parameters for the fit as the seed parameters for the root file, except the mean of the gaussian which is now moved due to the new calibration and use as seed for the mean of the gaussian the input parameter meanE_fit 
//After the fit, it  gets the signal and the signal error



void FitFromHistogram_jobs(int argc, char *argv[], std::string rootpath, int FitOption, double xfit_low, double xfit_max, double seed, std::string outpath)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  std::cout<<"output path: "<<outpath<<std::endl;

  bool store_ROOTfile = false;

  
  gROOT->SetStyle("ATLAS");
  TCanvas* c;

  gStyle->SetOptStat(1111);
  
  c = new TCanvas("c","c",0,0,1200,500);
  TPad* pad[2];
  pad[0] = new TPad("","",0,0,0.5,1);
  pad[1] = new TPad("","",0.5,0,1,1);

  pad[0]->Draw();
  pad[1]->Draw();      
  std::cout<<"Seed: "<<seed<<std::endl;

  TGraph* graph[2];
  double yy1[2] = {0, 50};

  //Output File                                                                                                        
  std::string outrootname;
  TFile* outfile;
  TTree* SBtree;
   
    
  TH1D* hTrue[1];
  TH1D* hReco[1];
  TH1D* hDeltaSignal;
  int Nbins = 200;
  //double xx1[2] = {22000, 25000};
  //double xx1delta[2] = {-1500, 600};

  double xx1[2] = {100, 300};
  double xx1delta[2] = {-100, 100};
  
  hTrue[0] = new TH1D ("Shistogram True", "", Nbins,  xx1[0], xx1[1]);
  hReco[0] = new TH1D ("Shistogram Reco", "", Nbins,  xx1[0], xx1[1]);
  hDeltaSignal = new TH1D ("#DeltaSignal", "", Nbins, xx1delta[0], xx1delta[1]);

  graph[0] = new TGraph (2,xx1,yy1);
  graph[0]->SetMarkerStyle(1);

  graph[1] = new TGraph (2,xx1delta,yy1);
  graph[1]->SetMarkerStyle(1);
  
  //***********READ DATA***********// 
  std::cout<<rootpath<<std::endl;
  int Njobs = 20;
  int nexperiments = 50;
  
  std::string TimeSim_ = "2000";
  double TimeSim = 2000;
  std::string RateSim = "1";
  std::string ResolSim = "0.001";

  double TrueSignal_, RecoSignal_;
 

  if(store_ROOTfile==true){
    if(FitOption==1000){
      outrootname = "Signal_BackgroundTrueReco_"+TimeSim_+"s_"+RateSim+"kHz_"+ResolSim+"MeV_BinnedChi2_20000bins.root";
    }
    if(FitOption==2000){
      outrootname = "Signal_BackgroundTrueReco_"+TimeSim_+"s_"+RateSim+"kHz_"+ResolSim+"MeV_BinnedLoglike_20000bins.root";
    }
      
    outfile = new TFile(outrootname.c_str(),"recreate");

    //Output tree
    SBtree = new TTree("SBtree", "SBtree");
    SBtree->Branch("TrueSignal_", &TrueSignal_);
    SBtree->Branch("RecoSignal_", &RecoSignal_);
  }



  
  std::stringstream stream_xfit_low, stream_xfit_max;
  stream_xfit_low << std::fixed << std::setprecision(3) << xfit_low;
  stream_xfit_max << std::fixed << std::setprecision(3) << xfit_max;
  
  std::cout<<"Time simulation: "<<TimeSim<<" Rate  simulation: "<<RateSim<<" Resolution simulation: "<<ResolSim<<std::endl;
  std::cout<<"Check time simulation: "<<TimeSim<<" s"<<std::endl;
  
  TMatrixD *SeedFitPars;
  TH1D *hSTM;
  TH1D *hSignal;

  TPaveStats *stat[2];
  double fit_range[2];
  fit_range[0] = xfit_low;
  fit_range[1] = xfit_max;
  
  for(int i = 0 ; i < Njobs ; i++) {
  //for(int i = 0 ; i < 1 ; i++) {
    //std::string rootname = rootpath+"/BinnedChi2_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
    std::string rootname = rootpath+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+".root";
    //std::string rootname = rootpath+"/BinnedLoglike_NOIntegral_50.00kHz_TimeSim_2000s_seed_0_0.0020MeV_50Runs_Job_"+std::to_string(i)+".root";
    //std::string rootname = rootpath+"/BinnedLoglike_NOIntegral_50.00kHz_TimeSim_2000s_seed_0_0.0020MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
    //std::string rootname = rootpath+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
    //std::string rootname = rootpath+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_10s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
    
    TFile *infile = new TFile(rootname.c_str());
    std::cout<<"ROOT FILE: "<<rootname<<std::endl;
     
     //***************GET HISTOGRAM AND FIT*****************//

     for( int j = 0 ; j <  nexperiments; j++) {
       //for( int j = 0 ; j <  1; j++) {
       std::cout<<"EXPERIMENT: "<<j+1<<std::endl;
       std::string str_hSTM = "hSTM"+std::to_string(j+1);
       char* char_hSTM = const_cast<char*>(str_hSTM.c_str());
       hSTM = (TH1D*)infile->Get(char_hSTM);
     
       std::string str_SeedFitPars = "SeedFitPars"+std::to_string(j+1);
       char* char_SeedFitPars = const_cast<char*>(str_SeedFitPars.c_str()); 
       SeedFitPars = (TMatrixD*)infile->Get(char_SeedFitPars);
       
       std::string str_hSignal = "hSignal;"+std::to_string(j+1);
       char* char_hSignal = const_cast<char*>(str_hSignal.c_str());
       hSignal = (TH1D*)infile->Get(char_hSignal);
     
       std::cout<<"Number of peaks simulated: "<<hSignal->GetEntries()<<std::endl;
       
       int Nentries = hSTM->GetEntries();
       int Nbins = hSTM->GetNbinsX();
       double bin_width = hSTM->GetBinWidth(0);
       
       std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;
       //***********Initialise Minuit Class***********//
       
       MinuitFitter *fitter = new MinuitFitter(FitOption, seed);
       
       fitter->Histo_Data(fit_range, hSTM); //Get nbins_fit, _binning, bincontent, bincontent_error, bincenter
       
       //Return nbins for the fit 
       unsigned long int nbins_fit = fitter->return_nbinsfit();  
       //Return dof
       unsigned long int dof = fitter->return_dof();
       std::cout<<"Number of bins for the fit: "<<nbins_fit<<" Degrees of freedom (ndof): "<<dof<<std::endl;
       //Return number of fit parameters NP
       int NP = fitter->return_NP();
       std::cout<<"Number of fit parameters, NP: "<<NP<<std::endl;
       

       //Initialise seeds for fit
       TMatrixD _SeedFitPars(1,NP,SeedFitPars->GetMatrixArray(),"");
       
       _SeedFitPars.Print();
       
       double seed_fitpar[NP];
       
       for( int i = 0 ; i < NP ; i++ ){
	 seed_fitpar[i] = _SeedFitPars[0][i];
       }
     
         
       double reco_peaks;
       double reco_sigma;
       
       
       //Initialise parameters value to do the fit
       double par_fit [NP];
       
       //Signal
       par_fit [0] = seed_fitpar[0];
       par_fit [1] = seed_fitpar[1];
       par_fit [2] = seed_fitpar[2];
       
       //Background
       par_fit [3] = seed_fitpar[3];
       par_fit [4] = seed_fitpar[4];
       par_fit [5] = seed_fitpar[5];
       par_fit [6] = seed_fitpar[6];
       
       //Initialise S+B function  
       std::string  genfunc_name = "fgenSB";
       char* char_genfunc_name = const_cast<char*>(genfunc_name.c_str());
       fitter->Init_SBfunc(char_genfunc_name, fit_range);
       fitter->SetPar_SBfunc(par_fit);
       
       //Fit with Minuit
       fitter->fit(par_fit, NP);
       
       double* p_minuit;
       double* perr_minuit;
       double min_fcn, error_def;
       TMatrixD* Covmatrix_newFit;
       TMatrixD* BestFitPars1_newFit;
       
       //Get Minuit fit parameters after the fit
       p_minuit = new double[NP];
       perr_minuit = new double[NP];
       
       p_minuit = fitter->return_p_minuit();
       perr_minuit = fitter->return_perr_minuit();
       min_fcn = fitter->return_amin();
       error_def = fitter->return_errdef();
       Covmatrix_newFit = fitter->return_Covmatrix();
       
       std::string  fitfunc_name = "ffit";
       char* char_fitfunc_name = const_cast<char*>(fitfunc_name.c_str());
       std::cout<<"Minuit parameters: "<<std::endl;
       std::cout<<"min FCN: "<<min_fcn<<" error definition: "<<error_def<<std::endl;
       
       double par_minuit [NP];
       for(int i = 0 ; i < NP; i++) {
	 std::cout<<p_minuit[i]<<" +- "<<perr_minuit[i]<<std::endl;
	 par_minuit [i] = p_minuit[i];
       }
       
       //Recovered number of peaks in signal
       reco_peaks = par_minuit [0] / bin_width;
       std::cout<<"Peaks recovered from Binned fit: "<< reco_peaks <<std::endl;
       TMatrixD _Covmatrix_newFit(7,7,Covmatrix_newFit->GetMatrixArray(),"");
       reco_sigma = sqrt(_Covmatrix_newFit[0][0]) / bin_width;
       //_Covmatrix_newFit.Print();
       

       std::cout<<"Sigma Signal: sqrt(cov matrix element)/bin width: "<<sqrt(_Covmatrix_newFit[0][0])<<"/"<<bin_width<<"="<<reco_sigma<<std::endl;
       std::cout<<"Reco signal: "<<reco_peaks<<std::endl;
       double accuracy = reco_sigma / reco_peaks;
       std::cout<<"Accuracy (sigma/signal): "<< accuracy <<std::endl;
       
       double accuracy_error;
       double sigma_sigmaS = 1./sqrt(2);
       double factor1 = sigma_sigmaS / reco_sigma;
       double factor2 = reco_sigma / reco_peaks;
       double factor_acc = sqrt((factor1 * factor1) + (factor2*factor2));
       accuracy_error = accuracy*factor_acc ;
       std::cout<<"Accuracy error: "<< accuracy_error <<std::endl;
       
       double time_acc = (accuracy/0.1)*(accuracy/0.1)*TimeSim;
       double time_acc_error = 2*accuracy*TimeSim*accuracy_error/(0.1*0.1);
       std::cout<<"Time for accuracy=0.1 = "<<time_acc<<" +- "<<time_acc_error<<" s"<<std::endl;

       TrueSignal_ = hSignal->GetEntries();
       RecoSignal_ = reco_peaks;

       if(store_ROOTfile==true){
	SBtree->Fill();
      }
       
       hTrue[0]->Fill(TrueSignal_);
       hReco[0]->Fill(RecoSignal_);
       double delta_signal = RecoSignal_-TrueSignal_;
       hDeltaSignal->Fill(delta_signal);
       
	}

	infile->Close();
   }


  if(store_ROOTfile==true){
    outfile->Write();
    outfile->Close();
  }


  pad[0]->cd();
  pad[0]->SetLeftMargin(0.15);
  pad[0]->SetRightMargin(0.1);

  hReco[0]->SetFillColor(kGreen-3);
  hReco[0]->SetLineColor(kBlack);
  hReco[0]->SetFillStyle(3001);
  hReco[0]->Draw("HIST");
  
  
    gPad->Update();
    stat[0] = (TPaveStats*)hReco[0]->FindObject("stats");
    stat[0]->SetY1NDC(.74);
    stat[0]->SetY2NDC(.91);
    stat[0]->SetX1NDC(0.2);
    stat[0]->SetX2NDC(0.52);
    stat[0]->SetTextSize(0.043);
    stat[0]->SetTextColor(kGreen-3);
    

    hTrue[0]->SetFillColor(kViolet);
    hTrue[0]->SetLineColor(kBlack);
    hTrue[0]->SetFillStyle(3001);
    hTrue[0]->Draw("HIST,sames");
    
    gPad->Update();
    stat[1] = (TPaveStats*)hTrue[0]->FindObject("stats");
    stat[1]->SetY1NDC(.56);
    stat[1]->SetY2NDC(.73);
    stat[1]->SetX1NDC(0.2);
    stat[1]->SetX2NDC(0.52);
    stat[1]->SetTextSize(0.043);
    stat[1]->SetTextColor(kViolet);
    
    graph[0]->Draw("ap");
    graph[0]->GetXaxis()->SetTitle("Counts in Signal");
    graph[0]->GetYaxis()->SetTitle("Experiments");
    hReco[0]->Draw("HIST,same");
    stat[0]->Draw("same");
    hTrue[0]->Draw("HIST,same");
    hTrue[0]->GetXaxis()->SetLabelSize(0.04);
    hReco[0]->GetXaxis()->SetLabelSize(0.04);
    graph[0]->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
    graph[0]->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    stat[1]->Draw("same");

    char* char_latex1;
    TLatex latex1;

    if(FitOption==2000){
      std::string str_latex1 = "Binned Log Fit";
      char_latex1 = const_cast<char*>(str_latex1.c_str());
      
    }
    if(FitOption==1000){
      std::string str_latex1 = "#chi^{2} Fit";
      char_latex1 = const_cast<char*>(str_latex1.c_str());  
    }

    latex1.DrawLatexNDC(.22,.47,char_latex1);

    pad[1]->cd();
    pad[1]->SetLeftMargin(0.15);
    pad[1]->SetRightMargin(0.1);

    hDeltaSignal->SetFillColor(kBlue);
    hDeltaSignal->SetLineColor(kBlack);
    hDeltaSignal->SetFillStyle(3001);
    hDeltaSignal->Draw("HIST");

    gPad->Update();
    stat[0] = (TPaveStats*)hDeltaSignal->FindObject("stats");
    stat[0]->SetY1NDC(.74);
    stat[0]->SetY2NDC(.91);
    stat[0]->SetX1NDC(0.2);
    stat[0]->SetX2NDC(0.52);
    stat[0]->SetTextSize(0.043);
    stat[0]->SetTextColor(kBlue);

    graph[1]->Draw("ap");
    graph[1]->GetXaxis()->SetTitle("#DeltaSignal: Reco-True");
    graph[1]->GetYaxis()->SetTitle("Experiments");
    stat[0]->Draw("same");
    graph[1]->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
    graph[1]->GetHistogram()->GetXaxis()->SetLabelSize(0.04);

    hDeltaSignal->Draw("HIST,same");



    std::string nameplot_png;
    
    if(FitOption==2000){
      nameplot_png = "Signal_BackgroundTrueReco_"+TimeSim_+"s_"+RateSim+"kHz_"+ResolSim+"MeV_BinnedLoglike_20000bins.png";
    }
    if(FitOption==1000){
      nameplot_png = "Signal_BackgroundTrueReco_"+TimeSim_+"s_"+RateSim+"kHz_"+ResolSim+"MeV_BinnedChi2_20000bins.png";
    }
   char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());
   std::cout<<"Generated plot in: "<<nameplot_png<<std::endl;
   c->Print(char_nameplot_png);
    
   
#if defined(USE_GRAPHICS)
   TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
   rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
   app.Run();
#endif
  
}


int main(int argc, char *argv[]) {

  std::string rootpath = argv[1];
  int FitOption = std::atoi(argv[2]);
  double xfit_low = std::stod(argv[3]);
  double xfit_max = std::stod(argv[4]);
  double seed = std::stod(argv[5]);
  std::string outpath = argv[6];

  FitFromHistogram_jobs(argc, argv, rootpath, FitOption, xfit_low, xfit_max, seed, outpath);

  return 0;
}
