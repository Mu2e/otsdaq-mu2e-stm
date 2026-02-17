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

  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  //double xx1[2]={-800, 100};
  double xx1[2]={-2500,-1000};
  //double xx1[2]={-3000, 0};  
 double yy1[2]={0,200};
 TGraph *graph1 = new TGraph (2,xx1,yy1);
 //graph1->GetXaxis()->SetRangeUser(-800, 100);
   graph1->GetXaxis()->SetRangeUser(-2500,-1000);
 // graph1->GetXaxis()->SetRangeUser(-3000,0);     
  graph1->GetYaxis()->SetRangeUser(0,200);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //TH1F*h1 = new TH1F("TH1","", 1000, -800, 100);
  //TH1F*h1 = new TH1F("TH1","", 1000, -2500, -1000);
  TH1F*h1 = new TH1F("TH1","", 1000, -3000, 0);  

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
  /*   TF1*Fitpeak1 = new TF1("Fitpeak1", "[0]*TMath::Gaus(x,[1],[2])", -150, 100);
  Fitpeak1->SetParameters(4.90329e+02 ,-6.03983e+01, -8.63137e+00);

  TF1*Fitpeak2 = new TF1("Fitpeak2", "[0]*TMath::Gaus(x,[1],[2])", -240, -150);
  Fitpeak2->SetParameters(4.72875e+02,-1.94998e+02,-9.22647e+00);

  TF1*Fitpeak3 = new TF1("Fitpeak3", "[0]*TMath::Gaus(x,[1],[2])", -440, -340);
  Fitpeak3->SetParameters(1.12356e+02,-3.95005e+02, -8.84378e+00);

  TF1*Fitpeak4 = new TF1("Fitpeak4", "[0]*TMath::Gaus(x,[1],[2])", -600, -500);
  Fitpeak4->SetParameters(3.13490e+02, -5.61123e+02 ,7.45398e+00);
  */
  TF1*Fitpeak5 = new TF1("Fitpeak5", "[0]*TMath::Gaus(x,[1],[2])", -1400, -1200);   
  Fitpeak5->SetParameters(8.34792e+01,-1.28757e+03 ,-8.64304e+00 );         
  TF1*Fitpeak6 = new TF1("Fitpeak6", "[0]*TMath::Gaus(x,[1],[2])", -1700, -1500);
  Fitpeak6->SetParameters(9.96288e+01 ,-1.59644e+03 ,7.97740e+00);                                                                              
  TF1*Fitpeak7 = new TF1("Fitpeak7", "[0]*TMath::Gaus(x,[1],[2])", -1900, -1750);     
  Fitpeak7->SetParameters(6.46241e+01,-1.80228e+03 , 7.87960e+00  );  
  TF1*Fitpeak8 = new TF1("Fitpeak8", "[0]*TMath::Gaus(x,[1],[2])", -1900, -1750);                                                  
  Fitpeak8->SetParameters( 7.60427e+01 ,-1.84623e+03,  6.65016e+00 );

  TF1*Fitpeak9 = new TF1("Fitpeak9", "[0]*TMath::Gaus(x,[1],[2])", -2400, -2300);
  Fitpeak9->SetParameters(9.98887e+01 , -2.33723e+03 ,8.00339e+00);

  h1->GetXaxis()->SetTitle("ADC Counts");
  h1->SetTitle("");
  h1->SetStats(0);
  // h1->Draw("same");

  Int_t i;
  const Int_t nbins = 1000;
  Double_t xmin     = -3000;
  Double_t xmax     = 0;
  Double_t source[nbins];
  TH1F *d    = new TH1F("d","",nbins,xmin,xmax);

  TSpectrum *s = new TSpectrum();
 
  for (i = 0; i < nbins; i++) {source[i] = h1->GetBinContent(i + 1);}
 
  // Estimate the background
  s->Background(source,nbins,6,TSpectrum::kBackIncreasingWindow,
		TSpectrum::kBackOrder2,kFALSE,
		TSpectrum::kBackSmoothing3,kFALSE);
  
  // Draw the estimated background
  for (i = 0; i < nbins; i++) {d->SetBinContent(i + 1,source[i]);}
  d->SetLineColor(kRed);
  //d->Draw("SAME L");

  h1->Add(d,-1);
  h1->Draw("same"); 
  // c1->Print("No_Background_run00110.pdf","pdf");                                                                          
  // c1->Print("No_Background_run00110.png","png");   
  
  /*   h1->Fit(Fitpeak1,"0","",-100, 20);
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
  TLine *linemean1=new TLine(-6.03983e+01 ,0,-6.03983e+01 ,700);
  TLine *linemean2=new TLine(-1.94998e+02,0,-1.94998e+02,700);
  TLine *linemean3=new TLine(-3.95005e+02,0,-3.95005e+02,700);
  TLine *linemean4=new TLine(-5.61123e+02,0,-5.61123e+02 ,700);
  
  linemean1->Draw("same");
  linemean2->Draw("same");
  linemean3->Draw("same");
  linemean4->Draw("same");

  
  //c1->Print("NoBackground_run00110FirstPeaks.pdf","pdf");
  //c1->Print("NoBackground_run00110FirstPeaks.png","png");
  */

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

  TLine *linemean5=new TLine(-1.28757e+03,0,-1.28757e+03,200);                                                                                     
  TLine *linemean6=new TLine( -1.59644e+03 ,0, -1.59644e+03 ,200);                                                                                     
  TLine *linemean7=new TLine(-1.80228e+03,0,-1.80228e+03,200);
  TLine *linemean8=new TLine(-1.84623e+03,0,-1.84623e+03,200);  
  TLine *linemean9=new TLine(-2.33723e+03 ,0,-2.33723e+03 ,200);

  linemean5->Draw("same");                                                                                                                         
  linemean6->Draw("same");                                                                                                                         
  linemean7->Draw("same");                                                                                                                         
  linemean8->Draw("same");
  linemean9->Draw("same");
  
  c1->Print("NoBackground_run00110SecondPeaks.pdf","pdf");                                                                       
  c1->Print("NoBackground_run00110SecondPeaks.png","png");  
}
