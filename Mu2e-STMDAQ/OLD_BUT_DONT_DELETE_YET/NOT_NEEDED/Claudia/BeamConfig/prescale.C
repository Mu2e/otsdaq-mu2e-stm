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

#include "BeamVars.hh"

void prescale() {

  int palette_number = 57;
  //int palette_number = 74;
  gStyle->SetPalette(palette_number);


  auto c03 = new TCanvas("c03","c03",800,700);

/*
  double months_taking_data = 9;
  double Mu2e_efficiency_year=0.95;
  double main_injectorcycle=1.4; //sec  
  double sec_taking_data= (months_taking_data/12)*365*24*3600*Mu2e_efficiency_year; //sec
  int MICycles_year=sec_taking_data/main_injectorcycle;
  double fADC=370; //MHz
  double spillsize=43.12*0.001; //sec
  double nspills=8;
  double spillgaps=5*0.001; //sec
  double spillmaingap=36*0.001; //sec
  double gapsize=main_injectorcycle-(8*spillsize+6*spillgaps+spillmaingap); //sec
  double beam_on_size=nspills*spillsize; //sec
  //At high rates we write all data in one spill
  //Maximum data that we can write per spill
  double limit_spilldata = beam_on_size*fADC*2/(1e6);//TB
  //Maximum data that we can write per gap
  double limit_gapdata = gapsize*fADC*2/(1e6);//TB 

  double ZPpeak_durationHPGe = 0.000007; //sec
  

  //Data allowance in disk per year
  double space_disk_TB=500; //Tbytes 
  double space_disk_Gb=space_disk_TB*1000*8; //Gbit  
  double speedlimit=space_disk_Gb/sec_taking_data; //Gbit/s
*/
  
  int gap_dimension = 16;
  int spill_dimension = 20;

  /*double rates_gap[16]; //kHz
  double rates_spill[20]; //kHz
  
  double rates_gap_GHz[16] = {0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8};
  double rates_spill_GHz[20] = {1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95};

  double GHz_to_kHz = 1000000;
  for(int i=0 ; i< gap_dimension; i++){
    rates_gap[i] = rates_gap_GHz[i]*GHz_to_kHz;
  }
  for(int i=0 ; i< spill_dimension; i++){
    rates_spill[i] = rates_spill_GHz[i]*GHz_to_kHz;
  }
  */
 
  double rates_gap[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}; //kHz 
  double rates_spill[20]={10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200}; //kHz 

  double spill_bin_width = rates_spill[1] - rates_spill[0];
  double gap_bin_width = rates_gap[1] - rates_gap[0];
  
  std::cout<<"Total allowance: "<<space_disk_TB<<" Tbytes"<<std::endl;
  std::cout<<"Main injector cycle: "<<main_injectorcycle<<", Spill size (8*"<<spillsize<<"s): "<<beam_on_size<<"s, spillsize(8)+small gaps(7): "<<beam_on_spills_gaps<<"s, Gap size: "<<gapsize<<"s, 7 spill small gaps of size each: "<<spillgaps<<"s"<<std::endl;
  std::cout<<"Months taking data: "<<months_taking_data<<" with efficiency per year: "<<Mu2e_efficiency_year<<" we are taking data during: "<<sec_taking_data<<"s"<<std::endl;
  std::cout<<"Number of My Cycles in one year (with efficiency): "<<MICycles_year<<std::endl;
  std::cout<<"Maximum data that we can write per spill: "<<limit_spilldata<<" TB"<<std::endl;
  std::cout<<"Maximum data that we can write per gap: "<<limit_gapdata<<" TB"<<std::endl;
  std::cout<<"Duration of ZS data stored per peak: "<<ZPpeak_durationHPGe<<"s, fADC: "<<fADC<<" MHz"<<std::endl;
  std::cout<<" "<<std::endl;

  vector<double> prescalespill, prescalegap, ratespillv, rategapv;
  
  //Number of peaks at the rate
  //Spill loop
  for(int i=0;i<spill_dimension;i++){
    int peaks_spill_1s= rates_spill[i]*1000;
    //peaks in spill
    //int peaks_spill=beam_on_size*peaks_spill_1s;
    int peaks_spill=beam_on_spills_gaps*peaks_spill_1s; 
    
    //ZP seconds of data to store
    double stored_us_spill=ZPpeak_durationHPGe*peaks_spill*1e6; //us
    
    //Number of bytes values to store per spill
    double stored_bytes_spill=stored_us_spill*fADC*2; //bytes
    double stored_TB_spill=stored_bytes_spill*1e-12; //Tbytes

    std::cout<<""<<std::endl;
    std::cout<<"Data stored in one spill: "<<stored_TB_spill<<"TB at "<<rates_spill[i]<<" kHz"<<std::endl;

    //Check if the spill data stored at this rate is bigger than the number of adc values (in TB) that we have per spill
    if(stored_TB_spill>limit_spilldata){std::cout<<"More data stored than available at this rate (we have overlapped pulses), the data written is equal to the spill size at high rates..."<<std::endl; stored_TB_spill=limit_spilldata;}

    std::cout<<"*******Rate spill: "<<rates_spill[i]<<" kHz,"<<" Bytes to write in ONE spill: "<<stored_bytes_spill<<" bytes ="<<stored_TB_spill<<" Tbytes"<<std::endl;

    //Gap loop
    for(int j=0;j<gap_dimension;j++){
      ratespillv.push_back(rates_spill[i]);
      rategapv.push_back(rates_gap[j]);

      int peaks_gap_1s= rates_gap[j]*1000;
      //peaks in gap
      int peaks_gap=gapsize*peaks_gap_1s;
      //ZP seconds of data to store
      double stored_us_gap=ZPpeak_durationHPGe*peaks_gap*1e6; //us
      //Number of bytes values to store
      double stored_bytes_gap=stored_us_gap*fADC*2; //bytes
      double stored_TB_gap=stored_bytes_gap*1e-12; //Tbytes
      std::cout<<"   *Rate gap: "<<rates_gap[j]<<" kHz,"<<" Bytes to write in ONE gap: "<<stored_bytes_gap<<" bytes = "<<stored_TB_gap<<" Tbytes"<<std::endl;
      
      double totaldataTB_MIcycle =stored_TB_spill+stored_TB_gap;
      double totaldataTB_yearSpill=stored_TB_spill*MICycles_year;
      double totaldataTB_yearGap=stored_TB_gap*MICycles_year;
      double totaldataTB_year =totaldataTB_yearSpill+totaldataTB_yearGap;
      std::cout<<"    -----Total data to write to disk in ONE YEAR: "<<totaldataTB_year<<" TB ("<< totaldataTB_yearSpill<<" TB from spills "<<totaldataTB_yearGap<<" TB from gap)"<<std::endl;
    
      int pres_spill=1;
      int pres_gap=1;


      //if spill data is lower than space_disk_TB, but the total amount of data to write in one year is bigger than the space available in disk, prescale gap data
      if(totaldataTB_yearSpill<space_disk_TB){std::cout<<"Prescale Gap Data"<<std::endl;
	while(totaldataTB_year>space_disk_TB){ 
          pres_gap++;
          totaldataTB_year =totaldataTB_yearSpill+totaldataTB_yearGap/pres_gap;
          cout<<"Gap prescale: "<<pres_gap<<", Total data year: "<<totaldataTB_year<<"TB,"<<"("<<totaldataTB_yearSpill<<" TB from spills "<<totaldataTB_yearGap/pres_gap<<" TB from gap)"<<" available:  "<<space_disk_TB<<" TB"<<std::endl;
        }
	std::cout<<"Using prescale for gap data: "<< pres_gap<<std::endl;
	prescalegap.push_back(pres_gap);
	std::cout<<"Using prescale for spill data: "<< pres_spill<<std::endl;
        prescalespill.push_back(pres_spill);
      }

	
      //Else prescale spill data until spill data is lower than space_disk_TB (this is the minimum prescale for spill data) and then prescale gap data until totaldataTB_yearGap is lower than space_disk_TB
      else{
	double aux=totaldataTB_yearSpill;
	while(totaldataTB_yearSpill>space_disk_TB){std::cout<<"Prescale Spill Data"<<std::endl;
	  pres_spill++;
	  totaldataTB_yearSpill =aux/pres_spill;
	  totaldataTB_year =totaldataTB_yearSpill+totaldataTB_yearGap;
	  cout<<"Spill prescale: "<<pres_spill<<", Total data year: "<<totaldataTB_year<<"TB,"<<"("<<totaldataTB_yearSpill<<" TB from spills "<<totaldataTB_yearGap<<" TB from gap)"<<"available:  "<<space_disk_TB<<" TB"<<std::endl;
	}
	std::cout<<"Using prescale for spill data: "<< pres_spill<<std::endl;
	prescalespill.push_back(pres_spill);

	if(totaldataTB_year<space_disk_TB){std::cout<<"Using prescale for gap data: "<<pres_gap<<std::endl;prescalegap.push_back(pres_gap);}
	//else prescale also gap data
	else{
	  while(totaldataTB_year>space_disk_TB){std::cout<<"Prescale Gap Data"<<std::endl;
	    pres_gap++;
	    totaldataTB_year =totaldataTB_yearSpill+totaldataTB_yearGap/pres_gap;
	    cout<<"Gap prescale: "<<pres_gap<<", Total data year: "<<totaldataTB_year<<"TB,"<<"("<<totaldataTB_yearSpill<<" TB from spills "<<totaldataTB_yearGap/pres_gap<<" TB from gap)"<<" available:  "<<space_disk_TB<<" TB"<<std::endl;
	  }
	  std::cout<<"Using prescale for gap data: "<< pres_gap<<std::endl;
	  prescalegap.push_back(pres_gap);}
      }


    }
  }





  //PLOT
  /*TGraph2D *gr = new TGraph2D(ratespillv.size(),&ratespillv[0],&rategapv[0],&prescalespill[0]);
  gr->SetTitle(";X-Ray spill rate [kHz];Gap rate [kHz];");
  gr->GetHistogram()->GetXaxis()->SetTitleColor(kBlack);
  gr->GetHistogram()->GetXaxis()->SetTitleSize(0.04);
  gr->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
  gr->GetHistogram()->GetYaxis()->SetTitleSize(0.04);
  gr->GetHistogram()->GetYaxis()->SetLabelSize(0.04);
  gr->GetHistogram()->GetZaxis()->SetLabelSize(0.04);
  gr->Draw("colz");
  */


  
  TH2F* hcolz_spill = new TH2F("hSpillColz","",spill_dimension,rates_spill[0],rates_spill[19]+spill_bin_width,gap_dimension,0,gap_dimension+gap_bin_width);
  TH2F* hpres_spill = new TH2F("hSpill","Prescales for spills",spill_dimension,rates_spill[0],rates_spill[19]+spill_bin_width,gap_dimension-gap_bin_width,0,gap_dimension);
  TH2F* hpres_gap = new TH2F("hGap","Prescales for gaps",spill_dimension,rates_spill[0],rates_spill[19]+spill_bin_width,gap_dimension-gap_bin_width,0,gap_dimension);
  int k=0;
  for(int i=0;i<20;i++){
    cout<<"* Rate spill: "<<rates_spill[i]<<std::endl;
    for(int j=0;j<16;j++){
      cout<<"  Rate gap:"<<rates_gap[j]<<std::endl;
      cout<<"     Prescale spill: "<<prescalespill.at(k)<<" Prescale gap: "<<prescalegap.at(k)<<std::endl;
      hcolz_spill->Fill(rates_spill[i],rates_gap[j],prescalespill.at(k));
      hpres_spill->Fill(rates_spill[i],rates_gap[j],prescalespill.at(k));
      hpres_gap->Fill(rates_spill[i],rates_gap[j],prescalegap.at(k));
      k++;
    }
    }


  hcolz_spill->GetXaxis()->SetTitle("Spill rate [kHz]");
  hcolz_spill->GetYaxis()->SetTitle("Gap rate [kHz]");  
  hcolz_spill->GetXaxis()->SetTitleColor(kBlack);
  hcolz_spill->GetXaxis()->SetTitleSize(0.04);
  hcolz_spill->GetXaxis()->SetLabelSize(0.04);
  hcolz_spill->GetYaxis()->SetTitleSize(0.04);
  hcolz_spill->GetYaxis()->SetLabelSize(0.04);
  hcolz_spill->GetZaxis()->SetLabelSize(0.04); 
  hcolz_spill->SetStats(0);
  hcolz_spill->Draw("colz"); 
  
  
  hpres_spill->SetBarOffset(0.2);
  hpres_spill->SetMarkerColor(kBlack);
  hpres_spill->Draw("TEXT SAME");

   
  hpres_gap->SetBarOffset(-0.2);
  hpres_gap->SetMarkerColor(kRed);
  hpres_gap->Draw("TEXT SAME");

   
  TLatex lspill;
  lspill.SetTextSize(0.03);
  lspill.SetTextAlign(13);
  lspill.SetTextColor(kBlack);
  lspill.DrawLatex(32.81,16.52,"Minimum Spill Prescale");

  TLatex lgap;
  lgap.SetTextSize(0.03);
  lgap.SetTextAlign(13);
  lgap.SetTextColor(kRed);
  lgap.DrawLatex(114.68,16.52,"Gap Prescale");

  c03->Print("Gap_SpillPrescales_3usZS_fadc320MHz_GHzrates.png");

}
