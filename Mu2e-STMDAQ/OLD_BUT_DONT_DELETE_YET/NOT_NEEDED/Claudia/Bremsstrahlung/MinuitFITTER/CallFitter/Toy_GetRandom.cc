#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <boost/chrono.hpp>
#include <random>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <numeric>
#include <list>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <time.h>
#include <memory.h>
#include <vector>
#include <algorithm>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"
#include "TMinuit.h"
#include "TRandom3.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TPad.h"
#include "TPaveStats.h"

#include "TApplication.h"
#include "TRootCanvas.h"

double Bremsfunc_fit(double x, double *par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A/(exp(B*x)+C)+D;
  return value;
};

double Bremsfunc(double* x, double* par)
{
  return Bremsfunc_fit(x[0],par);
};

double Signalfunc_fit(double x, double *par)
{
  double mu    = par[1];
  double sigma = par[2];
  double norm  = 1./sqrt(2.*TMath::Pi())/sigma;
  double G     = norm*exp(-0.5 *pow((x-mu)/sigma,2));

  return par[0] * G;
};

//Binned LogLikelihood and Chi2
double SBfunc_fit(double x, double *par)
{
  double* parS = new double[3];
  parS[0] = par[0];
  parS[1] = par[1];
  parS[2] = par[2];

  double* parB = new double[4];
  parB[0] = par[3];
  parB[1] = par[4];
  parB[2] = par[5];
  parB[3] = par[6];

  double f = Signalfunc_fit(x,parS) + Bremsfunc_fit(x,parB);
  delete parS; delete parB;
  return f;

};

double SBfunc(double *x, double *par)
{
  return SBfunc_fit(x[0],par);
};


void Toy_GetRandom(int argc, char *argv[], int nsample, double _seed)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  TCanvas* c = new TCanvas("c","c",0,0,1500,800);

  TRandom* rndm_ptr = new TRandom(_seed);

  //This is fixed
  unsigned long int data_size = 1005191;

  //Get data and Fill Signal histogram
  double* dataSB = new double[data_size];

  std::string namehSTM = "hSTM";
  char* namehSTM_char = const_cast<char*>(namehSTM.c_str());
  TH1D* hSTM = new TH1D(namehSTM_char, "", 20000, 0.04, 2);

  //Mem problem
  TF1* fSB = new TF1("fSB",SBfunc,0.04,2,7);
  //No mem problem
  //TF1* fSB = new TF1("fSB","([0]/(exp([1]*x)+[2]))+[3]",0.04,2); 
  //No problem
  //TF1* fSB = new TF1("fSB",Bremsfunc,0.04,2,4);   

  fSB->SetNpx(300000);

   for (int loop = 0; loop < nsample; loop++){

    std::cout<<"ITERATION: "<<loop<<std::endl;
    
    fSB->SetParameter(0,0.5759);
    fSB->SetParameter(1,0.347);
    fSB->SetParameter(2,0.001023);
    fSB->SetParameter(3,89.51);
    fSB->SetParameter(4,2.372);
    fSB->SetParameter(5,-0.9761);
    fSB->SetParameter(6,7.005);
    /*
    fSB->SetParameter(0,89.51);
    fSB->SetParameter(1,2.372);
    fSB->SetParameter(2,-0.9761);
    fSB->SetParameter(3,7.005);
    */
 
    for (unsigned long int i = 0 ; i < data_size ; i++) { 
      dataSB[i] = fSB->GetRandom(rndm_ptr,"");
      hSTM->Fill(dataSB[i]);
    }
    
  }//loop

   hSTM->Draw("");
 
  //***********END CODE***********//

#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
}




int main(int argc, char *argv[]) {

  int nsample = std::atoi(argv[1]);
  double seed = std::stod(argv[2]);

  Toy_GetRandom(argc, argv, nsample, seed);

  return 0;
}
