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

void E_Cs_Eu() {

  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={0, 2000};
  double yy1[2]={0,1800};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 2000);
  graph1->GetYaxis()->SetRangeUser(0,1800);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  TH1F*h1 = new TH1F("TH1","", 1000, 0, 2000);

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
      h1->Fill((peaks-6.50022)/(-1.66617));
    }
  }//for int file




  TH1F*h2 = new TH1F("TH2","", 1000, 0, 2000);

  fstream readfile2;
  readfile2.open("run_00109.txt",ios::in);
  string name2;
  vector<string> file_name2;
  file_name2.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .root                                                      
  while(1){
    readfile2>>name2;
    file_name2.push_back(name2);
    if(readfile2.eof())break;
    //cout<<name<<endl;                                                                                                     
  }

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
      if((peaks-6.50022)/(-1.66617)<700&&(peaks-6.50022)/(-1.66617)>600){
	h2->Fill((peaks-6.50022)/(-1.66617));}
    }
  }//for int file         


  TLine *line40=new TLine(40.1186,0,40.1186,1800);                                                                      
  line40->Draw("same");                                                                                                     
  auto leg1 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg1->AddEntry(line40, "40.1186 keV","l");  
                                                                                                                          
  TLine *line121=new TLine(121.78,0,121.78,1800);                                                                           
  line121->Draw("same");                                                                                                    
  auto leg2 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg2->AddEntry(line121, "121.78 keV","l");                                                                                
                                                                                                                            
  TLine *line244=new TLine(244.7,0,244.7,1800);                                                                             
  line244->Draw("same");                                                                                                    
  auto leg3 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg3->AddEntry(line244, "244.7 keV","l");                                                                                 
                                                                                                                            
  TLine *line344=new TLine(344.28,0,344.28,1800);                                                                           
  line344->Draw("same");                                                                                                    
  auto leg4 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg4->AddEntry(line244, "344.28 keV","l");                                                                                
                                                                                                                            
  TLine *line778=new TLine(778.91,0,778.91,1800);                                                                           
  line778->Draw("same");                                                                                                    
  auto leg5 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg5->AddEntry(line778, "778.91 keV","l");                                                                                
                                                                                                                            
  TLine *line964=new TLine(964.08,0,964.08,1800);                                                                           
  line964->Draw("same");                                                                                                    
  auto leg6 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg6->AddEntry(line964, "964.08 keV","l");                                                                                
                                                                                                                            
  TLine *line1085=new TLine(1085.837,0,1085.837,1800);                                                                      
  line1085->Draw("same");                                                                                                   
  auto leg7 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg7->AddEntry(line1085, "1085.837 keV","l");                                                                             
                                                                                                                            
  TLine *line1112=new TLine(1112.076,0,1112.076,1800);                                                                      
  line1112->Draw("same");                                                                                                   
  auto leg8 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg8->AddEntry(line1112, "1112.076 keV","l");                                                                             
                                                                                                                            
  TLine *line1408=new TLine(1408.13,0,1408.13,1800);                                                                        
  line1408->Draw("same");   
  auto leg9 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg9->AddEntry(line1408, "1408.013 keV","l");                                                                             

  TLine *line661=new TLine(661.7,0,661.7,1800);
  line661->Draw("same");
  auto leg10 = new TLegend(0.1,0.7,0.48,0.9);
  leg10->AddEntry(line661, "661.7 keV","l");
  
  h1->GetXaxis()->SetTitle("E (keV)");
  h1->SetTitle("");
  h1->SetStats(0);
  h1->SetFillColor(kGreen-9);
  h1->Draw("same");

  h2->GetXaxis()->SetTitle("E (keV)");
  h2->SetTitle("");
  h2->SetStats(0);
  h2->SetFillColor(kRed-9);
  h2->Draw("same");

  auto leg = new TLegend(0.1,0.7,0.48,0.9);
  leg->AddEntry(h1, "{}^{152}Eu","f");
  leg->AddEntry(h2, "{}^{137}Cs","f");
   leg1->Draw("same");
  leg2->Draw("same");
  leg3->Draw("same");
  leg4->Draw("same");
  leg5->Draw("same");
  leg6->Draw("same");
  leg7->Draw("same");
  leg8->Draw("same");
  leg9->Draw("same");
  leg10->Draw("same"); 
  leg->Draw("same");  
//  c1->Print("EnergyEu_run00110.pdf","pdf");
  // c1->Print("EnergyEu_run00110.png","png");  
}
