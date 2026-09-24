#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
//#include <boost/chrono.hpp>
#include <sys/stat.h>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"


#include "TLatex.h"
#include "TROOT.h"
//boost::chrono::milliseconds sumGlobal;

using namespace std;

void plotGradient(){
  gROOT->SetStyle("ATLAS");

  //write 01,05,10,20...
  //std::string rate = "200";

  //std::string  filename  = "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/GendataNoise_"+rate+"kHz.bin";   
  std::string  filename  = "/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.17mV/smallfiles/ZP_v3/SupdatadataNoise_01kHz_2370ADC_0.17noise.bin";
  std::cout << "filename = " << filename << std::endl;

  auto c1= new TCanvas("c1");
  //c1->SetGrid();


  // double xx1[2]={0, 5000};
  double xx1[2]={0,80};
  double yy1[2]={-4000, 100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");



  struct stat st;
  stat(filename.c_str(), &st);
  unsigned long int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)
  //int n = 9000000;
  int16_t* ADC = new int16_t[n];

  std::ifstream myFile;

  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  TGraph* grad = new TGraph();
  TGraph* gsignal = new TGraph();
  TGraph* gsupsignal = new TGraph();

  int window=100;
  int16_t* gradient = new int16_t[n-window];
  double* time = new double[n];
  double* trigtime = new double[n-window];
  int16_t* suppressed_data = new int16_t[n-window];
  //t adc in microsec
  //ADC samling frequency (Hz)
  const double fADC = 320.0520833313; //Change this to get the 0.0027us
  //const double fADC =370;
  //Sampling time of ADC (microsec)
  const double tadc=1/(fADC);

  for(int i=0;i<(n-window);i++){
    gradient[i]=ADC[i+window]-ADC[i];
  }
  for(int i=0;i<n;i++){
  //time in microsec                                                                                                                           
  time[i]=i*tadc; 
  }

  bool peak=false;
  int counter=0;
  //Find the trigger
  int threshold=-100;
  int trigger=0;

  //store tbefore microseconds of data to the left of the trigger
  double tbefore=2;
  int prenumADCstored=int(tbefore/tadc);

  //store tafter microseconds of data to the right of the trigger
  double tafter=10;
  int postnumADCstored=int(tafter/tadc);

  for(unsigned long int i=0;i<(n-window);i++){

    if(gradient[i]>threshold){peak=false;
      continue;}
    if((gradient[i]<threshold)&&(peak==true)){continue;}

    int triggerold=trigger;
    if((gradient[i]<threshold)&&(peak==false)){
      trigger=i;
      //if triggers found are closer than the window of stored data in time, rejected trigger, continue
      //it is necesary to convert the timing window to the index window: time[i]=i*tadc
      if(trigger-triggerold<int((tafter+tbefore)/tadc)){
	//If the trigger i is closer in time than tafter+tbefore, then the trigger i+1 should be compared to the trigger i-1 
	trigger=triggerold;
	continue;}

      cout<<"Trigger "<<trigger<<" Triggertime: "<<trigger*tadc<<endl;

      for(int k=prenumADCstored; k>0;k--){suppressed_data[counter]=ADC[trigger-k];
	peak=true;
	trigtime[counter]=trigger*tadc;
	counter++;
	}

      for(int j=0; j<postnumADCstored;j++){suppressed_data[counter]=ADC[trigger+j];
	peak=true;
	trigtime[counter]=trigger*tadc;
	counter++;
	}

    }
  }


  /*double trigtimeold =0;
  int j=0;
  //int n1=9000000;
  int n1=n;
  if(n1>counter){n1=counter;}

  for(int i=0;i<n1;i++){

    if(i>0){
    trigtimeold=trigtime[i-1];
    if(trigtimeold!=trigtime[i]){j=0;}
    else{j++;}
    }
    //if(suppressed_data[i]==0){
    //std::cout<<suppressed_data[i]<<" "<<trigtime[i]<<std::endl;
    //}
    
    //gsupsignal->SetPoint(i,trigtime[i]+j*tadc,suppressed_data[i]);
    if(i==0){j++;} 
    
    //std::cout<<time[i]<<" "<<suppressed_data[i]<<std::endl;
      //gsupsignal->SetPoint(i,time[i],suppressed_data[i]);
    grad->SetPoint(i,time[i],gradient[i]);
    gsignal->SetPoint(i,time[i],ADC[i]);
  }*/


  for(int i=0;i<n;i++){
    gsignal->SetPoint(i,time[i],ADC[i]); 
    std::cout<<time[i]<<" "<<ADC[i]<<std::endl;
  }



  TLine *line=new TLine(xx1[0],threshold,xx1[1],threshold);
  line->SetLineColor(kBlue);
  //line->Draw("same");


  gsignal->SetLineColor(kPink);
  gsignal->SetMarkerColor(kPink);
  gsignal->Draw("same,l");

  grad->SetMarkerColor(kOrange);
  grad->SetLineColor(kOrange);
  //grad->Draw("same,l");

  gsupsignal->SetLineColor(kPink);
  gsupsignal->SetMarkerColor(kPink);
  gsupsignal->SetMarkerStyle(6);
  //gsupsignal->Draw("same,p");

  //convert the string to int to remove the 0 when the rate is 01 or 05 kHz
  /*int rateint = stoi(rate); 
  std::string ratest= std::to_string(rateint);

  std::string rate_kHz = ratest+" kHz";
  char* ratechar = const_cast<char*>(rate_kHz.c_str());

  TLatex latex;
  latex.SetTextSize(0.06);
  latex.DrawLatex(100,-1500,ratechar);
  */

  auto legend = new TLegend(0.5,0.7,0.88,0.9);
  legend->AddEntry(gsignal,"Suppressed Data","l");
  //legend->AddEntry(grad,"Gradient","l");
  //legend->AddEntry(gsupsignal,"Suppressed Data","l");
  //legend->AddEntry(line,"Threshold","l");
  legend->Draw("same");


  //c1->Print("Gradientfluctuationlines.png");
  //c1->Print("Gradientfluctuationlines.pdf");
}
