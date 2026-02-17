#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>

#include "TH1.h"
#include "TF1.h"
#include "TROOT.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TRootCanvas.h"
/*
//Compile without graphics in C++
g++ -w NoiseAnalysis.cc `root-config --cflags --glibs` -o Noise_without_graphics.exe
//Compile with graphics in C++
g++ -w -DUSE_GRAPHICS NoiseAnalysis.cc `root-config --cflags --glibs` -o Noise_with_graphics.exe
*/
int main(int argc, char **argv)
{
#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif
  gROOT->SetStyle("ATLAS");
  TCanvas* c = new TCanvas("c");

  int nbins = 20;
  double xmin = -2;
  double xmax = 2;
  TH1F *hnoise= new TH1F("", "", nbins, xmin, xmax);
  //////////////////////////////////////////


  // ADC samling frequency (MHz)
  const double fADC = 320.0520833313;
  int sampletime = 400; //400 us
  int sampleNum = sampletime*fADC;
  int16_t* ADC = new int16_t[sampleNum];


  //Generate the noise as random numbers between -(noiseSD-1) and (noiseSD-1)
  /*const double noiseSD = 50;
  for(unsigned long int i=0; i<sampleNum;i++){
    //Noise
    double rd = (double)rand() / RAND_MAX;
    double noise = -noiseSD + rd*(2*noiseSD); // Random noise
    ADC[i]=noise;
    std::cout<<ADC[i]<<std::endl;
    hnoise->Fill(ADC[i]/(38.5));  
    }*/


  //Generate the noise as random numbers in a gaussian
  double sigma_noise_mV=0.50;
  double sigma_noise_ADC=sigma_noise_mV*38.5;
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0,sigma_noise_ADC);


    for(unsigned long int i=0; i<sampleNum;i++){
      double noisegen=distribution(generator);
      ADC[i]=noisegen;
      std::cout<<noisegen<<" "<<ADC[i]<<std::endl; 
      hnoise->Fill(ADC[i]/(38.5));                                                                              
      //hnoise->Fill(ADC[i]);
      //hnoise->Fill(distribution(generator));
    }


  //Normalize the y axis to the number of entries
  TH1*hnorm = (TH1*)(hnoise->Clone("hnorm"));
  hnorm->Scale(1./hnorm->Integral());
  hnorm->SetFillColor(kOrange-3);
  hnorm->SetLineColor(kOrange-3);
  //hnorm->GetYaxis()->SetRangeUser(0,0.08);
  hnorm->GetXaxis()->SetTitle("Noise [mV]");
  //DRAW NORMALIZED HISTOGRAM
  hnorm->Draw("HIST");

  /*TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(0,0.01,"#sigma= 0.17 mV");
    latex.DrawLatex(0,0.05,"#mu= 0.015 mV");
    latex.DrawLatex(0,0.05,"Simulation 1kHz");*/

  TF1*Fit2 = new TF1("Fit2", "[0]*TMath::Gaus(x,[1],[2])", -2, 2);
  Fit2->SetParameters(1,0,1);
  hnorm->Fit(Fit2,"0","",-0.5,0.5);
  Fit2->SetLineColor(kRed);
  Fit2->SetLineStyle(2);
  Fit2->Draw("same");
  c->Modified();
  c->Update();
  //////////////////////////////
#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
      #endif

  return 0;
}
