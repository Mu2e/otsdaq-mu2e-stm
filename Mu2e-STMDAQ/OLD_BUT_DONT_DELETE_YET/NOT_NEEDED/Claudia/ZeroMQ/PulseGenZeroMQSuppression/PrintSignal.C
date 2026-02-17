#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <sys/stat.h>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"


//#include "TTree.h"
//#include "TFile.h"


using namespace std;

void PrintSignal(){

  std::string  filename  = "pulsegen.bin";
  std::cout << "filename = " << filename << std::endl;

  auto c1= new TCanvas("c1");
  c1->SetGrid();


  double xx1[2]={0, 6000};
  double yy1[2]={-1500, 100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 6000);
  graph1->GetYaxis()->SetRangeUser(-1500,100);
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
  TGraph* gr = new TGraph();

  for(int i = 0; i < n; i++){
    time[i] = 0.0027*i;
    //std::cout<<ADC[i]<<endl;
    gr->SetPoint(i,time[i],ADC[i]);
  }

  gr->Draw("p*");
}
