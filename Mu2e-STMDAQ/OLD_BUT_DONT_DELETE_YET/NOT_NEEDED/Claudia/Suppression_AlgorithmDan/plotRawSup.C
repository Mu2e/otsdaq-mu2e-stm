#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include<fstream>


using namespace std;


void plotRawSup() {

  //double xx1[2]={88000, 4000000};
  double xx1[2]={0, 2500000};
  double yy1[2]={-1000, 2500};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 2500000);
  graph1->GetYaxis()->SetRangeUser(-1000, 2500);
  graph1->GetXaxis()->SetTitle("time (arb units)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->Draw("ap");



  //SUPPRESSED DATA
  //std::string  filenamesup  = std::string("run00109_suppressedsignal_threshold3_allfilebin_00.bin");
  std::string  filenamesup  = std::string("run00109_suppressedsignal_bin_00.bin");
  std::cout << "filename suppressed = " << filenamesup << std::endl;

  std::vector<int16_t> ADCsup;
  ADCsup.clear();
  std::ifstream myFilesup;
  myFilesup.open(filenamesup, std::ios::in | std::ios::binary);
  int16_t infsup;
  TGraph* grsup = new TGraph();

  while( myFilesup.read( reinterpret_cast<char*>( &infsup ), sizeof(infsup) )){
    ADCsup.push_back(infsup);
  }

  std::cout<<"Number of elements of suppressed data: "<<ADCsup.size()<<std::endl;

  for(int i=0;i<ADCsup.size();i++){
    grsup->SetPoint(i,i,ADCsup.at(i));
  }


  grsup->SetLineColor(kPink+2);
  grsup->SetMarkerStyle(5);
  grsup->Draw("same");



  //RAWDATA
  std::string  filename  = std::string("run00109.new.bin_00");
  std::cout << "filename = " << filename << std::endl;

  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf;
  //int n=90000000;
  int ADCread=0;

  TGraph* gr = new TGraph();

  //while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) &&ADCread<n){
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) )){
    ADC.push_back(inf);
    ADCread++;
  }

  std::cout<<"Number of elements of all data: "<<ADC.size()<<std::endl;

  int n=90000000;
  for(int i=0;i<n;i++){
    gr->SetPoint(i,i,ADC.at(i));
  }


  gr->SetLineColor(kBlack);
  gr->SetMarkerStyle(5);
  gr->Draw("same");



  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(gr,"Signal","l");
  legend->AddEntry(grsup,"Suppressed Signal","l");
  legend->Draw();



}
