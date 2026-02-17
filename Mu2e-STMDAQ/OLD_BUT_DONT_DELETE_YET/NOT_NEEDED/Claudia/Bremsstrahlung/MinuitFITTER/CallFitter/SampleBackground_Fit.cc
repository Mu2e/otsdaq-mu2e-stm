#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"

//1 - Read covariance matrix from best fit with t = 2000s.
//2 - Sample cov matrix 1,000 times just for background.
//3 - Generate background data with the sampled background values.
//4 - Add the true signal to the new background data generated in a histogram.
//5 - Fit the histogram with the same seeds values for the fit using fit range [0.04 , 2] MeV.

void SampleBackground_Fit(int argc, char *argv[], std::string rootname, int FitOption, double xfit_low, double xfit_max, double seed, int nsample, std::string outpath)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  std::cout<<"Executing SampleBackground_Fit.cc"<<std::endl;
  std::cout<<"Seed: "<<seed<<std::endl;

  bool store_dataLOG = true;
  bool store_dataROOT = true; //if this is true it doesn't plot the histogram in screen
  bool plot_recotrue_signal = true;
  bool plot_data = false;



  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  TCanvas* c;
  //TCanvas* c = new TCanvas("c");


  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;

  //***********READ DATA********************************//

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
  int ResolSimkeV = 1000*ResolSim;
  std::string resolution_str_keV = std::to_string(ResolSimkeV);
  
  //***************GET HISTOGRAM AND BEST FIT*****************//
  
  TH1D *histo = (TH1D*)infile->Get("histo");
  TH1D *hBrems = (TH1D*)infile->Get("hBrems");
  TH1D *hSignal = (TH1D*)infile->Get("hSignal");
  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix1");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars1");
  TMatrixD *SeedFitPars = (TMatrixD*)infile->Get("SeedFitPars1");
  
  int Nentries = histo->GetEntries();
  int Nbins = histo->GetNbinsX();
  double bin_width = histo->GetBinWidth(0);

  int Nentries_back = hBrems->GetEntries();
  int Nentries_signal = hSignal->GetEntries();
  
  std::cout<<"Entries(S+B): "<<Nentries<<" Entries(S): "<<Nentries_signal<<" Entries(B): "<<Nentries_back<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;

  //***********CREATE ROOT OUTPUT*********************//   

  std::stringstream stream_xfit_low, stream_xfit_max;
  stream_xfit_low << std::fixed << std::setprecision(3) << xfit_low;
  stream_xfit_max << std::fixed << std::setprecision(3) << xfit_max;
  //open ROOT file
  if( store_dataLOG==true ) {
    
    std::string logfile = outpath+"/SampleBackground_Fit_Fitrange_"+stream_xfit_low.str()+"_"+stream_xfit_max.str()+"_Timesim_"+std::to_string(int(TimeSim))+"s_"+rate_str+"kHz_"+resolution_str+"MeV.log";
    char* char_logfile = const_cast<char*>(logfile.c_str());

    gSystem->RedirectOutput(char_logfile, "w");
  }
 
  std::cout<<"Time simulation: "<<TimeSim<<" Rate  simulation: "<<RateSim<<" Resolution simulation: "<<ResolSim<<std::endl;
  
  TFile*output;
  std::string rootoutfile;
  if(store_dataROOT==true) {
    
    rootoutfile = outpath+"/SampledBackground_FitData_Timesim_"+std::to_string(int(TimeSim))+"s_"+rate_str+"kHz_"+resolution_str+"MeV.root";
    output = new TFile(rootoutfile.c_str(),"recreate");
  }



  std::cout<<"Check time simulation: "<<TimeSim<<" s"<<std::endl;

 //***********START CODE***************************//

  //Initialise Minuit Class
  MinuitFitter *fitter = new MinuitFitter(FitOption, seed);

  //Return number of fit parameters NP
  int NP = fitter->return_NP();
  std::cout<<"Number of fit parameters, NP: "<<NP<<std::endl;
  int NPbrems = fitter->return_NPbrems();
  std::cout<<"Number of fit parameters for background: "<<NPbrems<<std::endl;
  int NPsignal = fitter->return_NPsignal();
  std::cout<<"Number of fit parameters for signal: "<<NPsignal<<std::endl;

  
  //Get data size from input bremsstrahlung background, signal and both
  unsigned long int databack_size = hBrems->GetEntries();
  std::cout<<"Background data size: "<<databack_size<<std::endl;
  
  unsigned long int datasignal_size = hSignal->GetEntries();
  std::cout<<"Signal data size: "<<datasignal_size<<std::endl;

  unsigned long int data_size = histo->GetEntries();
  std::cout<<"Sig+Back data size: "<<data_size<<std::endl;
  
  //Calculate sqrt(cov matrix)
  TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");
  TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
  TMatrixD _SeedFitPars(1,NP,SeedFitPars->GetMatrixArray(),"");
  std::cout<<"Covariance matrix to sample...:"<<std::endl;
  _Covmatrix.Print();
  std::cout<<"Seeds for fit values to do the fits...:"<<std::endl;
  _SeedFitPars.Print();

  //Start the sampling
  TMatrixD sqrtCov( NP, NP );
  
  fitter->corset( _Covmatrix, sqrtCov );
  //std::cout<<"sqrt(Cov): "<<std::endl;
  //sqrtCov.Print();

  //Histograms with the data generated in every loop
  unsigned long int nbins = fitter->return_nbins();
  double frange[2];
  frange[0] = fitter->return_frange_min();
  frange[1] = fitter->return_frange_max();
  
  //Define the fit range 
  double fit_range[2] = {0.04,2};

  //Recover best fit parameters
  double best_fitpar[NP];
  double best_fitparbrems[NPbrems];
 
  for( int i = 0 ; i < NP ; i++ ){
    best_fitpar[i] = _BestFitPars[0][i];
  }

  //Best fit parameters for background
  for( int i = 0 ; i < NPbrems ; i++ ){
    best_fitparbrems[i] = best_fitpar[i+3];
  }

  //Recover the initial seed parameters for fit
  double seed_fitpar[NP];

  for( int i = 0 ; i < NP ; i++ ){
    seed_fitpar[i] = _SeedFitPars[0][i];
  }

  //Best fit brems background (we sample the background from this)
  std::string  func_bremsfor_sampling = "fbremssampling";
  char* char_func_bremsfor_sampling = const_cast<char*>(func_bremsfor_sampling.c_str());
  fitter->Init_Bremsfunc(char_func_bremsfor_sampling, fit_range);
   
  //Set Bremsstrahlung function parameters
  fitter->SetPar_Bremsfunc(best_fitparbrems);
  
  TF1* fbremssampling = fitter->return_fbrems();
  
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

  //Get data and Fill Signal histogram
  double* dataB = new double[databack_size];
  
  
  //Data from best fit
  double reco_peaks, reco_sigma, accuracy, true_peaks, diff_reco_true_bestfit;
  true_peaks = hSignal->GetEntries();
  std::cout<<"TRUE PEAKS IN SIGNAL: "<<true_peaks<<std::endl;
  //std::cout<<"BEST FIT PARAMETERS RESULT FOR RECO SIGNAL:"<<std::endl;
  reco_peaks = best_fitpar[0] / bin_width;
  std::cout<<"Peaks recovered from Binned fit: "<< reco_peaks <<std::endl;
  reco_sigma = sqrt(_Covmatrix[0][0]) / bin_width;
  std::cout<<"Sigma Signal: sqrt(cov matrix element/bin width): "<<sqrt(_Covmatrix[0][0])<<"/"<<bin_width<<"="<<reco_sigma<<std::endl;
  accuracy = reco_sigma / reco_peaks;
  std::cout<<"Accuracy (sigma/signal): "<< accuracy <<std::endl;
  diff_reco_true_bestfit = reco_peaks - true_peaks;
  std::cout<<"Reco - True: "<< diff_reco_true_bestfit <<std::endl;
  
  //********************PLOT****************************//

  double xreco_min = reco_peaks - 9*reco_sigma;
  double xreco_max = reco_peaks + 7*reco_sigma;

  double xdeltasignal_min = diff_reco_true_bestfit - 10*reco_sigma;
  double xdeltasignal_max = diff_reco_true_bestfit + 8*reco_sigma;
  
  //double xdeltasignal_min = diff_reco_true_bestfit - 20*diff_reco_true_bestfit;
  //double xdeltasignal_max = diff_reco_true_bestfit + 5*diff_reco_true_bestfit;
  
  TH1D* hReco = new TH1D ("Reco Signal", "", 200, xreco_min, xreco_max);
  TH1D* hDeltaSignal = new TH1D ("#DeltaSignal", "", 200, xdeltasignal_min, xdeltasignal_max);

  
  //********************INIT LOOP OF SAMPLING***********//
  
  TH1D* hSTM[nsample];
  
  TF1* fgenerated_brems_cov[nsample];

  std::string  fitfunc_name = "ffit";
  char* char_fitfunc_name = const_cast<char*>(fitfunc_name.c_str());
  TF1* ffit[nsample];
  
  //Sampled values for S+B
  double *valuesSB = new double[NP];
  //Sampled values just for background parameters
  double *values = new double[NPbrems];

  double* p_minuit;
  double* perr_minuit;
  TMatrixD* Covmatrix_newFit;
  TMatrixD* BestFitPars_newFit;
  
  fitter->InitBrems_Data(databack_size);

  for (int loop = 0; loop < nsample; loop++){

    std::cout<<""<<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<"ITERATION: "<<loop<<std::endl;
    t1 = boost::chrono::high_resolution_clock::now();

    fitter->corgen( sqrtCov, valuesSB , NP);
    
    //--sum the mean to the parameters for all Sampled values
    for (int i = 0; i < NP ; i++) {
      valuesSB[i]+=best_fitpar[i];
    }
    
    //--Get just the background sampled values
    int start_sampvalues = NPbrems - 1;
    for (int i = 0; i < NPbrems ; i++) {
      values[i] = valuesSB[i+3];
    }
    
    std::cout<<"Sampled values for brems: "<<values[0]<<" "<<values[1]<<" "<<values[2]<<" "<<values[3]<<std::endl;

    
    //Initialise Background function
    std::string  func_name = "fB"+std::to_string(loop);
    char* char_func_name = const_cast<char*>(func_name.c_str());
    fitter->Init_Bremsfunc(char_func_name, fit_range);

    //Set Bremsstrahlung function parameters 
    fitter->SetPar_Bremsfunc(values);

    fgenerated_brems_cov[loop] = fitter->return_fbrems();
    
    //Generate the data with the previous function
    fitter->GenBrems_Data(databack_size);

    dataB = fitter->return_dataBrems();

    
    std::string namehSTM = "hSTM"+std::to_string(loop);
    char* namehSTM_char = const_cast<char*>(namehSTM.c_str());
    hSTM[loop] = new TH1D(namehSTM_char, "", nbins,frange[0],frange[1]);
     
    for (unsigned long int i = 0 ; i < databack_size ; i++) { 
      hSTM[loop]->Fill(dataB[i]);
    }

    //Add the signal to the new generated background
    hSTM[loop]->Add(hSignal);

    //Check bin width
    double bin_width = hSTM[loop]->GetBinWidth(0);
    
    //Fit the new data generated
    //Get histogram bin content and errors in fitrange to form the data
    fitter->Histo_Data(fit_range,hSTM[loop]); //Get nbins_fit, _binning, bincontent, bincontent_error, bincenter

    //Fit with Minuit
    fitter->fit(par_fit, NP);
    
    p_minuit = new double[NP];
    perr_minuit = new double[NP];

    p_minuit = fitter->return_p_minuit();
    perr_minuit = fitter->return_perr_minuit();
    Covmatrix_newFit = fitter->return_Covmatrix();
    TMatrixD _Covmatrix_newFit(NP,NP,Covmatrix_newFit->GetMatrixArray(),"");
    TMatrixD _BestFitPars_newFit(NP,1);
    double par_minuit [NP];
    
    for(int i = 0 ; i < NP; i++) {
      std::cout<<p_minuit[i]<<" +- "<<perr_minuit[i]<<std::endl;
      par_minuit[i] = p_minuit[i];
      _BestFitPars_newFit[i][0] = par_minuit[i];
    }    

    BestFitPars_newFit = new TMatrixD(NP,1,_BestFitPars_newFit.GetMatrixArray(),"");

    std::cout<<"NEW FIT PARAMETERS RESULT FOR RECO SIGNAL:"<<std::endl;
    reco_peaks = par_minuit [0] / bin_width;
    std::cout<<"Peaks recovered from Binned fit: "<< reco_peaks <<std::endl;
    reco_sigma = sqrt(_Covmatrix_newFit[0][0]) / bin_width;
    std::cout<<"Sigma Signal: sqrt(cov matrix element/bin width): "<<sqrt(_Covmatrix_newFit[0][0])<<"/"<<bin_width<<"="<<reco_sigma<<std::endl;
    accuracy = reco_sigma / reco_peaks;
    std::cout<<"Accuracy (sigma/signal): "<< accuracy <<std::endl;
    diff_reco_true_bestfit = reco_peaks - true_peaks;
    std::cout<<"Reco - True: "<< diff_reco_true_bestfit <<std::endl;

    //Result function from the new fit
    fitter->Init_SBfunc(char_fitfunc_name, fit_range);
    fitter->SetPar_SBfunc(p_minuit);
    ffit[loop] = fitter->return_fSB();


    hReco->Fill( reco_peaks );
    hDeltaSignal->Fill( diff_reco_true_bestfit );
    
    
    if(store_dataROOT==true) {

      output->WriteObject(histo, "histo");
      
      std::string  f_name = "sampledfunc"+std::to_string(loop);
      char* char_f_name = const_cast<char*>(f_name.c_str());
      output->WriteObject(fgenerated_brems_cov[loop], char_f_name);
      
      std::string  h_name = "hSTMsampled"+std::to_string(loop);
      char* char_h_name = const_cast<char*>(h_name.c_str());
      output->WriteObject(hSTM[loop], char_h_name);

      std::string  fitf_name = "fitfunc"+std::to_string(loop);
      char* char_fitf_name = const_cast<char*>(fitf_name.c_str());
      output->WriteObject(ffit[loop], char_fitf_name);
      
      std::string  cov_name = "CovMatrix_fromSampling"+std::to_string(loop);
      char* char_cov_name = const_cast<char*>(cov_name.c_str());
      output->WriteObject(Covmatrix_newFit, char_cov_name);

      std::string  bestfitpar_name = "BestFitPar_fromSampling"+std::to_string(loop);
      char* char_bestfitpar_name = const_cast<char*>(bestfitpar_name.c_str());
      output->WriteObject(BestFitPars_newFit, char_bestfitpar_name);

    }

  
  }//loop

  if(store_dataROOT==true) {
    output->WriteObject(hSignal, "hSignalTrue");
  }
  
  //Cout the mean and the stdev of both of the histograms to get signal recovered
  double meanRecoSignal_fromexperiments = hReco->GetMean();
  double stdevRecoSignal_fromexperiments = hReco->GetRMS();

  double meanDiffSignal_fromexperiments = hDeltaSignal->GetMean();
  double stdevDiffSignal_fromexperiments = hDeltaSignal->GetRMS();

  std::cout<<"RECO SIGNAL HISTOGRAM: mean: "<<meanRecoSignal_fromexperiments<<" stdev: "<<stdevRecoSignal_fromexperiments<<std::endl;
  std::cout<<"DIFF RECO-TRUE HISTOGRAM: mean: "<<meanDiffSignal_fromexperiments<<" stdev: "<<stdevDiffSignal_fromexperiments<<std::endl;
  
  t2 = boost::chrono::high_resolution_clock::now();
  std::cout<< "Computing time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;
  
  TPad* pad[2];
  TPaveStats *stat[2];
 
  if( plot_data ==true ) {

    c = new TCanvas("c","c",0,0,1200,500);
    pad[0] = new TPad("","",0,0,0.5,1);
    pad[1] = new TPad("","",0.5,0,1,1);

    pad[0]->Draw();
    pad[1]->Draw();

    double ymax;
    double xlow =0.05;
    double xmax =0.5;
    histo->GetXaxis()->SetRangeUser(xlow,xmax);
    ymax = histo->GetMaximum()+50;
    std::cout<<"ymax: "<<ymax<<std::endl;
    double plot_y_range[2]={0,ymax};
    double plot_x_range[2]={xlow,xmax};
    histo->GetXaxis()->SetRangeUser(0.04,2);
    
    TGraph *graph1 = new TGraph (2, plot_x_range, plot_y_range);
    graph1->GetXaxis()->SetRangeUser(plot_x_range[0], plot_x_range[1]);
    graph1->GetYaxis()->SetRangeUser(plot_y_range[0], plot_y_range[1]);
    graph1->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
    graph1->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
    graph1->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    string yaxis_str;
    char*  yaxis_char;
    yaxis_str = "Counts / run time "+std::to_string(int(TimeSim))+" s";
    yaxis_char = const_cast<char*>(yaxis_str.c_str());
    graph1->GetYaxis()->SetTitle(yaxis_char);
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);



    pad[0]->cd();
    graph1->Draw("ap");

    histo->SetFillColor(kOrange-3);
    histo->SetLineColor(kOrange-3);
    histo->SetFillStyle(3001);
    histo->Draw("same");

    for(int i = 0 ; i <nsample ; i++){

      fgenerated_brems_cov[i]->SetLineColor(kCyan-3);
      fgenerated_brems_cov[i]->SetLineWidth(4);
      fgenerated_brems_cov[i]->Draw("same");
    }

   
    fbremssampling->SetLineColor(kRed);
    fbremssampling->SetLineWidth(1);
    fbremssampling->Draw("same");


    string Bremsrate_kHz_str = "#splitline{347 keV peak (resolution="+resolution_str_keV+" keV)+}{\n"+rate_str+" kHz Bremsstrahlung}";
    char*  Bremsrate_kHz_char = const_cast<char*>(Bremsrate_kHz_str.c_str());

    auto legend1 = new TLegend(0.27,0.55,0.63,0.9);
    legend1->AddEntry(histo,Bremsrate_kHz_char,"f");
    legend1->AddEntry(fgenerated_brems_cov[0],"#splitline{\nEstimated Bremsstrahlung}{Background}","l");
    legend1->AddEntry(fbremssampling,"Background from Best Fit","l");
    legend1->Draw("same");
    
    gPad->RedrawAxis();


    pad[1]->cd();

    graph1->Draw("ap");
 
    for(int i = 0 ; i <nsample ; i++){

      hSTM[i]->SetLineColor(kGreen-3);
      hSTM[i]->Draw("same");

      ffit[i]->SetLineColor(kBlack);
      ffit[i]->SetLineStyle(2);
      ffit[i]->Draw("same");
      
    }

    auto legend2 = new TLegend(0.27,0.55,0.63,0.9);
    legend2->AddEntry(hSTM[0],"Sampled data","l");
    legend2->AddEntry(ffit[0],"Fit Function","l");
    legend2->Draw("same");

    gPad->RedrawAxis();
    
  }



  
  
  if( plot_recotrue_signal ==true ) {

    double ymax;
    
    c = new TCanvas("c","c",0,0,1200,500);
    pad[0] = new TPad("","",0,0,0.5,1);
    pad[1] = new TPad("","",0.5,0,1,1);

    pad[0]->Draw();
    pad[1]->Draw();

    pad[0]->cd();
    pad[0]->SetLeftMargin(0.15);
    pad[0]->SetRightMargin(0.1);

    TGraph * graph[2];

    double plot_x_range1[2]={xreco_min, xreco_max};
    ymax = hReco->GetMaximum() + 10;
    std::cout<<"ymax: "<<ymax<<std::endl;
    double plot_y_range1[2]={0,ymax};
    
    graph[0] = new TGraph (2, plot_x_range1, plot_y_range1);
    graph[0]->GetXaxis()->SetRangeUser(plot_x_range1[0], plot_x_range1[1]);
    graph[0]->GetYaxis()->SetRangeUser(plot_y_range1[0], plot_y_range1[1]);
    graph[0]->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
    graph[0]->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    graph[0]->SetTitle("");
    graph[0]->SetMarkerStyle(1);


    hReco->SetFillColor(kGreen-3);
    hReco->SetLineColor(kBlack);
    hReco->SetFillStyle(3001);
    hReco->Draw("HIST");

    gPad->Update();
    stat[0] = (TPaveStats*)hReco->FindObject("stats");
    stat[0]->SetY1NDC(.74);
    stat[0]->SetY2NDC(.91);
    stat[0]->SetX1NDC(0.2);
    stat[0]->SetX2NDC(0.52);
    stat[0]->SetTextSize(0.043);
    stat[0]->SetTextColor(kGreen-3);

    

    gPad->Update();
 

    graph[0]->Draw("ap");
    TAxis *X0 = graph[0]->GetXaxis();
    X0->SetNdivisions(10,10,1);
    graph[0]->GetXaxis()->SetTitle("Counts in Signal");
    graph[0]->GetYaxis()->SetTitle("Experiments");
    hReco->Draw("HIST,same");
    stat[0]->Draw("same");


    /////////////
    pad[1]->cd();
    pad[1]->SetLeftMargin(0.15);
    pad[1]->SetRightMargin(0.1);

    double plot_x_range2[2]={xdeltasignal_min, xdeltasignal_max};
    ymax = hDeltaSignal->GetMaximum() + 10;
    std::cout<<"ymax: "<<ymax<<std::endl;
    double plot_y_range2[2]={0,ymax};
    
    graph[1] = new TGraph (2, plot_x_range2, plot_y_range2);
    graph[1]->GetXaxis()->SetRangeUser(plot_x_range2[0], plot_x_range2[1]);
    graph[1]->GetYaxis()->SetRangeUser(plot_y_range2[0], plot_y_range2[1]);
    graph[1]->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
    graph[1]->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    graph[1]->SetTitle("");
    graph[1]->SetMarkerStyle(1);

    hDeltaSignal->SetFillColor(kBlue);
    hDeltaSignal->SetLineColor(kBlack);
    hDeltaSignal->SetFillStyle(3001);
    hDeltaSignal->Draw("HIST");

    gPad->Update();
    stat[1] = (TPaveStats*)hDeltaSignal->FindObject("stats");
    stat[1]->SetY1NDC(.74);
    stat[1]->SetY2NDC(.91);
    stat[1]->SetX1NDC(0.2);
    stat[1]->SetX2NDC(0.52);
    stat[1]->SetTextSize(0.043);
    stat[1]->SetTextColor(kBlue);
    
    graph[1]->Draw("ap");
    TAxis *X = graph[1]->GetXaxis();
    X->SetNdivisions(10,10,1);
    graph[1]->GetXaxis()->SetTitle("#DeltaSignal: Reco-True");
    graph[1]->GetYaxis()->SetTitle("Experiments");
    hDeltaSignal->Draw("HIST,same");
    stat[1]->Draw("same");
  }

 
  
  std::string nameplot_png = "SampleBackground_seed_"+std::to_string(int(seed))+"_"+rate_str+"kHz_"+resolution_str+"MeV_SigmaRecoSignal.png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());
  std::string nameplot_pdf = "SampleBackground_seed_"+std::to_string(int(seed))+"_"+rate_str+"kHz_"+resolution_str+"MeV_SigmaRecoSignal.pdf";
  char* char_nameplot_pdf = const_cast<char*>(nameplot_pdf.c_str());

  c->Print(char_nameplot_png);
  c->Print(char_nameplot_pdf);

  if(store_dataROOT==true) { output->Close(); }

   
  //***********END CODE***********//

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
  int nsample = std::atoi(argv[6]);
  std::string outpath = argv[7];
  
  SampleBackground_Fit(argc, argv, rootname, FitOption, xfit_low, xfit_max, seed, nsample, outpath);


  return 0;
}
