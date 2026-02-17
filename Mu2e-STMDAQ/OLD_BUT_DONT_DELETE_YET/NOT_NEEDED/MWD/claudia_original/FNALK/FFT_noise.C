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
#include <iomanip>


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TVirtualFFT.h"

//Read FNAL data headers K
#include "/home/stm_mu2e/STMDAQ-TestBeam/MWD/FNALK/calcFuncs.h"

#define PI 3.14159265358979323846  /* pi */


void FFT_noise(std::string  input_filename) {

  gROOT->SetStyle("ATLAS");

  int N = 610000; //2,000us

  //Subtract headers
  std::vector<int16_t> ADC;
  Double_t *in = new Double_t[N];
  int16_t ADCcopy[610000]; 
  char* fname = const_cast<char*>(input_filename.c_str());
  calcFuncs fcalcFuncs;
  //Reads the file, subtract the headers and fill the data into a vector  
  std::cout<<"Reading file..."<<std::endl;
  ADC = fcalcFuncs.readFile(fname);
  unsigned long int n = ADC.size();

  double xmin = 0;
  double xmax = 0.001;
  double ymin = -1200;
  double ymax = 300;
  double fadc = 300;
  double t_Hz = (1./fadc)/1000000;
  double max;

  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Time [s]");
  graph1->GetYaxis()->SetTitle("Amplitude");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  TGraph *graph_data = new TGraph ();

  for(unsigned long int i = 0;i<N;i++){
    double t=i*t_Hz;
    ADCcopy[i]=ADC[i];
    in[i]=ADC[i];
    //graph_data->SetPoint(i,t,ADC_d[i]);
    max=t;
  }

  std::cout<<"Fourier transform..."<<std::endl;
  //time domain
  //graph_data->Draw("same,lp");
  //Generate a new binary file
  char file_name[] = "NoiseFFT.bin";                                                                               
  FILE * fp = fopen(file_name, "wb");                                                                                
  //2,000 us                                                                        
  fwrite(&ADCcopy[0], sizeof(int16_t), 600000, fp); 
  fclose(fp);


  /*  TVirtualFFT *fftr2c = TVirtualFFT::FFT(1, &N, "R2C ES K");

  fftr2c->SetPoints(in);
  fftr2c->Transform();
  fftr2c->GetPoints(in);
  TH1 *hr = 0;
  hr = TH1::TransformHisto(fftr2c, hr, "RE");
  //hr->Draw();
  hr->SetStats(kFALSE);
  hr->GetXaxis()->SetLabelSize(0.05);
  hr->GetYaxis()->SetLabelSize(0.05);
  hr->GetXaxis()->SetRangeUser(0,1000);

  int nbins = hr->GetXaxis()->GetNbins();
  TH1F *hnew = new TH1F("hfreq","",N,0,100);
  for (int i=1;i<=nbins;i++) {
    double y = hr->GetBinContent(i);
    double x = hr->GetXaxis()->GetBinCenter(i);
    double xnew = x/max; //your transformation
    double ynew = y/N;
    hnew->Fill(xnew,ynew);
  }

  //frequency domain
  hnew->GetYaxis()->SetTitle("Amplitude");
  hnew->GetXaxis()->SetTitle("freq [Hz]");
  hnew->Draw("l");
  */
}
