// Include ROOT headers/libraries
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TGraph.h"
#include "TROOT.h"

// Include C++ headers/libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>

using namespace std;                                 

void plotFile(){

  // The file specifier passed as the argument
  char* fname = "stm-daq_2023-11-09_19-09-36_HPGe_0_ADConly.bin";
  
  // ADC dta vector
  std::vector<int16_t> vec;

  // Read in the binary file
  std::ifstream is;
  is.open(fname, std::ios::binary);
  is.seekg(0, std::ios::end);
  size_t filesize=is.tellg();
  is.seekg(0, std::ios::beg);
  vec.resize(filesize/sizeof(int16_t));
  is.read((char *)vec.data(), filesize);

  gROOT->SetStyle("ATLAS");  
  TGraph* gr_adc = new TGraph();
  unsigned long int limit = 1950000; 

  int k=0;
  double T0=1.0/370; //us
  double* timevalue = new double[limit];

  for(unsigned long int i=0;i<limit;i++){   
    timevalue[k] = T0*i;
    gr_adc->SetPoint(k,timevalue[k],vec.at(i));
    k++;
  }
  double xmin = 0;//us
  double xmax = 5000;//us
  double ymin = -500;
  double ymax = 200;

  double xx1[2]={xmin , xmax};
  double yy1[2]={ymin, ymax};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xmin, xmax);
  graph1->GetYaxis()->SetRangeUser(ymin,ymax);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  gr_adc->SetMarkerColor(kBlack); 
  gr_adc->SetLineColor(kBlack);
  gr_adc->SetMarkerStyle(1);
  gr_adc->Draw("same,lp");

  //  loopDir();

}
