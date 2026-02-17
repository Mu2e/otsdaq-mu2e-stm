#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"


//1- This code reads the Data (a histogam with the energy spectrum with a given calibration), Seeds for fit and Best fit values from a root file
//2- Set the fit range and initial parameters for the fit as the seed parameters for the root file, except the mean of the gaussian which is now moved due to the new calibration and use as seed for the mean of the gaussian the input parameter meanE_fit 
//After the fit, it  gets the signal and the signal error



void FitFromHistogram_simhisto(int argc, char *argv[], int FitOption, double xfit_low, double xfit_max, double meanE_fit, double seed, std::string rate, double backgroundpeaks)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  gROOT->SetStyle("ATLAS");
  TCanvas* c = new TCanvas("c");

      
  std::cout<<"Seed: "<<seed<<std::endl;
 
  //***********READ DATA***********//
  double xx1[2]={0.04, 2};
  //double yy1[2]={0,10100};

  int Nbinshisto = 20000; 

  TH1D* hreco = new TH1D("hReco","",Nbinshisto,xx1[0],xx1[1]);
  
  //RECO
  fstream readfile_reco;
  std::vector<string> file_name_reco;
  std::string name;
  readfile_reco.open("/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/FlashXrayOverlappedSim/SUM_Flash12HzXrays/"+rate+"kHz/MWDM400L200_Noise/"+rate+"kHz_FlashReco.txt",ios::in);
  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile_reco>>name;
    file_name_reco.push_back(name);
    if(readfile_reco.eof())break;
    }


  std::cout<<"Size: "<<file_name_reco.size()<<std::endl;

  for (int file=0;file<(file_name_reco.size()-1);file++){

    string path;
    path=file_name_reco[file];
    std::cout<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());

    TTree* tree=(TTree*)input->Get("treeADC");
    double p;
    tree->SetBranchAddress("peaks",&p);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);
      double E = p*(-0.57);
      double EMeV = E/1000;
      hreco->Fill(EMeV);
    }

  }//for int file   

  
    
  std::stringstream stream_xfit_low, stream_xfit_max;
  stream_xfit_low << std::fixed << std::setprecision(3) << xfit_low;
  stream_xfit_max << std::fixed << std::setprecision(3) << xfit_max;

  
  //***************GET HISTOGRAM AND FIT*****************//
  
  int Nentries = hreco->GetEntries();
  int Nbins = hreco->GetNbinsX();
  double bin_width = hreco->GetBinWidth(0);

  std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;
  
  //***********Initialise Minuit Class***********//

  MinuitFitter *fitter = new MinuitFitter(FitOption, seed);

  double fit_range[2];
  fit_range[0] = xfit_low;
  fit_range[1] = xfit_max;
  
  fitter->Histo_Data(fit_range, hreco); //Get nbins_fit, _binning, bincontent, bincontent_error, bincenter
  
  //Return nbins for the fit 
  unsigned long int nbins_fit = fitter->return_nbinsfit();  
  //Return dof
  unsigned long int dof = fitter->return_dof();
  std::cout<<"Number of bins for the fit: "<<nbins_fit<<" Degrees of freedom (ndof): "<<dof<<std::endl;
  //Return number of fit parameters NP
  int NP = fitter->return_NP();
  std::cout<<"Number of fit parameters, NP: "<<NP<<std::endl;


  TF1* ffit;

  double reco_peaks;
  double reco_sigma;


  //Initialise parameters value to do the fit
  double par_fit [NP];

  //Initialise fit parameters
  //Signal

  //hreco->GetXaxis()->SetRangeUser(xfit_low, xfit_max);
  
  double p0 = 1.58e-3;
  double p1 = 2.37;
  double p2 = -9.75e-1;
  double p3 = 7.43e-2;
  
  //double sigma = 0.001;
  double sigma = 0.003;
  
  par_fit [0] = 600*bin_width;
  par_fit [1] = meanE_fit;
  par_fit [2] = sigma;
  //Background                                                          
  par_fit [3] = backgroundpeaks*p0*bin_width;
  par_fit [4] = p1;
  par_fit [5] = p2;
  par_fit [6] = backgroundpeaks*p3*bin_width;
 
  //For 100kHz
  /*  par_fit [0] = 0.0377572;
  par_fit [1] = 0.347884;
  par_fit [2] = 0.000586675;
  par_fit [3] = -36.6191;
  par_fit [4] = 14.4319;
  par_fit [5] = -120.375;
  par_fit [6] = 161.333;
  */

  //Initialise S+B function
  
  std::string  genfunc_name = "fgenSB";
  char* char_genfunc_name = const_cast<char*>(genfunc_name.c_str());
  fitter->Init_SBfunc(char_genfunc_name, fit_range);
  fitter->SetPar_SBfunc(par_fit);
  TF1* fgenSB;
  fgenSB = fitter->return_fSB();



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

  //double time_acc = (accuracy/0.1)*(accuracy/0.1)*TimeSim;
  //double time_acc_error = 2*accuracy*TimeSim*accuracy_error/(0.1*0.1);
  //std::cout<<"Time for accuracy=0.1 = "<<time_acc<<" +- "<<time_acc_error<<" s"<<std::endl;


  fitter->Init_SBfunc(char_fitfunc_name, fit_range);
  fitter->SetPar_SBfunc(par_minuit);
  ffit = fitter->return_fSB();


  double ymax, ymin;
  double plot_x_low = 0.335;
  double plot_x_max = 0.37;

  
  if( plot_x_low < 0.04){ plot_x_low = 0.04; }
 
  ymin=0;
  hreco->GetXaxis()->SetRangeUser(plot_x_low, plot_x_max);
  ymax = hreco->GetMaximum()+10;
  ymax = 150;
  
  double plot_y_range[2]={ymin,ymax};
  double plot_x_range[2]={plot_x_low,plot_x_max};

  TGraph *graph1 = new TGraph (2, plot_x_range, plot_y_range);
  graph1->GetXaxis()->SetRangeUser(plot_x_range[0], plot_x_range[1]);
  graph1->GetYaxis()->SetRangeUser(plot_y_range[0], plot_y_range[1]);
  graph1->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
  graph1->GetYaxis()->SetTitle("Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  hreco->SetFillColor(kPink+2);
  hreco->SetLineColor(kPink+2);
  //hreco->SetFillStyle(3001);
  hreco->SetFillStyle(3022); 
  hreco->Draw("same");
  //This is the fit function with the initial seeds
  //fgenSB->Draw("same");

  ffit->SetLineColor(kBlack);
  ffit->SetNpx(5000);
  ffit->SetLineStyle(2);
  ffit->Draw("same");

  std::stringstream stream_reco_peaks, stream_reco_sigma;
  stream_reco_peaks << std::fixed << std::setprecision(1) << reco_peaks;
  stream_reco_sigma << std::fixed << std::setprecision(1) << reco_sigma;

  
  std::string sim_legend_str = rate+" kHz Flash + 12 Hz X-rays";
  char* sim_legend = const_cast<char*>(sim_legend_str.c_str());

  std::string sim_legend_str_fit = "#splitline{# fitted signal events = }{"+stream_reco_peaks.str()+" #pm "+stream_reco_sigma.str()+" from 595 347-keV X-rays}";
  char* sim_legend_fit = const_cast<char*>(sim_legend_str_fit.c_str());
  
  auto legend = new TLegend(0.51,0.6,0.9,0.9);
  //auto legend = new TLegend(0.21,0.21,0.7,0.44); 
  legend->AddEntry(hreco,sim_legend,"f");
  legend->AddEntry(ffit,sim_legend_fit,"l");
  legend->Draw("same");

  c->Print("30khz_FittedSignal.png");
  
#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
  
}


int main(int argc, char *argv[]) {

  int FitOption = std::atoi(argv[1]);
  double xfit_low = std::stod(argv[2]);
  double xfit_max = std::stod(argv[3]);
  double meanE_fit = std::stod(argv[4]);
  double seed = std::stod(argv[5]);
  std::string rate = argv[6];
  double backgroundpeaks = std::stod(argv[7]);
  
  FitFromHistogram_simhisto(argc, argv, FitOption, xfit_low, xfit_max, meanE_fit, seed, rate, backgroundpeaks);

  return 0;
}
