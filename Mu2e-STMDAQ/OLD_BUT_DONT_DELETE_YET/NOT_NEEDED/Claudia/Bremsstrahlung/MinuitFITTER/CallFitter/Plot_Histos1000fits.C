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
#include <iomanip>


void Plot_Histos1000fits(std::string filename){


  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  TCanvas *c = new TCanvas("");
  double Strue[1000], Sreco[1000], StdevSreco[1000];


  TH1D* hDeltaSignal;
  int Nbins = 200;

  double xx1delta[2] = {-7, 4};
  double yy1[2] = {0, 50};

  hDeltaSignal = new TH1D ("#DeltaSignal/#sigma", "", Nbins, xx1delta[0], xx1delta[1]);
  
  //TH1D* hRecoSignal = new TH1D ("hReco", "", Nbins, xx1delta[0], xx1delta[1]);

  TGraph* graph = new TGraph (2,xx1delta,yy1);
  graph->SetMarkerStyle(1);


  
  //Read csv
  std::ifstream myFile(filename);

  double val;
  std::string line;
  int Ncol;
  int Nrow=0;
  
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

   while(std::getline(myFile, line))
     { 
        std::stringstream ss(line);
	Ncol=0;
	while(ss >> val){
	  std::cout<<"val: "<<val<<std::endl;
	  if(Ncol==0){Strue[Nrow] = val;}
	  if(Ncol==1){Sreco[Nrow] = val;}
	  if(Ncol==2){StdevSreco[Nrow] = val;}
	  
	  Ncol++;
	}
	double delta_signal = (Strue[Nrow] - Sreco[Nrow])/StdevSreco[Nrow];
	std::cout<<"true peaks: "<<Strue[Nrow]<<std::endl;
	std::cout<<"reco peaks: "<<Sreco[Nrow]<<std::endl;
	std::cout<<"reco sigma: "<<StdevSreco[Nrow]<<std::endl;
	std::cout<<"Delta signal: "<<delta_signal<<std::endl;
	hDeltaSignal->Fill(delta_signal);
	//hRecoSignal->Fill(Sreco[Nrow]);
	Nrow++;
     }
   
   myFile.close();

   TPaveStats *stat[1];
   
   
   hDeltaSignal->SetFillColor(kBlue);
   hDeltaSignal->SetLineColor(kBlack);
   hDeltaSignal->SetFillStyle(3001);
   hDeltaSignal->Draw("HIST");
   
   /*
   hRecoSignal->SetFillColor(kBlue);
   hRecoSignal->SetLineColor(kBlack);
   hRecoSignal->SetFillStyle(3001);
   hRecoSignal->Draw("HIST");
   */
   
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
   //hRecoSignal->Draw("HIST,same"); 
  
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


   c->Print("Minuit_SampledBckgrnd_Deltasignal_200kHz_3keV.png");

}
