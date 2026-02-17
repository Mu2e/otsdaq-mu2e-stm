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


void peakshapes(std::string filename, int peaknumber) {

  gROOT->SetStyle("ATLAS");
  double xx1[2]={0,1100}; 
  double yy1[2]={-5000, 1000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  const double fADC = 320.0520833313;
  //const double fADC = 370;
  //Sampling time of ADC (microsec)
  const double tadc=1/(fADC);

  std::cout << "Plot peaks from filename = " << filename<< std::endl;

  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf;
  TGraph* gr = new TGraph();
  int ADCread=0;

 
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) )){
    ADC.push_back(inf);
  }



  //plot 302us per pulse in ADC counts
  int sizepeak=302*fADC;
  //int startplot=peaknumber*sizepeak;
  //int endplot=startplot+sizepeak;
  int startplot=0;
  int endplot=1100; 
  //double trigtime=18621.8; //us
  //int startplot=(trigtime-3)*fADC;
  //int endplot=startplot+sizepeak; 
  int index=0;

  std::cout<<"Plotting from element: "<<startplot<<" to "<<endplot<<std::endl;
  std::cout<<"Number of elements: "<<ADC.size()<<std::endl;

  
  if(startplot>=ADC.size()){std::cout<<"This peak exceeds file size"<<std::endl;}
  if(endplot>ADC.size()){endplot=ADC.size();}
  
  /*
  for(int i=startplot;i<endplot;i++){
    gr->SetPoint(index,index*tadc,ADC.at(i)-920);
    index++;
  }
  */

  for(int i=0;i<ADC.size();i++){
    gr->SetPoint(index,index*tadc,ADC.at(i));
    index++;
  }

    gr->SetLineColor(kBlack);
    gr->SetMarkerColor(kBlack);
    gr->SetMarkerStyle(6);
    gr->Draw("same,p");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(gr,"Raw Data","l");
    legend->Draw("same");


 
}
