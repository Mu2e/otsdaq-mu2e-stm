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

void root_to_csv() {

  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  // auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={0, 2000};
  double yy1[2]={0,1800};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 2000);
  graph1->GetYaxis()->SetRangeUser(0,1800);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("");
  //graph1->Draw("ap");

   //Abrimos el txt
  fstream readfile;
  readfile.open("run_00110.txt",ios::in);
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

  //Abro el csv para escribir los valores ADC del Eu
  std::string output_filename="Europium_ADCpeakspace.csv";
  std::ofstream output_file;
  output_file.open(output_filename);


  for (int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
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
      //output_file << peaks << "\n";
      output_file << peaks << "\n"; 
    }
  }//for int file


  //Close the csv
  readfile.close();
  output_file.close();



}
