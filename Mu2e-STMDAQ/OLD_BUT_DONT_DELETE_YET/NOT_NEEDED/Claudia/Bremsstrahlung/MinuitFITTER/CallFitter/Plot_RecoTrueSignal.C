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

void Plot_RecoTrueSignal( std::string rootname , int FitOption){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(111111);
  
  //Plot signal reco and true and background in different ranges
  bool plot_4 = false;
  //Plot signal reco and true and their difference
  bool plot_2signal = true;
  //Plot signal reco to see st dev in 10 histos
  bool plot_recosignal_10histos = false;
  //Plot signal reco to see st dev in 20 histos 
  bool plot_recosignal_20histos = false;
  //Plot st dev of the reco signal splitted in histograms
  bool plot_stdev = false; // If this is true, one of these has to be true also: plot_recosignal_10histos, plot_recosignal_20histos
  
  TCanvas *c ;

  TPad *pad[20];

  if( plot_4==true ) {

    c = new TCanvas("c","c",0,0,1200,900);
    pad[0] = new TPad("","",0,0.55,0.45,1);
    pad[1] = new TPad("","",0.55,0.55,1,1);
    pad[2] = new TPad("","",0,0,0.45,0.45);
    pad[3] = new TPad("","",0.55,0,1,0.45);

    pad[0]->Draw();
    pad[1]->Draw();
    pad[2]->Draw();
    pad[3]->Draw();
  }

  if( plot_2signal==true ) {

    c = new TCanvas("c","c",0,0,1200,500);
    pad[0] = new TPad("","",0,0,0.5,1);
    pad[1] = new TPad("","",0.5,0,1,1);

    pad[0]->Draw();
    pad[1]->Draw();
  }

  if( plot_recosignal_10histos==true ) {

    c = new TCanvas("c","c",0,0,1200,500);
    pad[0] = new TPad("","",0,0.5,0.2,1);
    pad[1] = new TPad("","",0.2,0.5,0.4,1);
    pad[2] = new TPad("","",0.4,0.5,0.6,1);
    pad[3] = new TPad("","",0.6,0.5,0.8,1);
    pad[4] = new TPad("","",0.8,0.5,1,1);
    pad[5] = new TPad("","",0,0,0.2,0.5);
    pad[6] = new TPad("","",0.2,0,0.4,0.5);
    pad[7] = new TPad("","",0.4,0,0.6,0.5);
    pad[8] = new TPad("","",0.6,0,0.8,0.5);
    pad[9] = new TPad("","",0.8,0,1,0.5);
 

    pad[0]->Draw();
    pad[1]->Draw();
    pad[2]->Draw();
    pad[3]->Draw();
    pad[4]->Draw();
    pad[5]->Draw();
    pad[6]->Draw();
    pad[7]->Draw();
    pad[8]->Draw();
    pad[9]->Draw();
  }

  if( plot_recosignal_20histos==true ) {

    c = new TCanvas("c","c",0,0,1200,900);
    pad[0] = new TPad("","",0,0.75,0.2,1);
    pad[1] = new TPad("","",0.2,0.75,0.4,1);
    pad[2] = new TPad("","",0.4,0.75,0.6,1);
    pad[3] = new TPad("","",0.6,0.75,0.8,1);
    pad[4] = new TPad("","",0.8,0.75,1,1);
    pad[5] = new TPad("","",0,0.5,0.2,0.75);
    pad[6] = new TPad("","",0.2,0.5,0.4,0.75);
    pad[7] = new TPad("","",0.4,0.5,0.6,0.75);
    pad[8] = new TPad("","",0.6,0.5,0.8,0.75);
    pad[9] = new TPad("","",0.8,0.5,1,0.75);
    pad[10] = new TPad("","",0,0.25,0.2,0.5);
    pad[11] = new TPad("","",0.2,0.25,0.4,0.5);
    pad[12] = new TPad("","",0.4,0.25,0.6,0.5);
    pad[13] = new TPad("","",0.6,0.25,0.8,0.5);
    pad[14] = new TPad("","",0.8,0.25,1,0.5);
    pad[15] = new TPad("","",0,0,0.2,0.25);
    pad[16] = new TPad("","",0.2,0,0.4,0.25);
    pad[17] = new TPad("","",0.4,0,0.6,0.25);
    pad[18] = new TPad("","",0.6,0,0.8,0.25);
    pad[19] = new TPad("","",0.8,0,1,0.25);

    pad[0]->Draw();
    pad[1]->Draw();
    pad[2]->Draw();
    pad[3]->Draw();
    pad[4]->Draw();
    pad[5]->Draw();
    pad[6]->Draw();
    pad[7]->Draw();
    pad[8]->Draw();
    pad[9]->Draw();
    pad[10]->Draw();
    pad[11]->Draw();
    pad[12]->Draw();
    pad[13]->Draw();
    pad[14]->Draw();
    pad[15]->Draw();
    pad[16]->Draw();
    pad[17]->Draw();
    pad[18]->Draw();
    pad[19]->Draw();
    
  }


  int backsize = 3;
  //double back_rangesigma[3] = { 2.5, 3.5, 4.5 };
  double back_rangesigma[3] = { 1.5, 1.75, 2 };
  double TrueSignal_, RecoSignal_;
  double TrueBack_[backsize], RecoBack_[backsize];
  std::stringstream sigmarange[backsize];

  //Get The histogram from the root file
  std::cout<<"Input file: "<<rootname<<std::endl;
  TFile *infile = new TFile(rootname.c_str());

  //Read Tree
  TTree *treeread=(TTree*)infile->Get("SBtree");
  treeread->SetBranchAddress("TrueSignal_",&TrueSignal_);
  treeread->SetBranchAddress("RecoSignal_",&RecoSignal_);
  /*
  for( int k = 0; k < backsize; k++ ) {

    sigmarange[k] << std::fixed << std::setprecision(2) << back_rangesigma[k];
    std::string branch_nameTrue = "TrueBack_"+sigmarange[k].str()+"sigma";
    std::string branch_nameReco = "RecoBack_"+sigmarange[k].str()+"sigma";

    char* char_branch_nameTrue = const_cast<char*>(branch_nameTrue.c_str());
    char* char_branch_nameReco = const_cast<char*>(branch_nameReco.c_str());
    treeread->SetBranchAddress(char_branch_nameTrue, &TrueBack_[k]);
    treeread->SetBranchAddress(char_branch_nameReco, &RecoBack_[k]);

  }
  */

  int Nbins = 200;
  TGraph *graph[20];
  
  //double hxranges[10] = { 22000, 25000, 120000, 233000, 155000, 275000, 190000, 310000 , -1500, 600};
  //double hyranges[5] = { 50, 1050, 1050, 1050 , 50};

  //double hxranges[10] = { 20000, 26000, 120000, 233000, 155000, 275000, 190000, 310000 , -400, 1000};
  //double hyranges[5] = { 90, 1050, 1050, 1050 , 50};
  
  //double hxranges[10] = { 18000, 27000, 120000, 233000, 155000, 275000, 190000, 310000 , -5500, 4000}; 
  //double hyranges[5] = { 130, 1050, 1050, 1050 , 30};

  double hxranges[10] = {-200, 200, 120000, 233000, 155000, 275000, 190000, 310000 , -200, 50};
  double hyranges[5] = { 80, 1050, 1050, 1050 , 80};
  
  //Background and signal true and reco
  TH1D* hTrue[1];
  TH1D* hReco[1];
  //Delta signal true and reco
  TH1D* hDeltaSignal = 0;
  int count = 0;
  //Reco signal 
  TH1D* hRecoSignal[20];
  //Plot the sigmas of the reco signal
  double sigmax[2] = {800, 1200};
  double sigmay[2] = {0, 6};

  TH1D* hSigmas_RecoSignal = new TH1D ("Sigma histogram", "", 20, sigmax[0], sigmax[1]);

  for( int i = 0; i < 5; i++ ) {
    double xx1[2] = {hxranges[count], hxranges[count+1]};
    count = count+2;
    double yy1[2] = {0, hyranges[i]};
    graph[i] = new TGraph (2,xx1,yy1);
    graph[i]->SetMarkerStyle(1);
    if(i==0) {
      hTrue[i] = new TH1D ("Shistogram True", "", Nbins,  xx1[0], xx1[1]);
      hReco[i] = new TH1D ("Shistogram Reco", "", Nbins, xx1[0], xx1[1]);
    }
    /*if ((i>0)&&(i<4)){
      std::string hbacktrue_name = "Bhistogram True "+sigmarange[i-1].str()+"#sigma";
      std::string hbackreco_name = "Bhistogram Reco "+sigmarange[i-1].str()+"#sigma";
      char* char_hbacktrue_name = const_cast<char*>(hbacktrue_name.c_str());
      char* char_hbackreco_name = const_cast<char*>(hbackreco_name.c_str());

      hTrue[i] = new TH1D (char_hbacktrue_name, "", Nbins,  xx1[0], xx1[1]);
      hReco[i] = new TH1D (char_hbackreco_name, "", Nbins,  xx1[0], xx1[1]);
      }*/
    if(i==4){ hDeltaSignal = new TH1D ("#DeltaSignal", "", Nbins, xx1[0], xx1[1]);}
  }
  /*
  for( int i = 0; i < 20; i++ ) {
    double xx1[2] = {hxranges[0], hxranges[1]};
    double yy1[2] = {0, 1000};
    graph[i] = new TGraph (2,xx1,yy1);
    graph[i]->SetMarkerStyle(1);
    std::string hsignalreco_name = "Shistogram Reco "+std::to_string(i);
    char* char_hsignalreco_name = const_cast<char*>(hsignalreco_name.c_str());

    hRecoSignal[i] = new TH1D (char_hsignalreco_name, "", 50,  xx1[0], xx1[1]);
  }
  */
  
  int entries = treeread->GetEntries();
  double delta_signal;

  int histocount = 0;

  for( int j = 0; j < entries; j++ ) {

    treeread->GetEntry(j);
    std::cout<<j<<": "<<TrueSignal_<<std::endl;
    
    hTrue[0]->Fill(TrueSignal_);
    hReco[0]->Fill(RecoSignal_);
    delta_signal = RecoSignal_-TrueSignal_;
    hDeltaSignal->Fill(delta_signal);
    /*
    for(int i = 0 ; i < backsize; i++) {
      hTrue[i+1]->Fill(TrueBack_[i]);
      hReco[i+1]->Fill(RecoBack_[i]);
    }
    */
    if( plot_recosignal_10histos==true ) {
      hRecoSignal[histocount]->Fill(RecoSignal_);
      if((j==99)||(j==199)||(j==299)||(j==399)||(j==499)||(j==599)||(j==699)||(j==799)||(j==899)||(j==999)){ 
	double stdev = hRecoSignal[histocount]->GetRMS();
	hSigmas_RecoSignal->Fill(stdev);
	histocount++; }
    }
    
    if( plot_recosignal_20histos==true ) {
      hRecoSignal[histocount]->Fill(RecoSignal_);
      if((j==49)||(j==99)||(j==149)||(j==199)||(j==249)||(j==299)||(j==349)||(j==399)||(j==449)||(j==499)||(j==549)||(j==599)||(j==649)||(j==699)||(j==749)||(j==799)||(j==849)||(j==899)||(j==949)||(j==999)){ 
	double stdev = hRecoSignal[histocount]->GetRMS();
        hSigmas_RecoSignal->Fill(stdev);
	histocount++; }
    }

  }


  TPaveStats *stat[2];
  /*
  if( plot_4==true ) {

    for( int i = 0; i < 4; i++) {

      pad[i]->cd();
      pad[i]->SetLeftMargin(0.15);
      pad[i]->SetRightMargin(0.1);

      hReco[i]->SetFillColor(kGreen-3);
      hReco[i]->SetLineColor(kBlack);
      hReco[i]->SetFillStyle(3001);
      hReco[i]->Draw("HIST");

      gPad->Update();
      stat[0] = (TPaveStats*)hReco[i]->FindObject("stats");
      stat[0]->SetY1NDC(.74);
      stat[0]->SetY2NDC(.91);
      stat[0]->SetX1NDC(0.2);
      stat[0]->SetX2NDC(0.52);
      stat[0]->SetTextSize(0.043);
      stat[0]->SetTextColor(kGreen-3);

      hTrue[i]->SetFillColor(kViolet);
      hTrue[i]->SetLineColor(kBlack);
      hTrue[i]->SetFillStyle(3001);
      hTrue[i]->Draw("HIST,sames");

      gPad->Update();
      stat[1] = (TPaveStats*)hTrue[i]->FindObject("stats");
      stat[1]->SetY1NDC(.56);
      stat[1]->SetY2NDC(.73);
      stat[1]->SetX1NDC(0.2);
      stat[1]->SetX2NDC(0.52);
      stat[1]->SetTextSize(0.043);
      stat[1]->SetTextColor(kViolet);

      graph[i]->Draw("ap");
      if(i==0){
        graph[i]->GetXaxis()->SetTitle("Counts in Signal");
      }
      else{ graph[i]->GetXaxis()->SetTitle("Counts in Background"); }
      graph[i]->GetYaxis()->SetTitle("Experiments");
      hReco[i]->Draw("HIST,same");
      stat[0]->Draw("same");
      hTrue[i]->Draw("HIST,same");
      stat[1]->Draw("same");
    }
  }
  */
  if( plot_2signal==true ) {

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

    std::cout<<"Reco: Mean: "<<hReco[0]->GetMean()<<" +- "<<hReco[0]->GetMeanError()<<" Std Dev: "<<hReco[0]->GetRMS()<<" +- "<<hReco[0]->GetRMSError()<<std::endl;
    std::cout<<"True: Mean: "<<hTrue[0]->GetMean()<<" +- "<<hTrue[0]->GetMeanError()<<" Std Dev: "<<hTrue[0]->GetRMS()<<" +- "<<hTrue[0]->GetRMSError()<<std::endl;

    char* char_latex1;
    TLatex latex1;
    std::string str_latex1;
    
    if(FitOption==2000){
      str_latex1 = "Binned Log Fit";
    }
    if(FitOption==1000){
      str_latex1 = "#chi^{2} Fit";
    }
     if(FitOption==3000){
       str_latex1 = "Unbinned Log Fit";
    }
     char_latex1 = const_cast<char*>(str_latex1.c_str());
    latex1.DrawLatexNDC(.22,.47,char_latex1);


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

    graph[4]->Draw("ap");
    graph[4]->GetXaxis()->SetTitle("#DeltaSignal: Reco-True");
    graph[4]->GetYaxis()->SetTitle("Experiments");
    stat[0]->Draw("same");
    graph[4]->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
    graph[4]->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    hDeltaSignal->Draw("HIST,same");

  }


  int limit = 0;

  if( plot_recosignal_10histos==true ) { limit = 10; }

  if( plot_recosignal_20histos==true ) { limit = 20; }

  if(( plot_recosignal_10histos==true )||( plot_recosignal_20histos==true )){
    
    for( int i = 0; i < limit; i++) {

      pad[i]->cd();
      pad[i]->SetLeftMargin(0.15);
      pad[i]->SetRightMargin(0.1);

      hRecoSignal[i]->SetFillColor(kGreen-3);
      hRecoSignal[i]->SetLineColor(kBlack);
      hRecoSignal[i]->SetFillStyle(3001);
      hRecoSignal[i]->Draw("HIST");

      gPad->Update();
      stat[0] = (TPaveStats*)hRecoSignal[i]->FindObject("stats");
      stat[0]->SetY1NDC(.74);
      stat[0]->SetY2NDC(.91);
      stat[0]->SetX1NDC(0.2);
      stat[0]->SetX2NDC(0.62);
      stat[0]->SetTextSize(0.043);
      stat[0]->SetTextColor(kGreen-3);

      graph[i]->Draw("ap");
      graph[i]->GetXaxis()->SetTitle("Counts in Signal");
      graph[i]->GetYaxis()->SetTitle("Experiments");
      hRecoSignal[i]->Draw("HIST,same");
      stat[0]->Draw("same");
    }
      if(plot_stdev == true){
	
	c = new TCanvas();
	TGraph *graphsigma = new TGraph (2,sigmax,sigmay);
	graphsigma->SetMarkerStyle(1);
	hSigmas_RecoSignal->Draw();
	hSigmas_RecoSignal->SetFillColor(kRed-3);
	hSigmas_RecoSignal->SetLineColor(kBlack);
	hSigmas_RecoSignal->SetFillStyle(3001);
	hSigmas_RecoSignal->Draw("HIST");
	gPad->Update();
	stat[1] = (TPaveStats*)hSigmas_RecoSignal->FindObject("stats");
	stat[1]->SetY1NDC(.74);
	stat[1]->SetY2NDC(.91);
	stat[1]->SetX1NDC(0.2);
	stat[1]->SetX2NDC(0.45);
	stat[1]->SetTextSize(0.043);
	stat[1]->SetTextColor(kRed-3);

	graphsigma->Draw("ap");
	graphsigma->GetXaxis()->SetTitle("#sigma_{Reco Signal}");
	graphsigma->GetYaxis()->SetTitle("Entries");
	hSigmas_RecoSignal->Draw("HIST,same");
	stat[1]->Draw("same");


      }
  }




  //c->Print("Signal_BackgroundTrueReco_10s_1kHz_0.001MeV_BinnedLoglike_20000bins.png");
  //c->Print("Signal_BackgroundTrueReco_2000s_1kHz_0.001MeV_BinnedChi2_200bins.pdf");

  //infile->Close();
}
