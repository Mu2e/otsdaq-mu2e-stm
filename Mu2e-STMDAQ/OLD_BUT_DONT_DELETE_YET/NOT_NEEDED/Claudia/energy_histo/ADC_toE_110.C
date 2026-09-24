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

void ADC_toE_110() {

  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  // double xx1[2]={0, 800};
  double xx1[2]={800, 2000}; 
  double yy1[2]={0,900};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  //graph1->GetXaxis()->SetRangeUser(0, 800);
  graph1->GetXaxis()->SetRangeUser(800, 2000); 
  graph1->GetYaxis()->SetRangeUser(0,900);
  graph1->SetTitle("{}^{152}Eu");
  graph1->GetXaxis()->SetTitle("E (keV)");
  graph1->GetYaxis()->SetTitle("");
  graph1->Draw("ap");

  //TH1F*h1 = new TH1F("TH1","", 1404, 0, 800); //0.57 kev binning
  TH1F*h1 = new TH1F("TH1","", 2456, 800, 2000); //0.57 kev binning 

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
      if((peaks+0.2704)/(-1.742)>30){
      h1->Fill((peaks+0.2704)/(-1.742));
      }
    }
  }//for int file


  //Fit Energy peak Europium
  /*TF1*Fitpeak1 = new TF1("Fitpeak1", "[0]*TMath::Gaus(x,[1],[2])", 25,60);
  Fitpeak1->SetParameters(8.53016e+02,40,-3.54625e+01);

  TF1*Fitpeak2 = new TF1("Fitpeak2", "[0]*TMath::Gaus(x,[1],[2])", 110, 130);
  Fitpeak2->SetParameters(4.93847e+02,120,-5.10241e+01);

  TF1*Fitpeak3 = new TF1("Fitpeak3", "[0]*TMath::Gaus(x,[1],[2])", 230,260 );
  Fitpeak3->SetParameters(2.04668e+02,240,-5.13933e+01);

  TF1*Fitpeak4 = new TF1("Fitpeak4", "[0]*TMath::Gaus(x,[1],[2])", 330, 350);
  Fitpeak4->SetParameters(2.04668e+02,340,-5.13933e+01);

  TF1*Fitpeak5 = new TF1("Fitpeak5", "[0]*TMath::Gaus(x,[1],[2])", 770, 790);
  Fitpeak5->SetParameters(2.04668e+02,775,-5.13933e+01);
  */

  TF1*Fitpeak6 = new TF1("Fitpeak6", "[0]*TMath::Gaus(x,[1],[2])", 950,980);                                                                                    
  Fitpeak6->SetParameters(8.53016e+02,960,-3.54625e+01);                                                                                                                                                                                                                                                                   
  TF1*Fitpeak7 = new TF1("Fitpeak7", "[0]*TMath::Gaus(x,[1],[2])", 1060, 1100);                                                                                 
  Fitpeak7->SetParameters(4.93847e+02,1080,-5.10241e+01);                                                                                                         
  TF1*Fitpeak8 = new TF1("Fitpeak8", "[0]*TMath::Gaus(x,[1],[2])", 1100,1120);
  Fitpeak8->SetParameters(2.04668e+02,1112,-5.13933e+01);                                                                                                                                                                                                                                
  TF1*Fitpeak9 = new TF1("Fitpeak9", "[0]*TMath::Gaus(x,[1],[2])", 1390, 1430);
  Fitpeak9->SetParameters(200,1408,3);  

  h1->GetXaxis()->SetTitle("E (keV)");
  h1->SetTitle("");
  h1->SetStats(0);
  h1->Draw("same");


  /*  h1->Fit(Fitpeak1,"0","",30, 55);
  h1->Fit(Fitpeak2,"0","",110, 130);
  h1->Fit(Fitpeak3,"0","",240, 250);
  h1->Fit(Fitpeak4,"0","",330, 350);
  h1->Fit(Fitpeak5,"0","",770, 790);

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

  Fitpeak5->SetLineColor(kRed);
  Fitpeak5->SetLineStyle(2);
  Fitpeak5->Draw("same");*/


  h1->Fit(Fitpeak6,"0","",950,970);                                                                                                                                                                                                                        
  h1->Fit(Fitpeak7,"0","",1060, 1100);                                                                                                                            h1->Fit(Fitpeak8,"0","",1100,1120);
  h1->Fit(Fitpeak9,"0","",1400,1420);                                                         
 
  Fitpeak6->SetLineColor(kRed);                                                                                                                                   Fitpeak6->SetLineStyle(2); 
  Fitpeak6->Draw("same");                                                                                                                                                                                                                                     
  Fitpeak7->SetLineColor(kRed);                                                                                                                                   Fitpeak7->SetLineStyle(2);
  Fitpeak7->Draw("same");                                                                                                                                                                                                                                     
  Fitpeak8->SetLineColor(kRed);                                                                                                                                                                                                                                Fitpeak8->SetLineStyle(2);                                                                                                                                                                                                                                   Fitpeak8->Draw("same");                                                                                                                                                                                                                                     
  Fitpeak9->SetLineColor(kRed);                                                                                                                                                                                                                                Fitpeak9->SetLineStyle(2);                                                                                                                                                                                                                                   Fitpeak9->Draw("same");         

  //cout<<"Mean: "<<h1->GetMean()<<endl;
  //cout<<"RMS: "<<h1->GetRMS()<<endl;
  //cout<<"MeanError: "<<h1->GetMeanError()<<endl;
 
  /*  TLine *line40=new TLine(40.1186,0,40.1186,1000);
  line40->Draw("same");
  auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
  leg1->AddEntry(line40, "40.1186 keV","l");
  leg1->Draw("same");

  TLine *line121=new TLine(121.78,0,121.78,1000);
  line121->Draw("same");
  auto leg2 = new TLegend(0.1,0.7,0.48,0.9);
  leg2->AddEntry(line121, "121.78 keV","l");
  leg2->Draw("same");

  TLine *line244=new TLine(244.7,0,244.7,1000);
  line244->Draw("same");
  auto leg3 = new TLegend(0.1,0.7,0.48,0.9);
  leg3->AddEntry(line244, "244.7 keV","l");
  leg3->Draw("same");

  TLine *line344=new TLine(344.28,0,344.28,1000);
  line344->Draw("same");
  auto leg4 = new TLegend(0.1,0.7,0.48,0.9);
  leg4->AddEntry(line244, "344.28 keV","l");
  leg4->Draw("same");

  TLine *line778=new TLine(778.91,0,778.91,1000);
  line778->Draw("same");
  auto leg5 = new TLegend(0.1,0.7,0.48,0.9);
  leg5->AddEntry(line778, "778.91 keV","l");
  leg5->Draw("same");

  TLine *line964=new TLine(964.08,0,964.08,1000);
  line964->Draw("same");
  auto leg6 = new TLegend(0.1,0.7,0.48,0.9);
  leg6->AddEntry(line964, "964.08 keV","l");
  leg6->Draw("same");

  TLine *line1085=new TLine(1085.837,0,1085.837,1000);
  line1085->Draw("same");
  auto leg7 = new TLegend(0.1,0.7,0.48,0.9);
  leg7->AddEntry(line1085, "1085.837 keV","l");
  leg7->Draw("same");

  TLine *line1112=new TLine(1112.076,0,1112.076,1000);
  line1112->Draw("same");
  auto leg8 = new TLegend(0.1,0.7,0.48,0.9);
  leg8->AddEntry(line1112, "1112.076 keV","l");
  leg8->Draw("same");

  TLine *line1408=new TLine(1408.13,0,1408.13,1000);
  line1408->Draw("same");
  auto leg9 = new TLegend(0.1,0.7,0.48,0.9);
  leg9->AddEntry(line1408, "1408.013 keV","l");
  leg9->Draw("same");*/
  //    c1->Print("EnergyEu_run00110.pdf","pdf");                                                                                                     
  //  c1->Print("EnergyEu_run00110.png","png");  
}
