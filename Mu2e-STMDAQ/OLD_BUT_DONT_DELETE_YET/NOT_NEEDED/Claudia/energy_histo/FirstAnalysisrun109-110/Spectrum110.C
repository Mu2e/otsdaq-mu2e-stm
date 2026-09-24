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

void Spectrum110() {

  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  //  double xx1[2]={-800, 100};
  double xx1[2]={-2500,-1000};
  // double xx1[2]={-3000, 0};  
  double yy1[2]={0,80};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  // graph1->GetXaxis()->SetRangeUser(-800, 100);
  graph1->GetXaxis()->SetRangeUser(-2500,-1000);
  //graph1->GetXaxis()->SetRangeUser(-3000,0);     
  graph1->GetYaxis()->SetRangeUser(0,80);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //TH1F*h1 = new TH1F("TH1","", 1000, -800, 100);
  TH1F*h1 = new TH1F("TH1","", 1000, -2500, -1000);
  //TH1F*h1 = new TH1F("TH1","", 1000, -3000, 0);  

  //Abrimos el txt
  fstream readfile;
  readfile.open("run_00110.txt",ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .csv
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
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree->GetEntry(i);
      h1->Fill(peaks);
    }
  }//for int file


  //Fit Energy peak Europium
  /* TF1*Fitpeak1 = new TF1("Fitpeak1", "[0]*TMath::Gaus(x,[1],[2])", -150, 100);
  Fitpeak1->SetParameters(1.63885e+02,-6.18779e+01, -1.58347e+01 );

  TF1*Fitpeak2 = new TF1("Fitpeak2", "[0]*TMath::Gaus(x,[1],[2])", -240, -150);
  Fitpeak2->SetParameters(2.16464e+02 ,-1.94620e+02,-1.38816e+01);

  TF1*Fitpeak3 = new TF1("Fitpeak3", "[0]*TMath::Gaus(x,[1],[2])", -440, -340);
  Fitpeak3->SetParameters(5.35282e+01  ,-3.92728e+02 ,-2.19788e+01 );

  TF1*Fitpeak4 = new TF1("Fitpeak4", "[0]*TMath::Gaus(x,[1],[2])", -600, -500);
  Fitpeak4->SetParameters(1.03599e+02 ,-5.61632e+02 , 1.25747e+01);*/

  TF1*Fitpeak5 = new TF1("Fitpeak5", "[0]*TMath::Gaus(x,[1],[2])", -1400, -1200);                                                                
  Fitpeak5->SetParameters(4.36110e+01,-1.28797e+03,2.05245e+01 );                                                                                
  TF1*Fitpeak6 = new TF1("Fitpeak6", "[0]*TMath::Gaus(x,[1],[2])", -1700, -1500);                                            Fitpeak6->SetParameters(5.12069e+01  ,-1.59611e+03 ,1.32459e+01);                                                                              
  TF1*Fitpeak7 = new TF1("Fitpeak7", "[0]*TMath::Gaus(x,[1],[2])", -1900, -1750);                                                                  
  Fitpeak7->SetParameters(3.78894e+01 , -1.80148e+03, 1.37981e+01 );                                                                                 
  TF1*Fitpeak8 = new TF1("Fitpeak8", "[0]*TMath::Gaus(x,[1],[2])", -1900, -1750);                                                  
  Fitpeak8->SetParameters( 3.23390e+01 , -1.84193e+03, 1.97891e+01 );
  TF1*Fitpeak9 = new TF1("Fitpeak9", "[0]*TMath::Gaus(x,[1],[2])", -2400, -2300);
  Fitpeak9->SetParameters(5.39600e+01 ,-2.33767e+03, 1.01883e+01 );

  h1->GetXaxis()->SetTitle("ADC Counts");
  h1->SetTitle("");
  h1->SetStats(0);
  h1->Draw("same");


  /*  h1->Fit(Fitpeak1,"0","",-100, 20);
  h1->Fit(Fitpeak2,"0","",-220, -180);
  h1->Fit(Fitpeak3,"0","",-420, -360);
  h1->Fit(Fitpeak4,"0","",-600, -540);

  Fitpeak1->SetLineColor(kRed);
  Fitpeak1->SetLineStyle(2);
  Fitpeak1->Draw("same");

  Fitpeak2->SetLineColor(kRed);
  Fitpeak2->SetLineStyle(2);
  Fitpeak2->Draw("same");

  Fitpeak3->SetLineColor(kRed);
  Fitpeak3->SetLineStyle(2);
  Fitpeak3->Draw("same");

  Fitpeak4->SetLineColor(kRed);
  Fitpeak4->SetLineStyle(2);
  Fitpeak4->Draw("same");
  TLine *linemean1=new TLine(-6.18779e+01,0,-6.18779e+01,280);
  TLine *linemean2=new TLine(-1.94620e+02,0,-1.94620e+02,280);
  TLine *linemean3=new TLine(-3.92728e+02,0,-3.92728e+02,280);
  TLine *linemean4=new TLine( -5.61632e+02 ,0, -5.61632e+02 ,280);
  //c1->Print("ADCCounts_run00110.pdf","pdf");
  //c1->Print("ADCCounts_run00110.png","png");

  linemean1->Draw("same");
  linemean2->Draw("same");
  linemean3->Draw("same");
  linemean4->Draw("same");

  */
  //c1->Print("ADCCountsFit_run00110FirstPeaks.pdf","pdf");
  //c1->Print("ADCCountsFit_run00110FirstPeaks.png","png");


  h1->Fit(Fitpeak5,"0","",-1350, -1250);                                                                                                           
  h1->Fit(Fitpeak6,"0","",-1650, -1550);                                                                                                           
  h1->Fit(Fitpeak7,"0","",-1820, -1750);                                                                                                           
  h1->Fit(Fitpeak8,"0","",-1900, -1810);
  h1->Fit(Fitpeak9,"0","",-2400, -2300);                                                                                                                                   
  Fitpeak5->SetLineColor(kRed);                                                                                                                    
  Fitpeak5->SetLineStyle(2);                                                                                                                       
  Fitpeak5->Draw("same");                                                                                                                                                                                                                                                                   
  Fitpeak6->SetLineColor(kRed);                                                                                                                
  Fitpeak6->SetLineStyle(2);                                                                                                                       
  Fitpeak6->Draw("same");                                                                                                               
                                                                                                                                           
  Fitpeak7->SetLineColor(kRed);                                                                                                                    
  Fitpeak7->SetLineStyle(2);                                                                                                                       
  Fitpeak7->Draw("same");                                                                                                                          
                                                                                                                                                  
  Fitpeak8->SetLineColor(kRed);                                                                                                                    
  Fitpeak8->SetLineStyle(2);                                                                                                                       
  Fitpeak8->Draw("same"); 

  Fitpeak9->SetLineColor(kRed);
  Fitpeak9->SetLineStyle(2);
  Fitpeak9->Draw("same");

  TLine *linemean5=new TLine(-1.28797e+03  ,0,-1.28797e+03  ,80);                                                                                     
  TLine *linemean6=new TLine(-1.59611e+03 ,0,-1.59611e+03 ,80);                                                                                     
  TLine *linemean7=new TLine(-1.80148e+03 ,0,-1.80148e+03 ,80);                                                                                   
  TLine *linemean8=new TLine(-1.84193e+03  ,0,-1.84193e+03  ,80);  
  TLine *linemean9=new TLine(-2.33767e+03,0,-2.33767e+03,80);

  linemean5->Draw("same");                                                                                                                         
  linemean6->Draw("same");                                                                                                                         
  linemean7->Draw("same");                                                                                                                         
  linemean8->Draw("same");
  linemean9->Draw("same");
  //   c1->Print("ADCCountsFit_run00110SecondPeaks.pdf","pdf");                                                                                       
  // c1->Print("ADCCountsFit_run00110SecondPeaks.png","png");           
}
