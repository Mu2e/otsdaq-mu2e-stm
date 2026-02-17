#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"


void PlotDiffExp(int argc, char *argv[], std::string rootpath)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif
  
  gROOT->SetStyle("ATLAS");
  TCanvas* c;

  gStyle->SetOptStat(1111);
  
  c = new TCanvas("c","c",600,500);
  //Output File                                                                                        
  std::string outrootname;
  TFile* outfile;
  TTree* SBtree;
   
 
  TH1D* hDeltaSignal;
  int Nbins = 200;
  int Njobs = 20;
  int nexperiments = 50;
  
  double xx1delta[2] = {-7, 4};
  double yy1[2] = {0, 50};
  
  hDeltaSignal = new TH1D ("#DeltaSignal/#sigma", "", Nbins, xx1delta[0], xx1delta[1]);

  TGraph* graph = new TGraph (2,xx1delta,yy1);
  graph->SetMarkerStyle(1);
  
  //***********READ DATA***********// 
  
  MinuitFitter *fitter = new MinuitFitter();
  int NP = fitter->return_NP();
  int NPbrems = fitter->return_NPbrems();
  std::cout<<"NP: "<<NP<<" NPbrems: "<<NPbrems<<std::endl;
  
  std::string TimeSim_ = "2000";
  double TimeSim = 2000;
  std::string RateSim = "1";
  std::string ResolSim = "0.003";

  double TrueSignal_, RecoSignal_;
  
  std::cout<<"Time simulation: "<<TimeSim<<" Rate  simulation: "<<RateSim<<" Resolution simulation: "<<ResolSim<<std::endl;

  TH1D *hSignal, *hSTM;
  TMatrixD* BestFitPars, *Covmatrix;
  
  TPaveStats *stat[1];
       
  TFile *infile;
  double *best_fitpar = new double[NP];
 
  for(int i=0;i<Njobs;i++){
    
    std::string rootname = rootpath+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+".root";
    infile = new TFile(rootname.c_str());
    std::cout<<"ROOT FILE: "<<rootname<<std::endl;
    
     for( int j = 0 ; j <  nexperiments; j++) {
       std::cout<<"EXPERIMENT: "<<j+1<<std::endl;

       std::string str_hSignal = "hSignal;"+std::to_string(j+1);
       char* char_hSignal = const_cast<char*>(str_hSignal.c_str());
       hSignal = (TH1D*)infile->Get(char_hSignal);
       double true_peaks = hSignal->GetEntries();
       std::cout<<"Number of peaks simulated: "<<true_peaks<<std::endl;
  
       std::string str_hSTM = "hSTM"+std::to_string(j+1);
       char* char_hSTM = const_cast<char*>(str_hSTM.c_str());
       hSTM = (TH1D*)infile->Get(char_hSTM);

       std::cout<<"Read Cov Matrix"<<std::endl;
       //CovMatrix
       std::string str_covmatrix = "Covmatrix"+std::to_string(j+1);
       char* char_covmatrix = const_cast<char*>(str_covmatrix.c_str());
       Covmatrix = (TMatrixD*)infile->Get(char_covmatrix);
       TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");

       std::cout<<"Read Best fit pars"<<std::endl;
       //Best fit params
       std::string str_BestFitPars = "BestFitPars"+std::to_string(j+1);
       char* char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());
       BestFitPars = (TMatrixD*)infile->Get(char_BestFitPars);
       TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
       
       for( int k = 0 ; k < NP ; k++ ){
	 best_fitpar[k] = _BestFitPars[0][k];
	 std::cout<<"best_fitpar[k]: "<<best_fitpar[k]<<std::endl;
       }


       int Nentries = hSTM->GetEntries();
       int Nbins = hSTM->GetNbinsX();
       double bin_width = hSTM->GetBinWidth(0);
       
       std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;
           
       double reco_peaks;
       double reco_sigma;
              
       //Recovered number of peaks in signal
       reco_peaks = best_fitpar[0] / bin_width;
       std::cout<<"BestFitPar[0]: "<<best_fitpar[0]<<" bin width: "<<bin_width<<std::endl;
       std::cout<<"Peaks recovered from Binned fit: "<< reco_peaks <<std::endl;
       reco_sigma = sqrt(_Covmatrix[0][0]) / bin_width;
       std::cout<<"_Covmatrix[0][0]: "<<_Covmatrix[0][0]<<" bin_width: "<<bin_width<<" reco_sigma: "<<reco_sigma<<std::endl;
       

       std::cout<<"Sigma Signal: sqrt(cov matrix element)/bin width: "<<sqrt(_Covmatrix[0][0])<<"/"<<bin_width<<"="<<reco_sigma<<std::endl;
       std::cout<<"Reco signal: "<<reco_peaks<<std::endl;
       double accuracy = reco_sigma / reco_peaks;
       std::cout<<"Accuracy (sigma/signal): "<< accuracy <<std::endl;
       
       double delta_signal = (true_peaks - reco_peaks)/reco_sigma;
       std::cout<<"true peaks: "<<true_peaks<<std::endl;
       std::cout<<"reco peaks: "<<reco_peaks<<std::endl;
       std::cout<<"reco sigma: "<<reco_sigma<<std::endl;
       std::cout<<"Delta signal: "<<delta_signal<<std::endl;
       hDeltaSignal->Fill(delta_signal);
       
     }
     infile->Close();
  }


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

    graph->Draw("ap");
    graph->GetXaxis()->SetTitle("(S_{true}- S_{reco})/#sigma_{S_{reco}}");
    graph->GetYaxis()->SetTitle("#Experiments");
    //stat[0]->Draw("same");
    graph->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
    graph->GetHistogram()->GetXaxis()->SetLabelSize(0.04);

    hDeltaSignal->Draw("HIST,same");


    //Fit gaussian
    TF1 *fit = new TF1("fit","gaus",-4,4);
    fit->SetParameters(hDeltaSignal->GetMaximum(),hDeltaSignal->GetMean(),hDeltaSignal->GetRMS());
    hDeltaSignal->Fit("fit","");
    fit->SetLineColor(kRed);
    fit->SetLineStyle(2);
    fit->Draw("same");

    double mean = fit->GetParameter(1);
    double meanerror = fit->GetParError(1);
    double rms = fit->GetParameter(2);
    double rms_error = fit->GetParError(2);

    std::stringstream stream_mean, stream_meanerror, stream_rms, stream_rmserror;
    stream_mean << std::fixed << std::setprecision(3) << mean;
    stream_meanerror << std::fixed << std::setprecision(3) << meanerror;
    stream_rms << std::fixed << std::setprecision(3) << rms;
    stream_rmserror << std::fixed << std::setprecision(3) << rms_error;

    std::cout<<"mean: "<<stream_mean.str()<<" +- "<<stream_meanerror.str()<<std::endl;
    std::cout<<"rms: "<<stream_rms.str()<<" +- "<<stream_rmserror.str()<<std::endl;


    char* char_latex1;
    TLatex latex1;
    std::string str_latex1;
    str_latex1 = "#color[2]{#splitline{Mean = "+stream_mean.str()+" #pm "+stream_meanerror.str()+"}{Std Dev = "+stream_rms.str()+" #pm "+stream_rmserror.str()+"}}";
    char_latex1 = const_cast<char*>(str_latex1.c_str());
    latex1.DrawLatexNDC(.22,.85,char_latex1);


    std::string nameplot_png = "DeltaSignal_sigmareco_1kHz_0.001MeV_2000s_fit.png";

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

  PlotDiffExp(argc, argv, rootpath);

  return 0;
}
