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
#include <iomanip>

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
#include "TGraphErrors.h"


void Read_sinewaveAmplitudes() {
  gROOT->SetStyle("ATLAS");

  auto c1= new TCanvas("c1");

  c1->SetGrid();

  double xx1[2]={0, 1.2};
  double yy1[2]={0, 72000};
  double fitlimit=1;

  bool convert_to_mv=true;

  double ADC_to_mV_conv=(0.5/32767)*1000;
  double V_to_mV_conv=1000;

  if(convert_to_mv==true){
    xx1[1]=xx1[1]*V_to_mV_conv;
    yy1[1]=yy1[1]*ADC_to_mV_conv;
    fitlimit=fitlimit*V_to_mV_conv;
  }

  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->GetXaxis()->SetTitleColor(kBlack);
  //graph1->GetXaxis()->SetTitleSize(0.04);
  graph1->GetXaxis()->SetLabelSize(0.04);
  //graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetLabelSize(0.04);

  if(convert_to_mv==true){
    graph1->GetXaxis()->SetTitle("Input Voltage (p-p) [mV]");
    graph1->GetYaxis()->SetTitle("ADC Voltage (p-p) [mV]");
  }
  else{
    graph1->GetXaxis()->SetTitle("Input Voltage (p-p) [V]");
    graph1->GetYaxis()->SetTitle("ADC Amplitude (p-p)");
  }
  graph1->GetYaxis()->SetTitleOffset(1.3);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  
  std::vector<double> Vpp, Vpp_err, amplitudepp_ADC, amplitudepp_ADC_err;

  //Voltage amplitudes p-p 50kHz
  std::string RatekHz = "50.000"; 
  //std::vector<string> Vpp_str = {"0.005", "0.010", "0.050", "0.100", "0.200", "0.250", "0.300", "0.350", "0.400", "0.450", "0.500", "0.550", "0.600", "0.650", "0.750", "0.800", "0.850", "0.900", "0.950", "1.000", "1.020", "1.040", "1.060", "1.080", "1.100", "1.200"};
  //Removing 2 bad points
  std::vector<string> Vpp_str = {"0.005", "0.010", "0.050", "0.100", "0.200", "0.250", "0.300", "0.350", "0.400", "0.450", "0.500", "0.550", "0.750", "0.800", "0.850", "0.900", "0.950", "1.000", "1.020", "1.040", "1.060", "1.080", "1.100", "1.200"}; 

  //Voltage amplitudes p-p 3000kHz
  //std::string RatekHz = "3000.000";
  //std::vector<string> Vpp_str = {"0.005", "0.010", "0.050", "0.100", "0.300", "0.400", "0.500", "0.600", "0.700", "0.800", "0.900", "1.000", "1.050", "1.080", "1.100", "1.120", "1.140"};

  double amplitude, amplitude_err, Vpp_V;
  std::string line;

  unsigned long int n = Vpp_str.size();

  //Abrimos el txt
  ifstream readfile;
  for(unsigned long int i =0 ; i<n; i++){
  //for(unsigned long int i =0 ; i<1; i++){ 
    std::string filename = "/data1/cgarcia/UCLsinewave_fit/data-fit-sinewave"+Vpp_str.at(i)+"Vpp_"+RatekHz+"kHz.log";
    std::cout<<filename<<std::endl;
    readfile.open(filename.c_str(),ios::in);

    while(getline(readfile, line))
      {
	if (line.find("p0") != std::string::npos){
	  std::cout << "p0 was found in line: " << line << std::endl;
	  std::size_t pos1 = line.find("=")+1;
	  std::size_t pos2 = line.find("+");
	  std::size_t pos3 = line.find("-")+1;
	  std::size_t pos4 = line.length();
	  std::size_t amplitudelength = pos2-pos1;
	  std::size_t amplitude_err_length = pos4-pos3;
	  std::string amplitude_str = line.substr(pos1,amplitudelength);
	  std::string amplitude_err_str = line.substr(pos3,amplitude_err_length);

	  amplitude = std::stod(amplitude_str);
	  amplitude_err = std::stod(amplitude_err_str);
	  Vpp_V = std::stod(Vpp_str.at(i));

	  if(convert_to_mv==true){
	    amplitude=amplitude*ADC_to_mV_conv;
	    amplitude_err=amplitude_err*ADC_to_mV_conv;
	    Vpp_V=Vpp_V*V_to_mV_conv;
	  }

	  //2 because it's peak to peak
	  amplitudepp_ADC.push_back(2*amplitude);
          amplitudepp_ADC_err.push_back(amplitude_err);
	  Vpp.push_back(Vpp_V);
	  Vpp_err.push_back(0);

	  std::cout<<"Voltage: "<<Vpp_V<<"V amplitude: "<<amplitude<<" +- amplitude error: "<<amplitude_err<<" ADC"<<std::endl;
	}
      }
      
    readfile.close();
  }//for Resol.size()



  for(unsigned long int i =0;i<n; i++){
    Vpp_V = std::stod(Vpp_str.at(i));
    Vpp.push_back(Vpp_V);
    Vpp_err.push_back(0);
  }




  auto ge= new TGraphErrors(n,&Vpp[0],&amplitudepp_ADC[0],&Vpp_err[0],&amplitudepp_ADC_err[0]);
  ge->SetMarkerStyle(20);
  ge->SetMarkerColor(kViolet+1);
  ge->Draw("p");

  TF1*FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", -100, 3000);
  FitCalibrationErrors->SetParameters(-2.54344e+00,-2.72671e+02);
  ge->Fit(FitCalibrationErrors,"0","",0,fitlimit);
  FitCalibrationErrors->SetLineColor(kRed+1);
  FitCalibrationErrors->SetLineStyle(2);
  FitCalibrationErrors->Draw("same");

  double Chi2errors;
  Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
  cout<<"Chi2= "<<Chi2errors<<endl;



  TLatex latex;
  latex.SetTextSize(0.05);
  latex.SetTextAlign(13);

  std::stringstream streamrate, gradient;
  streamrate << std::fixed << std::setprecision(0) << std::stod(RatekHz);
  gradient << std::fixed << std::setprecision(3) << FitCalibrationErrors->GetParameter(1);

  std::string rate_string;

  if(convert_to_mv==false){
    rate_string = "#splitline{Gradient = "+gradient.str()+" ADC/V}{Sine wave freq = "+streamrate.str()+" kHz}";  
    char* rate_char = const_cast<char*>(rate_string.c_str());
    latex.DrawLatex(0.1,yy1[1]-3000,rate_char);
  }
  else{
    rate_string = "#splitline{Gradient = "+gradient.str()+"}{Sine wave freq = "+streamrate.str()+" kHz}";
    char* rate_char = const_cast<char*>(rate_string.c_str());
    latex.DrawLatex(100,yy1[1]-50,rate_char);
  }


  gPad->SetTicks();
  gPad->RedrawAxis();

  c1->Print("mVtomV_50kHz<1V_excl2badpoints.png");
  c1->Print("mVtomV_50kHz<1V_excl2badpoints.pdf");
}
