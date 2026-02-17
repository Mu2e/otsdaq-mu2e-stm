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
#include "TAxis.h"
#include "TH1F.h"

void SubtractHistos(std::string pathfile1, std::string pathfile2) {
  gROOT->SetStyle("ATLAS");

  auto c1= new TCanvas("c1");

  double xx1[2]={-8000, 0};
  double yy1[2]={0,120};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  //Complete
  TH1F*h1 = new TH1F("TH1","", 1000, -8000, 0);
  TH1F*h2 = new TH1F("TH2","", 1000, -8000, 0);
  TH1F*h3 = new TH1F("TH3","", 1000, -8000, 0);

  //Abrimos el txt
  fstream readfile1;
  readfile1.open(pathfile1,ios::in);
  string name1;
  vector<string> file_name1;
  file_name1.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .csv
  while(1){
    readfile1>>name1;
    file_name1.push_back(name1);
    if(readfile1.eof())break;
    cout<<name1<<endl;
  }
  std::cout<<"Size: "<<file_name1.size()<<std::endl;
  for (int file=0;file<(file_name1.size()-1);file++){
    string path1;
    path1=file_name1[file];
    //cout<<file_name.size()<<endl;
    cout<<path1.c_str()<<endl;

    TFile *input1=new TFile(path1.c_str());
    TTree* tree1=(TTree*)input1->Get("treeADC");
    double peaks;

    tree1->SetBranchAddress("peaks",&peaks);

    unsigned long entries1=tree1->GetEntries();

    cout<<"entries: "<<entries1<<endl;
    for(unsigned long i=0;i<entries1;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree1->GetEntry(i);
      h1->Fill(peaks);
    }
  }//for int file





  //Abrimos el txt
  fstream readfile2;
  readfile2.open(pathfile2,ios::in);
  string name2;
  vector<string> file_name2;
  file_name2.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .csv
  while(1){
    readfile2>>name2;
    file_name2.push_back(name2);
    if(readfile2.eof())break;
    cout<<name2<<endl;
  }
  std::cout<<"Size: "<<file_name2.size()<<std::endl;
  for (int file=0;file<(file_name2.size()-1);file++){
    string path2;
    path2=file_name2[file];
    //cout<<file_name.size()<<endl;
    cout<<path2.c_str()<<endl;

    TFile *input2=new TFile(path2.c_str());
    TTree* tree2=(TTree*)input2->Get("treeADC");
    double peaks;

    tree2->SetBranchAddress("peaks",&peaks);

    unsigned long entries2=tree2->GetEntries();

    cout<<"entries: "<<entries2<<endl;
    for(unsigned long i=0;i<entries2;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree2->GetEntry(i);
      h2->Fill(peaks);
    }
  }//for int file


  h1->GetXaxis()->SetTitle("ADC Counts");
  h1->SetTitle("");
  //h1->Draw("same");
  h2->GetXaxis()->SetTitle("ADC Counts");
  h2->SetTitle("");
  //h2->Draw("same");


  //Subtract bin by bin
  TAxis *xaxis2 = h1->GetXaxis();
  TAxis *xaxis1 = h2->GetXaxis();
  int nbins1 = xaxis1->GetNbins();
  int nbins2 = xaxis2->GetNbins();
  std::cout<<"nbins1: "<<nbins1<<" nbins2: "<<nbins2<<std::endl;
  for(int bin=0;bin<nbins1;bin++){
    double k = h1->GetBinContent(bin);
    double c = h2->GetBinContent(bin);
    double s = k-c;
    std::cout<<"K: "<<k<<" Cosmics: "<<c<<std::endl;
    std::cout<<"Final bin content: "<<s<<std::endl;
    h3->SetBinContent(bin,s);
  }

  h3->Draw("same");

  double integral_fullhisto1 = h1->Integral(h1->FindFixBin(xx1[0]), h1->FindFixBin(xx1[1]));
  std::cout<<"(back+peaks, source) Integral between "<<xx1[0]<<" and "<<xx1[1]<<" keV: "<<integral_fullhisto1<<std::endl;

  //Subtraction
  //h1->Add(h2,-1);
  //h1->Draw("same");

  double integral_fullhisto2 = h2->Integral(h2->FindFixBin(xx1[0]), h2->FindFixBin(xx1[1]));
  std::cout<<"(back+peaks, cosmics) Integral between "<<xx1[0]<<" and "<<xx1[1]<<" keV: "<<integral_fullhisto2<<std::endl;

  double integral_fullhisto3 = h3->Integral(h3->FindFixBin(xx1[0]), h3->FindFixBin(xx1[1]));
  std::cout<<"(source-Cosmics) Integral between "<<xx1[0]<<" and "<<xx1[1]<<" keV: "<<integral_fullhisto3<<std::endl;


  //linemean->Draw("same");
  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(h1,"#Delta data(Source-Cosmic)","F");
  legend->Draw("same");

  //gPad->SetLogy();

  c1->Print("ADC_Potassium-CosmicsSpectrum_65files.png","png"); 

}
