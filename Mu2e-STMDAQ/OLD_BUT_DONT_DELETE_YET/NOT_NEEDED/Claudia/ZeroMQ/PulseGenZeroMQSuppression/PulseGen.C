#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"


using namespace std;


void PulseGen() {

  double xx1[2]={0,900};
  double yy1[2]={-2000, 100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,900);
  graph1->GetYaxis()->SetRangeUser(-2000, 100);
  graph1->GetXaxis()->SetTitle("time (#mus)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->Draw("ap");

  
  TF1* f3 = new TF1("f3", "-([0] / (1 + exp(-(x - [1])*[2])) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x - [1])*[3]))))+[4]*sin(x)", 0,3000);

  f3->SetNpx(10000);
  //The amplitude of the pulse is the half of this value
  f3->SetParameter(0,2370);
  //The x point in where we have the pulse (100 us)
  f3->SetParameter(1,50);
  //fall from baseline time
  f3->SetParameter(2,6);   
  //decaytime(rise to baseline)
  f3->SetParameter(3,0.028);
  //Amplitude of noise
  f3->SetParameter(4,10);

  //Number of ADC values in one peak ([1]+220)/0.0027
  unsigned long n=100000;
  //Number of bytes in one peak
  unsigned long nbytes=n*2;
  //Number of peaks in one event
  unsigned long npeaks=1;
  //Number of bytes per event
  unsigned long byteschunk=npeaks*nbytes;
  std::cout<<"Bytes in one event: "<<byteschunk<<std::endl;
  
  //Number of events
  unsigned long nevents=6;

  double* x = new double[n];
  int16_t* data_element = new int16_t[n];

  TGraph* grADC = new TGraph();

  unsigned long j=0;

 
    
    for (unsigned long i=0;i<n;i++){
    x[i]=0.0027*i;
    data_element[i]=-(2370 / (1 + exp(-(x[i] - 50)*6)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x[i] - 50)*0.028))))+10*sin(x[i]);
    grADC->SetPoint(j,x[i],data_element[i]);
    j++;
  }

 
  f3->Draw("same");
  grADC->Draw("p*");



}
