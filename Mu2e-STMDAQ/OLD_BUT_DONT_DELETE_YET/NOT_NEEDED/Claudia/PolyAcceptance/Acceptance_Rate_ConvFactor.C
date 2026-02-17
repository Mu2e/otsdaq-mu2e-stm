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
#include "TFitResult.h"
#include "TMatrixD.h"

#define mm_to_cm 10


//================================================================
void PlotAcceptance_E(std::string ArtFiles_location, double acceptance_VD10_15, double acceptance_VD10_15_error, double acceptance_VD101_8990, double acceptance_VD101_8990_error, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int VDnumber, double cutVD10, double branch){

  gROOT->SetStyle("ATLAS");

  //gStyle->SetOptStat(1110);
  
  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  //gPad->SetLogx();

  int N = 0;
  
  //Calculate the acceptance VD15-VD101
  double p[4] = {1.14, 0.3481, 0.155, 0.1211};
  double acceptance_VD15_101;

  double nparticles_sim_costhetarange = 100000;

  double nparticles_md2020_costhetarange , nparticles_md2020_full;
  double Acc, deltaAcc;


    double xlimit = 80;
    int Nbins_in = 1600;
    
    TH1D*h1 = new TH1D("h1","", Nbins_in, -0.5, xlimit);
    TH1D*h2 = new TH1D("h2","", Nbins_in, -0.5, xlimit);
 
    double POT = 200000000;
      
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

	  if((r < (cutVD10*mm_to_cm))&&(pz > 0)){
	    h1->Fill(mom);
	  }
	}
	
      }//sid

     }//entries

  }//file

  //Plot histogram
  TAxis *X = h2->GetXaxis();
  X->SetNdivisions(5,3,0);
  h2->SetFillStyle(3001);
  h2->SetFillColor(kGreen+2);

  int j =0;

  //Get bins and multiply each bin by the acceptance:
  int Nbins = h1->GetNbinsX();
  double Full_Acceptance;
  double E_times_acc;

  double acceptance_phot[Nbins], acceptance[Nbins], gammasdet_stm[Nbins];
  double acceptance_error[Nbins], gammasdet_stm_error[Nbins];

  
  for(int i = 0; i < Nbins; i++){

    double bincontent = h1->GetBinContent(i);
    double bincenter = h1->GetXaxis()->GetBinCenter(i);

    double energyMeV = bincenter;


    //Introduce the number of photons in costheta vs. the number of photons in the full range from MDCD2020 as energy dependent
    if(energyMeV<0.1){
      nparticles_md2020_costhetarange = 64.0;
      nparticles_md2020_full = 480178.0;
    }
    else if((energyMeV>=0.1)&&(energyMeV<0.3)){
      nparticles_md2020_costhetarange = 42.0;
      nparticles_md2020_full = 258533.0;
    }
    else if((energyMeV>=0.3)&&(energyMeV<0.4)){
      nparticles_md2020_costhetarange = 15.0;
      nparticles_md2020_full = 66424.0;
    }
    else if((energyMeV>=0.4)&&(energyMeV<0.5)){
      nparticles_md2020_costhetarange = 11.0;
      nparticles_md2020_full = 48253.0;
    }
    else if((energyMeV>=0.5)&&(energyMeV<1)){
      nparticles_md2020_costhetarange = 77.0;
      nparticles_md2020_full = 272177.0;
    }
    else if((energyMeV>=1)&&(energyMeV<5)){
      nparticles_md2020_costhetarange = 42.0;
      nparticles_md2020_full = 211057.0;
    }
    else{
      nparticles_md2020_costhetarange = 12.0;
      nparticles_md2020_full = 78004.0;
    }
    
     Acc = nparticles_md2020_costhetarange / nparticles_md2020_full; //p binomial error
     deltaAcc = sqrt(Acc*(1-Acc)/nparticles_md2020_full);

     std::cout<<"energyMeV: "<<energyMeV<<std::endl;
     std::cout<<"ratio MDC2020: "<<Acc<<" +- "<<deltaAcc<<" binomial error"<<std::endl;
     double nparticles_sim_full = nparticles_sim_costhetarange / Acc;
     double nparticles_sim_full_error = nparticles_sim_full * deltaAcc / Acc;
     std::cout<<"Total number of particles in the acceptance for this energy: "<<nparticles_sim_full<<" +- "<<nparticles_sim_full_error<<std::endl;

     acceptance_VD15_101 = p[0] + 1./(p[1]*energyMeV) - sqrt(1./(p[2]*energyMeV) + 1./(p[3]*energyMeV*energyMeV));
     
     if(VDnumber == 10){
       if(bincenter<0.03){acceptance_phot[i] = 0;}
       else{acceptance_phot[i] = acceptance_VD10_15 * acceptance_VD15_101 *acceptance_VD101_8990;}
     }


     gammasdet_stm[i] = nparticles_sim_costhetarange * acceptance_phot[i];
     
     gammasdet_stm_error[i] = gammasdet_stm[i] * sqrt((acceptance_VD10_15_error/acceptance_VD10_15)*(acceptance_VD10_15_error/acceptance_VD10_15) + (acceptance_VD101_8990_error/acceptance_VD101_8990)*(acceptance_VD101_8990_error/acceptance_VD101_8990));

     acceptance[i] = gammasdet_stm[i] / nparticles_sim_full;

     acceptance_error[i] = acceptance[i] * sqrt((nparticles_sim_full_error/nparticles_sim_full)*(nparticles_sim_full_error/nparticles_sim_full)+(gammasdet_stm_error[i]/gammasdet_stm[i])*(gammasdet_stm_error[i]/gammasdet_stm[i])) ;

    
    double Full_Acceptance = acceptance[i];

    E_times_acc = bincontent * Full_Acceptance;
    std::cout<<"Bin content: "<<bincontent<<" counts, Bin center: "<<bincenter<<" MeV, Acceptance: "<<Full_Acceptance<<std::endl;
    std::cout<<"New bin content for bin "<<i<<": "<<E_times_acc<<std::endl;

    h2->SetBinContent( i , E_times_acc );
  }

  
  
  bool drawhisto = true;
  
  if(drawhisto ==true){
    char* Xtitle = const_cast<char*>(Xtitlest.c_str());
    char* Ytitle = const_cast<char*>(Ytitlest.c_str());
    h2->GetXaxis()->SetTitle(Xtitle);
    h2->GetYaxis()->SetTitle(Ytitle);
    /*    
    h2->Draw("");

    c1->Update();
    TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
    ps->SetX1NDC(0.71);
    ps->SetX2NDC(0.91);
    ps->SetY1NDC(0.76);
    ps->SetY2NDC(0.9);
    ps->SetTextSize(0.032);
    ps->SetName("mystats");
    
    h2->SetStats(0);
    ps->GetLineWith("Entries")->SetTextColor(kBlue);

    c1->Modified();
    */

    //h2->Scale(1./POT);
    //h2->Draw("HIST");
    h2->Draw("");
    
    gPad->SetLogy();

    double xlowlim = 0;

    //Integral of the histogram to get the rate
    double integral =  h2->Integral(h2->FindFixBin(xlowlim), h2->FindFixBin(xlimit), "");
    std::cout<<"Integral of the histogram "<<xlowlim<<"-"<<xlimit<<" MeV: "<<integral<<std::endl;
    double integral_width =  h2->Integral(h2->FindFixBin(xlowlim), h2->FindFixBin(xlimit), "width");
    std::cout<<"Integral of the histogram (multiplying by bin width) "<<xlowlim<<"-"<<xlimit<<" MeV: "<<integral_width<<std::endl;

    int Nbinsh2 = h2->GetNbinsX();
    double sum=0;
    for(int i = 0; i < Nbinsh2; i++){
      double bincontent = h2->GetBinContent(i);
      double bincenter = h2->GetXaxis()->GetBinCenter(i);
      if((bincenter>xlowlim)&&(bincenter<xlimit)){sum=sum+bincontent;}
    }
    std::cout<<"Integral of the histogram by summing the bin content "<<xlowlim<<"-"<<xlimit<<" MeV: "<<sum<<std::endl;

    double timeMDC2020 = 0.00001695; //s
    double rate = integral / (1000*timeMDC2020); //kHz
    std::cout<<"Time mdc2020: "<<timeMDC2020<<" sec"<<std::endl;
    std::cout<<"Rate of flash photons at STM: "<<rate<<" kHz"<<std::endl;
  



  }//If draw histo


  
  c1->Print("MDC2020MuonsEle_FlashPhotonsEdistrVD10cut_Acc_1_0.0032_80MeVlogscale_ConvFactorEdep_nolegend.png");
  
}


//================================================================

void Acceptance_Rate_ConvFactor(std::string ArtFiles_location, double acceptance_VD10_15, double acceptance_VD10_15_error, double acceptance_VD101_8990, double acceptance_VD101_8990_error, int VDnumber, double cutVD10){

  
  PlotAcceptance_E(ArtFiles_location, acceptance_VD10_15, acceptance_VD10_15_error, acceptance_VD101_8990,  acceptance_VD101_8990_error, 0, 6, 0, 0.0000005, "E_{#gamma} [MeV]", "Counts #times Acceptance(VD10-VD89,90)", VDnumber, cutVD10, 1);

}
