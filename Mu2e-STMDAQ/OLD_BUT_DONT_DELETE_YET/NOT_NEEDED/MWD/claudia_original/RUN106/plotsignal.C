#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cstdio>

#include <sys/stat.h>
#include "TGraph.h"
#include "TCanvas.h"

void plotsignal() {
  auto c1= new TCanvas("c1","Title",400,10,1000,500);

  double xx1[2]={0,25000000};
  double yy1[2]={-1000,1000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,25000000);
  graph1->GetYaxis()->SetRangeUser(-1000,1000);
  graph1->GetXaxis()->SetTitle("Time (ns)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");

  std::string  filename  = "../../../DATA/MWD_Analysis/RUN106/run00106.bin";
  struct stat st;
  stat(filename.c_str(), &st);
  int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)                                             
  int16_t* ADC = new int16_t[n];

  std::ifstream myFile;

  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();


  TGraph* gr = new TGraph();
  TGraph* dec = new TGraph();
  TGraph* dif = new TGraph();
  TGraph* av = new TGraph(); 
   float* time = new float[n];
   float t = 0;
   float T0 = 2.7;

  //   double* time = new double[n];
  //double t = 0;
  // double T0 = 2.7;     
  for(int i = 0; i < 10000000; i++){
    time[i] = t;
    gr->SetPoint(i,time[i],ADC[i]);
    t+=T0;
  }

  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////                                                

  //Deconvolution                                                                                                                                        
  float tau = 55748.2;
  float* a = new float[n];
  //double tau = 55748.2;                                                                                                                                 
  //double* a = new double[n];   
  a[0] = ADC[0];

  for(int i=1; i<10000000; i++){
    a[i] = ADC[i]-(1-(T0/tau))*ADC[i-1] + a[i-1];
    dec->SetPoint(i,time[i],a[i]);
  }

  delete ADC;


  //Differentiation                                                                                                                                      

  float* D = new float[n];
  //double* D = new double[n];  
  int M=8000;

  memcpy( D, a, M*sizeof(a) );
  //  for (int i = 0; i < M; ++i) {                                                                                                                    
  // D[i] = a[i];                                                                                                                                      
  // }                                                                                                                                                 

  for (int i = M; i < 10000000; ++i) {
    D[i] = a[i] - a[i-M];
    dif->SetPoint(i,time[i],D[i]);
  }

  delete a;

  //Averaging                                                                                                                                            

  float* l = new float[n];
  int L = 1000;
  float sum = 0.;
  //double* l = new double[n]; 
  //double sum = 0.;  

  memcpy( l, D, (L-1)*sizeof(D) );
  for (int i = 0; i < L-1; ++i) {
    sum += D[i];
    //    l[i] = D[i];                                                                                                                                   
  }

  sum += D[L-1];
  l[L-1] = sum/L;

  for (int i = L; i < 10000000; ++i) {
    l[i] = sum/L;
    av->SetPoint(i,time[i],l[i]);
    sum += D[i]-D[i-L];
  }

  delete D;






  graph1->Draw("ap");

  gr->SetLineColor(kBlack);
  gr->SetMarkerStyle(5);
  gr->Draw("same");

  dec->SetMarkerColor(kRed);
  dec->SetLineColor(kRed);
  dec->SetMarkerStyle(5);
  dec->Draw("same");

  dif->SetMarkerColor(kGreen);
  dif->SetLineColor(kGreen);
  dif->SetMarkerStyle(5);
  dif->Draw("same");

  av->SetMarkerColor(kBlue);
  av->SetLineColor(kBlue);
  av->SetMarkerStyle(5);
  av->Draw("same");

  float mean =336.308;
  float sigma=20;
 float threesigmas= 4*sigma;

 //double mean =336.308;                                                                                                                                 
 //  double sigma=20;
 // double threesigmas= 4*sigma;    
  TLine *line=new TLine(0,mean,25000000,mean);
  line->SetLineColor(kRed);
  line->Draw("same");

  TLine *line2=new TLine(0,mean+4*sigma,25000000,mean+4*sigma);
  line2->SetLineColor(kBlack);
  line2->Draw("same");
  TLine *line3=new TLine(0,mean-4*sigma,25000000,mean-4*sigma);
  line3->SetLineColor(kBlack);
  line3->Draw("same");


}
