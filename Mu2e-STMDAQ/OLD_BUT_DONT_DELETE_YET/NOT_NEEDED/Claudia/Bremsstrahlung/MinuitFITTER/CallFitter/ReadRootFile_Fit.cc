
#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"


//1- This code reads the Data, Cov Matrix and Best fit values from a root file                                                            
//2- Changes the fit range and get the signal and the signal error



void ReadRootFile_Fit(int argc, char *argv[], std::string rootname, int FitOption, double xfit_low, double xfit_max, double seed, std::string outpath)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif
  
  bool store_dataLOG = true;
  bool store_dataROOT = true;
  
  gROOT->SetStyle("ATLAS");
  TCanvas* c = new TCanvas("c");

      
  std::cout<<"Seed: "<<seed<<std::endl;
 
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

  pos1 = rootname.find("seed_0_") + 7;
  pos2 = rootname.find("MeV_1");
  diff = pos2-pos1;
  std::string resolution_str = rootname.substr(pos1, diff);
  double ResolSim = std::stod(resolution_str);


  
  std::stringstream stream_xfit_low, stream_xfit_max;
  stream_xfit_low << std::fixed << std::setprecision(3) << xfit_low;
  stream_xfit_max << std::fixed << std::setprecision(3) << xfit_max;
    
  if( store_dataLOG==true ) {
  
    std::string logfile = outpath+"/ReadRootFile_Fitrange_"+stream_xfit_low.str()+"_"+stream_xfit_max.str()+"_Timesim_"+std::to_string(int(TimeSim))+"s_"+rate_str+"kHz_"+resolution_str+"MeV.log";
    char* char_logfile = const_cast<char*>(logfile.c_str());
    
    gSystem->RedirectOutput(char_logfile, "w");
  }

  std::cout<<"Executing... ReadRootFile_Fit.cc"<<std::endl;
  std::cout<<"Time simulation: "<<TimeSim<<" Rate  simulation: "<<RateSim<<" Resolution simulation: "<<ResolSim<<std::endl;

  TFile*output;
  std::string rootoutfile;
  if(store_dataROOT==true) {

    rootoutfile = outpath+"/ReadRootFile_Fitrange_"+stream_xfit_low.str()+"_"+stream_xfit_max.str()+"_Timesim_"+std::to_string(int(TimeSim))+"s_"+rate_str+"kHz_"+resolution_str+"MeV.root";
    output = new TFile(rootoutfile.c_str(),"recreate");
  }


  
  std::cout<<"Check time simulation: "<<TimeSim<<" s"<<std::endl;

  
  //***************GET HISTOGRAM AND FIT*****************//
  
  TH1D *histo = (TH1D*)infile->Get("histo");
  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix1");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars1");
  TMatrixD *SeedFitPars = (TMatrixD*)infile->Get("SeedFitPars1");
  
  //Covmatrix->Print();
  //BestFitPars->Print();
  //histo->Draw("");  

  int Nentries = histo->GetEntries();
  int Nbins = histo->GetNbinsX();
  double bin_width = histo->GetBinWidth(0);

  std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;
  //***********Initialise Minuit Class***********//
  
  MinuitFitter *fitter = new MinuitFitter(FitOption, seed);

  double fit_range[2];
  fit_range[0] = xfit_low;
  fit_range[1] = xfit_max;
  
  fitter->Histo_Data(fit_range, histo); //Get nbins_fit, _binning, bincontent, bincontent_error, bincenter
  
  //Return nbins for the fit 
  unsigned long int nbins_fit = fitter->return_nbinsfit();  
  //Return dof
  unsigned long int dof = fitter->return_dof();
  std::cout<<"Number of bins for the fit: "<<nbins_fit<<" Degrees of freedom (ndof): "<<dof<<std::endl;
  //Return number of fit parameters NP
  int NP = fitter->return_NP();
  std::cout<<"Number of fit parameters, NP: "<<NP<<std::endl;



  //Calculate sqrt(cov matrix)
  TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");
  TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
  TMatrixD _SeedFitPars(1,NP,SeedFitPars->GetMatrixArray(),"");

  _Covmatrix.Print();
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


  //Initialise parameters value to do the fit
  double par_fit [NP];

  //When using same initial parameters as in previous studies get the same result
  //but if we best fit parameters as initial parameters to fit it
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
  TMatrixD* BestFitPars_newFit;
  
  //Get Minuit fit parameters after the fit
  p_minuit = new double[NP];
  perr_minuit = new double[NP];
  
  p_minuit = fitter->return_p_minuit();
  perr_minuit = fitter->return_perr_minuit();
  min_fcn = fitter->return_amin();
  error_def = fitter->return_errdef();
  Covmatrix_newFit = fitter->return_Covmatrix();





  
  //***********PLOT DATA***********//
  //Draw Minuit fit
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


  fitter->Init_SBfunc(char_fitfunc_name, fit_range);
  fitter->SetPar_SBfunc(par_minuit);
  ffit = fitter->return_fSB();


  double ymax;
  ymax = histo->GetMaximum();
  double plot_y_range[2]={0,ymax};

  double plot_x_low = xfit_low - 0.1;
  double plot_x_max = xfit_max + 0.1;
  //double plot_x_low = 0.325;
  //double plot_x_max = 0.37;
  
  if( plot_x_low < 0.04){ plot_x_low = 0.04; }
  
  double plot_x_range[2]={plot_x_low,plot_x_max};

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
  
  histo->SetFillColor(kOrange-3);
  histo->SetLineColor(kOrange-3);
  histo->SetFillStyle(3001);
  histo->Draw("same");
  
  ffit->SetLineColor(kBlack);
  ffit->SetLineStyle(2);
  ffit->Draw("same");







  if(store_dataROOT==true) {
    TTree* Signaltree;

    std::vector<double> RecoSignal, RecoSignal_sigma, xfitlow, xfitmax;
      
    Signaltree = new TTree("Signaltree", "Signaltree");
    Signaltree->Branch("RecoSignal", &RecoSignal);
    Signaltree->Branch("RecoSignal_sigma", &RecoSignal_sigma);
    Signaltree->Branch("xfitlow", &xfitlow);
    Signaltree->Branch("xfitmax", &xfitmax);
    
    RecoSignal.push_back(reco_peaks);
    RecoSignal_sigma.push_back(reco_sigma);
    xfitlow.push_back(xfit_low);
    xfitmax.push_back(xfit_max);
    
    Signaltree->Fill();
    output->Write();
    

    output->WriteObject(histo, "histo");

    std::string str_SeedFitPars = "SeedFitPars";
    char* char_SeedFitPars = const_cast<char*>(str_SeedFitPars.c_str());
    output->WriteObject(SeedFitPars, char_SeedFitPars);
    
    std::string str_Covmatrix_fitrange = "Covmatrix_fitrange";
    char* char_Covmatrix_fitrange = const_cast<char*>(str_Covmatrix_fitrange.c_str());

    output->WriteObject(Covmatrix_newFit, char_Covmatrix_fitrange);
    
    std::string str_BestFitPars_fitrange = "BestFitPars_fitrange";
    char* char_BestFitPars_fitrange = const_cast<char*>(str_BestFitPars_fitrange.c_str());
    TMatrixD _BestFitPars_fitrange(NP,1);
    for( int j = 0; j < NP; j++) {
      _BestFitPars_fitrange[j][0] = par_minuit[j];
    }
    TMatrixD* BestFitPars_newFit = new TMatrixD(NP,1,_BestFitPars_fitrange.GetMatrixArray(),"");
    
    output->WriteObject(BestFitPars_newFit, char_BestFitPars_fitrange);

    output->Close();
  }
  
  





  
  std::string nameplot_png = outpath+"/ReadRootFile_Fitrange_"+stream_xfit_low.str()+"_"+stream_xfit_max.str()+"_Timesim_"+std::to_string(int(TimeSim))+"s_"+rate_str+"kHz_"+resolution_str+"MeV.png";
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

  std::string rootname = argv[1];
  int FitOption = std::atoi(argv[2]);
  double xfit_low = std::stod(argv[3]);
  double xfit_max = std::stod(argv[4]);
  double seed = std::stod(argv[5]);
  std::string outpath = argv[6];

  ReadRootFile_Fit(argc, argv, rootname, FitOption, xfit_low, xfit_max, seed, outpath);

  return 0;
}
