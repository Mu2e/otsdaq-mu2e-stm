#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
//#include <boost/chrono.hpp>
#include <sys/stat.h>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"


//#include "TFile.h"
//boost::chrono::milliseconds sumGlobal;

using namespace std;

void MeanSigma(){

  //std::string  filename  = std::string(arg[1]);
  std::string  filename  = "pulsegen.bin";
  std::cout << "filename = " << filename << std::endl;

  auto c1= new TCanvas("c1");
  c1->SetGrid();


  double xx1[2]={0, 3000};
  double yy1[2]={-2000, 100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 3000);
  graph1->GetYaxis()->SetRangeUser(-2000, 100);
  graph1->GetXaxis()->SetTitle("time (#mus)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->Draw("ap");



  struct stat st;
  stat(filename.c_str(), &st);
  int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)
  int16_t* ADC = new int16_t[n];

  std::ifstream myFile;

  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  double* time = new double[n];
  double T0 = 0.0027;
  TGraph* gr = new TGraph();
  
  for(int i=0;i<n;i++){
    time[i]=T0*i;
    gr->SetPoint(i,time[i],ADC[i]);
  }


  gr->SetMarkerColor(kBlack);
  gr->SetLineColor(kBlack);
  gr->SetMarkerStyle(5);
  gr->Draw("same, l");
  //gr->Draw("same, p*");

  //Get mean and sigma of noise
  TH1F *hsignal = new TH1F("hsignal", "", 100, 0, 10000000);
  //t=0, t=20us
  double t=15;
  //i=t/0.0027
  int upperlimit = int(20/0.0027);
  for(int i=0;i<upperlimit;i++){
    hsignal->Fill(ADC[i]);
  }
  double mean=hsignal->GetMean();
  std::cout<<"Mean Noise: "<<mean<<endl;

  TLine *linemean=new TLine(xx1[0],mean,xx1[1],mean);
  linemean->SetLineColor(kRed);
  linemean->Draw("same");
 
  double sigma=hsignal->GetRMS();
  std::cout<<"St dev Noise: "<<sigma<<endl;
  //Threshold
  double factor=10;
  double threshold=factor*sigma;
  std::cout<<"Threshold: "<<threshold<<endl;
 

  TLine *lineup=new TLine(xx1[0],mean+threshold,xx1[1],mean+threshold);
  lineup->SetLineColor(kOrange);
  lineup->Draw("same");
  TLine *linedown=new TLine(xx1[0],mean-threshold,xx1[1],mean-threshold);
  linedown->SetLineColor(kOrange);
  linedown->Draw("same");

  
}
