#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"

void readsuppressionbinary() {
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)                                                                                                                                                                            
  auto c1= new TCanvas("c1","Title",400,10,1500,500);
 
  std::vector<int16_t> ADC;
  ADC.clear();

  //  std::ifstream myFile("run00109.new.bin_00", ios::in | ios::binary);
  std::ifstream myFile("run00075.bin", ios::in | ios::binary); 
  int16_t inf ;
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) ){
    ADC.push_back(inf);
  }

  std::cout << "Read data ... " << std::endl;

 
 
  double xx1[2]={0,20000000};
  double yy1[2]={-3000,3000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,20000000);
  graph1->GetYaxis()->SetRangeUser(-3000,3000);
  graph1->SetTitle(" ");
  graph1->GetXaxis()->SetTitle("Time (ns)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->Draw("ap");


  //std::ifstream myFilesuppressed("run00109_suppressedsignal_bin_00.bin", ios::out | ios::binary);
  std::ifstream myFilesuppressed("run00075_suppressedsignal_bin.bin", ios::out | ios::binary);
std::vector<int16_t> ADCsuppressed;
  int16_t ADCVolts;
  while( myFilesuppressed.read( reinterpret_cast<char*>( &ADCVolts ), sizeof(ADCVolts) ) ){
    ADCsuppressed.push_back(ADCVolts);
  }

  TGraph* gr = new TGraph();
  TGraph* gr2 = new TGraph();
   double t=0;

  for(int i=0;i<10000000;i++){
      
       gr->SetPoint(i,t,ADCsuppressed.at(i));
       gr2->SetPoint(i,t,ADC.at(i));
       t+=2.7;
  }

 
  gr->SetLineColor(kBlack);
  gr->SetMarkerStyle(5);
  gr->Draw("same");


  gr2->SetLineColor(kBlue);
  gr2->SetMarkerStyle(5);
  gr2->Draw("same");

  auto leg = new TLegend(0.1,0.7,0.48,0.9);
  leg->AddEntry(gr2,"Signal","l");
  leg->AddEntry(gr, "Suppressed Signal","l");
  leg->Draw("same");

  //  c1->Print("Suppressedrun109.pdf","pdf");
  // c1->Print("Suppressed109.png","png");

}
