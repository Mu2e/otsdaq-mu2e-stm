#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"


void PlotSampledBackground(int argc, char *argv[], std::string rootname)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif
  
  gROOT->SetStyle("ATLAS");
  TCanvas* c;

  gStyle->SetOptStat(1111);
  
  c = new TCanvas("c","c",0,0,600,500);

  double yy1[2] = {0, 40};

  //Output File                                                                                                        
  std::string outrootname;
  TFile* outfile;
  TTree* SBtree;
   
 
  TH1D* hDeltaSignal;
  int Nbins = 200;

  double xx1delta[2] = {-7, 4};
  
  hDeltaSignal = new TH1D ("#DeltaSignal/#sigma", "", Nbins, xx1delta[0], xx1delta[1]);

  TGraph* graph = new TGraph (2,xx1delta,yy1);
  graph->SetMarkerStyle(1);
  
  //***********READ DATA***********// 
  std::cout<<rootname<<std::endl;
  
  MinuitFitter *fitter = new MinuitFitter();
  int NP = fitter->return_NP();
  int NPbrems = fitter->return_NPbrems();
  int nexperiments = 1000;
  
  std::string TimeSim_ = "2000";
  double TimeSim = 2000;
  std::string RateSim = "1";
  std::string ResolSim = "0.003";

  double TrueSignal_, RecoSignal_;
  
  std::cout<<"Time simulation: "<<TimeSim<<" Rate  simulation: "<<RateSim<<" Resolution simulation: "<<ResolSim<<std::endl;

  TH1D *hSignal, *hSTM;
  TMatrixD* BestFitPars, *Covmatrix;
  
  TPaveStats *stat;
  
  //std::string rootname = rootpath+""+std::to_string(i)+".root";
     
  TFile *infile = new TFile(rootname.c_str());
  double *best_fitpar = new double[NP];
  
  std::string str_hSignal = "hSignalTrue";
  char* char_hSignal = const_cast<char*>(str_hSignal.c_str());
  hSignal = (TH1D*)infile->Get(char_hSignal);
  double true_peaks = hSignal->GetEntries();
  std::cout<<"Number of peaks simulated: "<<true_peaks<<std::endl;
  
  std::cout<<"ROOT FILE: "<<rootname<<std::endl;
  
     for( int j = 0 ; j <  nexperiments; j++) {
       std::cout<<"EXPERIMENT: "<<j<<std::endl;

       std::string str_hSTM = "hSTMsampled"+std::to_string(j);
       char* char_hSTM = const_cast<char*>(str_hSTM.c_str());
       hSTM = (TH1D*)infile->Get(char_hSTM);
         
       //CovMatrix
       std::string str_covmatrix = "CovMatrix_fromSampling"+std::to_string(j);
       char* char_covmatrix = const_cast<char*>(str_covmatrix.c_str());
       Covmatrix = (TMatrixD*)infile->Get(char_covmatrix);
       TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");

       //Best fit params
       std::string str_BestFitPars = "BestFitPar_fromSampling"+std::to_string(j);
       char* char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());
       BestFitPars = (TMatrixD*)infile->Get(char_BestFitPars);
       TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");

       for( int k = 0 ; k < NP ; k++ ){
	 best_fitpar[k] = _BestFitPars[0][k];
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
       hDeltaSignal->Fill(delta_signal);

     }

    hDeltaSignal->SetFillColor(kBlue);
    hDeltaSignal->SetLineColor(kBlack);
    hDeltaSignal->SetFillStyle(3001);
    hDeltaSignal->Draw("HIST");

    gPad->Update();
    stat = (TPaveStats*)hDeltaSignal->FindObject("stats");
    stat->SetY1NDC(.74);
    stat->SetY2NDC(.91);
    stat->SetX1NDC(0.2);
    stat->SetX2NDC(0.52);
    stat->SetTextSize(0.043);
    stat->SetTextColor(kBlue);

    graph->Draw("ap");
    graph->GetXaxis()->SetTitle("(S_{true}- S_{reco})/#sigma_{S_{reco}}");
    graph->GetYaxis()->SetTitle("#Experiments");
    //stat->Draw("same");
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
    
    
    infile->Close();

  
    std::string nameplot_png = "SampleBackground_seed_0_1kHz_0.0030MeV_SigmaRecoSignal_sigma.png";
    

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

  PlotSampledBackground(argc, argv, rootname);

  return 0;
}
