#include "TFile.h"
#include "TH1D.h"
#include "TMatrixD.h"


void BinContent(std::string rootname) {

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(111111);

  TCanvas* c = new TCanvas();
    
  TFile *infile = new TFile(rootname.c_str());
  TH1D *hSTM = (TH1D*)infile->Get("hSTM50");
  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix50");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars50");

  //This tree corresponds to the last data generated so the one in hSTM50
  std::vector<double> *PeaksSignal=0, *PeaksBackground=0;
  
  int Nentries = hSTM->GetEntries();
  int Nbins = hSTM->GetNbinsX();
  double bin_width = hSTM->GetBinWidth(1);
  
  std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<" Bin Width: "<<bin_width<<std::endl;

  //TH1D *hbincontent = new TH1D ("hBin", "", 150, 0, 150);
  TH1D *hbincontent = new TH1D ("hBin", "", 20, 0, 20);
  //TH1D *hbincontent = new TH1D ("hBin", "", 150, 400, 4000);  
  
  int nbins_less20counts = 0;

  std::vector<double> less20counts;
  std::vector<double> more500counts;
  std::vector<double> less20counts_content;
  std::vector<double> more500counts_content;
  
  //PLOT THE BIN CONTENT IN A HISTOGRAM
  for (unsigned long int i = 0; i < Nbins; i++) {

    double bincontent = hSTM->GetBinContent(i);
    double bincenter = hSTM->GetBinCenter(i);

    if((bincontent<20)&&(bincontent>=0)){
      //if((bincontent<20)&&(bincontent>0)){
      less20counts.push_back(bincenter);
      less20counts_content.push_back(bincontent);
      //std::cout<<"Bin content: "<<hSTM->GetBinContent(i)<<" bin error: "<<hSTM->GetBinError(i)<<" sqrt(content) = "<<sqrt(hSTM->GetBinContent(i))<<std::endl;
      nbins_less20counts++;
    }
    if(bincontent>500){
      //std::cout<<"Bin content: "<<hSTM->GetBinContent(i)<<" bin error: "<<hSTM->GetBinError(i)<<" sqrt(content) = "<<sqrt(hSTM->GetBinContent(i))<<std::endl;
      more500counts.push_back(bincenter);
      more500counts_content.push_back(bincontent);
    }
      
    hbincontent->Fill( bincontent );

  }
  
  hbincontent->GetXaxis()->SetTitle("Bin Content (in 1kHz, 0.001MeV, 10s data)");
  hbincontent->GetYaxis()->SetTitle("# Bins");
  hbincontent->SetFillStyle(3001);
  hbincontent->SetFillColor(kBlue-9);
  hbincontent->SetLineColor(kBlack);
  hbincontent->Draw("");
  
  TPaveStats* stat[1];
  
  gPad->Update();
  stat[0] = (TPaveStats*)hbincontent->FindObject("stats");
  stat[0]->SetY1NDC(.6);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.6);
  stat[0]->SetX2NDC(0.92);
  stat[0]->SetTextSize(0.043);
  stat[0]->SetTextColor(kBlue-9);
  hbincontent->Draw("");
  stat[0]->Draw("same");
  
  char* char_latex1;
  TLatex latex1;

  std::string str_latex1 = "#Bins with bin content < 20: "+std::to_string(nbins_less20counts);
  char_latex1 = const_cast<char*>(str_latex1.c_str());

  latex1.DrawLatexNDC(.45,.5,char_latex1);
  

  /*  
  TTree *TreePeaksGen = (TTree*)infile->Get("AllPeaksGen");
  TreePeaksGen->SetBranchAddress("PeaksSignal", &PeaksSignal);
  TreePeaksGen->SetBranchAddress("PeaksBackground", &PeaksBackground);

  double mincounts = less20counts.at(0);
  double mincounts_lowE =  mincounts - bin_width/2;
  double mincounts_maxE	= mincounts + bin_width/2;
  
  double maxcounts = more500counts.at(0);
  double maxcounts_lowE	= maxcounts - bin_width/2;
  double maxcounts_maxE = maxcounts + bin_width/2;


  std::cout<<"Low stats bin contains : "<<less20counts_content.at(0)<<" counts, from: "<<mincounts_lowE<<" to "<<mincounts_maxE<<" MeV, bin center:  "<<mincounts<<std::endl;
  std::cout<<"High stats bin contains : "<<more500counts_content.at(0)<<" counts, from: "<<maxcounts_lowE<<" to "<<maxcounts_maxE<<" MeV, bin center: "<<maxcounts<<std::endl;
  
  TH1D *hbin_lowstats = new TH1D ("hbin_lowstats", "", 200, mincounts_lowE-0.00005, mincounts_maxE+0.00005);    //---> Poisson distributed
  TH1D *hbin_highstats = new TH1D ("hbin_highstats", "", 200, maxcounts_lowE-0.00005, maxcounts_maxE+0.00005);  //---> Normal distributed
  
  //PLOT THE DATA IN ONE BIN
  unsigned long int peaksvector = TreePeaksGen->GetEntry(0);

  for(unsigned long int i = 0 ; i < PeaksSignal->size() ; i++){
    double EnergyMeV = PeaksSignal->at(i); //MeV

    if((EnergyMeV > mincounts_lowE)&&(EnergyMeV < mincounts_maxE)){ hbin_lowstats->Fill(EnergyMeV) ;}
    if((EnergyMeV > maxcounts_lowE)&&(EnergyMeV < maxcounts_maxE)){ hbin_highstats->Fill(EnergyMeV) ;}
  }

for(unsigned long int i = 0 ; i < PeaksBackground->size() ; i++){
    double EnergyMeV = PeaksBackground->at(i); //MeV
    
    if((EnergyMeV > mincounts_lowE)&&(EnergyMeV < mincounts_maxE)){ hbin_lowstats->Fill(EnergyMeV) ;}
    if((EnergyMeV > maxcounts_lowE)&&(EnergyMeV < maxcounts_maxE)){ hbin_highstats->Fill(EnergyMeV) ;}
 }



 hbin_highstats->Draw("");
 //hbin_lowstats->Draw("");
 */
  c->Print("BinContent_1kHz_0.001MeV_10s_20000binsdata_bl.png");
}
