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


void LastChunkSize_TriggerGapMode() {
  gROOT->SetStyle("ATLAS");

  int headersize=19; //input header size in ADC values
  int NRAMblocks=912;
  double hardwareclockSPILL=1.695; //us
  double hardwareclockGAP=100; //us
  //double ADCRAM=1024; //ADC values per RAM block
  double ADCRAM=2048; //ADC values per RAM block
  double fADC=370; //MHz
  int nADC_triggerSPILL=fADC*hardwareclockSPILL;
  double nADC_triggerGAP=fADC*hardwareclockGAP;
  double tbefore=1; //us
  double tafter=2; //us 
  double ADCbefore=fADC*tbefore;
  double ADCafter=fADC*tafter;
  double ADCstoredperpeak=ADCbefore+ADCafter;

  double numBlockRAMS[912];
  double chunksize[912];
  double chunksize_noheaders[912];
  int  numSpillTrigger_chunk[912];
  double numADC[912];
  double percentage_overlapping[912];
  double start_next_chunk[912];
  int realstart_trignum[912];
  double realstart_next_chunkSPILL[912];
  double num_complete_chunkspertrig[912];
  double numADC_lastchunk_pertrig[912];
  

  for(int i=0;i<NRAMblocks;i++){
    numBlockRAMS[i]=i+1;
    chunksize[i]=numBlockRAMS[i]*ADCRAM;
    chunksize_noheaders[i]=chunksize[i]-headersize;
    numSpillTrigger_chunk[i]=int(chunksize_noheaders[i]/nADC_triggerSPILL);
    numADC[i]=numSpillTrigger_chunk[i]*nADC_triggerSPILL;
   
    percentage_overlapping[i]=((numADC[i]-1)-ADCafter+1-ADCbefore)/numADC[i];
    start_next_chunk[i]=percentage_overlapping[i]*numADC[i];
    realstart_trignum[i]=int(start_next_chunk[i]/nADC_triggerSPILL);
    realstart_next_chunkSPILL[i]=realstart_trignum[i]*nADC_triggerSPILL;
    num_complete_chunkspertrig[i]=int(nADC_triggerGAP/start_next_chunk[i]);
    if(start_next_chunk[i]*(num_complete_chunkspertrig[i]-1)+numADC[i]>nADC_triggerGAP){num_complete_chunkspertrig[i]=num_complete_chunkspertrig[i]-1;}
    numADC_lastchunk_pertrig[i]=nADC_triggerGAP-num_complete_chunkspertrig[i]*start_next_chunk[i];
  
    std::cout<<"Num ADC per trigger in Spill: "<<nADC_triggerSPILL<<" Num ADC per trigger in Gap: "<<nADC_triggerGAP<<std::endl;
    std::cout<<"RAM Blocks: "<<numBlockRAMS[i]<<" Chunk size to send in ADC: "<<chunksize[i]<<" Chunk size without headers: "<<chunksize_noheaders[i]<<std::endl;
    std::cout<<"Number of triggers per chunk in Spill Mode: "<<numSpillTrigger_chunk[i]<<" Number of ADC values to send to ZP: "<<numADC[i]<<std::endl;
    std::cout<<"Percentage to start overlapping data: "<<percentage_overlapping[i]<<" ADC in which starts next chunk in GAP mode: "<<start_next_chunk[i]<<" Num triggers to start overlapping data in SPILL mode: "<<realstart_trignum[i]<<" ADC in which starts overlapping data in SPILL mode (having into account triggers): "<< realstart_next_chunkSPILL[i]<<std::endl;
    std::cout<<"Number of complete chunks per trigger in Gap: "<<num_complete_chunkspertrig[i]<<std::endl;
    std::cout<<"NUMBER OF ADC VALUES IN LAST CHUNK OF THE TRIGGER IN GAP MODE (it has to be bigger than the num of ADC values stored per triggger("<<ADCstoredperpeak<<" ADC values): "<<numADC_lastchunk_pertrig[i]<<std::endl;
    std::cout<<" "<<std::endl;
  }


  int xmin1 = 1;
  int xmax1 = 50;
  int ymin1 = 0;
  int ymax1 = 50;

  //Only plot sending 50 block rams
  auto graph1 = new TGraph (50, numBlockRAMS, num_complete_chunkspertrig);
  graph1->GetXaxis()->SetRangeUser(xmin1, xmax1);
  graph1->GetYaxis()->SetRangeUser(ymin1,ymax1);
  graph1->GetXaxis()->SetTitle("# Block RAMs");
  graph1->GetYaxis()->SetTitle("Complete chunks per trigger in Gap Mode");
  graph1->SetTitle("");
  graph1->SetMarkerColor(kBlue+2);
  graph1->SetMarkerStyle(20);
  //graph1->Draw("ap");

  ///////////////////////////////////////

  int xmin2 = 1;
  int xmax2 = 50;
  int ymin2 = -10;
  int ymax2 = 40000;
 
  auto graph2 = new TGraph (50, numBlockRAMS, numADC_lastchunk_pertrig);
  graph2->GetXaxis()->SetRangeUser(xmin2, xmax2);
  graph2->GetYaxis()->SetRangeUser(ymin2,ymax2);
  graph2->GetXaxis()->SetTitle("# Block RAMs");
  graph2->GetYaxis()->SetTitle("ADC values in last chunk per trigger in Gap Mode");
  graph2->GetYaxis()->SetTitleSize(0.04);
  graph2->GetYaxis()->SetTitleOffset(1.92);
  graph2->SetTitle("");
  graph2->SetMarkerColor(kRed+2);
  graph2->SetMarkerStyle(20);
  graph2->Draw("ap");

  TLine *line1=new TLine(xmin2,ADCstoredperpeak,xmax2,ADCstoredperpeak);
  line1->SetLineColor(kBlue+2);
  line1->SetLineWidth(2);
  line1->SetLineStyle(3); 
  line1->Draw("same");

  TLine *line2=new TLine(xmin2,nADC_triggerGAP,xmax2,nADC_triggerGAP);
  line2->SetLineColor(kGreen+2);
  line2->SetLineWidth(2);
  line2->SetLineStyle(3);
  line2->Draw("same");


  auto leg1 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg1->AddEntry(line1, "#splitline{Number of ADC values stored}{per peak found by ZS algorithm}","l");  
  leg1->AddEntry(line2, "ADC values per trigger in Gap Mode","l");
  leg1->Draw("same");

  //////////////////////////////////////////

  /*  TH1F*h1 = new TH1F("TH1","",24,1,25);
  TH1F*h2 = new TH1F("TH2","",24,1,25);

  for(int i=0;i<NRAMblocks;i++){
    for(int j=0;j<chunksize[i];j++){
      h1->Fill(i+1);}
    for(int k=0;k<numADC[i];k++){
      h2->Fill(i+1);}

  }
  h1->SetLineColor(kOrange+2);
  h1->SetFillColor(kOrange+2);
  h1->GetXaxis()->SetTitle("# Block RAMs");
  h1->Draw("");
  h2->SetLineColor(kViolet+2);
  h2->SetFillColor(kViolet+2);
  h2->Draw("same");

  auto leg2 = new TLegend(0.1,0.7,0.48,0.9);
  leg2->AddEntry(h1, "Chunk Size","l");
  leg2->AddEntry(h2, "ADC values to ZS in Chunk","l");
  leg2->Draw("same");
 
  */


}
