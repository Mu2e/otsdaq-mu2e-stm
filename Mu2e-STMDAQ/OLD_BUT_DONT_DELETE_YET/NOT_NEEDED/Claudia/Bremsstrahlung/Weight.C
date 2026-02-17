#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <iomanip>

#include "TGraph.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"
#include "TF1.h"
#include "TH1F.h"
#include "TSpectrum.h"
#include "TLegend.h"
#include "TRandom3.h"
#include "TLine.h"

#include "TApplication.h"
#include "TRootCanvas.h"



void Weight(){
  
  bool weighted_histo = false;
  
  gROOT->SetStyle("ATLAS");
  TCanvas* canvas = new TCanvas("c");
  
  double nbins=1000;
  double xmin_histo = 0;
  double xmax_histo = 2;
  double binning = (xmax_histo-xmin_histo)/nbins;

  unsigned long int pulseNumBrems = 500;

  TH1F *hBrems = new TH1F("hBrems", "", nbins,xmin_histo,xmax_histo);  
 
  if(weighted_histo==true){
    for(unsigned long int i=0 ; i<pulseNumBrems;i++){
      hBrems->Fill(1);}
  }
  else{
    double pulseNumBrems_aux = 1000;
    //fill the histogram for 1kHz
    for(int i =0;i<pulseNumBrems_aux;i++){
      hBrems->Fill(1);}

    //Get the bins and the content for the bin
    for(unsigned long int i=0; i<nbins;i++){
      double weight_aux = hBrems->GetBinContent(i);
      double weight = pulseNumBrems * weight_aux / pulseNumBrems_aux;
      std::cout<<"bin: "<<i<<" weight: "<<weight<<std::endl;
      //bin number and weight to increment the bin height
      hBrems->SetBinContent(i,weight);
    }

  }
  
  

  //Histogram bremsstrahlung background
  hBrems->SetFillColor(kOrange-3);
  hBrems->SetLineColor(kOrange-3);
  hBrems->Draw("same");

}


