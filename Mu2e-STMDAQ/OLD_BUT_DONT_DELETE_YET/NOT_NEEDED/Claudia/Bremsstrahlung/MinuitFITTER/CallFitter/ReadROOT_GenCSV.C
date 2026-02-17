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


void ReadROOT_GenCSV( std::string rootname) {
  
  TCanvas* c = new TCanvas("c");

  
  //***********READ DATA***********// 
  std::cout<<rootname<<std::endl;
  
  TFile *infile = new TFile(rootname.c_str());

  TH1D *h2 = (TH1D*)infile->Get("h2");
  h2->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
  h2->GetYaxis()->SetTitle("");
  h2->SetFillColor(kOrange-3);
  h2->SetLineColor(kOrange-3);


  int Nentries = h2->GetEntries();
  int Nbins = h2->GetNbinsX();
  double bin_width = h2->GetBinWidth(0);

  std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" bin width: "<<bin_width<<std::endl;
  
  std::ofstream output_file[2];
  std::string   output_filename[2];
  output_filename[0]="BinEntries.csv";
  output_filename[1]="BinIntervals.csv";
  
  output_file[0].open(output_filename[0]);
  output_file[1].open(output_filename[1]);
  
  std::vector<double> entries;
  std::vector<double> binedge;

  
  for (int i=0; i< Nentries; i++){
    double element = h2->GetBinContent(i);
    double binend = h2->GetBinCenter(i) + bin_width/2;
    output_file[0] << element << "\n";
    output_file[1] << binend << "\n";
  }


  output_file[0].close();
  output_file[1].close();
}

