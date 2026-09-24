
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



  double xx1[2]={-800, -30};
  //double xx1[2]={-2500,-1000};
  // double xx1[2]={-3000, 0};  
  double yy1[2]={0,800};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
   graph1->GetXaxis()->SetRangeUser(-800, -30);
  //graph1->GetXaxis()->SetRangeUser(-2500,-1000);
  //graph1->GetXaxis()->SetRangeUser(-3000,0);     
  graph1->GetYaxis()->SetRangeUser(0,800);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  TH1F*h1 = new TH1F("TH1","", 1000, -800, 0);
  //TH1F*h1 = new TH1F("TH1","", 1000, -2500, -1000);
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
  TF1*Fitpeak1 = new TF1("Fitpeak1", "[0]*TMath::Gaus(x,[1],[2])", -100, -30);
  Fitpeak1->SetParameters(3.76986e+02 ,-7.31723e+01,-8.51890e+00 );

  TF1*Fitpeak2 = new TF1("Fitpeak2", "[0]*TMath::Gaus(x,[1],[2])", -230, -200);
  Fitpeak2->SetParameters( 6.09078e+02 ,-2.13458e+02, -4.16254e+00);

  TF1*Fitpeak3 = new TF1("Fitpeak3", "[0]*TMath::Gaus(x,[1],[2])", -450, -400);
  Fitpeak3->SetParameters(9.85416e+01 , -4.23283e+02,-6.86131e+00 );

  TF1*Fitpeak4 = new TF1("Fitpeak4", "[0]*TMath::Gaus(x,[1],[2])", -620, -580);
  Fitpeak4->SetParameters( 2.84262e+02 ,-5.97022e+02,4.32377e+00);

  TF1*Fitpeak10 = new TF1("Fitpeak10", "[0]*TMath::Gaus(x,[1],[2])",-750,-700);
  Fitpeak10->SetParameters( 2.06218e+01 ,-7.15891e+02,-1.19732e+01 );

  TF1*Fitpeak11 = new TF1("Fitpeak11", "[0]*TMath::Gaus(x,[1],[2])",-790,-760);
  Fitpeak11->SetParameters(2.77554e+01,-7.72270e+02,-7.12861e+00);

  /*TF1*Fitpeak5 = new TF1("Fitpeak5", "[0]*TMath::Gaus(x,[1],[2])", -1400, -1300);                                                           
  Fitpeak5->SetParameters(1.07648e+02,-1.35799e+03 ,4.83114e+00);                                        

  TF1*Fitpeak6 = new TF1("Fitpeak6", "[0]*TMath::Gaus(x,[1],[2])", -1750, -1650);                        
   Fitpeak6->SetParameters(1.16572e+02 ,-1.68191e+03 ,4.26439e+00 );                                                                              
  TF1*Fitpeak7 = new TF1("Fitpeak7", "[0]*TMath::Gaus(x,[1],[2])", -1930, -1850);   
  Fitpeak7->SetParameters(7.33733e+01 ,-1.89645e+03,4.96903e+00);                                                                                 
  TF1*Fitpeak8 = new TF1("Fitpeak8", "[0]*TMath::Gaus(x,[1],[2])", -2000, -1930);                                                  
  Fitpeak8->SetParameters(8.80746e+01,-1.93990e+03 ,-4.33598e+00 );
  TF1*Fitpeak9 = new TF1("Fitpeak9", "[0]*TMath::Gaus(x,[1],[2])", -2500, -2410);
  Fitpeak9->SetParameters(1.36845e+02 ,-2.45713e+03,3.64893e+00);

  TF1*Fitpeak12 = new TF1("Fitpeak12", "[0]*TMath::Gaus(x,[1],[2])", -1550, -1490);
  Fitpeak12->SetParameters(2.42267e+01 , -1.51432e+03,1.03950e+01);
  */
  
  h1->GetXaxis()->SetTitle("ADC Counts");
  h1->SetTitle("");
  h1->SetStats(0);
  h1->Draw("same");


  h1->Fit(Fitpeak1,"0","",-100, -60);
  h1->Fit(Fitpeak2,"0","",-230, -200);
  h1->Fit(Fitpeak3,"0","",-440, -410);
  h1->Fit(Fitpeak4,"0","",-620, -580);
  h1->Fit(Fitpeak10,"0","",-740,-700);
  h1->Fit(Fitpeak11,"0","",-790,-760);


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

  Fitpeak10->SetLineColor(kRed);
  Fitpeak10->SetLineStyle(2);
  Fitpeak10->Draw("same");

  Fitpeak11->SetLineColor(kRed);
  Fitpeak11->SetLineStyle(2);
  Fitpeak11->Draw("same");

  TLine *linemean1=new TLine(-7.31723e+01,0,-7.31723e+01,800);
  TLine *linemean2=new TLine(-2.13458e+02,0,-2.13458e+02,800);
  TLine *linemean3=new TLine(-4.23283e+02 ,0,-4.23283e+02 ,800);
  TLine *linemean4=new TLine(-5.97022e+02 ,0, -5.97022e+02 ,800);
  TLine *linemean10=new TLine(-7.15891e+02,0,-7.15891e+02,800);
  TLine *linemean11=new TLine( -7.72270e+02,0, -7.72270e+02,800);
  //c1->Print("ADCCounts_run00110.pdf","pdf");
  //c1->Print("ADCCounts_run00110.png","png");

  linemean1->Draw("same");
  linemean2->Draw("same");
  linemean3->Draw("same");
  linemean4->Draw("same");
  linemean10->Draw("same");
  linemean11->Draw("same");
  
  //c1->Print("ADCCountsFit_run00110FirstPeaksnewprogram.pdf","pdf");
  //c1->Print("ADCCountsFit_run00110FirstPeaksnewprogram.png","png");
  

  /*h1->Fit(Fitpeak5,"0","",-1400, -1350);                                                                                                           
  h1->Fit(Fitpeak6,"0","",-1700, -1650);                                                                                                           
  h1->Fit(Fitpeak7,"0","",-1930, -1850);                                                                                                           
  h1->Fit(Fitpeak8,"0","",-2000, -1930);
  h1->Fit(Fitpeak9,"0","",-2500, -2450);                                                                                                          h1->Fit(Fitpeak12,"0","",-1530, -1490);           
                
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

  Fitpeak12->SetLineColor(kRed);
  Fitpeak12->SetLineStyle(2);
  Fitpeak12->Draw("same");

  TLine *linemean5=new TLine(-1.35799e+03 ,0,-1.35799e+03 ,400);                                                                                     
  TLine *linemean6=new TLine(-1.68191e+03 ,0,-1.68191e+03 ,400);                                                                                     
  TLine *linemean7=new TLine(-1.89645e+03 ,0,-1.89645e+03 ,400);                                                                                   
  TLine *linemean8=new TLine(-1.93990e+03 ,0,-1.93990e+03 ,400);  
  TLine *linemean9=new TLine(-2.45713e+03 ,0,-2.45713e+03,400);
  TLine *linemean12=new TLine(-1.51432e+03 ,0,-1.51432e+03 ,400);

  linemean5->Draw("same");                                                                                                                         
  linemean6->Draw("same");                                                                                                                         
  linemean7->Draw("same");                                                                                                                         
  linemean8->Draw("same");
  linemean9->Draw("same");
  linemean12->Draw("same");*/
  //c1->Print("ADCCountsFit_run00110SecondPeaksnewprogram.pdf","pdf");                                                                                       
  //c1->Print("ADCCountsFit_run00110SecondPeaksnewprogram.png","png");           
}
