#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

//#include "/home/stm_mu2e/claudiaa/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
//#include "/work/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"

//1- This code generates the signal (with a given resolution and energy -input) and background (at a certain rate -input) for a given time: TimeSim
//2- The data is stored in a binned histogram to fit it with Binned Chi2 or Log likelihood function
//3- The data is also stored in an array to fit with the Unbinned Log likelihood function
//4- The fits provide the best fit parameters and a covariance matrix from Minuit
//5- If the seed is set to 0, get different resut everytime we run it. If the seed is fixed to a number then we get the same result everytime we run it with the same seed
//6- If check_ROOTfit=true then the fit is also done with ->fit() in ROOT (not calling Minuit)
//7- If check_MINOS=true it calculates the assymetric MINOS error hard-coded to check with MINOS result from Minuit
//8- If store_dataROOT=true it stores the data in a TH1D and the covariance matrix and best fit parameters in 2 TMatrix
//9- It prints in a histogram the data (signal + background) and fit function with Minuit

void FitterROOT(int argc, char *argv[], double Bremsrate_kHz, double TimeSim, double meanE, double sigma, int FitOption, unsigned long int seed, int nexperiments, int count_experiment, int jobNum, std::string outpath, std::vector<double> &TrueSignal, std::vector<double> &RecoSignal, std::string txtBackgroundParameters) {

  std::cout<<"Executing... FitterROOT.cc"<<std::endl;
  
  std::stringstream streamBremsrate_kHz;
  streamBremsrate_kHz << std::fixed << std::setprecision(2) << Bremsrate_kHz;
   
  std::string rate_string = streamBremsrate_kHz.str();
  std::string TimeSim_string = std::to_string(int(TimeSim));
  std::string FitOption_string = std::to_string(int(FitOption));
  std::string seed_string = std::to_string(seed);

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  gROOT->SetStyle("ATLAS");
  TCanvas* c = new TCanvas("c");
  
  //Turn ON/OFF checks
  bool debugout = true;
  bool check_ROOTfit = false;
  bool check_MINOS = false;
  bool store_dataROOT = true; // store signal+brems gen data and fit cov matrix in a ROOT file
  bool drawplot = true; //signal+background (for just 1 experiment) 

  //One of these two have to be false  otherwise it stores to much data
  bool store_all_peaksROOT = false; //this can only be true if store_dataROOT is true (signal and background together)
  bool store_separate_peaksROOT = false; //this can only be true if store_dataROOT is true (signal and background separate) 
  
  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;
  
  //***********START CODE***********//
  t1 = boost::chrono::high_resolution_clock::now();

  //Initialise Minuit Class
  MinuitFitter *fitter = new MinuitFitter(FitOption, seed);

  bool useintegral = fitter->return_useintegral();
  bool usebin_loglike = fitter->return_usebin_loglike();
  bool usebin_chi2 = fitter->return_usebin_chi2();
  bool useunbin_loglike = fitter->return_useunbin_loglike();

  int NPbrems = fitter->return_NPbrems();
  int NPsignal = fitter->return_NPsignal();
  int NP = fitter->return_NP();
  unsigned long int nbins = fitter->return_nbins();
  double frange[2];
  frange[0] = fitter->return_frange_min();
  frange[1] = fitter->return_frange_max();

  std::stringstream stream_sigma;
  stream_sigma << std::fixed << std::setprecision(4) << sigma;

  std::string name_string;
  if(useintegral && usebin_chi2){ name_string = outpath+"/BinnedChi2_Integral_"+rate_string+"kHz_TimeSim_"+TimeSim_string+"s_seed_"+seed_string+"_"+stream_sigma.str()+"MeV"; }
  if(!useintegral && usebin_chi2){ name_string = outpath+"/BinnedChi2_NOIntegral_"+rate_string+"kHz_TimeSim_"+TimeSim_string+"s_seed_"+seed_string+"_"+stream_sigma.str()+"MeV"; }
  if(useintegral && usebin_loglike){ name_string = outpath+"/BinnedLoglike_Integral_"+rate_string+"kHz_TimeSim_"+TimeSim_string+"s_seed_"+seed_string+"_"+stream_sigma.str()+"MeV"; }
  if(!useintegral && usebin_loglike){ name_string = outpath+"/BinnedLoglike_NOIntegral_"+rate_string+"kHz_TimeSim_"+TimeSim_string+"s_seed_"+seed_string+"_"+stream_sigma.str()+"MeV"; }
  if(useunbin_loglike){ name_string = outpath+"/UnbinnedLogLike_"+rate_string+"kHz_TimeSim_"+TimeSim_string+"s_seed_"+seed_string+"_"+stream_sigma.str()+"MeV"; }


  //Rate Hz
  double rateHz = Bremsrate_kHz*1000;

  //Number of peaks to simulate: 1 or 4 (full Al spectrum)
  int dimpeaks;
  if (meanE==0) { dimpeaks = 4; }
  else { dimpeaks = 1; }

  //Initialise mean energy array in MeV
  double mean_E[dimpeaks];
  if (meanE==0) {
    mean_E[0] = 0.066;
    mean_E[1] = 0.347;
    mean_E[2] = 0.844;
    mean_E[3] = 1.809;
  }
  else {
    mean_E[0] = meanE;
  }


  std::string bremsfuncnoNorm_name = "Brems_noNorm";
  char* char_bremsfuncnoNorm_name = const_cast<char*>(bremsfuncnoNorm_name.c_str());
  fitter->Init_Bremsfunc(char_bremsfuncnoNorm_name, frange);
  double p0, p1, p2, p3;

  //Read the brems background values from a txt file
  std::ifstream input_file;
  std::string BremsBackground_txt = "/work/mu2e/data1/cgarcia/SignaltoBackground_Mu2eAvRunI/HPGe/SystematicsBackgroundShapes/backgroundshapes/"+txtBackgroundParameters;

  double pos1 = txtBackgroundParameters.find("Brems_") + 6;
  double pos2 = txtBackgroundParameters.find(".txt");
  double diff = pos2-pos1;
  std::string background_type = txtBackgroundParameters.substr(pos1, diff);

  //Get background number
  double posnum = txtBackgroundParameters.find("Brems_Shape") + 11;
  diff = pos2-posnum;
  std::string background_num_str = txtBackgroundParameters.substr(posnum, diff);
  std::cout<<"Background Shape: "<<background_num_str<<std::endl;

  double background_num;
  std::string bckshapenom = "Nominal";
  
  if (background_num_str.compare(bckshapenom) == 0){background_num = 0; }
  else {background_num = std::stod(background_num_str);}
  
  input_file.open(BremsBackground_txt);
  if(!input_file.is_open()) throw std::runtime_error("Could not open file");
  
  std::string line;
  std::string element;


  while(std::getline(input_file, line))
    {
      // Create a stringstream of the current line 
      std::stringstream ss(line);
      while(ss >> element){
	
	if(element.compare("p0=") == 0) ss >> p0;
	if(element.compare("p1=") == 0) ss >> p1;
	if(element.compare("p2=") == 0) ss >> p2;
	if(element.compare("p3=") == 0) ss >> p3;
	
      }
    }
  
  //Rerun electron beam nominal parameters
  /*
    p0 = 0.00158168;
    p1 = 2.36728;
    p2 = -0.974756;
    p3 = 0.00011748;
  */
  
   std::cout<<"Brems parameters before normalisation: p0= "<<p0<<" p1= "<<p1<<" p2= "<<p2<<" p3= "<<p3<<std::endl;
   double par_nonorm [NPbrems] = {p0, p1, p2, p3};
   
   fitter->SetPar_Bremsfunc(par_nonorm);
   TF1* Bremsfunc_noNorm = fitter->return_fbrems();
   double brem_integral = Bremsfunc_noNorm->Integral(frange[0],frange[1]);
   std::cout<<"Brems integral to normalise: "<<brem_integral<<std::endl;
  
   p0 = p0 / brem_integral;
   p3 = p3 / brem_integral;

   std::cout<<"Brems parameters after normalisation: p0= "<<p0<<" p1= "<<p1<<" p2= "<<p2<<" p3= "<<p3<<std::endl;
  
   double par [NPbrems] = {p0, p1, p2, p3};

   
  //Open output ROOT file
  TFile*output;
  std::string rootfile;
  if(store_dataROOT==true) {
    //see stats
    gStyle->SetOptStat(1111);

    rootfile = name_string+"_"+std::to_string(nexperiments)+"Runs_Job_"+std::to_string(jobNum)+"_Background_"+background_type+".root";

    if( count_experiment==1 ) {
      output = new TFile(rootfile.c_str(),"recreate");
    }
    else { output = new TFile(rootfile.c_str(),"update");}
    }


  TTree* AllPeaksGen;
  std::vector<double> Peaks, PeaksSignal, PeaksBackground;

  //through an exception
  if((store_all_peaksROOT==true)&&(store_separate_peaksROOT==true)){ std::cout<<"This option stores the data twice, set one of the 2 to false..."<<std::endl; exit(0);}

  if((store_dataROOT==true)&&(store_all_peaksROOT==true)){
    AllPeaksGen = new TTree("AllPeaksGen", "AllPeaksGen");
    AllPeaksGen->Branch("Peaks", &Peaks);
  }
  
  if((store_dataROOT==true)&&(store_separate_peaksROOT==true)){
    AllPeaksGen = new TTree("AllPeaksGen", "AllPeaksGen");
    AllPeaksGen->Branch("PeaksSignal", &PeaksSignal);
    AllPeaksGen->Branch("PeaksBackground", &PeaksBackground);
  }

  
   //Define fit range for the background
   double fit_range[2];
   //fit_range[0] = mean_E[0]-100*sigmacut*sigma;
   //fit_range[1] = mean_E[0]+100*sigmacut*sigma;
   fit_range[0] = 0.04;
   fit_range[1] = 2;
   
  
  //***************GENERATE BREMS BACKGROUND***************//    
 
  //Number of bremsstrahlung photons in the sample
  unsigned long int pulseNumBrems = rateHz*TimeSim;

  if(debugout==true) {
    std::cout<<"Theoretical pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl;
    std::cout<<"--Sample number of pulses with a Poisson distribution---"<<std::endl;
  }

  //Sample number of bremsstrahlung pulses with a Poisson distribution
  pulseNumBrems = Random::Instance()->PoissValue(1./pulseNumBrems);

  if(debugout==true) { std::cout<<"Number of pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl; }

  //Initialise Bremsstrahlung function
  std::string bremsfunc_name = "fbrems";
  char* char_bremsfunc_name = const_cast<char*>(bremsfunc_name.c_str());

  fitter->Init_Bremsfunc(char_bremsfunc_name, frange);
  fitter->SetPar_Bremsfunc(par);
  TF1* fbrems = fitter->return_fbrems();
  
  //Initialise the brems data pointer
  fitter->InitBrems_Data(pulseNumBrems);
  //Generate the brems data
  fitter->GenBrems_Data(pulseNumBrems);
 
  //Get data and Fill Bremsstrahlung histogram
  double* dataBrems = new double[pulseNumBrems];
  dataBrems = fitter->return_dataBrems();

  //Get number of counts in the background in photopeak range
  double photopeak_347_low = 0.344;
  double photopeak_347_high = 0.35;
  unsigned long int counts_bremsphotopeak = 0;
  for(unsigned long int i =0; i < pulseNumBrems; i++){
    if((dataBrems[i]>photopeak_347_low)&&(dataBrems[i]<photopeak_347_high)){counts_bremsphotopeak++;}
  }

  
  //Fill histogram with data
  //If want to generate brems-only histogram
  
  TH1D *hBrems = new TH1D("hBrems", "", nbins,frange[0],frange[1]);
  for(unsigned long int i =0; i < pulseNumBrems; i++){
    hBrems->Fill(dataBrems[i]);
    if((store_dataROOT==true)&&(store_separate_peaksROOT==true)){ PeaksBackground.push_back(dataBrems[i]); }
  }
  
  //***************GENERATE SIGNAL***************//

  //Number of XRrays in the sample
  unsigned long int pulseNumXrays[dimpeaks];  
  //old way of calculating num Xrays
  /*
  if(meanE==0.066) { pulseNumXrays[0] = rate_66keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"66keV XRay pulses in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl; }
  }
  else if(meanE==0.347) { pulseNumXrays[0] = rate_347keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"347keV XRay pulses in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl; }
  }
  else if(meanE==0.844) { pulseNumXrays[0] = rate_844keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"844keV XRay pulses in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl; }
  }
  else if(meanE==1.809) { pulseNumXrays[0] = rate_1809keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"1809keV XRay pulses in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl; }
  }
  else if(meanE==0) {
    pulseNumXrays[0] = rate_66keV_s_STM*TimeSim;
    pulseNumXrays[1] = rate_347keV_s_STM*TimeSim;
    pulseNumXrays[2] = rate_844keV_s_STM*TimeSim;
    pulseNumXrays[3] = rate_1809keV_s_STM*TimeSim;
  }
  else { std::cout<<"X-Ray energy not valid"<<std::endl; exit(0); }
  */
  
  //double flashXray347keV_factor = 1.13;
  double XrayCteRate_OnSpillHz = 12;
  pulseNumXrays[0] = XrayCteRate_OnSpillHz * TimeSim;
  //pulseNumXrays[0] = counts_bremsphotopeak * flashXray347keV_factor;
  pulseNumXrays[1] = 0;
  pulseNumXrays[2] = 0;
  pulseNumXrays[3] = 0;
  
  
  //Sample number of X-rays with a Poisson distribution
  double par_signal [NPsignal];
  TF1* fsignal[dimpeaks];
  //TH1D* hSignal[dimpeaks];
  double* dataSignal;

  //Data size: signal + brems
  unsigned long int data_size = pulseNumBrems;

  //Generate number of pulses in signal, add signal size to data_size
  for(int i = 0; i < dimpeaks; i++) {
    
    if(debugout==true) {
      std::cout<<"Theoretical pulses in Signal: "<<pulseNumXrays[i]<<std::endl;
    }
    
    pulseNumXrays[i] = Random::Instance()->PoissValue(1./pulseNumXrays[i]);

    if(debugout==true) {
      std::cout<<"--Sample number of pulses with energy: "<<mean_E[i]<<"MeV using a Poisson distribution---"<<std::endl;
      std::cout<<"Number of pulses in X-Ray peak: "<<pulseNumXrays[i]<<std::endl;
    }
    data_size += pulseNumXrays[i];
  }

  
  std::cout<<"Number of Flash  photons in range "<<photopeak_347_low<<", "<<photopeak_347_high<<" MeV: "<<counts_bremsphotopeak<<" and X-rays: "<<pulseNumXrays[0]<<std::endl;

  double ratioXrayFlash = 1.0*pulseNumXrays[0]/counts_bremsphotopeak;

  double ratioXrayFlash_error = ratioXrayFlash * sqrt((sqrt(pulseNumXrays[0])/pulseNumXrays[0])*(sqrt(pulseNumXrays[0])/pulseNumXrays[0])+(sqrt(counts_bremsphotopeak)/counts_bremsphotopeak)*(sqrt(counts_bremsphotopeak)/counts_bremsphotopeak));
    
  std::cout<<"Xrays/Flash(347keV): "<<ratioXrayFlash<<" +- "<<ratioXrayFlash_error<<std::endl;
    
  //Initialise signal+brems dataset
  if(debugout==true) { std::cout<<"Data size: "<<data_size<<" counts (brems+signal in range)"<<std::endl; }
  double* dataset = new double[data_size];

  //memcpy brems data to full dataset
  memcpy(&dataset[0],&dataBrems[0],8*pulseNumBrems); //doubles are 8 bytes

  //memcpy init index
  unsigned long int init = pulseNumBrems;
 
  for(int i = 0; i < dimpeaks; i++) {

    //Generate the signal as a gaussian
    par_signal [0] = 1;
    par_signal [1] = mean_E[i];
    par_signal [2] = sigma;
    
    //Initialise signal function
    string signalfunc_name = "fsignal"+std::to_string(i);
    char*  char_signalfunc_name = const_cast<char*>(signalfunc_name.c_str());

    fitter->Init_Signalfunc(char_signalfunc_name, frange);
    fitter->SetPar_Signalfunc(par_signal);
    fsignal[i] = fitter->return_fsignal();

    //Initialise the signal data pointer
    fitter->InitSignal_Data(pulseNumXrays[i]);
    //Generate the signal data
    fitter->GenSignal_Data(pulseNumXrays[i]);

    //Get data and Fill Signal histogram
    dataSignal = new double[pulseNumXrays[i]];
    dataSignal = fitter->return_dataSignal();

    //Fill histogram with data
    string signalhisto_name = "hsignal"+std::to_string(i);
    char*  char_signalhisto_name = const_cast<char*>(signalhisto_name.c_str());
      
    //memcpy signal to full dataset
    memcpy(&dataset[init],&dataSignal[0],8*pulseNumXrays[i]);

    //move dataset start
    init = init + pulseNumXrays[i];
  }


  TH1D* hSignal = new TH1D("hSignal", "", nbins,frange[0],frange[1]);
  for(unsigned long int j =0; j < pulseNumXrays[0]; j++){
    hSignal->Fill(dataSignal[j]);
    if((store_dataROOT==true)&&(store_separate_peaksROOT==true)){ PeaksSignal.push_back(dataSignal[j]); }
    //std::cout<<"SIG: "<<j<<" "<<dataSignal[j]<<std::endl;
  }

      
  //******HISTO WITH BREMSSTRAHLUNG AND SIGNAL TOGETHER***//

  //Fill brems + signal histogram
  std::cout<<"-------"<<std::endl;
  
  std::string str_hSTM = "hSTM"+std::to_string(count_experiment);
  char* char_hSTM = const_cast<char*>(str_hSTM.c_str());

  TH1D* hSTM = new TH1D(char_hSTM, "", nbins,frange[0],frange[1]);
  
  for (unsigned long int i = 0 ; i < data_size ; i++) {
    hSTM->Fill(dataset[i]);

    if((store_dataROOT==true)&&(store_all_peaksROOT==true)){Peaks.push_back(dataset[i]);}
    
  }

  std::cout<<"Histogram S+B filled"<<std::endl;
  
  if(store_dataROOT==true) {
    
    if((store_all_peaksROOT==true)||(store_separate_peaksROOT==true)){AllPeaksGen->Fill();} //This fails for 200kHz
    output->WriteObject(hSTM, char_hSTM);
    output->WriteObject(hSignal, "hSignal");
    output->WriteObject(hBrems, "hBrems");
    
  }

  //***************GET HISTOGRAM AND FIT*****************//

  //Get histogram data given the fit range and binwidth
  //If want to fit just background data use hBrems
  //If want to fit just signal data use hSignal
  //Else use hSTM
  TH1D *histo = (TH1D*)hSTM->Clone("histo");

  double bin_width = histo->GetBinWidth(0);
  unsigned long int nbins_fit;
  double* p_minuit;
  double* perr_minuit;
  double min_fcn, error_def;
  TMatrixD* Covmatrix;

  TF1* ffit;
  
  double reco_peaks;
  double reco_sigma;
  char* char_Covmatrix;
  char* char_BestFitPars;
  char* char_SeedFitPars;
 
  if(usebin_chi2 || usebin_loglike){
    
    //Get histogram bin content and errors in fitrange
    fitter->Histo_Data(fit_range, histo);
    //bin_width = fitter->return_binning();
    if(debugout==true){
      std::cout<<"Fitting using bin width: "<<bin_width<<" MeV"<<std::endl;
    }

    //Return nbins for the fit
    nbins_fit = fitter->return_nbinsfit();

    //////*********JUST USING BREMS FUNCTION TO FIT THE BREMS+SIGNAL HISTOGRAM---CHANGE THIS IN FCN FIT FUNCT***///
    //Initialise parameters value to do the fit 
    double par_fit [NP];

    //Signal
    par_fit [0] = pulseNumXrays[0]*bin_width;
    par_fit [1] = meanE;
    par_fit [2] = sigma;
    //Background
    par_fit [3] = pulseNumBrems*p0*bin_width;
    par_fit [4] = p1;
    par_fit [5] = p2;
    par_fit [6] = pulseNumBrems*p3*bin_width;

    //Initialise S+B function
    std::string  genfunc_name = "fgenSB";
    char* char_genfunc_name = const_cast<char*>(genfunc_name.c_str());
    fitter->Init_SBfunc(char_genfunc_name, fit_range);
    fitter->SetPar_SBfunc(par_fit);

    //TF1* fcheck = fitter->return_fSB();
    //fcheck->SetLineColor(kBlue);
    //fcheck->Draw("same");

    //Fit with Minuit
    fitter->fit(par_fit, NP);

    //Get Minuit fit parameters
    p_minuit = new double[NP];
    perr_minuit = new double[NP];
    
    p_minuit = fitter->return_p_minuit();
    perr_minuit = fitter->return_perr_minuit();
    min_fcn = fitter->return_amin();
    error_def = fitter->return_errdef();
    Covmatrix = fitter->return_Covmatrix();
    //std::cout<<"This is a check::: Covariance Matrix: "<<std::endl;
    //Covmatrix->Print();

    /* IF FIT JUST BREMSSTRAHLUNG
    double height_histo1 = pulseNumBrems*p0*bin_width;
    double height_histo2 = pulseNumBrems*p3*bin_width;

    double par_fit [NPbrems] = {height_histo1, p1, p2, height_histo2};
    std::cout<<"Initial fit values: p0 : "<<par_fit[0]<<" p1: "<<par_fit[1]<<" p2: "<<par_fit[2]<<" p3: "<<par_fit[3]<<std::endl;
    
    //Fit with Minuit
    fitter->fit(par_fit, NPbrems);

    //Get Minuit fit parameters
    p_minuit = new double[NPbrems];
    perr_minuit = new double[NPbrems];
    
    p_minuit = fitter->return_p_minuit();
    perr_minuit = fitter->return_perr_minuit();
    min_fcn = fitter->return_amin();
    error_def = fitter->return_errdef();
    */
    ///////////////////////////////
 
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
    TMatrixD _Covmatrix(7,7,Covmatrix->GetMatrixArray(),"");
    reco_sigma = sqrt(_Covmatrix[0][0]) / bin_width;
    _Covmatrix.Print();

    std::cout<<"Sigma Signal: sqrt(cov matrix element)/bin width: "<<sqrt(_Covmatrix[0][0])<<"/"<<bin_width<<"="<<reco_sigma<<std::endl;
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


    if(store_dataROOT==true) { 

      std::string str_SeedFitPars = "SeedFitPars"+std::to_string(count_experiment);
      char_SeedFitPars = const_cast<char*>(str_SeedFitPars.c_str());
      TMatrixD _SeedFitPars(NP,1);
      for( int j = 0; j < NP; j++) {
        _SeedFitPars[j][0] = par_fit[j];
      }
      TMatrixD* SeedFitPars = new TMatrixD(NP,1,_SeedFitPars.GetMatrixArray(),"");

      output->WriteObject(SeedFitPars, char_SeedFitPars);

      std::string str_Covmatrix = "Covmatrix"+std::to_string(count_experiment);
      char_Covmatrix = const_cast<char*>(str_Covmatrix.c_str());
      
      output->WriteObject(Covmatrix, char_Covmatrix);

      std::string str_BestFitPars = "BestFitPars"+std::to_string(count_experiment);
      char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());
      TMatrixD _BestFitPars(NP,1);
      for( int j = 0; j < NP; j++) {
	_BestFitPars[j][0] = par_minuit[j];
      }
      TMatrixD* BestFitPars = new TMatrixD(NP,1,_BestFitPars.GetMatrixArray(),"");

      output->WriteObject(BestFitPars, char_BestFitPars);
    }

  } //usebin_chi2 || usebin_loglike


  if (useunbin_loglike) {

    //Initialise full dataset (brems+signal) and size in class to call fcn
    fitter->Init_UnBinnedData(data_size, dataset);

    //Initialise parameters value to do the fit 
    double par_fit [NP];

    //Signal
    par_fit [0] = meanE;
    par_fit [1] = sigma;
    //Background
    par_fit [2] = pulseNumBrems*p0*bin_width;
    par_fit [3] = p1;
    par_fit [4] = p2;
    par_fit [5] = pulseNumBrems*p3*bin_width;
    //fs
    par_fit [6] = (double) pulseNumXrays[0] / data_size; //be careful with int division

    //Fit with minuit
    fitter->fit(par_fit, NP);

    //Get Minuit fit parameters
    p_minuit = new double[NP];
    perr_minuit = new double[NP];

    p_minuit = fitter->return_p_minuit();
    perr_minuit = fitter->return_perr_minuit();
    min_fcn = fitter->return_amin();
    error_def = fitter->return_errdef();
    Covmatrix = fitter->return_Covmatrix();

    //Draw Minuit fit
    std::string  fitfunc_name = "ffit";
    char* char_fitfunc_name = const_cast<char*>(fitfunc_name.c_str());
    std::cout<<"Minuit parameters: "<<std::endl;
    std::cout<<"min FCN: "<<min_fcn<<" error definition: "<<error_def<<std::endl;
    
    double par_minuit [NP];
    for(int i = 0 ; i < NP; i++) {
      std::cout<<p_minuit[i]<<" +- "<<perr_minuit[i]<<std::endl;      
      par_minuit[i] = p_minuit[i];
    }

    //Recovered number of peaks in signal
    double fs = par_minuit[NP-1];
    reco_peaks = fs * data_size;
    std::cout<<"Peaks recovered from Unbinned LogLikelihood fit: "<< reco_peaks <<std::endl;
    TMatrixD _Covmatrix(7,7,Covmatrix->GetMatrixArray(),"");
    reco_sigma = sqrt(_Covmatrix[NP-1][NP-1]) * data_size;
    //_Covmatrix.Print();

    std::cout<<"Sigma Signal: sqrt(cov matrix element/bin width): "<<sqrt(_Covmatrix[0][0])<<"/"<<bin_width<<"="<<reco_sigma<<std::endl;
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

    //This funtion returned by minuit is normalised, need to reescale it 
    double scale = data_size * bin_width;
    ffit = fitter->plotMinuit_NORM(char_fitfunc_name, par_minuit, fit_range,scale);
    

    if(store_dataROOT==true) {

      std::string str_SeedFitPars = "SeedFitPars"+std::to_string(count_experiment);
      char_SeedFitPars = const_cast<char*>(str_SeedFitPars.c_str());
      TMatrixD _SeedFitPars(NP,1);
      for( int j = 0; j < NP; j++) {
        _SeedFitPars[j][0] = par_fit[j];
      }
      TMatrixD* SeedFitPars = new TMatrixD(NP,1,_SeedFitPars.GetMatrixArray(),"");

      output->WriteObject(SeedFitPars, char_SeedFitPars);
      
      std::string str_Covmatrix = "Covmatrix"+std::to_string(count_experiment);
      char_Covmatrix = const_cast<char*>(str_Covmatrix.c_str());
     
      output->WriteObject(Covmatrix, char_Covmatrix);
      
      std::string str_BestFitPars = "BestFitPars"+std::to_string(count_experiment);
      char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());
      TMatrixD _BestFitPars(NP,1);
      for( int j = 0; j < NP; j++) {
        _BestFitPars[j][0] = par_minuit[j];
      }
      TMatrixD* BestFitPars = new TMatrixD(NP,1,_BestFitPars.GetMatrixArray(),"");
      
      output->WriteObject(BestFitPars, char_BestFitPars);
    }

  }


  //Plotting
  if( drawplot==true ) {
    double ymax;
    ymax = histo->GetMaximum();
    double plot_y_range[2]={0,ymax};
    //double plot_x_range[2]={0.04,2};
    //double plot_y_range[2]={0,1000};
    //double plot_x_range[2]={0.32,0.37};
    double plot_x_range[2]={0.04,1};
    //double plot_x_range[2]={0.05,0.5};  
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

    if(background_num == 0)  {
      histo->SetFillColor(kOrange-3);
      histo->SetLineColor(kOrange-3);
    }
    else {
      histo->SetFillColor(background_num+1);
      histo->SetLineColor(background_num+1);
    }

    histo->SetFillStyle(3001);
    histo->Draw("same");

    ffit->SetLineColor(kBlack);
    ffit->SetLineStyle(2);
    ffit->Draw("same");

    char* char_latex1;
    std::string str_latex1;
    TLatex latex1;

    if(FitOption==2000){
      str_latex1 = "Binned Log Fit";
    }
    else if(FitOption==1000){
      str_latex1 = "#chi^{2} Fit";
    }
    else if(FitOption==3000){
      str_latex1 = "Unbinned Log Fit";
    }
    else{
       str_latex1 = "";
    }

    char_latex1 = const_cast<char*>(str_latex1.c_str());
    std::cout<<char_latex1<<std::endl;
    //latex1.DrawLatexNDC(.35,.87,char_latex1);
  }


  //Check ROOT fitting
  if(check_ROOTfit==true) {

    if(usebin_loglike) {
      std::cout<<"****************************************************"<<std::endl;
      std::cout<<"******ROOT: FITTING WITH BINNED LOG-LIKELIHOOD******"<<std::endl;
      std::cout<<"****************************************************"<<std::endl;
      if(useintegral==true) { histo->Fit(fbrems,"LIRN","", fit_range[0], fit_range[1]); }
      else { histo->Fit(fbrems,"LRN","", fit_range[0], fit_range[1]); }
      double chi2_rootfit = fbrems->GetChisquare();
      double dof_rootfit = fbrems->GetNDF();
      double prob_rootfit = fbrems->GetProb();
      std::cout<<"Root fit chi2: "<<chi2_rootfit<<", dof: "<<dof_rootfit<<", chi2/dof: "<<chi2_rootfit/dof_rootfit<<", prob: "<<prob_rootfit<<std::endl;
      fbrems->SetLineColor(kGreen);
      fbrems->SetLineWidth(1);
      fbrems->SetRange(fit_range[0],fit_range[1]);
      fbrems->Draw("same");
    }
    
    if(usebin_chi2) {
      std::cout<<"**********************************************************"<<std::endl;
      std::cout<<"******ROOT: FITTING WITH BINNED LEAST SQUARES - CHI2******"<<std::endl;
      std::cout<<"**********************************************************"<<std::endl;
      if(useintegral==true) { histo->Fit(fbrems,"IRN","", fit_range[0], fit_range[1]); }
      else { histo->Fit(fbrems,"RN","", fit_range[0], fit_range[1]); }
      double chi2_rootfit = fbrems->GetChisquare();
      double dof_rootfit = fbrems->GetNDF();
      double prob_rootfit = fbrems->GetProb();
      std::cout<<"Root fit chi2: "<<chi2_rootfit<<", dof: "<<dof_rootfit<<", chi2/dof: "<<chi2_rootfit/dof_rootfit<<", prob: "<<prob_rootfit<<std::endl;
      fbrems->SetLineColor(kGreen);
      fbrems->SetLineWidth(1);
      fbrems->SetRange(fit_range[0],fit_range[1]);
      fbrems->Draw("same");
    }
  } //Check ROOT fitting 


  //Check MINOS error for 1st fit parameter, this is true given no correlation
  if(check_MINOS==true) {

    TGraph *grMinos = new TGraph();

    int npoints = 10000;
    double p_minos[NPbrems];
    int npar = NPbrems;
    double gin[NPbrems];
    int iflag = 0;
    //Init values
    p_minos[0] = 4000;
    p_minos[1] = p_minuit[1];
    p_minos[2] = p_minuit[2];
    p_minos[3] = p_minuit[3];

    if(usebin_loglike) {
  
      double loglike_min = 0;
      std::vector<double> loglike_vector;

      for(int i = 0 ; i < npoints ; i++) {
	
	p_minos[0] += 1;
	fitter->MinuitfcnLogLike(npar,gin,loglike_min,p_minos,iflag);
	loglike_vector.push_back(loglike_min);
   
	if((loglike_min < (min_fcn+error_def+0.1))&&(loglike_min > (min_fcn+error_def-0.1))) { std::cout<<"Loglike 1sigma: "<<loglike_min<<" p0 range: "<<p_minos[0]<<" error: "<<(p_minos[0]-p_minuit[0])<<std::endl; }

	grMinos->SetPoint(i, p_minos[0] , loglike_min);
      }
      auto minimum = *std::min_element(loglike_vector.begin(), loglike_vector.end());
      std::cout<<"The minimum value is: "<<minimum<<std::endl;
      grMinos->GetXaxis()->SetTitle("p_{0}");
      grMinos->GetYaxis()->SetTitle("-log(L)");
      grMinos->GetYaxis()->SetRangeUser(109,110);
      grMinos->GetXaxis()->SetRangeUser(8900,9070);
      grMinos->SetTitle("");
      grMinos->SetMarkerStyle(1);
      grMinos->Draw("");
    }

    if(usebin_chi2) {

      double chi2_min = 0;
      std::vector<double> chi2_vector;

      for(int i = 0 ; i < npoints ; i++) {

        p_minos[0] += 1;
        fitter->MinuitfcnBinnedChisq(npar,gin,chi2_min,p_minos,iflag);
        chi2_vector.push_back(chi2_min);

        if((chi2_min < (min_fcn+error_def+0.1))&&(chi2_min > (min_fcn+error_def-0.1))) { std::cout<<"Least-Squares 1sigma: "<<chi2_min<<" p0 range: "<<p_minos[0]<<" error: "<<(p_minos[0]-p_minuit[0])<<std::endl; }

        grMinos->SetPoint(i, p_minos[0] , chi2_min);
      }
      auto minimum = *std::min_element(chi2_vector.begin(), chi2_vector.end());
      std::cout<<"The minimum value is: "<<minimum<<std::endl;
      grMinos->GetXaxis()->SetTitle("p_{0}");
      grMinos->GetYaxis()->SetTitle("#chi^{2}");
      grMinos->GetYaxis()->SetRangeUser(215,235);
      grMinos->GetXaxis()->SetRangeUser(8900,9100);
      grMinos->SetTitle("");
      grMinos->SetMarkerStyle(1);
      grMinos->Draw("");

    }

  } //Check MINOS error

  if( drawplot==true ) {
    std::string nameplot_png = name_string+"_"+std::to_string(nexperiments)+"Runs_Job_"+std::to_string(jobNum)+"_Background_"+background_type+".png";
    char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());
    std::string nameplot_pdf = name_string+".pdf";
    char* char_nameplot_pdf = const_cast<char*>(nameplot_pdf.c_str());

    std::cout<<"Generated plot in: "<<nameplot_png<<std::endl;
    c->Print(char_nameplot_png);
    //c->Print(char_nameplot_pdf);
  }


  //Remove this before filling the TTree, otherwise, histograms are stored twice...
  if(drawplot==false){
    delete gROOT->FindObject("c");   
    delete gROOT->FindObject("histo");
    delete gROOT->FindObject("ffit");
    delete gROOT->FindObject(char_hSTM); 
    delete gROOT->FindObject(char_Covmatrix);
    delete gROOT->FindObject(char_BestFitPars);
    delete gROOT->FindObject(char_SeedFitPars);
  }


  
  //If count_experiment == initialise the true and reco signal tree, else just fill it
  //If nexperiments == count_experiment (Fill the root file)
  

  TTree* Signaltree;
 
  if(store_dataROOT==true) {
    if(( count_experiment == 1 )&&( count_experiment == nexperiments)){
      Signaltree = new TTree("Signaltree", "Signaltree");
      Signaltree->Branch("TrueSignal", &TrueSignal);
      Signaltree->Branch("RecoSignal", &RecoSignal);
      
      TrueSignal.push_back(pulseNumXrays[0]);
      RecoSignal.push_back(reco_peaks);
      
      Signaltree->Fill();
      output->Write();
      //delete gROOT->FindObject("Signaltree");
      }
    else if( count_experiment == 1 ) {
      TrueSignal.push_back(pulseNumXrays[0]);
      RecoSignal.push_back(reco_peaks);

    }
    else if( count_experiment < nexperiments ) {
      TrueSignal.push_back(pulseNumXrays[0]);
      RecoSignal.push_back(reco_peaks);
    }
    //count_experiment == nexperiments
    else {
      Signaltree = new TTree("Signaltree", "Signaltree");
      Signaltree->Branch("TrueSignal", &TrueSignal);
      Signaltree->Branch("RecoSignal", &RecoSignal);

      TrueSignal.push_back(pulseNumXrays[0]);
      RecoSignal.push_back(reco_peaks);

      Signaltree->Fill();
      output->Write();

      //delete gROOT->FindObject("Signaltree");
      }
    output->Close();
  }


  t2 = boost::chrono::high_resolution_clock::now();
  std::cout<< "Computing time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;


  delete dataBrems; delete dataset; delete dataSignal; delete p_minuit; delete perr_minuit;

  //***********END CODE***********// 


#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif


}





int main(int argc, char *argv[]) {

  double Bremsrate_kHz = std::stod(argv[1]);
  double TimeSim = std::stod(argv[2]);
  double meanE = std::stod(argv[3]);
  double sigma = std::stod(argv[4]);
  unsigned long int seed = std::stoul(argv[5]);
  int FitOption = std::atoi(argv[6]);
  //Run Config
  int nexperiments = std::atoi(argv[7]); //It will do more than 1 experiment with no pop up plot
  int jobNum = std::atoi(argv[8]); //this is a counter for root files
  std::string outpath = argv[9];
  std::string txtBackgroundParameters = argv[10];

  bool store_dataLOG = true;

  double pos1 = txtBackgroundParameters.find("Brems_");
  double pos2 = txtBackgroundParameters.find(".txt");
  double diff = pos2-pos1;
  std::string background_type = txtBackgroundParameters.substr(pos1, diff);
  
  
  if( store_dataLOG==true ) {
    std::string logfile = outpath+"/seed_"+std::to_string(seed)+"_"+std::to_string(nexperiments)+"Runs_Job_"+std::to_string(jobNum)+"_Timesim_"+std::to_string(int(TimeSim))+"_BremsRate_"+argv[1]+"kHz_resol_"+argv[4]+"MeV_"+background_type+"_FitOpt_"+std::to_string(FitOption)+".log";
    char* char_logfile = const_cast<char*>(logfile.c_str());

    gSystem->RedirectOutput(char_logfile, "w");
  }

  std::cout<<"INPUT CONFIG, Bremsrate(kHz): "<<Bremsrate_kHz<<", TimeSim(s): "<<TimeSim<<", meanE(MeV): "<<meanE<<", sigma(MeV): "<<sigma<<", seed: "<<seed<<", FitOption: "<<FitOption<<", nexperiments: "<<nexperiments<<std::endl;

  // Use seed = 0 if the function FitterROOT is going to be run in different jobs and not in the nexperiments loop (to get different outputs)
  // If the provided seed is zero...
  if(seed == 0) {
    // Initliase with random seed
    Random::Init(); // Number of pulses in background and signal
  }
  // Else set the seed that is provided
  else {
    Random::Init(seed);
  }

  std::vector<double> TrueSignal, RecoSignal;
  //TMatrixD* Covmatrix;

  //Run the code nexperiments times
  for(int i = 0; i < nexperiments; i++) {

    int count_experiment = i+1;
    
    std::cout<<" "<<std::endl;
    std::cout<<"--------ROOT seed to fill histogram: "<<seed<<std::endl;
    std::cout<<"--------Call Function for experiment #: "<<count_experiment<<std::endl;

    FitterROOT(argc, argv, Bremsrate_kHz, TimeSim, meanE, sigma, FitOption, seed, nexperiments, count_experiment, jobNum, outpath, TrueSignal, RecoSignal, txtBackgroundParameters);
 
   }


  return 0;
}
