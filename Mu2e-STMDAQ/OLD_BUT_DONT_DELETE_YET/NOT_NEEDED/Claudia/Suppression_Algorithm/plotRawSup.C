#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TLatex.h"
#include<fstream>

using namespace std;


void plotRawSup() {

  gROOT->SetStyle("ATLAS");
  //double xx1[2]={88000, 4000000};
  //double xx1[2]={0, 2500000};
  double xx1[2]={4260,4300}; 
  double yy1[2]={-1500, 1000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  //const double fADC = 320.0520833313;
  const double fADCsup = 370;
  //Sampling time of ADC (microsec)
  const double tadcsup=1/(fADCsup);


  bool printraw=false; //true for printing raw, false for printing suppressed
  bool printboth=false; //print suppressed data from run109 and simulation

  //write 01,05,10,20...  
  std::string rate = "20";


  //SUPPRESSED DATA
  //std::string  filenamesup  = std::string("run00109_suppressedsignal_bin_00.bin");
  //std::string  filenamesup  = std::string("/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/SuppresedFiles_tbefore2us_tafter10us/SupdataNoise_"+rate+"kHz.bin");
  //std::string  filenamesup  = std::string("/work/cgarcia/DATA/Claudia/PulseShape_Analysis/SupRun109/Suppressed_run109.bin_00"); 
  //std::string  filenamesup  = std::string("/work/cgarcia/DATA/Claudia/PulseShape_Analysis/SupSim1kHz/SupdataNoise_01kHz.bin");
  std::string  filenamesup  = std::string("/work/cgarcia/DATA/Claudia/HLSData/SpillData_tbefore1ustafter2us/ZPdata_run109_00.bin");
  std::cout << "filename suppressed = " << filenamesup << std::endl;

  std::vector<int16_t> ADCsup;
  ADCsup.clear();
  std::ifstream myFilesup;
  myFilesup.open(filenamesup, std::ios::in | std::ios::binary);
  int16_t infsup;
  TGraph* grsup = new TGraph();

  while( myFilesup.read( reinterpret_cast<char*>( &infsup ), sizeof(infsup) )){
    ADCsup.push_back(infsup);
  }

  std::cout<<"Number of elements of suppressed data: "<<ADCsup.size()<<std::endl;

  //int nsup= 9000000;
  int nsup=ADCsup.size();
  if(nsup>ADCsup.size()){nsup=ADCsup.size();}

  for(int i=0;i<nsup;i++){
    grsup->SetPoint(i,i*tadcsup,ADCsup.at(i)-920);
  }




  //RAWDATA

  //const double fADCraw = 320.0520833313;
  const double fADCraw = 370;   
  //Sampling time of ADC (microsec)
  const double tadcraw=1/(fADCraw);

  //std::string  filename  = std::string("/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/GendataNoise_"+rate+"kHz.bin");
  //std::string  filename  = std::string("/work/cgarcia/DATA/Claudia/PulseShape_Analysis/SupSim1kHz/SupdataNoise_01kHz_fadc320_noise0.17.bin");
  std::string  filename  = std::string("/work/cgarcia/DATA/Claudia/PulseShape_Analysis/SupSim1kHz/SupdataNoise_01kHz_fadc370_noise0.32.bin"); 
  std::cout << "filename = " << filename << std::endl;

  int ratedoub = stoi(rate);
  std::string rate_kHz = std::to_string(ratedoub)+" kHz";
  char* ratechar = const_cast<char*>(rate_kHz.c_str());

  TLatex latex;
  latex.SetTextSize(0.06);
  //latex.DrawLatex(20,-2500,ratechar);


  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf;

  int ADCread=0;

  TGraph* gr = new TGraph();

  //while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) &&ADCread<n){
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) )){
    ADC.push_back(inf);
    ADCread++;
  }

  std::cout<<"Number of elements of all data: "<<ADC.size()<<std::endl;
  //int n=ADC.size();
  int n= 9000000;
  if(n>ADC.size()){n=ADC.size();}

  for(int i=0;i<n;i++){
    gr->SetPoint(i,i*tadcraw,ADC.at(i));
  }
  

 


  if(printraw==true&&printboth==false){
    gr->SetLineColor(kBlack);
    gr->SetMarkerColor(kBlack);
    gr->SetMarkerStyle(6);
    gr->Draw("same,l");
    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(gr,"Raw Data","l");
    legend->Draw("same");

  }

  if(printraw==false&&printboth==false){
    grsup->SetLineColor(kPink);
    grsup->SetMarkerStyle(6);
    grsup->SetMarkerColor(kPink);
    grsup->Draw("same,p");
    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grsup,"Suppressed Data","l");   
    legend->Draw("same");

  }
 
  if(printboth==true){
    grsup->SetLineColor(kBlack);
    grsup->SetMarkerStyle(6);
    grsup->SetMarkerColor(kBlack);

    gr->SetLineColor(kAzure+1);
    gr->SetMarkerColor(kAzure+1);
    gr->SetMarkerStyle(6);

    gr->Draw("same,p");
    grsup->Draw("same,p");
    
    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(gr,"Simulation","l");
    legend->AddEntry(grsup,"Data","l");
    legend->Draw("same");
        
  }


 
}
