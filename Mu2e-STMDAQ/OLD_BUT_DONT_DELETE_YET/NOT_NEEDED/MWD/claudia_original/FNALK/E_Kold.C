//Plotting E Spectrum of Cs and Eu together
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
#include "TLegend.h"
#include "TLine.h"

void E_K(std::string pathfile) {
  gROOT->SetStyle("ATLAS");
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1");

  double xx1[2]={100,5000}; 
  //double xx1[2]={1400,1600}; 
  double yy1[2]={0,150}; 

  //double xx1[2]={600,3000};
  //double yy1[2]={0,700};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");

  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("Counts / (427.0787s)");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  int nbins = 5000;
  double xmin_histo=0;
  double xmax_histo=5000;
  //Change the binning fopr TSpectrum
  /*  int nbins=200;
  double xmin_histo=0;
  double xmax_histo=3000;*/

  //Best K calibration
  //double p0 = -192.8;
  //double p1 = -2.552;

  //double p0 = -412;
  //double p1 = -2.411;

  double p0 = -300.9;
  double p1 = -2.543;

  TH1F*h1 = new TH1F("TH1","", nbins, xmin_histo, xmax_histo);//0.57kev bin

  //Abrimos el txt
  fstream readfile;
  readfile.open(pathfile ,ios::in);
  //Potassium(?)
  //readfile.open("/data1/STM_VST_DATA/MWD_K_Data/M400L200_notrigcut/PotassiumPeaks_notrigcut.txt",ios::in);
  //Cosmics
  //readfile.open("/data1/STM_VST_DATA/MWD_Cosmics_Data/M400L200_notrigcut/CosmicPeaks_notrigcut.txt",ios::in);

  string name;
  vector<string> file_name;
  file_name.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    //cout<<name<<endl;
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
      
      //Using old roots Liverpool Calibration M=8000 L=1000
      //double peakEnergy=(peaks-0.354947)/(-1.75514);
  
      //Using Liverpool Calibration with M=400, L=200
      //double peakEnergy=(peaks-1.738)/(-1.767);

      //Using Liverpool Calibration with M=8000, L=1000
      //double peakEnergy=(peaks+3.392)/(-1.753);

      //Using Liverpool Calibration with M=1000, L=500
      //double peakEnergy=(peaks-2.73)/(-1.76);

      //Simple calibration
      //double peakEnergy=(peaks)*(-0.57); 
      //double peakEnergy=(peaks + 1.76)/(-1.785);
      
      //Potassium calibration
      //double peakEnergy = (peaks+272.671)/(-2.54344);

      //Potassium best calibration
      double peakEnergy = (peaks-p0)/p1;

      //Cut at 30 keV  
     if(peakEnergy>30){
      h1->Fill(peakEnergy);
      }
    }
  }//for int file


  h1->GetXaxis()->SetTitle("E [keV]");
  h1->SetTitle("");
  h1->SetStats(0);
  h1->SetLineColor(kRed+1);
  //h1->SetFillColor(kRed+1);
  h1->SetLineWidth(2);
  h1->Draw("same");


  TH1* h1peak = (TH1*)h1->Clone();
  h1peak->SetName("h1peak");

  //Remove Background
  Double_t source[nbins];
  TH1F *d    = new TH1F("d","",nbins,xmin_histo,xmax_histo);

  TSpectrum *s = new TSpectrum();
  for (int i = 0; i < nbins; i++) {source[i] = h1peak->GetBinContent(i + 1);}
  
  s->Background(source,nbins,2,TSpectrum::kBackIncreasingWindow,
                TSpectrum::kBackOrder2,kFALSE,
                TSpectrum::kBackSmoothing3,kFALSE);
  // Draw the estimated background
  for (int i = 0; i < nbins; i++) {d->SetBinContent(i + 1,source[i]);}
  d->SetLineColor(kBlue);
  //d->Draw("SAME L");


  double integral_fullhisto = h1->Integral(h1->FindFixBin(xmin_histo), h1->FindFixBin(xmax_histo));
  std::cout<<"(back+peaks) Integral between "<<xmin_histo<<" and "<<xmax_histo<<" keV: "<<integral_fullhisto<<std::endl;

  double range1 = 600;
  double range2 = 3000;
  double integral_range = h1->Integral(h1->FindFixBin(range1), h1->FindFixBin(range2));
  std::cout<<"(back+peaks) Integral between "<<range1<<" and "<<range2<<" keV: "<<integral_range<<std::endl;

  double integral_back_range = d->Integral(d->FindFixBin(range1), d->FindFixBin(range2));
  std::cout<<"(back) Integral between "<<range1<<" and "<<range2<<" keV: "<<integral_back_range<<std::endl;

  gPad->SetLogy();

  double E[3]={1460.8,511,2614.51};
  double EbindGe = 11.103;
  E[0]=E[0]-EbindGe;
  E[1]=E[1]-EbindGe;
  E[2]=E[2]-EbindGe;

  TLatex T1;
  T1.DrawLatexNDC(.4,.7, "1460.8 keV");
  T1.DrawLatexNDC(.25,.85, "511 keV");
  T1.DrawLatexNDC(.6,.55, "2614.51 keV");

  TLine *lineK=new TLine(E[0],yy1[0],E[0],yy1[1]);
  lineK->SetLineColor(kBlue);
  lineK->SetLineWidth(2);
  lineK->SetLineStyle(3); 
  lineK->Draw("same");

  TLine *line511=new TLine(E[1],yy1[0],E[1],yy1[1]);
  line511->SetLineColor(kBlue);
  line511->SetLineWidth(2);
  line511->SetLineStyle(3);
  line511->Draw("same");

  TLine *linePb=new TLine(E[2],yy1[0],E[2],yy1[1]);
  linePb->SetLineColor(kBlue);
  linePb->SetLineWidth(2);
  linePb->SetLineStyle(3);
  linePb->Draw("same");
  //auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
  //leg1->AddEntry(lineK, "1460.8 keV","l");


   auto leg = new TLegend(0.6,0.8,0.9,0.9);
  leg->AddEntry(h1, "{}^{40}K + Cosmics","f");
  //leg1->Draw("same")
  leg->Draw("same");  

  c1->Print("FitK_Spectrum_error10ADC_E-bindE_log.png","png");
}
