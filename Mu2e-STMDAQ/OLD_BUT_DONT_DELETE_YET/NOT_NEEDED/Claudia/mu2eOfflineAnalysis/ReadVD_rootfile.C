#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
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
#include "TF1.h"
#include "TPaveStats.h"
#include "TLatex.h"

#include "Math/DistFunc.h"
#include "TFitResult.h"
#include "TMatrixD.h"

//================================================================
//This program reads readvd/ntvd or readvd/ntvext trees: Virtual detectors analyser
//Reads the root files from a txt and analyse the trees
//================================================================  

void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, string Xtitlest, string Ytitlest, int branch, int savetorootfile, string outputfilename){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  TCanvas *c1 = new TCanvas("");
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  float bremsMom, annihilMom, comptMom, photMom;

  TFile*output=new TFile(outputfilename.c_str(),"recreate");
  TTree*Bremstree=new TTree("Bremstree", "Bremstree");
  TTree*Annihiltree=new TTree("Annihiltree", "Annihiltree");
  TTree*Compttree=new TTree("Compttree", "Compttree");
  TTree*Phottree=new TTree("Phottree", "Phottree");

  
  Bremstree->Branch("bremsMom",&bremsMom);
  Annihiltree->Branch("annihilMom",&annihilMom);
  Compttree->Branch("comptMom",&comptMom);
  Phottree->Branch("photMom",&photMom);

  unsigned long int counter=0;

  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

  TH1F*h1 = new TH1F("TH1","", nbins, xmin, xmax);
  
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
  
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;

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
      float VDnumber =15;
      if(sid==VDnumber){
	
	double mom=sqrt(px*px+py*py+pz*pz);
	double pt = sqrt(px*px+py*py);
	
	if((branch==1)&&(pdg==22)){
	  h1->Fill(mom);

	  if((code!=16)&&(code!=2)&&(code!=12)&&(code!=40)){
	    std::cout<<"VDid: "<<sid<<" PDGid: "<<pdg<<" px: "<<px<<" py: "<<py<<" pz: "<<pz<<" mom: "<<mom<<" Creation code: "<<code<<std::endl;
	    counter++;  
	  }
	
	    if(code==16){
	    //std::cout<<"Bremsstrahlung, PDGid: "<<pdg<<" px: "<<px<<" py: "<<py<<" pz: "<<pz<<" mom: "<<mom<<" Creation code: "<<code<<std::endl;
	    bremsMom=mom;
	    Bremstree->Fill();
	  }

	  if(code==2){
	    //std::cout<<"Annihilation, PDGid: "<<pdg<<" px: "<<px<<" py: "<<py<<" pz: "<<pz<<" mom: "<<mom<<" Creation code: "<<code<<std::endl; 
	    annihilMom=mom;
            Annihiltree->Fill();
	  }

	  if(code==12){
	    //std::cout<<"Compton, PDGid: "<<pdg<<" px: "<<px<<" py: "<<py<<" pz: "<<pz<<" mom: "<<mom<<" Creation code: "<<code<<std::endl;  
	    comptMom=mom;
	    Compttree->Fill();
	  }

	  if(code==40){
	    //std::cout<<"Photoelectric, PDGid: "<<pdg<<" px: "<<px<<" py: "<<py<<" pz: "<<pz<<" mom: "<<mom<<" Creation code: "<<code<<std::endl;  
            photMom=mom;
            Phottree->Fill();
          }
	}

	} //if sid
      
    }//entries
    input->Close();
  }//loop over files
  std::cout<<"Number of photons not coming from brems, annihil, phot or compt: "<<counter<<std::endl;

  h1->Draw("");
  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
 
  if(branch==1){h1->SetFillColor(kGreen+2);}
 

  //if save to root file is 1, the histogram is not plotted on screen, it is stored in the root file 
   if(savetorootfile==1){
    output->Write();
    output->Close();
   }
  
  
}




void Readroot(std::string rootinput, int nbins, double xmin, double xmax, string Xtitlest, string Ytitlest, int branch){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);
  //No print prob
  //gStyle->SetOptFit(01111);
  //Print prob
  gStyle->SetOptFit(11111);

  TCanvas *c1 = new TCanvas("");
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  std::cout<<"Reading code from: "<<rootinput<<std::endl;

  TFile *input=new TFile(rootinput.c_str());
  TTree* treeBrems=(TTree*)input->Get("Bremstree");
  TTree* treeAnnihil=(TTree*)input->Get("Annihiltree");
  TTree* treePhot=(TTree*)input->Get("Phottree");
  TTree* treeCompt=(TTree*)input->Get("Compttree");

  float bremsMom, annihilMom, photMom, comptMom;
  treeBrems->SetBranchAddress("bremsMom",&bremsMom);
  treeAnnihil->SetBranchAddress("annihilMom",&annihilMom);
  treePhot->SetBranchAddress("photMom",&photMom);
  treeCompt->SetBranchAddress("comptMom",&comptMom);

  TH1F*h1 = new TH1F("TH1","", nbins, xmin, xmax);

  double binning = (xmax - xmin) / nbins;

  unsigned long entriesBrems=treeBrems->GetEntries();
  std::cout<<"entriesBrems: "<<entriesBrems<<std::endl;

  unsigned long entriesAnnihil=treeAnnihil->GetEntries();
  std::cout<<"entriesAnnihil: "<<entriesAnnihil<<std::endl;

  unsigned long entriesPhot=treePhot->GetEntries();
  std::cout<<"entriesPhot: "<<entriesPhot<<std::endl;
  
  unsigned long entriesCompt=treeCompt->GetEntries();
  std::cout<<"entriesCompt: "<<entriesCompt<<std::endl;

  if(branch==0){
    for(unsigned long i=0;i<entriesBrems;i++){
      treeBrems->GetEntry(i);
      h1->Fill(bremsMom);
    }
  }

  if(branch==1){
    for(unsigned long i=0;i<entriesAnnihil;i++){
      treeAnnihil->GetEntry(i);
      h1->Fill(annihilMom);
    }
  }

  if(branch==2){
    for(unsigned long i=0;i<entriesPhot;i++){
      treePhot->GetEntry(i);
      h1->Fill(photMom);
    }
  }
    if(branch==3){
      for(unsigned long i=0;i<entriesCompt;i++){
	treeCompt->GetEntry(i);
	h1->Fill(comptMom);
      }
    }


  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);

  if(branch==0){h1->SetFillColor(kOrange-3);h1->SetLineColor(kOrange-3);}
  if(branch==1){h1->SetFillColor(kOrange-2);}
  if(branch==2){h1->SetFillColor(kOrange-1);}
  if(branch==3){h1->SetFillColor(kOrange-4);}
  //h1->Draw("");

  //DO FIT
  
  //fit bremsstrahlung
  //double rangex1=0.02;
  double rangex1=0.04;
  double rangex2=1.809;

  //NO NORMALISED FITTING  
  //TF1*FitBrems = new TF1("FitBrems", "[0]*exp([1]*x)+[2]", rangex1, rangex2);
  //FitBrems->SetParameters(3.04314e+04,-6.84989,5.94630e+02);
  TF1*FitBrems = new TF1("FitBrems", "([0]/(exp([1]*x)+[2]))+[3]", rangex1, rangex2);
  FitBrems->SetParameters(3.66562e+03,2.2,-0.9947,3.23198e+02);
  h1->Fit(FitBrems,"ES0","",rangex1,rangex2);
  FitBrems->SetLineColor(kRed);
  FitBrems->SetLineStyle(2);
  //FitBrems->Draw("same");
  double Integral1 = h1->Integral(h1->FindFixBin(0), h1->FindFixBin(2), "");
  std::cout<<"Integral histo 0,2: "<<Integral1<<std::endl;
  double Integral2 = h1->Integral(h1->FindFixBin(xmin), h1->FindFixBin(xmax), "");
  std::cout<<"Integral histo "<<xmin<<","<<xmax<<": "<<Integral2<<std::endl;
  double Integral3 = h1->Integral();
  std::cout<<"Integral histo in full range...: "<<Integral3<<std::endl;
  double Integral4 = h1->Integral(h1->FindFixBin(0.04), h1->FindFixBin(2), "WIDTH");
  std::cout<<"Integral histo 0.04,2 (width): "<<Integral4<<std::endl;
  double Integral5 = h1->Integral(h1->FindFixBin(0.04), h1->FindFixBin(2), "");
  std::cout<<"Integral histo 0.04,2 (no width): "<<Integral5<<std::endl;


  //NORMALISED FITTING
  TH1F*hnorm1 = (TH1F*)(h1->Clone("TH1"));
  //hnorm1->Scale(1./hnorm1->Integral());
  hnorm1->Scale(1./Integral2);
  std::cout<<"Normalising to integral: "<<Integral2<<std::endl;
  //hnorm1->GetYaxis()->SetRangeUser(0., 0.002);
  hnorm1->Draw("HIST");
 
  //TF1*FitnormBrems = new TF1("FitnormBrems", "[0]*exp([1]*x)+[2]", rangex1, rangex2);
  //FitnormBrems->SetParameters(3.04314e+04,-6.84989,5.94630e+02);
  TF1*FitnormBrems = new TF1("FitnormBrems", "([0]/(exp([1]*x)+[2]))+[3]", rangex1, rangex2);
  FitnormBrems->SetParameters(1.75733e-02,2.20020e+00, -9.94744e-01, 1.54944e-03);

  TFitResultPtr r = hnorm1->Fit(FitnormBrems,"ES0","",rangex1,rangex2);
  TMatrixD cov = r->GetCovarianceMatrix();
  cov.Print();

  FitnormBrems->SetLineColor(kRed);
  FitnormBrems->SetLineStyle(2);
  FitnormBrems->Draw("same");

  double Chi2errors=hnorm1->GetFunction("FitnormBrems")->GetChisquare();
  std::cout<<"Chi2: "<<Chi2errors<<std::endl;

  //Calculate number of degrees of freedom (bins in fit range - fit parameters)
  double ndof = 0;
  for(int i = 0 ; i < nbins ; i++){
    double bincenter = hnorm1->GetBinCenter(i);
    double bincontent = hnorm1->GetBinContent(i);
    double binerror = hnorm1->GetBinError(i);
    std::cout<<sqrt(bincontent)<<" "<<binerror<<std::endl;
    if((bincenter > rangex1)&&(bincenter < rangex2)&&(bincontent!=0)&&(binerror!=0)){
      ndof++;
    }

  }
  ndof = ndof - 4 ; //number of fit parameters 
  std::cout<<"ndof: "<<ndof<<std::endl;

  double prob = TMath::Prob(Chi2errors,ndof);
  std::cout<<"Prob: "<<prob<<std::endl;

  double Integralnorm1 = hnorm1->Integral(hnorm1->FindFixBin(0), hnorm1->FindFixBin(2), "");
  std::cout<<"Integral norm histo 0,2: "<<Integralnorm1<<std::endl;
  double Integralnorm2 = hnorm1->Integral(hnorm1->FindFixBin(xmin), hnorm1->FindFixBin(xmax), "");
  std::cout<<"Integral norm histo -0.5,2: "<<Integralnorm2<<std::endl;
  double Integralnorm3 = hnorm1->Integral();
  std::cout<<"Integral norm histo -0.5,2: "<<Integralnorm3<<std::endl;
  double Integralnorm4 = hnorm1->Integral(hnorm1->FindFixBin(0.04), hnorm1->FindFixBin(2), "WIDTH");
  std::cout<<"Integral norm histo 0.04,2 (width): "<<Integralnorm4<<std::endl;
  double Integralnorm5 = hnorm1->Integral(hnorm1->FindFixBin(0.04), hnorm1->FindFixBin(2), "");
  std::cout<<"Integral norm histo 0.04,2 (no width): "<<Integralnorm5<<std::endl;
  std::cout<<""<<std::endl;
  double Integralfunc1 = FitnormBrems->Integral(0.04,2);
  std::cout<<"Integral func histo 0.04,2: "<<Integralfunc1<<std::endl;
  double ratio = Integralnorm4/Integralfunc1;
  std::cout<<"----Ratio histogram/function (width)---: "<<ratio<<std::endl;
  std::cout<<"----Ratio histogram/function (no width)---: "<<Integralnorm5/Integralfunc1<<std::endl;
  std::cout<<"Nbins: "<<nbins<<std::endl;

  std::stringstream streamratio;
  streamratio << std::fixed << setprecision(3) << ratio;
  std::string str_latex = "Histogram/Fit Integral = "+streamratio.str();
  char* char_latex = const_cast<char*>(str_latex.c_str());
  TLatex latex;
  latex.DrawLatexNDC(.35,.85,char_latex);


  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.55); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.32); //new y start position
  ps->SetY2NDC(0.8); //new y end position

  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats 
  h1->SetStats(0);
  //ps->GetLineWith("Entries")->SetTextColor(kBlue);

  /*  
  ps->GetLineWith("Entries")->SetTextSize(0.027);
  ps->GetLineWith("Mean")->SetTextSize(0.027);
  ps->GetLineWith("Std Dev")->SetTextSize(0.027);
  */

  c1->Modified();


  //c1->Print("ComptonSpectrumPhotVD15_new.png");
  //c1->Print("ComptonSpectrumPhotVD15_new.pdf");

  //input->Close(); doesn't work for plotting histogram
}



//================================================================
void ReadVD_rootfile(std::string ArtFiles_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 

  //Open txt
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
    //std::cout<<name<<std::endl;
  }


  //1 is to print momentum of photons of particles at VD, branch, savetorootfile
  //PrintHisto(file_name, 100, -1, 4, "p_{VD=15, #gamma} [MeV]", "Counts", 1, 1, "VD15BremsstrahlungPhotMomentum.root");
  
  //To print generated root file 
  //branch 0: Photons that come from bremsstrahlung
  Readroot("/data1/cgarcia/EleBeamGenPOTVDana/VDData/VD15BremsstrahlungPhotMomentum.root", 1000, 0.04, 2, "p_{bremsstrahlung #gamma, VD=15} [MeV]", "Normalised Counts/(0.00196 MeV)", 0);
  //branch 1: Photons that come from annihilation
  //Readroot("/data1/cgarcia/EleBeamGenPOTVDana/VDData/VD15BremsstrahlungPhotMomentum.root", 100, -1, 4, "p_{annihilation #gamma, VD=15} [MeV]", "Counts", 1);
  //branch 2: Photons that come from photoelectric effect 
  //Readroot("/data1/cgarcia/EleBeamGenPOTVDana/VDData/VD15BremsstrahlungPhotMomentum.root", 100, -0.5, 0.5, "p_{photoelectric effect #gamma, VD=15} [MeV]", "Counts", 2);
  //branch 3: Photons that come from compton effect                                                                                         
  //Readroot("/data1/cgarcia/EleBeamGenPOTVDana/VDData/VD15BremsstrahlungPhotMomentum.root", 100, -0.5, 0.5, "p_{compton effect #gamma, VD=15} [MeV]", "Counts", 3);
  readfile.close();
}

//================================================================
