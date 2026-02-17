#include "TFile.h"
#include "TH1D.h"
#include "TMatrixD.h"

#include<algorithm>

void BinContent_jobs(std::string input_path) {

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1);
  
  gStyle->SetPadRightMargin(0.08);
  
  TCanvas* c = new TCanvas();

  int Njobs =20;
  int nexperiments =50;
  std::string rootname;
  TH1D* hSTM;
  
  std::vector<double> Nentries_distr;
  
 for(int i = 0 ; i < Njobs ; i++) {

   //rootname = input_path+"/UnbinnedLogLike_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";                               
   //rootname = input_path+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+".root";
   rootname = input_path+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_10s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
   std::cout<<"Input file: "<<rootname<<std::endl;

    TFile *infile = new TFile(rootname.c_str());


    for( int j = 0 ; j <  nexperiments; j++) {

      std::cout<<""<<std::endl;
      std::cout<< "Experiment: " << j+1 << std::endl;

      
      //Get S+B histogram
      std::string str_hSTM = "hSTM"+std::to_string(j+1);
      char* char_hSTM = const_cast<char*>(str_hSTM.c_str());
      hSTM = (TH1D*)infile->Get(char_hSTM);
      
      double bin_width = hSTM->GetBinWidth(0);  
      int Nentries = hSTM->GetEntries();
      int Nbins = hSTM->GetNbinsX();
            
      std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" Bin Width: "<<bin_width<<std::endl;
      Nentries_distr.push_back(Nentries);
    }
 }


 //get max and min of a vector
 
 int smallest_element = *min_element(Nentries_distr.begin(),Nentries_distr.end());
 
 int largest_element  = *max_element(Nentries_distr.begin(),Nentries_distr.end());

 double xmin =smallest_element -100;
 double xmax =largest_element +500; 

//double xmin =smallest_element -500;
//double xmax =largest_element +5000;
 
 TH1D *hbincontent = new TH1D ("hDataEntries", "", 100, xmin, xmax);
   
 //PLOT THE BIN CONTENT IN A HISTOGRAM
 for (unsigned long int i = 0; i < Nentries_distr.size(); i++) {
    
   hbincontent->Fill( Nentries_distr.at(i) );

  }
  
  hbincontent->GetXaxis()->SetTitle("Data Size (1kHz, 0.001MeV, 10s)");
  hbincontent->GetYaxis()->SetTitle("# Data Samples");
  hbincontent->SetFillStyle(3001);
  hbincontent->SetFillColor(kBlue-9);
  hbincontent->SetLineColor(kBlack);
  hbincontent->Draw("");
  
  TPaveStats* stat[1];
  
  gPad->Update();
  stat[0] = (TPaveStats*)hbincontent->FindObject("stats");
  stat[0]->SetY1NDC(.5);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.56);
  stat[0]->SetX2NDC(0.86);
  stat[0]->SetTextSize(0.043);
  stat[0]->SetTextColor(kBlue-9);
  
  hbincontent->Draw("");
  stat[0]->Draw("same");

  TF1 *fit = new TF1("fit","[0]*TMath::Gaus(x,[1],[2])",xmin,xmax);
  fit->SetParameters(hbincontent->GetMaximum(), hbincontent->GetMean(), hbincontent->GetRMS());
  fit->SetParNames("A","Fit Mean","RMS");
  hbincontent->Fit("fit","0Q");
  fit->SetLineColor(kRed);
  fit->SetLineStyle(2);
  fit->Draw("same");


   c->Update();
   TPaveStats *p = (TPaveStats*)hbincontent->GetListOfFunctions()->FindObject("stats");
   hbincontent->GetListOfFunctions()->Remove(p);
   hbincontent->SetStats(0);
   p->GetLineWith("#chi^{2} / ndf")->SetTextColor(kRed);
   p->GetLineWith("A")->SetTextColor(kRed);
   p->GetLineWith("Fit Mean")->SetTextColor(kRed);
   p->GetLineWith("RMS")->SetTextColor(kRed);
   p->GetLineWith("#chi^{2} / ndf")->SetTextSize(0.027);
   p->GetLineWith("A")->SetTextSize(0.027);
   p->GetLineWith("Fit Mean")->SetTextSize(0.027);
   p->GetLineWith("RMS")->SetTextSize(0.027);

   p->Draw("same");
  /*
  char* char_latex1;
  TLatex latex1;
  std::string str_latex1 = "#Bins with bin content < 20: "+std::to_string(nbins_less20counts);
  char_latex1 = const_cast<char*>(str_latex1.c_str());

  latex1.DrawLatexNDC(.45,.5,char_latex1);
  */

  
  c->Print("BinEntries_10srun_1kHz_1000experiments_bl.png");
}
