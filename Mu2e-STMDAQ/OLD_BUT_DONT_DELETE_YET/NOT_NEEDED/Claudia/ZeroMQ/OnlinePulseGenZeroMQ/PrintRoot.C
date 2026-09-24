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

void PrintRoot() {

  auto c1= new TCanvas("c1");
  c1->SetGrid();


  double xx1[2]={0, 6000};
  double yy1[2]={-1500, 100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 6000);
  graph1->GetYaxis()->SetRangeUser(-1500,100);
  graph1->GetXaxis()->SetTitle("time (#mus)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->Draw("ap");

  vector<double> time;
  TGraph* gr = new TGraph();
  int j=0;
  //Abrimos el txt
  fstream readfile;
  readfile.open("pulsegen.txt",ios::in);
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
  
for (int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    //cout<<file_name.size()<<endl;
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    int16_t ADC;

    tree->SetBranchAddress("ADC",&ADC);

    unsigned long entries=tree->GetEntries();
    
    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
           tree->GetEntry(i);
	   time.push_back(0.0027*j);
	   gr->SetPoint(j,time.at(j),ADC);
	   j++;
    }
  }//for int file

gr->Draw("p*");



}
