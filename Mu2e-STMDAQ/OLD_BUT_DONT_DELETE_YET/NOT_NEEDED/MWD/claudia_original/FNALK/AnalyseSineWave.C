// Include ROOT headers/libraries                                                                                               
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TLegend.h"
#include "TProfile.h"
// Include C++ headers/libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include<cstdio>
#include <iomanip>
#include <sys/stat.h>

#define PI 3.14159265358979323846  /* pi */

using namespace std;
//Vpp volts, f kHz
void AnalyseSineWave(std::string filename, double Vpp, double f){

  double A=20000*Vpp;
  double B=2*PI*f/1000; //MHz
  double C=0;
  double D=1;

  /*double A=20000*Vpp;                                                                                                                   
  double B=2*PI*f/1000; //MHz
  double C=0;
  double D=1;
  double E=0;
  */

  //For 0.90V, 3MHz...
  //double C=10;
  //double D=1;

  std::stringstream streamVpp, streamf;
  streamVpp << std::fixed << std::setprecision(3) << Vpp;
  streamf << std::fixed << std::setprecision(3) << f;
  std::string name = "data-fit-sinewave"+streamVpp.str()+"Vpp_"+streamf.str()+"kHz_TProfile.png";
  std::string logname = "data-fit-sinewave"+streamVpp.str()+"Vpp_"+streamf.str()+"kHz_TProfile.log";

  char* imagename = const_cast<char*>(name.c_str());
  char* lognames = const_cast<char*>(logname.c_str());

  //gSystem->RedirectOutput(lognames);

  std::vector<int16_t> DataADC;

  struct stat st;
  stat(filename.c_str(), &st);
  unsigned long int n = st.st_size/2; //n ADC
  int16_t* ADC = new int16_t[n];

  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  for(unsigned long int i=0;i<n;i++){
    DataADC.push_back(ADC[i]);
  }


  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptFit(00111);
  gStyle->SetOptFit(0); 

  auto c1= new TCanvas("c1"); 
   
  TGraph* gr_adc = new TGraph();
  //auto ge= new TGraphErrors(n,x,y,ex,ey);
  unsigned long int limit = 1950000;
  //unsigned long int limit = 60000;  

  double ADC_digClock = 300;//MHz
  double T0=1.0/ADC_digClock; //us

  std::cout<<"Number of ADCs in file: "<<DataADC.size()<<std::endl;
  int k=0;
  double* timevalue = new double[limit];

  
  for(unsigned long int i=0;i<limit;i++){
    timevalue[k] = T0*i;
    gr_adc->SetPoint(k,timevalue[k],DataADC.at(i));
    k++;
  }

  gr_adc->SetMarkerColor(kBlack); 
  gr_adc->SetLineColor(kBlack);
  gr_adc->SetMarkerStyle(22);

  double xmin = 0;//us
  double xmax = 100;//us 
  
  //Fit 50kHz
  //double xmax = 100;//us
  //Fit 3MHz
  //double xmax = 3;//us 

  //If we want to to fit the sine wave signal
  /*TF1* fitsin = new TF1("sinx", "[0]*sin([1]*x+[2]*x*x+[3])+[4]",xmin,xmax);
  fitsin->SetParameters(A, B, C, D, E);*/
  TF1* fitsin = new TF1("sinx", "[0]*sin([1]*x+[2])+[3]",xmin,xmax);
  fitsin->SetParameters(A, B, C, D);
  
  fitsin->SetLineColor(kRed);
  fitsin->SetLineStyle(2);
  gr_adc-> Fit(fitsin,"0","",xmin,xmax);

  double ymin = (-1)*fitsin->GetParameter(0)-1000;
  double ymax = fitsin->GetParameter(0)+1000;

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

  gr_adc->SetMarkerStyle(22);
  gr_adc->Draw("same,p");

  fitsin->Draw("same");


  //COMMENT EVERYTHING BELOW NOT TO PRINT PROFILE
  TH1F* hdiff = new TH1F("hdiff","", 100, 0, 1000);
  double xminprof = 0;
  double xmaxprof = 1000000;
  int binsprofile = (xmaxprof-xminprof)/50; //10us bin
  TProfile *hprofx = new TProfile("","",binsprofile, xminprof, xmaxprof,"");
  
  //See the difference between fit function and data for sine wave
  /*  double Amplitude = fitsin->GetParameter(0);
  double frequency = fitsin->GetParameter(1);
  double phase2 = fitsin->GetParameter(2);
  double phase = fitsin->GetParameter(3);
  double ydev = fitsin->GetParameter(4);
  */

  for(unsigned long int i =0 ; i<DataADC.size(); i++){
    double t =T0*i;
    //double sinx_fit = (Amplitude*sin(frequency*t+phase2*t*t+phase) )+ ydev;
    double sinx_fit = fitsin->Eval(t);
    double diff = DataADC.at(i)-sinx_fit;
    //Take absolute value of diff
    //if(diff<0){diff=(-1)*diff;}
    hprofx->Fill(t,diff);
    //std::cout<<"t: "<<t<<" data: "<<DataADC.at(i)<<" fit: "<<sinx_fit<<" Diff = "<<diff<<std::endl; 
    
    if(diff>1000){
      hdiff->Fill(t);
      //std::cout<<"t: "<<t<<" data: "<<DataADC.at(i)<<" fit: "<<sinx_fit<<" Diff = "<<diff<<std::endl;
    }
    
  }
  
  //HISTOGRAM
  //hdiff->GetXaxis()->SetTitle("Time for |Data-Fit|>1,000ADC values [#mus]");
  //hdiff->Draw("");
  
  //PROFILE
  hprofx->GetYaxis()->SetTitle("Data-Fit [ADC values]"); 
  hprofx->GetXaxis()->SetTitle("Time [#mus]");
  hprofx->SetMarkerStyle(21);
  hprofx->SetMarkerColor(kMagenta-2);
  hprofx->SetMarkerSize(1);
  hprofx->Draw("");

  //c1->Print("0.85V_50kHz_fullrange_noabs_50usbin.png");  

  //This is just if we run the macro with the bash script  
  //c1->Print(imagename,"png");
  //exit(0);

}
