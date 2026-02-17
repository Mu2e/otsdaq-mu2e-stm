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
#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooAddPdf.h"
#include "RooDataHist.h"
#include "RooGenericPdf.h"
#include "RooCmdArg.h"
#include "RooFit.h"
#include "RooAbsReal.h"
#include "RooAbsRealLValue.h"
#include "RooPlot.h"
#include "RooDataSet.h"
#include "RooGlobalFunc.h"

using namespace RooFit;

int main (int argc, char *argv[]) {

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif
  
  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  TCanvas* c ;
  TPad *pad[2];
  TGraph *graph[2];
  TPaveStats *stat[2];
  TH1D* hTrue=0;
  TH1D* hRecoRooFit=0;
  TH1D* hDeltaSignal=0;

  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;

  //***********START CODE***********//
  t1 = boost::chrono::high_resolution_clock::now();  

  //Read all files mu2e-stm-01 / mu2edaq1
  //std::string path = "/data1/cgarcia/MinuitFit_1kHz_100000s_347keV_1keVres/1000JOBS/";
  std::string path = "/data1/cgarcia/SignaltoBackground/MinuitFit_1kHz_100000s_347keV_1keVres/1000JOBS/"; 
  //20 root files with 50 fits each
  int njobs = 20;
  int nexperiments = 50;

  if( nexperiments != 1 ) {

    c = new TCanvas("c","c",0,0,1200,500);
    pad[0] = new TPad("","",0,0,0.5,1);
    pad[1] = new TPad("","",0.5,0,1,1);

    pad[0]->Draw();
    pad[1]->Draw();
  }
  else { c = new TCanvas(); } 

  TFile *infile;
  TTree *treeread;
  TH1D *hSTM;
  TMatrixD *BestFitPars;
  std::vector<double> *TrueSignal=0, *RecoSignal=0;
  double fit_range[2] = { 0.04, 2 };
  //double fit_range[2] = { 0.3, 0.4 };  
  std::cout<<"Fit range: "<<fit_range[0]<<" "<<fit_range[1]<<std::endl;

  //plot ranges
  double hxranges[4] = { 582000, 592000, -5000, 5000 };
  double hyranges[2] = { 50 , 90 };

  double xx1_sig[2] = {hxranges[0], hxranges[1]};
  double yy1_sig[2] = {0, hyranges[0]};
  graph[0] = new TGraph (2,xx1_sig,yy1_sig);
  graph[0]->SetMarkerStyle(1);
  hTrue = new TH1D ("Shistogram True", "", 200,  xx1_sig[0], xx1_sig[1]);
  hRecoRooFit = new TH1D ("Shistogram Reco", "", 200, xx1_sig[0], xx1_sig[1]);

  double xx1_diff[2] = {hxranges[2], hxranges[3]};
  double yy1_diff[2] = {0, hyranges[1]};
  graph[1] = new TGraph (2,xx1_diff,yy1_diff);
  graph[1]->SetMarkerStyle(1);
  hDeltaSignal = new TH1D ("#DeltaSignal", "", 100, xx1_diff[0], xx1_diff[1]);

  std::vector<double> TrueSignalsim, RecoSignalRooFit;

  TFile*output = new TFile("Out_RooFit.root","recreate");

  TTree* Signaltree = new TTree("Signaltree", "Signaltree");
  Signaltree->Branch("TrueSignalsim", &TrueSignalsim);
  Signaltree->Branch("RecoSignalRooFit", &RecoSignalRooFit);

  for(int i = 0; i < njobs; i++) {

    std::string file_name  = path+"BinnedLoglike_NOIntegral_1kHz_TimeSim_100000s_seed_0_50Runs_Job_"+std::to_string(i)+".root";
    infile = new TFile(file_name.c_str());
    std::cout<<"File: "<<file_name<<std::endl;

    //Get true and reco signal from minos from TTree
    treeread=(TTree*)infile->Get("Signaltree");
    treeread->SetBranchAddress("TrueSignal",&TrueSignal);
    treeread->SetBranchAddress("RecoSignal",&RecoSignal);

    treeread->GetEntry(0);
    //int nexperiments = TrueSignal->size();
    std::cout<<"1 vector: "<<treeread->GetEntries()<<" of entries: "<<TrueSignal->size()<<std::endl;

    for(int j = 0 ; j < nexperiments ; j++){

      //Get S+B histogram
      std::string str_hSTM = "hSTM"+std::to_string(j+1);
      char* char_hSTM = const_cast<char*>(str_hSTM.c_str());
      hSTM = (TH1D*)infile->Get(char_hSTM);
      double bin_width = hSTM->GetBinWidth(0);
      double datasize = hSTM->GetEntries();
      
      std::cout<<"Full data size (signal + brems): "<<datasize<<std::endl;

      //sum bin content in range to get data size in fit range
      double lowbin = hSTM->FindFixBin(fit_range[0]); 
      double upbin = hSTM->FindFixBin(fit_range[1]);
      datasize = 0 ;
      for ( int i = lowbin ; i < upbin ; i++) {
	datasize = datasize + hSTM->GetBinContent(i);
      } 

      std::cout<<"Data size in fit range (signal + brems): "<<datasize<<std::endl;
      
      //Nbins for plotting
      int Nbins = (fit_range[1] - fit_range[0])/bin_width;

      //Best Fit Parameters
      std::string str_BestFitPars = "BestFitPars"+std::to_string(j+1);
      char* char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());
      BestFitPars = (TMatrixD*)infile->Get(char_BestFitPars);
      TMatrixD _BestFitPars(1,7,BestFitPars->GetMatrixArray(),"");
      std::cout<<"Opened: "<<str_hSTM<<" "<<str_BestFitPars<<std::endl;

      double *best_fitpar = new double[7];

      for( int k = 0 ; k < 7 ; k++ ){
	best_fitpar[k] = _BestFitPars[0][k];
	std::cout<<k<<" "<<_BestFitPars[0][k]<<std::endl;
      }


      //Binned data is the read histogram in range
      RooRealVar x("x","E_{#gamma}(STM) [MeV]", fit_range[0], fit_range[1]);
      RooDataHist bindata("bindata","bindata",RooArgList(x),hSTM);
      bindata.Print("v");

      //Initialise fit parameters to the minuit best fit parameters and give it a range
      //Signal
      double fs_var = (best_fitpar[0]/bin_width) / datasize; //here best_fitpar[0] is the amplitude for a given binning, we need to provide the normalised parameter so we need to divide by the data size
      RooRealVar mu("mu","mu",best_fitpar[1],0.2,0.5);
      RooRealVar sigma("sigma","sigma",best_fitpar[2],0.0005,0.0015);
      RooGaussian gaus("gaus","gaus",x,mu,sigma);
      
      //std::cout<<"fs: "<<fs_var<<" signal: "<<best_fitpar[0]/hSTM->GetBinWidth(0)<<" entries "<<hSTM->GetEntries()<<std::endl;
      //Background
      RooRealVar a("a","a", best_fitpar[3], 8000, 9000);
      RooRealVar b("b","b", best_fitpar[4],  2.1, 2.5);
      RooRealVar c("c","c", best_fitpar[5],  -1, -0.9);
      RooRealVar d("d","d", best_fitpar[6],  600, 800);

      RooGenericPdf back("back","Generic PDF","(a/(exp(b*abs(x))+c))+d", RooArgSet(x,a,b,c,d));
      
      //Combine signal+background
      RooRealVar frac("frac","frac",fs_var,0.,1.);
      RooAddPdf model("model","model",RooArgList(gaus,back),RooArgList(frac));

      //Fit
      model.fitTo(bindata,Minos(false));
      //Get best fit parameters from RooFit
      double frac_fit = frac.getValV();
      double mu_fit = mu.getValV();
      double sigma_fit = sigma.getValV();
      double a_fit = a.getValV();
      //a->getAsymErrorHi();
      //a->getAsymErrorLo();
      double b_fit = b.getValV();
      double c_fit = c.getValV();
      double d_fit = d.getValV();

      //Function is normalised to 1 peak so need to multiply frac_fit by data size
      double reco_peaks = frac_fit * datasize;

      std::cout<<"--- RooFit best fit parameters..."<<std::endl;
      std::cout<<"fs: "<<frac_fit<<std::endl;
      std::cout<<"mu: "<<mu_fit<<std::endl;
      std::cout<<"sigma: "<<sigma_fit<<std::endl;
      std::cout<<"a: "<<a_fit<<std::endl;
      std::cout<<"b: "<<b_fit<<std::endl;
      std::cout<<"c: "<<c_fit<<std::endl;
      std::cout<<"d: "<<d_fit<<std::endl;
      std::cout<<"--True simulated signal: "<<TrueSignal->at(j)<<std::endl;
      std::cout<<"--Roofit reco signal: "<<reco_peaks<<std::endl;
      std::cout<<"--Minos reco signal: "<<RecoSignal->at(j)<<std::endl;

      hTrue->Fill(TrueSignal->at(j));
      hRecoRooFit->Fill(reco_peaks);
      double delta_signal = reco_peaks-TrueSignal->at(j);
      hDeltaSignal->Fill(delta_signal);
 
      TrueSignalsim.push_back(TrueSignal->at(j));
      RecoSignalRooFit.push_back(reco_peaks);
      Signaltree->Fill();
      output->Write();

      if( (njobs==1)&&(nexperiments==1) ) {
	//Plot data and model after fitting
	RooPlot* frame_x = x.frame();
	bindata.plotOn(frame_x, Bins(Nbins));   
	//bindata.plotOn(frame_x, DrawOption("F"), FillColor(kOrange));
	model.plotOn(frame_x, LineColor(kBlue));
	frame_x->Draw("");    
      }
      
    } //Nexperiments

  } //Njobs

    output->Close(); 

  //Plot the signal
   if( nexperiments!= 1 ){

    pad[0]->cd();
    pad[0]->SetLeftMargin(0.15);
    pad[0]->SetRightMargin(0.1);

    hRecoRooFit->SetFillColor(kGreen-3);
    hRecoRooFit->SetLineColor(kBlack);
    hRecoRooFit->SetFillStyle(3001);
    hRecoRooFit->Draw("HIST");

    gPad->Update();
    stat[0] = (TPaveStats*)hRecoRooFit->FindObject("stats");
    stat[0]->SetY1NDC(.74);
    stat[0]->SetY2NDC(.91);
    stat[0]->SetX1NDC(0.2);
    stat[0]->SetX2NDC(0.52);
    stat[0]->SetTextSize(0.043);
    stat[0]->SetTextColor(kGreen-3);

    hTrue->SetFillColor(kViolet);
    hTrue->SetLineColor(kBlack);
    hTrue->SetFillStyle(3001);
    hTrue->Draw("HIST,sames");

    gPad->Update();
    stat[1] = (TPaveStats*)hTrue->FindObject("stats");
    stat[1]->SetY1NDC(.56);
    stat[1]->SetY2NDC(.73);
    stat[1]->SetX1NDC(0.2);
    stat[1]->SetX2NDC(0.52);
    stat[1]->SetTextSize(0.043);
    stat[1]->SetTextColor(kViolet);

    graph[0]->Draw("ap");
    graph[0]->GetXaxis()->SetTitle("Counts in Signal");
    graph[0]->GetYaxis()->SetTitle("Experiments");
    hRecoRooFit->Draw("HIST,same");
    stat[0]->Draw("same");
    hTrue->Draw("HIST,same");
    stat[1]->Draw("same");
    /////////////

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
    hDeltaSignal->Draw("HIST,same");

  }

  c->Print("SignalTrueReco_Diff_RooFit_new.png");
  c->Print("SignalTrueReco_Diff_RooFit_new.pdf");
  
  t2 = boost::chrono::high_resolution_clock::now();
  std::cout<< "Computing time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;


  //***********END CODE***********// 

#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
  
  return 0;
}
