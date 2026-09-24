#include <iostream>
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
#include<iomanip>

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
#include "TRandom3.h"

#define mm_to_cm 10
#define PI 3.14159265358979323846  /* pi */

//================================================================
//This program reads readGens tree
//Reads the root files from a txt and analyse the trees
//================================================================  
void PrintGraph(vector<string> file_name, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  gStyle->SetOptStat(1110);
  int palette_number = 55;
  gStyle->SetPalette(palette_number);
  //just for TH2D
  gStyle->SetPadRightMargin(0.14);
  //gStyle->SetPadRightMargin(0.1);

  double nparts_gen, PDGId, xMu2e, yMu2e, zMu2e, mom, px, py, pz, Etot, time;

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

  TGraph *gr = new TGraph ();
  TH2D *h = new TH2D("TH2D","",100,xmin,xmax,100,ymin,ymax);

  std::cout<<"---Loop over files---"<<std::endl;
  unsigned long int p=0;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<""<<std::endl;
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<path.c_str()<<std::endl;

   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readGens/treeGen");

    tree->SetBranchAddress("nparts_gen",&nparts_gen);
    tree->SetBranchAddress("PDGId",&PDGId);
    tree->SetBranchAddress("xMu2e",&xMu2e);
    tree->SetBranchAddress("yMu2e",&yMu2e);
    tree->SetBranchAddress("zMu2e",&zMu2e);
    tree->SetBranchAddress("mom",&mom);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("Etot",&Etot);
    tree->SetBranchAddress("time",&time);   
   

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;


    for(unsigned long i=0;i<entries;i++){

      
      tree->GetEntry(i);
      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
      }
      
      if((branch==0)&&(PDGId==22)){gr->SetPoint(p,xMu2e, yMu2e);p++;}
      if((branch==1)&&(PDGId==22)){h->Fill(xMu2e/mm_to_cm,yMu2e/mm_to_cm);

	/*std::cout<<"Entry: "<<i<<", pdg: "<<PDGId<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<" MeV, time: "<<time<<" ns, xmu2e= "<<xMu2e<<" ymu2e= "<<yMu2e<<", zmu2e= "<<zMu2e<<" mm"<<std::endl;*/}
      if((branch==2)&&(PDGId==22)&&((zMu2e/mm_to_cm)>615)){h->Fill(xMu2e/mm_to_cm,yMu2e/mm_to_cm);}

    }//entries    
    
    input->Close();
  }//for int file


  gr->SetTitle("");
  gr->GetXaxis()->SetTitle(Xtitle);
  gr->GetYaxis()->SetTitle(Ytitle);
  gr->SetMarkerStyle(7);
  gr->SetMarkerColor(kBlack);
  //gr->Draw("same,p");
  

  h->GetXaxis()->SetTitle(Xtitle);
  h->GetYaxis()->SetTitle(Ytitle);
  h->GetZaxis()->SetLabelSize(0.04);
  h->Draw("colz");


  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.62); //new x start position
  ps->SetX2NDC(0.82); //new x end position
  ps->SetY1NDC(0.76); //new y start position
  ps->SetY2NDC(0.9); //new y end position

  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats
  h->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);
  //ps->GetLineWith("Entries")->SetTextSize(0.027);
  c1->Modified();


  c1->Print("xyheatmap-stoppedmuons_example_genpos.png");
  //c1->Print("100Mxyphotonsgenerated_STgap.pdf");
}

//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //Removes title of the histogram (TH1)
  gStyle->SetOptStat(1110);

  std::fstream writefile;
  bool write_to_file=false;

  double POT=200000000;
  double POT_pulse=16000000;
    
  TFile*output;

  if(branch==9){
    std::string rootfile = "StoppedPositions.root";
    output=new TFile(rootfile.c_str(),"recreate");
  }

  if(write_to_file==true){
    writefile.open("ST_phot_postions_mom_sangle_099_1.txt",std::ios::out);
  }


  double nparts_gen, PDGId, xMu2e, yMu2e, zMu2e, mom, px, py, pz, Etot, time;

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
  
  TH1D*h1 = new TH1D("TH1","", nbins, Xrange[0], Xrange[1]);
  TH1F*hx = new TH1F("TH1x","", nbins, -3980, -3800);
  TH1F*hy = new TH1F("TH1y","", nbins, -80, 80);
  TH1F*hz = new TH1F("TH1z","", nbins, 5400, 6300);

  TH3D* hxyzgen=new TH3D("hxyz", "Position at Production; x_{POT}(Mu2e) [mm]; z_{POT}(Mu2e) [mm];y_{POT}(Mu2e) [mm]", 100, 3925, 3935,100, -6058, -6057, 100, -5,5);
  TH3D* hpxpypzgen=new TH3D("hpxpypzgen", "Momentum at Production; px [MeV]; pz [MeV]; py [MeV]", 100, -0.6, 0.6, 100, -0.6, 0.6, 100, -0.6, 0.6);

  std::cout<<"---Loop over files---"<<std::endl;

  double sigma_smearing=32; //ns
  
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<""<<std::endl;
    std::cout<<"------------------------------"<<std::endl;
    std::cout<<path.c_str()<<std::endl;

   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readGens/treeGen");

    tree->SetBranchAddress("nparts_gen",&nparts_gen);
    tree->SetBranchAddress("PDGId",&PDGId);
    tree->SetBranchAddress("xMu2e",&xMu2e);
    tree->SetBranchAddress("yMu2e",&yMu2e);
    tree->SetBranchAddress("zMu2e",&zMu2e);
    tree->SetBranchAddress("mom",&mom);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("Etot",&Etot);
    tree->SetBranchAddress("time",&time);   
   

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;
    
    double theta,costheta,phi;

    for(unsigned long i=0;i<entries;i++){

      
      tree->GetEntry(i);
      if(debugout==true){
	std::cout<<"------------Event: "<<i<<"------------"<<std::endl;
      }

      /*      if(i==0){
	      std::cout<<"PDGId: "<<PDGId<<" nparts_gen: "<<nparts_gen<<std::setprecision(20)<<" xMu2e: "<<xMu2e<<" yMu2e: "<<yMu2e<<" zMu2e: "<<zMu2e<<" px: "<<px<<" py: "<<py<<" pz: "<<pz<<" mom: "<<mom<<" Etot: "<<Etot<<" time: "<<time<<std::endl;
	      }*/

      costheta = pz/mom;
      theta = acos(costheta);
      phi = atan(py/px);
      //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
      if((px<0)&&(py>0)){
	//these angles are negative angles (-phi) in the 4th quadrant that should be in 2nd quadrant
	phi = PI + phi;
	//std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;
      }
      if((px<0)&&(py<0)){
	//These are positive angles in the 1st quadrant that should be in 3rd quadrant
	phi = PI + phi;
	//std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;
      }
      //Conversion so that 4th quadrant returns positive angles
      //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
      if((px>0)&&(py<0)){
	//these angles are negative angles (-phi) in the 4th quadrant that should be positive angles in the 4th quadrant
	phi = 2*PI + phi;
	//std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;
      }
      //std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;



      //if((branch==0)&&(PDGId==22)&&(zMu2e>0)){h1->Fill(mom); std::cout<<"zmu2e: "<<zMu2e<<std::endl;}
      if((branch==0)&&(PDGId==13)&&(zMu2e>0)){h1->Fill(mom);}
      //if((branch==0)&&((PDGId==11)||(PDGId==-11))&&(zMu2e>0)){h1->Fill(mom);}
      //if((branch==0)&&(PDGId==11)&&(zMu2e>0)){h1->Fill(mom);}
      //if((branch==0)&&(PDGId==-11)&&(zMu2e>0)){h1->Fill(mom);}
      //if(branch==0){h1->Fill(px);} 
      //if(branch==0){h1->Fill(py);} 
      //if(branch==0){h1->Fill(pz);}
      //if(branch==1){h1->Fill(nparts_gen);}
      //if(branch==1){h1->Fill(time);}
      if(branch==1){h1->Fill(gRandom->Gaus(time,sigma_smearing));}
      if(branch==2){hxyzgen->Fill(xMu2e,zMu2e,yMu2e);}
      //if((branch==3)&&((zMu2e/mm_to_cm)>615)){h1->Fill(xMu2e/mm_to_cm);}  
      //if((branch==3)&&((zMu2e/mm_to_cm)>615)){h1->Fill(yMu2e/mm_to_cm);} 
      //if(branch==3){h1->Fill(xMu2e/mm_to_cm);}
      if(branch==3){h1->Fill(yMu2e/mm_to_cm);}
      //if(branch==3){h1->Fill(zMu2e/mm_to_cm);}
      if(branch==4){hpxpypzgen->Fill(px,pz,py);}
      if(branch==5){h1->Fill(mom);}
      if(branch==6){ //theta
        //std::cout<<"px:"<<px<<" pz: "<<pz<<" mom: "<<mom<<" theta: "<<theta<<" rad, "<<theta*360/(2*PI)<<std::endl;
	h1->Fill(theta);
      }
      if(branch==7){ //phi
        h1->Fill(phi);
      }
      if(branch==8){ //cos(theta)
	//if(costheta<0.999996){
	//std::cout<<"costheta: "<<costheta<<std::endl;}
	h1->Fill(costheta);
      }
      if(branch==9){
	hx->Fill(xMu2e);
	hy->Fill(yMu2e);
	hz->Fill(zMu2e);
    }


      if(write_to_file==true){
        //pos in mm, mom in MeV
        writefile<<xMu2e<<" "<<yMu2e<<" "<<zMu2e<<" "<<mom<<" "<<px<<" "<<py<<" "<<pz<<std::endl;
	  }



    }//entries
    

    input->Close();
  }//for int file




  if(branch==9){
    output->WriteObject(hx, "hx");
    output->WriteObject(hy, "hy");
    output->WriteObject(hz, "hz");
  }


  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);

    
  if(branch==0){TAxis *X = h1->GetXaxis();
    X->SetNdivisions(5,3,0);
    h1->SetFillColor(kCyan-3);}
  if(branch==1){h1->SetFillColor(kGray);}
  if(branch==2){hxyzgen->SetMarkerColor(kBlack); hxyzgen->SetMarkerStyle(1);}
  if(branch==3){h1->SetLineColor(kBlue+1);}
  if(branch==4){hpxpypzgen->SetMarkerColor(kCyan-3);hpxpypzgen->SetMarkerStyle(1);h1->Scale(1./POT);}
  if(branch==5){h1->SetFillColor(kCyan-3); h1->SetFillStyle(3001); h1->SetLineColor(kCyan-3);}
  if(branch==6){h1->SetFillColor(kGreen+3); h1->SetLineColor(kGreen+3);}
  if(branch==7){h1->SetFillColor(kGreen-5); h1->SetLineColor(kGreen-5);}
  if(branch==8){
    h1->SetFillColor(kGreen-6); h1->SetLineColor(kGreen-6);
    TAxis *X = h1->GetXaxis();
    X->SetNdivisions(5,3,0);
    /*h1->GetXaxis()->SetLabelSize(0.02);*/}
  //hxyzgen->Draw("");
  //hpxpypzgen->Draw(""); 
  //gPad->SetLogy();

  h1->Draw("");  
  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.71); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.76); //new y start position
  ps->SetY2NDC(0.9); //new y end position
  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats
  h1->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);
  //ps->GetLineWith("Entries")->SetTextSize(0.027);
  c1->Modified();
  
  //h1->Scale(POT_pulse/h1->Integral());
  //h1->Scale(1./POT);
  //h1->Draw("HIST");
  
  writefile.close();
  if(branch==9){
    output->Close();
  }

  //c1->Print("y-stoppedmuons_example_genpos.png");
  //c1->Print("arrivaltimesPOTMDC2020_EleBeam_norm_smearing.png");
}



//================================================================
void ReadGeneratorPart_plot(std::string ArtFiles_location) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 

  //Open txt
  fstream readfile;
  readfile.open(ArtFiles_location,ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();
  int i=0;
  //Read each art file from txt
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    std::cout<<i<<" "<<name<<std::endl;
    i++;
  }

    
  //0 is to print total gen momentum if gen particle are gammas
  //PrintHisto(file_name, 100, -0.5, 6, 0, 3000, "p_{#gamma, z>0, VD=8} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, -5, 100, 0, 3000, "p_{#mu^{-}, z>0, VD=8} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, -5, 80, 0, 3000, "p_{e^{#pm}, z>0, VD=8} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, -5, 80, 0, 3000, "p_{e^{-}, z>0, VD=8} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, -5, 80, 0, 3000, "p_{e^{+}, z>0, VD=8} [MeV]", "Counts", 0); 
  //PrintHisto(file_name, 100, -0.003, 0.003, 0, 3000, "p_{x gen #gamma, VD=15} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, -0.003, 0.003, 0, 3000, "p_{y gen #gamma, VD=15} [MeV]", "Counts", 0);
  //PrintHisto(file_name, 100, 0.04, 0.06, 0, 3000, "p_{z gen #gamma, VD=15} [MeV]", "Counts", 0);
  //0 is to print total gen momentum if gen particle are protons
  //PrintHisto(file_name, 100, 8888, 8889, 0, 3000, "p_{gen, POT protons} [MeV]", "Counts", 0);
  //1 is to print POT producing e- hit
  //PrintHisto(file_name, 100, -1, 1, 0, 3000, "t (at PT)", "POT producing e^{-} hit", 1);  
  //PrintHisto(file_name, 40, -200, 200, 0, 3000, "t+smearing (at PT) [ns]", "(1.6#times10^{7} POT per pulse) / (10 ns)", 1);
  //1 is to print number of particles per event
  //PrintHisto(file_name, 100, 0, 5, 0, 3000, "# simulated POT protons per event [MeV]", "Counts", 1);
  //PrintHisto(file_name, 100, -300, 300, 0, 3000, "time POT", "Counts", 1); 
  //2 is to print TH3 of x, y, z of gen particles
  //PrintHisto(file_name, 100, 0, 5, 0, 3000, "TH3", "Counts", 2);
  //3 is to print z 
  //PrintHisto(file_name, 100, 544, 630, 0, 3000, "z_{gen-#gamma} ST [cm]", "Counts", 3);
  //PrintHisto(file_name, 100, -400, -376, 0, 3000, "x_{gen-#gamma} ST [cm]", "Counts", 3);
  //PrintHisto(file_name, 100, -10, 10, 0, 3000, "y_{gen-#gamma} ST [cm]", "Counts", 3);
  //PrintHisto(file_name, 100, -400, -380, 0, 3000, "x_{X-ray #gamma, ST} [cm]", "Counts", 3);
  //PrintHisto(file_name, 100, -10, 10, 0, 3000, "y_{X-ray #gamma, ST} [cm]", "Counts", 3);
  //4 is to print px py pz
  //PrintHisto(file_name, 100, 0, 5, 0, 3000, "TH3", "", 4);  
  //5 is to print momentum
  //PrintHisto(file_name, 100, 0, 0.7, 0, 3000, "p_{gen #gamma X-Rays} [MeV]", "", 5);
  PrintHisto(file_name, 100, 0, 2, 0, 3000, "E_{gen #gamma X-Rays} [MeV]", "Counts/POT", 5); 
  //6 is to print theta angle
  //PrintHisto(file_name, 100, -1, 3.5, 0, 500, "#theta_{#gamma} [rad]", "Counts", 6); 
  //7 is to print phi angle
  //PrintHisto(file_name, 100, -1, 9.45, 0, 500, "#phi_{#gamma} [rad]", "Counts", 7); 
  //8 is to print cos(theta)
  //PrintHisto(file_name, 100, 0.99998, 1, 0, 500, "cos(#theta_{#gamma, VD=15})", "Counts", 8);

 


  //0 is to print x, y coordinates in Mu2e system
  //PrintGraph(file_name, -4800, -3000, -1000, 1000, "x_{#gamma, VD=15}(Mu2e) [mm]", "y_{#gamma, VD=15}(Mu2e) [mm]", 0); 
  //1 is x,y position hit of gammas in VD (TH2)
  //PrintGraph(file_name,-490, -230, -100, 100, "x_{gen #gamma, VD=15}(Mu2e) [cm]", "y_{gen #gamma, VD=15}(Mu2e) [cm]", 1);
  //PrintGraph(file_name,-400, -376, -10, 10, "x_{gen-#gamma} ST [cm]", "y_{gen-#gamma} ST [cm]", 1);
  //PrintGraph(file_name,-410, -370, -15, 15, "x_{gen #gamma, ST (z>615cm)}(Mu2e) [cm]", "y_{gen #gamma, ST (z>615cm)} [cm]", 2);
  
  readfile.close();
}

//================================================================
