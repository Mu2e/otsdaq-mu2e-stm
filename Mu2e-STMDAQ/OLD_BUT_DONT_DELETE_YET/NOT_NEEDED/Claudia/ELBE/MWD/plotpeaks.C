#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include<fstream>

void plotpeaks(char file_name[], int nbins, double xmin, double xmax){

  gROOT->SetStyle("ATLAS");

  //Read all the file
  //char file_name[] = "stm_hzdr_mwd__run0000018_subrun00000.bin";
  
  FILE *myFile = fopen(file_name, "rb");
  if (myFile==NULL)
    exit(1);

  //TCanvas *c1 = new TCanvas("c1","multipads",900,700);
  //c1->Divide(2,2);

  //int numtriggers=2;

  int* num_triggers = new int[1]();
  fread(&num_triggers[0], sizeof(int), 1, myFile);
  int numtriggers = num_triggers[0];
  std::cout<<" Number of total triggers "<<numtriggers<<std::endl;
  std::cout<<" "<<std::endl;


  uint32_t* cafeacid = new uint32_t[1];


  int numbertotalpeaks=0;
  unsigned long numbertotalADC=0;

  uint32_t* trigger_number = new uint32_t[numtriggers]();
  std::vector<uint32_t> trigger_numberv;
  uint16_t* trigger_type = new uint16_t[numtriggers];
  std::vector<uint16_t> trigger_typev;
  int* npeaks=new int[numtriggers];
  std::vector<int> npeaksv;
  int* nADC=new int[numtriggers];
  std::vector<int> nADCv;

  std::vector<double> peaksv;
  std::vector<double> tpeaksv;





  for(int i=0;i<numtriggers;i++){
    //Read cafe acid First
    fread(&cafeacid[0], sizeof(uint32_t), 1, myFile);
    std::cout<<"cafeacid: "<<std::hex << cafeacid[0]<<std::dec<<std::endl;

    fread(&trigger_number[i], sizeof(uint32_t), 1, myFile);
    trigger_numberv.push_back(trigger_number[i]);
    std::cout<<" Trigger number "<<trigger_number[i]<<std::endl;
    fread(&trigger_type[i], sizeof(uint16_t), 1, myFile);
    trigger_typev.push_back(trigger_type[i]);
    std::cout<<" Trigger type "<<trigger_type[i]<<std::endl;
    fread(&npeaks[i], sizeof(int), 1, myFile);
    npeaksv.push_back(npeaks[i]);
    std::cout<<" Number of peaks "<<npeaks[i]<<std::endl;
    fread(&nADC[i], sizeof(int), 1, myFile);
    nADCv.push_back(nADC[i]);
    std::cout<<" Number of ADC values "<<nADC[i]<<std::endl;


    numbertotalpeaks=numbertotalpeaks+npeaks[i];
    numbertotalADC=numbertotalADC+nADC[i];

    //double* peaks = new double[npeaks[i]];
    if(npeaks[i]==0){
      std::cout<<" "<<std::endl;
      continue;}
    double *peaks;
    double peaksarr [npeaks[i]];
    peaks=peaksarr;
    fread(&peaks[0], sizeof(double), npeaks[i], myFile);
    for(int j=0;j<npeaks[i];j++){
      peaksv.push_back(peaks[j]);
      std::cout<<" peak "<<peaks[j]<<std::endl;}

    //double* tpeaks = new double[npeaks[i]];
    double *tpeaks;
    double tpeaksarr [npeaks[i]];
    tpeaks=tpeaksarr;
    fread(&tpeaks[0], sizeof(double), npeaks[i], myFile);
    for(int j=0;j<npeaks[i];j++){
      tpeaksv.push_back(tpeaks[j]);
      std::cout<<" time "<<tpeaks[j]<<std::endl;}

    std::cout<<" "<<std::endl;
    //delete peaks;
    //delete tpeaks;

  }





  //Plot times and peaks MWD
  TGraph *gr = new TGraph ();
  //int nbins=200;
  //double xmin=0;
  //double xmax=5000;
  TH1F *hpeaks= new TH1F("BothModes", "", nbins, xmin, xmax);
  TH1F *hpeaks0= new TH1F("External-Mode0", "", 200, -10000, 0);
  TH1F *hpeaks1= new TH1F("Internal-Mode1", "", 200, -10000, 0);

  int k=0;
  //total number internal triggers and internal peaks
  int n_int_trig=0;
  int n_peaks_internal_trig=0;
  int npeaks1000_int=0;

  //total number external trigger and external peaks
  int n_ext_trig=0;
  int n_peaks_external_trig=0;
  int npeaks1000_ext=0;
  int numbercounts=0;  

  for(int i=0;i<numtriggers;i++){
    uint16_t mode=trigger_typev.at(i);
    std::cout<<"mode "<<mode<<endl;
    if(mode==0){n_ext_trig++;}
    if(mode==1){n_int_trig++;}

    for(int j=0;j<(npeaksv.at(i));j++){
      gr->SetPoint(k,tpeaksv.at(k),peaksv.at(k));
      //mode 0 and 1 histo
      //Calibration ADC energy
      std::cout<<peaksv.at(k)<<" "<<(peaksv.at(k)-0.354947)/(-1.75514)<<std::endl;
      //Fill like this to get the energy spectrum
      //hpeaks->Fill((peaksv.at(k)-0.354947)/(-1.75514));
      //Fill like this to get the ADC spectrum
      hpeaks->Fill(peaksv.at(k));


      //if(((peaksv.at(k)-0.354947)/(-1.75514)>525)&&((peaksv.at(k)-0.354947)/(-1.75514)<535)){
      if(((peaksv.at(k)-0.354947)/(-1.75514)>665)&&((peaksv.at(k)-0.354947)/(-1.75514)<685)){  
      numbercounts++;
      }

      //mode 0: external
      if(mode==0){hpeaks0->Fill(peaksv.at(k));
	if(peaksv.at(k)<=-1000){npeaks1000_ext++;}
	//std::cout<<peaksv.at(k)<<std::endl;
	n_peaks_external_trig++;
      }
      //mode 1: internal
      else if(mode==1){hpeaks1->Fill(peaksv.at(k));
	if(peaksv.at(k)<=-1000){npeaks1000_int++;}
	//std::cout<<peaksv.at(k)<<std::endl;
	n_peaks_internal_trig++;
      }

      //std::cout<<k<<" "<<tpeaksv.at(k)<<" "<<peaksv.at(k)<<std::endl;
      k++;}
  }

  // gStyle->SetOptStat(111111);
  std::cout<<"Number of total triggers "<<numtriggers<<std::endl;
  std::cout<<"Number of total ext trigs: "<<n_ext_trig<<" Number of ext peaks: "<<n_peaks_external_trig<<" Number of peaks below -1000 ADC counts: "<< npeaks1000_ext<<std::endl;
  std::cout<<"Number of total int trigs: "<<n_int_trig<<" Number of int peaks: "<<n_peaks_internal_trig<<" Number of peaks below -1000 ADC counts: "<< npeaks1000_int<<std::endl;
  std::cout<<"Number of total peaks: "<<numbertotalpeaks<<std::endl;
  double npeaks_count = hpeaks->Integral(hpeaks->FindFixBin(xmin), hpeaks->FindFixBin(xmax), "");
  std::cout<<"Number of total peaks in plotted range: "<<npeaks_count<<std::endl;
  std::cout<<"Number of total ADC values: "<<numbertotalADC<<std::endl;
  std::cout<<"Time range in which we find peaks: "<<(numbertotalADC/320.0520833313)*1e-6<<" seg"<<std::endl;
  std::cout<<"1 peak in: "<<((numbertotalADC/320.0520833313)*1e-6/(numbertotalpeaks))<<" seg"<<std::endl;
  std::cout<<"Rate: "<<1e-3/((numbertotalADC/320.0520833313)*1e-6/(numbertotalpeaks))<<" kHz"<<std::endl;


  gr->SetMarkerStyle(20);
  //gr->Draw("ap");
  
  //Normalize the y axis
  TH1*hnorm = (TH1*)(hpeaks->Clone("hnorm"));
  hnorm->Scale(1./hnorm->Integral());
  //hnorm->GetXaxis()->SetTitle("E [keV]");
  hnorm->GetXaxis()->SetTitle("ADC Counts"); 
  hnorm->SetLineColor(kAzure-3);
  hnorm->SetLineWidth(2);  
  //hnorm->GetYaxis()->SetRangeUser(0,0.04);
  //DRAW NORMALIZED HISTOGRAM                                                                                                                 
  //hpeaks->Draw("");  
  hnorm->Draw("HIST");

  //Fit 511 keV peak
  /*TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", 500, 600);
  Fit->SetParameters(1,530,2);
  hnorm->Fit(Fit,"0","",525, 535);
  Fit->SetLineColor(kRed);
  Fit->SetLineStyle(2);
  Fit->Draw("same");
  Double_t p1 = Fit->GetParameter(2);
  */
 

  //Fit Cs peak
  TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", 665, 685);
  Fit->SetParameters(1,675,2);
  hnorm->Fit(Fit,"0","",665, 685);
  Fit->SetLineColor(kRed);
  Fit->SetLineStyle(2);
  Fit->Draw("same");
  Double_t p1 = Fit->GetParameter(2);
  
  std::cout<<"Ncounts: "<<numbercounts<<" Sigma: "<<p1<<" Error sigma: "<<p1/sqrt(numbercounts*2)<<std::endl;
  //c1->cd(1);
  /*  hpeaks->GetXaxis()->SetTitle("E [keV]");
  hpeaks->GetYaxis()->SetTitle("Number of Peaks");
  hpeaks->SetLineColor(kAzure-3);
  hpeaks->SetLineWidth(2);
  hpeaks->Draw("");
  Fit->Draw("same");
  */

  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  //legend->AddEntry(hnorm,"Run22, Beam On","f");
  //legend->AddEntry(Fit,"#sigma= keV","l");
  legend->AddEntry(hpeaks,"Beam Off, Source In","f");
  //legend->AddEntry(hpeaks,"Beam Off, Source Out","f");
  legend->Draw("same");

  /*c1->cd(2);
hpeaks0->Draw("");

c1->cd(3);
hpeaks1->Draw("");*/

}
