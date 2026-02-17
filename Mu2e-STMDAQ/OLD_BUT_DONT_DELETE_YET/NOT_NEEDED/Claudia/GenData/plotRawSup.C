#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include<fstream>
#include "TROOT.h"

using namespace std;


void plotRawSup() {
  gROOT->SetStyle("ATLAS");
  double xx1[2]={0, 1000};
  double yy1[2]={-16000, 1000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");



  //SUPPRESSED DATA
  /*//std::string  filenamesup  = std::string("run00109_suppressedsignal_threshold3_allfilebin_00.bin");
  //std::string  filenamesup  = std::string("run00109_suppressedsignal_bin_00.bin");
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

  for(int i=0;i<ADCsup.size();i++){
    grsup->SetPoint(i,i,ADCsup.at(i));
  }


  grsup->SetLineColor(kPink+2);
  grsup->SetMarkerStyle(5);
  grsup->Draw("same");
*/

  // ADC samling frequency (Hz)                                                                     
  const double fADC = 370*1e6; //Change this to get the 0.0027us                                    
  //Sampling time of ADC (microsec)                                                                 
  const double tadc=1/(fADC*1e-6);

  //RAWDATA
  std::string  filename  = std::string("hi.bin");
  std::cout << "filename = " << filename << std::endl;

  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf;
  
  int ADCread=0;
  int n=10000000;

  TGraph* gr = new TGraph();

  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) &&ADCread<n){
  //while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) )){
    ADC.push_back(inf);
    //ADCread++;
  }

  std::cout<<"Number of elements of all data: "<<ADC.size()<<std::endl;

 
  for(int i=0;i<n;i++){
    gr->SetPoint(i,i*tadc,ADC.at(i));
    //std::cout<<ADC.at(i)<<std::endl;
  }


  gr->SetLineColor(kBlack);
  gr->SetMarkerStyle(5);
  gr->Draw("same");

  /*  const double xshift=100;
  double peak1=1746.31+xshift;
  double peak2=1868.85+xshift;
  double peak3=2276.47+xshift;
  double peak4=2388.04+xshift;
  double peak5=3361.26+xshift;
  
  double peak8=12149.3+xshift;
  double peak9=12297.2+xshift;
  double peak10=12316.4+xshift;
  double peak11=13005.1+xshift;
  double peak12=14382.4+xshift;


  TLine *l1 = new TLine(peak9,yy1[0],peak9,yy1[1]);
  l1->SetLineColor(kOrange-3);
  l1->SetLineStyle(2);
  l1->SetLineWidth(2);
  //l1->Draw("same");

  TLine *l2 = new TLine(peak10,yy1[0],peak10,yy1[1]);
  l2->SetLineColor(kOrange-3);
  l2->SetLineStyle(2);
  l2->SetLineWidth(2);
  //l2->Draw("same");

  TLine *l3 = new TLine(peak11,yy1[0],peak11,yy1[1]);
  l3->SetLineColor(kOrange-3);
  l3->SetLineStyle(2);
  l3->SetLineWidth(2);
  //l3->Draw("same");

  TLine *l4 = new TLine(peak12,yy1[0],peak12,yy1[1]);
  l4->SetLineColor(kOrange-3);
  l4->SetLineStyle(2);
  l4->SetLineWidth(2);
  //l4->Draw("same");

  TLine *l5 = new TLine(peak8,yy1[0],peak8,yy1[1]);
  l5->SetLineColor(kOrange-3);
  l5->SetLineStyle(2);
  l5->SetLineWidth(2);
  //l5->Draw("same");


  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(gr,"Simulated Signal","l");
  legend->AddEntry(l1,"Peak Start Time","l");
  //legend->AddEntry(grsup,"Suppressed Signal","l");
  //legend->Draw("same");
  */


}
