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
#include "TH2F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TGraph2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h" 
#include "TStyle.h"
#include "TAxis.h"
#include "TLatex.h"
#include "TProfile2D.h"
#include "TPaletteAxis.h"
#include "TColor.h"
#include "TGraphErrors.h"

using namespace std;

void NotReco347keV(){
  
  gROOT->SetStyle("ATLAS");

  double n = 7;
  
  double rate[7]={1,5,10,30,50,75,100};  
  double rate_error[7]={0,0,0,0,0,0,0};

  double Sreco[7]={23, 39, 44, 89, 115, 126, 137};
  double Sreco_error[7], frac[7], frac_error[7];

  double Strue = 595;
  
  for(int i =0; i < n ; i++){
    
    frac[i] = Sreco[i]/Strue;
    frac_error[i] = frac[i] * sqrt(Sreco[i])/Sreco[i];
    
  }

  
  TGraphErrors *gr = new TGraphErrors(n,&rate[0],&frac[0], &rate_error[0],&frac_error[0]);
  gr->SetMarkerColor(kMagenta+1);

  double xx1[2]={0,105};
  double yy1[2]={0,0.35};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  graph1->GetYaxis()->SetTitle("Fraction of 347 keV not reconstructed");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  gr->Draw("same,p");
  
  
}
