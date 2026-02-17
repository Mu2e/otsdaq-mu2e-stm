#include <iostream>
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
#include "TGaxis.h"


void Read10percentAccuracy() {
  //gROOT->SetStyle("ATLAS");

  int palette_number = 62;
  gStyle->SetPalette(palette_number);

  auto c1= new TCanvas("c1");
  //TCanvas *c1  = new TCanvas("c1","c1",0,0,700,500);
  //c1->SetGrid();

  double xx1[2]={10, 500};
  double yy1[2]={1, 3};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->GetXaxis()->SetTitleColor(kBlack);
  graph1->GetXaxis()->SetTitleSize(0.04);
  graph1->GetXaxis()->SetLabelSize(0.04);
  graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetLabelSize(0.04);
  graph1->GetXaxis()->SetTitle("Bremsstrahlung Rate [kHz]");
  graph1->GetYaxis()->SetTitle("Resolution [keV]");
  graph1->GetYaxis()->SetTitleOffset(1.3);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  



  //STM resolution, Bremsstrahlung rate
  std::vector<string> Resol = {"0.001", "0.0011", "0.0012", "0.0013", "0.0014", "0.0015", "0.0016", "0.0017", "0.0018", "0.0019", "0.002", "0.0021", "0.0022", "0.0023", "0.0024", "0.0025", "0.0026", "0.0027", "0.0028", "0.0029", "0.003"};
  std::vector<int> BremsRate = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 400, 410, 420, 430, 440, 450, 460, 470, 480, 490, 500};
  double ratebrems, TimeSim, res, sigmacut, Energy, meanEnergycounts, RMS, uncertainty;
  vector<double> ratebremsv, TimeSimv, resv, sigmacutv, Energyv, meanEnergycountsv, RMSv, uncertaintyv;
  std::string name;

  //Abrimos el txt
  ifstream readfile;
  for(unsigned long int i =0 ; i<BremsRate.size(); i++){
    
    for(unsigned long int j =0 ; j<Resol.size(); j++){

      std::string filename = "/work/stm/cgarcia/brems/data/10s_0.347MeV_3.5sigmacut/logfile"+std::to_string(BremsRate.at(i))+"kHz_resolution"+Resol.at(j)+"MeV.log";
      std::cout<<filename<<std::endl;
      readfile.open(filename.c_str(),ios::in);

      
      readfile>>name;
      readfile>>ratebrems;
      std::cout<<name<<" "<<ratebrems<<std::endl;
      readfile>>name;
      readfile>>TimeSim;
      std::cout<<name<<" "<<TimeSim<<std::endl;
      readfile>>name;
      readfile>>res;
      std::cout<<name<<" "<<res<<std::endl;
      readfile>>name;
      readfile>>sigmacut;
      std::cout<<name<<" "<<sigmacut<<std::endl;
      readfile>>name;
      readfile>>Energy;
      std::cout<<name<<" "<<Energy<<std::endl;
      readfile>>name;
      readfile>>meanEnergycounts;
      std::cout<<name<<" "<<meanEnergycounts<<std::endl;
      readfile>>name;
      readfile>>RMS;
      std::cout<<name<<" "<<RMS<<std::endl;
      readfile>>name;
      readfile>>uncertainty;
      std::cout<<name<<" "<<uncertainty<<std::endl;
      std::cout<<"Brems rate: "<<ratebrems<<" kHz, resol: "<<res<<" MeV, Uncertainty: "<<uncertainty<<std::endl;
      ratebremsv.push_back(ratebrems);
      TimeSimv.push_back(TimeSim);
      resv.push_back(res*1000);
      sigmacutv.push_back(sigmacut);
      Energyv.push_back(Energy);
      meanEnergycountsv.push_back(meanEnergycounts);
      RMSv.push_back(RMS);
      uncertaintyv.push_back(uncertainty*100);
      
      readfile.close();
    }//for BremsRate.size()
  }//for Resol.size()

  TGraph2D *gr = new TGraph2D(ratebremsv.size(),&ratebremsv[0],&resv[0],&uncertaintyv[0]);

  gr->GetHistogram()->GetZaxis()->SetLabelSize(0.038);
  gr->Draw("same,colz");

  TLatex latex;
  latex.SetTextSize(0.04);
  latex.SetTextAlign(13);

  latex.DrawLatex(510,3.1,"[%]");

  /*std::string rate_string = std::to_string(rate_brems)+" kHz";
char* rate_char = const_cast<char*>(rate_string.c_str());
latex.DrawLatex(latexmin,yy1[1]-latexmax2,rate_char);
  */

  /*std::string png = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Res_NewSim.png";
std::string pdf = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Res_NewSim.pdf";

char* png_char = const_cast<char*>(png.c_str());
char* pdf_char = const_cast<char*>(pdf.c_str());
  */

  gPad->SetTicks();
  gPad->RedrawAxis();

}
