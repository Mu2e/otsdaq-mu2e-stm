#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TLatex.h"
#include<fstream>

using namespace std;


void plotSimfromBinary() {

  gROOT->SetStyle("ATLAS");
  double xx1[2]={0,51000}; 
  //double xx1[2]={3010,3013};  
  double yy1[2]={-15000, 1000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  const double fADC = 370;
  const double tadc=1/(fADC);

  //write 01,05,10,20...  
  std::string rate = "01";

  //New Sim
  //std::string  filename  = std::string("/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/675keV_0.32mV/genData675.000000_"+rate+"kHz.bin"); 
  //Old Sim
  std::string  filename  = std::string("/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.32mV/GendataNoise_"+rate+"kHz_2370ADC_0.32noise.bin");       
std::cout << "filename  = " << filename << std::endl;

  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf;
  TGraph* gr = new TGraph();

  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) )){
    ADC.push_back(inf);
  }

  std::cout<<"Number of elements: "<<ADC.size()<<std::endl;

  //unsigned long int n= 9000000;
  //if(n>ADC.size()){n=ADC.size();}

  unsigned long int n= ADC.size(); 
  for(unsigned long int i=0;i<n;i++){
    gr->SetPoint(i,i*tadc,ADC.at(i));
  }


  int ratedoub = stoi(rate);
  std::string rate_kHz = std::to_string(ratedoub)+" kHz";
  char* ratechar = const_cast<char*>(rate_kHz.c_str());

  TLatex latex;
  latex.SetTextSize(0.06);
  //latex.DrawLatex(20,-1000,ratechar);
    
  gr->SetLineColor(kRed+1);
  gr->SetMarkerColor(kRed+1);
  gr->SetMarkerStyle(6);
  gr->Draw("same,l");

  
  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(gr,"Simulation","l");
  //legend->Draw("same");
  
}
