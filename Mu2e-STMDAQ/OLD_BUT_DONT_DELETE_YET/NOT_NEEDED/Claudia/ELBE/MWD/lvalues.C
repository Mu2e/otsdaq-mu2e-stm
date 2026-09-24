#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include<list>
#include <sys/stat.h>

void lvalues(std::string  filename, double xmin, double xmax, double ymin, double ymax){
  gROOT->SetStyle("ATLAS");
 
  //std::string  filename  = "/work/cgarcia/DATA/ELBETestBeam/NewAnalysis/claudia_run_23_lvalues_ext_0.bin";
  

  double xx1[2]={xmin,xmax};
  double yy1[2]={ymin,ymax};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xmin,xmax);
  graph1->GetYaxis()->SetRangeUser(ymin,ymax);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("MWD Output");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");



  //get size
  struct stat st;
  stat(filename.c_str(), &st);
  int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each value is 4 bytes)
  std::cout<<"n elements= "<<n<<std::endl;
  
  //int n1=800000;
  int n1=n;
  double* l = new double[n];
  //int16_t* l = new int16_t[n];
  

  double* time = new double[n];
  //read file
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) l, n*sizeof(l[0]));
  myFile.close();

  const double fADC = 320.0520833313; //MHz
  const double tadc = (1./fADC); //us

  TGraph *grl = new TGraph ();

  for(int i = 0; i < n1; i++){
    time[i]=i*tadc;
    //cout<<l[i]<<endl;
    grl->SetPoint(i,time[i],l[i]);
    //std::cout<<l[i]<<std::endl;
  }


  grl->SetMarkerStyle(7);
  //grl->SetMarkerColor(kViolet+2);
  grl->SetMarkerColor(kBlue);
  grl->Draw("same,p");
  //grl->Draw("ap");

  TH1F *hbaseline = new TH1F("hbaseline", "", 100, -500, 500);
  for(int i=0; i<n;i++){
    hbaseline->Fill(l[i]);
  }
  //hbaseline->Draw("");


  delete hbaseline;
  //delete graph1;
  //delete grl;
}
