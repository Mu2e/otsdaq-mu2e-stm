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
#include <random>

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
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TGraph2D.h"
#include "TH3.h"
#include "TH3D.h"
#include "TRandom.h"

#define mm_to_cm 10
#define PI 3.14159265358979323846  /* pi */


//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int VDnumber, int branch){

  double POT= 200000000;
  bool debugout=false;
  
  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //Removes title of the histogram (TH1)
  //gStyle->SetOptStat(1110);
  //gStyle->SetOptStat(111110);
  //Three significant digits, doesn't work for entries
  //gStyle->SetStatFormat("6.3g"); 

  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

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
  double countentries =0;
  TH1F*h1 = new TH1F("TH1","", nbins, Xrange[0], Xrange[1]);
  TH1F*h2 = new TH1F("TH2","", nbins, Xrange[0], Xrange[1]);
  TH1F*h3 = new TH1F("TH3","", nbins, Xrange[0], Xrange[1]);
  TH1F*h4 = new TH1F("TH4","", nbins, Xrange[0], Xrange[1]);
 
  
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    //for (long unsigned int file=0;file<1;file++){
    string path;
    path=file_name[file];
    std::cout<<"file #: "<<file<<" "<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");
    //TTree* tree=(TTree*)input->Get("readvd/ntvdext");

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
    countentries=countentries+entries;
    std::cout<<"entries: "<<entries<<std::endl;

     
    for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);

      if(sid==VDnumber){

	double mom = sqrt(px*px+py*py+pz*pz);
	double pt = sqrt(px*px+py*py);
	double costheta = pz/mom;

	  
	if((debugout==true)/*&&(file==0)*/){
	  std::cout<<"Virtual Detector number: "<<sid<<std::endl;
	  std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" mom: "<<mom<<" MeV, time: "<<time<<std::setprecision(30)<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm, creation code: "<<code<<" trk: "<<trk<<std::endl;
	  //if((file==0)&&(i<=200000)){std::cout<<"Evt: "<<evt<<" VDid: "<<sid<<" pdg: "<<pdg<<" ke: "<<ke<<" MeV "<<"|p|: "<<mom<<" z: "<<z<<" mm "<<"thit: "<<time<<" ns"<<std::endl;}
	  //if((pdg!=11)&&(pdg!=-11)){std::cout<<"pdg: "<<pdg<<" WARNING!!: Not e- or e+ at VD 8, should remove this particle from analysis"<<std::endl;}
	  //if((ke>80)&&((pdg==11)||(pdg==-11))){std::cout<<"pdg: "<<pdg<<" with ke>80MeV, ke= "<<ke<<" MeV"<<std::endl;}
	  //if(time>500){std::cout<<"pdg: "<<pdg<<" with arrival time>500ns, time= "<<time<<" ns"<<std::endl;}
	  //if((px>50)||(py>50)||(pz>100)){std::cout<<"pdg: "<<pdg<<" with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<std::endl;}
	  //if((pdg!=11)&&(pdg!=-11)&&(pz>100)){std::cout<<"pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<"MeV, time: "<<time<<" ns"<<std::endl;} 
	  //if((pdg!=11)&&(pdg!=-11)&&(time>500)){std::cout<<"pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<"MeV, time: "<<time<<" ns"<<std::endl;} 
	  //if((pdg==11)||(pdg==-11)){std::cout<<"pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<"MeV, time: "<<time<<" ns, xmu2eVD8= "<<x<<" ymu2eVD8= "<<y<<", zmu2eVD8= "<<z<<" mm"<<std::endl;}
	}


	if((branch==0)&&((pdg==11)||(pdg==-11))){h1->Fill(mom);/*std::cout<<"Event: "<<i<< " VD: "<<sid<<", Pdgid: "<<pdg<<", Creation Code: "<<code<<" mom: "<<mom<<", trk: "<<trk<<", x: "<<x<<" y: "<<y<<" z: "<<z<<" time: "<<time<<std::endl;*/}
	if((branch==0)&&(pdg==22)){h2->Fill(mom);}

	
	//if((branch==1)&&(pdg==13)){h1->Fill(mom);}
	//if((branch==1)&&(pdg==-211)){h2->Fill(mom);}
	bool smearing=true;
	double sigma_smearing = 32; //ns
  
	if((branch==1)&&(pdg==13)){if(smearing==true){double t = gRandom->Gaus(time,sigma_smearing); h1->Fill(time);}}
        if((branch==1)&&(pdg==-211)){if(smearing==true){double t = gRandom->Gaus(time,sigma_smearing); h2->Fill(time);}}
	
      } //if sid
   
    }//entries

    input->Close();
  }//for int files

  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  //h1->SetFillStyle(3001);

   if(branch==0){
    h1->SetFillColor(kBlue);
    h1->SetMarkerColor(kBlue);
    h1->SetLineColor(kBlue);


    h2->SetFillColor(kRed);
    h2->SetMarkerColor(kRed);
    h2->SetLineColor(kRed);
  }

   
  if(branch==1){
    h1->SetFillColor(kBlue-6);
    h1->SetMarkerColor(kBlue-6);
    h1->SetLineColor(kBlue-6);
    h1->SetFillStyle(3354);

    h2->SetFillColor(kMagenta-6);
    h2->SetMarkerColor(kMagenta-6);
    h2->SetLineColor(kMagenta-6);
    h2->SetFillStyle(3345);
  }
  /*
  if(branch==1){
    TAxis *X = h1->GetXaxis();
    X->SetNdivisions(5,3,0);
    h1->SetFillColor(kGreen+2);
    //h1->SetFillColor(kOrange-3);
  }
  */
  
  double integral2 = double(h2->GetEntries()) / POT;
  double error2 = sqrt(double(h2->GetEntries())) / POT;
  double integral1= double(h1->GetEntries()) / POT;
  double error1 = sqrt(double(h1->GetEntries())) / POT;
  
    
    if(branch==0){
      c1->SetLogy();
      h1->Scale(1./POT);
      h1->GetYaxis()->SetRangeUser(0.000000001,0.1);
      h1->Draw("p");
      
      h2->Scale(1./POT);
      h2->Draw("p, same");

      c1->Update();
      
      std::cout<<"e+e-/POT: "<<integral1<<"+-"<<error1<<std::endl;
      std::cout<<"gamma/POT: "<<integral2<<"+-"<<error2<<std::endl;
  
      std::stringstream stream_photPOT, stream_photPOT_error, stream_ePOT, stream_ePOT_error;

      stream_photPOT << std::fixed << std::setprecision(5) << integral2;
      stream_photPOT_error << std::fixed << std::setprecision(0) << (error2*100000);
      stream_ePOT << std::fixed << std::setprecision(5) << integral1;
      stream_ePOT_error << std::fixed << std::setprecision(0) << (error1*100000);

      std::string name_e = "Integrated e^{#pm}/POT: "+stream_ePOT.str()+"["+stream_ePOT_error.str()+"]";
      std::string name_phot = "Integrated #gamma/POT: "+stream_photPOT.str()+"["+stream_photPOT_error.str()+"]";
      /*
      std::string str_latex = "#splitline{#scale[1]{#color[4]{"+name_e+"}}}{#scale[1]{#color[2]{"+name_phot+"}}}";
      char* char_latex = const_cast<char*>(str_latex.c_str());
      TLatex latex;
      latex.DrawLatexNDC(.25,.82,char_latex);
      */
      std::string str_latex1 = "#scale[1]{#color[4]{"+name_e+"}}";
      char* char_latex1 = const_cast<char*>(str_latex1.c_str());

       std::string str_latex2 = "#scale[1]{#color[2]{"+name_phot+"}}";
       char* char_latex2 = const_cast<char*>(str_latex2.c_str());

       auto leg = new TLegend(0.45,0.75,0.9,0.9);
       leg->AddEntry(h1, char_latex1 ,"pe");
       leg->AddEntry(h2, char_latex2 ,"pe");
       leg->Draw("same");
  }

    
    if(branch==1){
      
      gPad->SetLogy();
      h1->Scale(1./POT);
      h1->Draw("HIST");
	
      h2->Scale(1./POT);
      h2->Draw("HIST,same");
    	
      std::cout<<"mu-/POT: "<<integral1<<"+-"<<error1<<std::endl;
      std::cout<<"pi-/POT: "<<integral2<<"+-"<<error2<<std::endl;

      std::stringstream stream_piPOT, stream_piPOT_error, stream_muPOT, stream_muPOT_error;

      stream_piPOT << std::fixed << std::setprecision(3) << integral2;
      stream_piPOT_error << std::fixed << std::setprecision(3) << error2;
      stream_muPOT << std::fixed << std::setprecision(3) << integral1;
      stream_muPOT_error << std::fixed << std::setprecision(3) << error1;

      std::string name_mu = "Tot. #mu^{-}/POT yield: "+stream_muPOT.str()+" #pm "+stream_muPOT_error.str()+"%";
      std::string name_pi = "Tot. #pi/POT yield: "+stream_piPOT.str()+" #pm "+stream_piPOT_error.str()+"%";

      std::string str_latex = "#splitline{#scale[1]{#color[4]{"+name_mu+"}}}{#scale[1]{#color[6]{"+name_pi+"}}}";
      char* char_latex = const_cast<char*>(str_latex.c_str());
      TLatex latex;
      //latex.DrawLatexNDC(.25,.82,char_latex);

       auto leg = new TLegend(0.75,0.75,0.9,0.9);
      leg->AddEntry(h1, "#mu^{-}" ,"f");
      leg->AddEntry(h2, "#pi^{-}" ,"f");
      leg->Draw("same");
  }
    gPad->RedrawAxis();

    //c1->Print("MDC2020MuonsEle_momFlashPhotonsEplusEminusVD10_100MeV_log.png"); 
    c1->Print("MDC2020MuonsEle_timesmearingFlashMu_PiVD9_1000ns_log.png");
    //c1->Print("VD10_115NuclearCapture_time.pdf");
}



//================================================================
void ReadVD_2hist(std::string ArtFiles_location, int VDnumber ) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 

  std::string p_gamma_name = "p_{#gamma, VD="+std::to_string(VDnumber)+"} [MeV]";
  char* p_gamma_name_char = const_cast<char*>(p_gamma_name.c_str());

  std::string x_gamma_name = "x_{#gamma, VD="+std::to_string(VDnumber)+"} [cm]";
  char* x_gamma_name_char = const_cast<char*>(x_gamma_name.c_str());

  std::string y_gamma_name = "y_{#gamma, VD="+std::to_string(VDnumber)+"} [cm]";
  char* y_gamma_name_char = const_cast<char*>(y_gamma_name.c_str());
  
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
    std::cout<<name<<std::endl;
  }




  
  //0 is to print momentum of e+, e- and gamma at VD
  //PrintHisto(file_name, 500, 0, 100, 0, 500, "p_{ST exit, VD=10} [MeV]", "Counts/POT", VDnumber, 0); 

  //0 is to print momentum of pi, mu and gamma at VD
  //PrintHisto(file_name, 500, 0, 150, 0, 500, "p_{ST exit, VD=10} [MeV]", "Counts/POT", VDnumber, 1);
  //PrintHisto(file_name, 500, 0, 150, 0, 500, "p_{ST entrance, VD=9} [MeV]", "Counts/POT", VDnumber, 1);
  //PrintHisto(file_name, 500, 0, 150, 0, 500, "p_{DS entrance, VD=8} [MeV]", "Counts/POT", VDnumber, 1); 
  //PrintHisto(file_name, 100, 0, 1000, 0, 500, "t_{DS entrance, VD=8}+smearing [ns]", "(Counts / 10ns)/POT", VDnumber, 1);
   PrintHisto(file_name, 100, 0, 1000, 0, 500, "t_{ST entrance, VD=9}+smearing [ns]", "(Counts / 10ns)/POT", VDnumber, 1);
   
  readfile.close();
}

//================================================================
