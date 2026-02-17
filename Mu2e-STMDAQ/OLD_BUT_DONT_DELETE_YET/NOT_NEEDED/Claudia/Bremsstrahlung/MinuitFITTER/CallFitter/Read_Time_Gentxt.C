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
#include "TGaxis.h"
#include "TGraph2D.h"
#include "TH2D.h"

#include <iomanip>
#include <string>
#include <stdlib.h>
#include <vector>
#include <utility>
#include <sstream>
#include <fstream>

void Read_Time_Gentxt( std::string root_file ) {

  //std::string root_file = "/data1/cgarcia/SignaltoBackground/p0_p1_rate_resolution_time_accuracy.root";

  TFile* infile  = new TFile(root_file.c_str());

  //TreeFit
  std::vector<double> *p0_fit=0, *p1_fit=0, *p0error_fit=0, *p1error_fit=0, *chi2_fit=0, *rate_fit=0, *resolution_fit=0, *time_10percent_fit=0, *Nparams_fit=0;
  //TreeRoot
  std::vector<double> *rate_root=0, *resolution_root=0, *time_root=0, *accuracy_root=0;

  //Read trees
  TTree *TreeFit = (TTree*)infile->Get("TreeFit");
  TreeFit->SetBranchAddress("p0_fit",&p0_fit);
  TreeFit->SetBranchAddress("p1_fit",&p1_fit);
  TreeFit->SetBranchAddress("p0error_fit",&p0error_fit);
  TreeFit->SetBranchAddress("p1error_fit",&p1error_fit);
  TreeFit->SetBranchAddress("chi2_fit",&chi2_fit);
  TreeFit->SetBranchAddress("rate_fit",&rate_fit);
  TreeFit->SetBranchAddress("resolution_fit",&resolution_fit);
  TreeFit->SetBranchAddress("time_10percent_fit",&time_10percent_fit);
  TreeFit->SetBranchAddress("Nparams_fit",&Nparams_fit);

  TreeFit->GetEntry(0);

  TTree *TreeRoot = (TTree*)infile->Get("TreeRoot");
  TreeRoot->SetBranchAddress("rate_root", &rate_root);
  TreeRoot->SetBranchAddress("resolution_root", &resolution_root);
  TreeRoot->SetBranchAddress("time_root", &time_root);
  TreeRoot->SetBranchAddress("accuracy_root", &accuracy_root);

  TreeRoot->GetEntry(0);

  gROOT->SetStyle("ATLAS");
  //int palette_number = 62;
  int palette_number = 55;  
  gStyle->SetPalette(palette_number);
  gStyle->SetPadRightMargin(0.15);
  //gStyle->SetPadLeftMargin(0.2);
  gStyle->SetPadTopMargin(0.08);

  TCanvas* c = new TCanvas("c");

  std::cout<<"Number of fits: "<<p0_fit->size()<<" Number of root files analysed: "<<rate_root->size()<<std::endl;
  unsigned long int Nfits = p0_fit->size();
  std::vector<double> rate_fitv, resolution_fitv, time_10percent_fitv;

  std::ofstream outfile;
  outfile.open("Resolution_Rates_Time.txt");
    
  for(unsigned long int i = 0 ; i < Nfits; i++) {
    double rate_kHz = rate_fit->at(i);
    double res_keV = resolution_fit->at(i)*1000;
    double time_s = time_10percent_fit->at(i);
    std::cout<<"rate: "<<rate_kHz<<" res: "<<res_keV<<" time: "<<time_s<<std::endl; 
    rate_fitv.push_back(rate_kHz);
    resolution_fitv.push_back(res_keV);
    time_10percent_fitv.push_back(time_s);
  }

  //Store rates in a vector
  std::vector<double> rates_store, res_store;

  double rate_aux = 0;
  int num_resol = 11;
  double vals_res[11]={0.001, 0.0012, 0.0014, 0.0016, 0.0018, 0.002, 0.0022, 0.0024, 0.0026, 0.0028, 0.003};
  outfile << " ";
  //Write 1st row of rates and store rates and resolutions in a vector: rates_store, res_store
  for(unsigned long int i = 0 ; i < Nfits; i++) {
    double rate_kHz = rate_fit->at(i);
    double res_MeV = resolution_fit->at(i);

    if(rate_kHz != rate_aux){
      outfile << rate_kHz << " ";
      rates_store.push_back(rate_kHz);
      rate_aux = rate_kHz;
    }

  }

  for (int i = 0; i < num_resol; i++) {
    res_store.push_back(vals_res[i]);
  }


  outfile << std::endl;

  //Write resolutions in first column and timing inside the matrix
  for( unsigned long int j = 0 ; j < res_store.size(); j++){
    std::cout<<"res "<<res_store.at(j)<<std::endl;
    outfile << res_store.at(j) << " ";
    for( unsigned long int k = 0 ; k < rates_store.size(); k++){
      std::cout<<"rate "<<rates_store.at(k)<<std::endl;
      for(unsigned long int i = 0 ; i < Nfits; i++) {
	double rate_kHz = rate_fit->at(i);
	double res_MeV = resolution_fit->at(i);
	double time_s = time_10percent_fit->at(i);
	std::cout<<"--rate: "<<rate_kHz<<" res: "<<res_MeV<<" time: "<<time_s<<std::endl;
	std::cout<<"COMPARE: "<<rates_store.at(k)<<" "<<res_store.at(j)<<std::endl;
	if((res_store.at(j)==res_MeV)&&(rates_store.at(k)==rate_kHz)){ outfile << time_s << " "; std::cout<<"WRITE"<<std::endl;}
      } //i
    }//k
    outfile << std::endl;
  }

  outfile.close();

  TGraph2D *gracc = new TGraph2D(Nfits,&rate_fitv[0],&resolution_fitv[0],&time_10percent_fitv[0]);    

  double xx1[2]={0, 200};
  double yy1[2]={1, 3};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->GetXaxis()->SetTitleColor(kBlack);
  graph1->GetXaxis()->SetTitleSize(0.05);
  graph1->GetXaxis()->SetLabelSize(0.04);
  graph1->GetYaxis()->SetTitleSize(0.05);
  graph1->GetYaxis()->SetLabelSize(0.04);
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("Resolution [keV]");
  graph1->GetYaxis()->SetTitleOffset(1.3);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  gracc->GetHistogram()->GetZaxis()->SetLabelSize(0.04);
  gracc->Draw("same,colz");

  TLatex latex;
  latex.SetTextSize(0.05);
  latex.SetTextAlign(13);

  latex.DrawLatex(150,3.15,"Time(#sigma_{S}/S = 0.1) [s]");

  gPad->SetTicks();
  gPad->RedrawAxis();


}
