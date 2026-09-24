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


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"

void Background110() {

  gROOT->SetStyle("ATLAS");
  
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);

  double xx1[2]={850,900};
  double yy1[2]={0,500};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("{}^{152}Eu");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  TH1F*h1 = new TH1F("TH1","", 2807, 0, 1600);//0.57 kev bin  

  //Abrimos el txt
  fstream readfile;
  readfile.open("/work/mu2e/data1/cgarcia/DATA/MWD_Analysis/RUN110/M1000L500/run_00110.txt",ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    cout<<name<<endl;
  }
  std::cout<<"Size: "<<file_name.size()<<std::endl;


  int xrays[12];
  for(int i=0;i<12;i++){
    xrays[i]=0;
  }

  
  for (int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    //cout<<file_name.size()<<endl;
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree->GetEntry(i);
      double peakEnergy = peaks*(-0.57);
      if(peakEnergy>30){
	h1->Fill(peakEnergy);}
       //Number of X-Rays inside the gaussian 0-800                                                                                        
        if(peakEnergy<45&&peakEnergy>35){xrays[0]++;}
        if(peakEnergy<125&&peakEnergy>117){xrays[1]++;}
        if(peakEnergy<249&&peakEnergy>238){xrays[2]++;}
        if(peakEnergy<348&&peakEnergy>338){xrays[3]++;}
        if(peakEnergy<418&&peakEnergy>404){xrays[5]++;}
        if(peakEnergy<450&&peakEnergy>438){xrays[6]++;}
        if(peakEnergy<786&&peakEnergy>774){xrays[4]++;}
        //Number of X-Rays inside the gaussian 800-2000                                                                                     
        if(peakEnergy<876&&peakEnergy>864){xrays[7]++;}
        if(peakEnergy<972&&peakEnergy>961){xrays[8]++;}
        if(peakEnergy<1097&&peakEnergy>1081){xrays[9]++;}
        if(peakEnergy<1120&&peakEnergy>1110){xrays[10]++;}
        if(peakEnergy<1416&&peakEnergy>1403){xrays[11]++;}

    }
  }//for int file


  h1->GetXaxis()->SetTitle("E (keV)");
  h1->SetTitle("");
  h1->SetStats(0);
  //h1->Draw("same");

  Int_t i;
  const Int_t nbins = 2807;
  Double_t xmin     = 0;
  Double_t xmax     = 1600;
  Double_t source[nbins];
  TH1F *d    = new TH1F("d","",nbins,xmin,xmax);

  TSpectrum *s = new TSpectrum();
 
  for (i = 0; i < nbins; i++) {source[i] = h1->GetBinContent(i + 1);}
 
  // Estimate the background
  s->Background(source,nbins,12,TSpectrum::kBackIncreasingWindow,
		TSpectrum::kBackOrder2,kFALSE,
		TSpectrum::kBackSmoothing3,kFALSE);
  
  // Draw the estimated background
  for (i = 0; i < nbins; i++) {d->SetBinContent(i + 1,source[i]);}
  d->SetLineColor(kRed);
  //d->Draw("SAME L");

  h1->Add(d,-1);
  h1->Draw("same"); 

   //0-800                                                                                                                                   
   std::cout<<"Number of X-Rays for the 40.1186 keV peak: "<<xrays[0]<<std::endl;
   std::cout<<"Number of X-Rays for the 121.78 keV peak: "<<xrays[1]<<std::endl;
   std::cout<<"Number of X-Rays for the 244.7 keV peak: "<<xrays[2]<<std::endl;
   std::cout<<"Number of X-Rays for the 344.28 keV peak: "<<xrays[3]<<std::endl;
   std::cout<<"Number of X-Rays for the 411.1165 keV peak: "<<xrays[4]<<std::endl;
   std::cout<<"Number of X-Rays for the 443.965 keV peak: "<<xrays[5]<<std::endl;
   std::cout<<"Number of X-Rays for the 778.91 keV peak: "<<xrays[6]<<std::endl;
   //800-2000                                                                                                                               
   std::cout<<"Number of X-Rays for the 867.380 keV peak: "<<xrays[7]<<std::endl;
   std::cout<<"Number of X-Rays for the 964.08 keV peak: "<<xrays[8]<<std::endl;
   std::cout<<"Number of X-Rays for the 1085.837 keV peak: "<<xrays[9]<<std::endl;
   std::cout<<"Number of X-Rays for the 1112.076 keV peak: "<<xrays[10]<<std::endl;
   std::cout<<"Number of X-Rays for the 1410.65 keV peak: "<<xrays[11]<<std::endl;


   //Counts after background substraction:

   double numberentries40 = h1->Integral(h1->FindFixBin(36), h1->FindFixBin(43), "");
   double numberentries121 = h1->Integral(h1->FindFixBin(116), h1->FindFixBin(126), "");
   double numberentries244 = h1->Integral(h1->FindFixBin(240), h1->FindFixBin(247), "");
   double numberentries344 = h1->Integral(h1->FindFixBin(337), h1->FindFixBin(352), "");
   double numberentries411 = h1->Integral(h1->FindFixBin(409), h1->FindFixBin(413), "");
   double numberentries443 = h1->Integral(h1->FindFixBin(442), h1->FindFixBin(448), "");
   double numberentries778 = h1->Integral(h1->FindFixBin(772), h1->FindFixBin(790), "");

   double numberentries867 = h1->Integral(h1->FindFixBin(866), h1->FindFixBin(876), "");
   double numberentries964 = h1->Integral(h1->FindFixBin(960), h1->FindFixBin(974), "");
   double numberentries1085 = h1->Integral(h1->FindFixBin(1080), h1->FindFixBin(1100), "");
   double numberentries1112 = h1->Integral(h1->FindFixBin(1105), h1->FindFixBin(1125), "");
   double numberentries1410 = h1->Integral(h1->FindFixBin(1402), h1->FindFixBin(1418), "");
   
   std::cout<<"Number of photons in 40 keV peak: "<<numberentries40<<std::endl;
   std::cout<<"Number of photons in 121 keV peak: "<<numberentries121<<std::endl;
   std::cout<<"Number of photons in 244 keV peak: "<<numberentries244<<std::endl;
   std::cout<<"Number of photons in 344 keV peak: "<<numberentries344<<std::endl;
   std::cout<<"Number of photons in 411 keV peak: "<<numberentries411<<std::endl;
   std::cout<<"Number of photons in 443 keV peak: "<<numberentries443<<std::endl;
   std::cout<<"Number of photons in 778 keV peak: "<<numberentries778<<std::endl;

   std::cout<<"Number of photons in 867 keV peak: "<<numberentries867<<std::endl;
   std::cout<<"Number of photons in 964 keV peak: "<<numberentries964<<std::endl;
   std::cout<<"Number of photons in 1085 keV peak: "<<numberentries1085<<std::endl;
   std::cout<<"Number of photons in 1112 keV peak: "<<numberentries1112<<std::endl;
   std::cout<<"Number of photons in 1410 keV peak: "<<numberentries1410<<std::endl;

   //c1->Print("EuBackgroundSubtracted.png");
}
