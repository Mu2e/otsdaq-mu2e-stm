#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cstdio>
#include<stdio.h>
#include<stdlib.h>
#include <iomanip>


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TRandom.h"

#define mm_to_cm 10


//================================================================
void PlotAcceptance_histo(std::string ArtFiles_location, double xmin, double xmax, string Xtitlest, int N, double Elimit_MeV, double cutVD10, double VDnumber,  double branch){

  gROOT->SetStyle("ATLAS");

  gStyle->SetOptStat(1110);
  gStyle->SetOptFit(01111);
  gStyle->SetPadRightMargin(0.08);
 
  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};

  //int N = 100; //nbins;
  //double Elimit_MeV = 5;
  bool draw_spectrum = false;
  bool draw_acceptance = true;
    
  double j = 0;
  int nbins;

  std::cout<<"cutVD10: "<<cutVD10<<std::endl;

  double p[4];
  
  //cut 7.50224 cut VD10
  if( cutVD10 == 7.50224 ){
    p[0] = 6.748e-7;
    p[1] = 5.923e5;
    p[2] = 4.455e11;
    p[3] = 3.506e11;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;
  }

  //no cut VD10
  if( cutVD10 == 0 ){
    p[0] = 1.106e-7;
    p[1] = 3.557e6;
    p[2] = 1.633e13;
    p[3] = 1.265e13;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;
  }

  //cut 7.86 VD10
  if( cutVD10 == 7.86 ){
    p[0] = 6.424e-7;
    p[1] = 6.171e5;
    p[2] = 4.876e11;
    p[3] = 3.806e11;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;
    
  }

  //cut 5.5cm VD10
  if( cutVD10 == 5.5 ){
    p[0] = 9.391e-7;
    p[1] = 4.205e5;
    p[2] = 2.273e11;
    p[3] = 1.767e11;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;
    
  }

  //cut 5cm VD10
  if( cutVD10 == 5 ){
    p[0] = 9.933e-7;
    p[1] = 4.041e5;
    p[2] = 2.065e11;
    p[3] = 1.632e11;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;

  }
  
  //cut 4.5cm VD10
  if( cutVD10 == 4.5 ){
    p[0] = 1.046e-6;
    p[1] = 3.766e5;
    p[2] = 1.828e11;
    p[3] = 1.417e11;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;
    
  }

  //cut 3.5cm VD10
  if( cutVD10 == 3.5 ){
    p[0] = 1.055e-6;
    p[1] = 3.767e5;
    p[2] = 1.813e11;
    p[3] = 1.418e11;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;

  }
  
  //cut 2.5cm VD10 
  if( cutVD10 == 2.5 ){
    p[0] = 5.501e-7;
    p[1] = 7.115e5;
    p[2] = 6.569e11;
    p[3] = 5.059e11;
    std::cout<<"p[0]: "<<p[0]<<" p[1]: "<<p[1]<<" p[2]: "<<p[2]<<" p[3]: "<<p[3]<<std::endl;
    
 }

  //TFile*output;
  //std::string rootfile = "HistogramVD10.root";
  //output=new TFile(rootfile.c_str(),"recreate");
  
  double weight[N];

  double int_rangelow = 0.344;
  double int_rangehigh = 0.350;

  double counts_range=0;
  
  std::cout<<"Using number of bins: "<<N<<std::endl;

  //Read Tree with energies at VD
  TH1D*h1 = new TH1D("h1","", N, 0, Elimit_MeV); //pz>0

  TH1D*h2 = new TH1D("h2","", N, xmin, xmax); //weighted acceptance

  //TH1D*h3 = new TH1D("h3","", N, 5, 4000); //mom > 5 MeV
  
  fstream readfile;
  readfile.open(ArtFiles_location,ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Read each art file from txt
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    std::cout<<name<<std::endl;
  }

   double check_totphotons = 0;
   double check_highEphotons = 0;
  
  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

  
  //Read tree
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    //for (long unsigned int file=0;file<1;file++){  

    string path;
    path=file_name[file];
    std::cout<<"file #: "<<file<<" "<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");

    tree->SetBranchAddress("evt",&evt);
    tree->SetBranchAddress("trk",&trk);
    tree->SetBranchAddress("sid",&sid);
    tree->SetBranchAddress("pdg",&pdg);
    tree->SetBranchAddress("run",&run);
    tree->SetBranchAddress("subrun",&subrun);
    tree->SetBranchAddress("time",&time);
    tree->SetBranchAddress("x",&x);
    tree->SetBranchAddress("y",&y);
    tree->SetBranchAddress("z",&z);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("ke",&ke);
    tree->SetBranchAddress("code",&code);
    
    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;


     for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);

      //Just fill at Virtual detector ID number:

      if(sid==VDnumber){

	double mom = sqrt(px*px+py*py+pz*pz);

	if( pdg==22 ){
	  
	  double xcentre = -3904; //mm
          double ycentre = 0;
          double r = sqrt((x-xcentre)*(x-xcentre)+(y-ycentre)*(y-ycentre));

	  //no r cut
	  if((cutVD10==0)&&(pz > 0)){ h1->Fill(mom);
	    if(( mom > int_rangelow )&&(mom < int_rangehigh)){ counts_range++; }
	    
	  }
          if((cutVD10!=0)&&(r < (cutVD10*mm_to_cm))&&(pz > 0)){

	    //if(mom > Elimit_MeV){
	    //std::cout<<"Photon with momentum bigger than 5MeV: "<<mom<<std::endl;
	    //h3->Fill(mom); check_highEphotons++; } 
	    
	    h1->Fill(mom);

	    if(( mom > int_rangelow )&&(mom < int_rangehigh)){ counts_range++; }
	    
	    check_totphotons++;
	    
	    if(mom>5){
	      check_highEphotons++;
		}
	  } //r
	}//pdg
	}//sid

     }//entries

  }//file

 
  std::cout<<"Total number of photons: "<<check_totphotons<<std::endl;
  std::cout<<"From which these photons have E>5MeV: "<<check_highEphotons<<std::endl;

  double Full_Acceptance;

  double calculateMeanHisto_zero;
  double calculateMeanHisto_diffzero;
  double sumContentHisto_zero = 0;
  double sumContentHisto_diffzero = 0;
  int N_w = 0;
  double sum_acceptance = 0;
  
  //Fill Acceptance histogram
     for(int i =0 ; i < N; i++){
       //Calculate the acceptance
       double bincenter = h1->GetXaxis()->GetBinCenter(i);

       if(bincenter < 0.036){ Full_Acceptance = 0;}
       else { Full_Acceptance = p[0] + 1./(p[1]*bincenter) - sqrt(1./(p[2]*bincenter) + 1./(p[3]*bincenter*bincenter));}
	 
       
       weight[i] = h1->GetBinContent(i);
       
       h2->Fill(Full_Acceptance, weight[i]);
       std::cout<<"Energy: "<<bincenter<<" MeV, acceptance: "<<Full_Acceptance<<" weight: "<<weight[i]<<std::endl;
	 
       
       
       sum_acceptance = sum_acceptance + Full_Acceptance*weight[i];
       N_w = N_w + weight[i];

       
       std::cout<<"SUM acceptance = acceptancexweight:  "<< sum_acceptance <<std::endl;
       std::cout<<"n weight: "<<N_w<<std::endl;
       std::cout<<"Calculated acceptance: "<< sum_acceptance/N_w <<std::endl;
       std::cout<<"Acceptance: "<<h2->GetMean()<<" +- "<<h2->GetMeanError()<<" RMS: "<<h2->GetRMS()<<std::endl;
       std::cout<<""<<endl;	 
	 
       //h2->SetBinContent(i, acceptance[i]);
       //h2->AddBinContent (i, weight[i])
     }

     std::cout<<"Sum of weights: "<<N_w<<std::endl;
     std::cout<<"Calculated acceptance (mean): "<< sum_acceptance/N_w <<" +- "<<sqrt(h2->GetRMS())/N_w<<std::endl;
     std::cout<<"Acceptance (get mean): "<<h2->GetMean()<<" +- "<<h2->GetMeanError()<<" RMS: "<<h2->GetRMS()<<std::endl;
     
     //Plot histogram
     TAxis *X = h2->GetXaxis();
     X->SetNdivisions(5,3,0);

  if(draw_acceptance==true){
    h2->SetFillStyle(3001);
    h2->SetFillColor(kBlue+2);
    h2->SetLineColor(kBlue+2);
   
    char* Xtitle = const_cast<char*>(Xtitlest.c_str());
    //char* Ytitle = const_cast<char*>(Ytitlest.c_str());
    h2->GetXaxis()->SetTitle(Xtitle);
    h2->GetYaxis()->SetTitle("Counts");

    h2->Draw("HIST");
    
    c1->Update();
    TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
    ps->SetX1NDC(0.68);
    ps->SetX2NDC(0.88);
    ps->SetY1NDC(0.76);
    ps->SetY2NDC(0.9);
    ps->SetTextSize(0.032);
    ps->SetName("mystats");
  
    h2->SetStats(0);
    ps->GetLineWith("Entries")->SetTextColor(kBlue);
    
    c1->Modified();
    
    //h2->Draw("HIST");
    ps->Draw("same");
  }

  if(draw_spectrum==true){

     h1->SetFillColor(kGreen+2);
     h1->GetXaxis()->SetTitle("p_{#gamma, VD=10 (cut ST size, p_{z}>0)} [MeV]");
     h1->GetYaxis()->SetTitle("Counts");
     
     h1->Draw("");
     
     c1->Update();
     TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
     ps->SetX1NDC(0.68);
     ps->SetX2NDC(0.88);
     ps->SetY1NDC(0.76);
     ps->SetY2NDC(0.9);
     ps->SetTextSize(0.032);
     ps->SetName("mystats");

     //gPad->SetLogy();
     h1->SetStats(0);
     ps->GetLineWith("Entries")->SetTextColor(kBlue);

     c1->Modified();
     
     //h1->Draw("");
     //ps->Draw("same");

  }
  //Integral of the histogram to get the rate
  //double integral =  h2->Integral(h2->FindFixBin(0), h2->FindFixBin(5), "");
  //std::cout<<"Integral of the histogram 0-5MeV: "<<integral<<std::endl;
  //double timeMDC2020 = 0.00001; //s
  //double rate = integral / (1000*timeMDC2020); //kHz
  //std::cout<<"Rate of flash photons at STM: "<<rate<<" kHz"<<std::endl;
  

  double integral =  h1->Integral(h1->FindFixBin(int_rangelow), h1->FindFixBin(int_rangehigh), "");
  std::cout<<"Integral of the histogram between "<<int_rangelow<<" "<<int_rangehigh<<" keV: "<<integral<<std::endl;
  std::cout<<"Count integral "<<int_rangelow<<" "<<int_rangehigh<<" keV: "<<counts_range<<std::endl;
  std::cout<<"Sum bin entries: "<<std::endl;

  int first_bin = h1->FindFixBin(int_rangelow);
  int last_bin = h1->FindFixBin(int_rangehigh);
  double bin_width = (Elimit_MeV-0)/N;
  double sum = 0.0;
  
  std::cout<<"First bin: "<<first_bin<<" last bin: "<<last_bin<<std::endl;
  for(int i = first_bin; i <= last_bin; i++){
    
    double content = h1->GetBinContent(i);
   
    sum = sum+content;
    
    std::cout<<"Bin: "<<i<<" E = "<<h1->GetXaxis()->GetBinCenter(i)<<" content: "<<content<<" sum: "<<sum<<std::endl;
  }

  
  std::stringstream streamcutVD10;
  streamcutVD10 << std::fixed << std::setprecision(4) << cutVD10;
  std::string name_plot = "MDC2020MuonsEle_momFlashPhotonsVD10_reducedshapecut"+streamcutVD10.str()+"_pz_"+std::to_string(int(Elimit_MeV))+"MeV_log.png";
  //std::string name_plot = "HistoAccVD10_weighted_reducedshapecut"+streamcutVD10.str()+"_pz_"+std::to_string(int(Elimit_MeV))+"MeV_"+std::to_string(int(N))+"bins.png";
  char* imagename = const_cast<char*>(name_plot.c_str());

  
    
  //c1->Print("MDC2020MuonsEle_momFlashPhotonsVD10_reducedshapecut_pz_50MeV.pdf");
  //c1->Print(imagename);

  //output->WriteObject(h1, "h1");
  //output->Close();
}


//================================================================

void Acceptance_Weighted(std::string ArtFiles_location, int N, double Elimit_MeV, double cutVD10, double VDnumber){

  //N is nbins
  
  PlotAcceptance_histo(ArtFiles_location, 0, 0.55e-6, "Acceptance(VD10-VD89,90), weighted by energy", N, Elimit_MeV, cutVD10, VDnumber, 0);  
}
